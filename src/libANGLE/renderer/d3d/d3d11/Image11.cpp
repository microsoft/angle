//
// Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Image11.h: Implements the rx::Image11 class, which acts as the interface to
// the actual underlying resources of a Texture

#include "libANGLE/renderer/d3d/d3d11/Image11.h"

#include "common/utilities.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/renderer/d3d/loadimage.h"
#include "libANGLE/renderer/d3d/d3d11/formatutils11.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"
#include "libANGLE/renderer/d3d/d3d11/RenderTarget11.h"
#include "libANGLE/renderer/d3d/d3d11/TextureStorage11.h"

namespace rx
{

Image11::Image11(Renderer11 *renderer)
    : mRenderer(renderer),
      mDXGIFormat(DXGI_FORMAT_UNKNOWN),
      mStagingTexture(nullptr),
      mStagingSubresource(0),
      mRecoverFromStorage(false),
      mAssociatedStorage(nullptr),
      mAssociatedImageIndex(gl::ImageIndex::MakeInvalid()),
      mRecoveredFromStorageCount(0),
      mUsesAlternateRenderableFormat(false)
{
}

Image11::~Image11()
{
    disassociateStorage();
    releaseStagingTexture();
}

gl::Error Image11::generateMipmap(Image11 *dest, Image11 *src)
{
    ASSERT(src->getDXGIFormat() == dest->getDXGIFormat());
    ASSERT(src->getWidth() == 1 || src->getWidth() / 2 == dest->getWidth());
    ASSERT(src->getHeight() == 1 || src->getHeight() / 2 == dest->getHeight());

    const d3d11::DXGIFormat &dxgiFormatInfo = d3d11::GetDXGIFormatInfo(src->getDXGIFormat());
    ASSERT(dxgiFormatInfo.mipGenerationFunction != nullptr);

    D3D11_MAPPED_SUBRESOURCE destMapped;
    gl::Error error = dest->map(D3D11_MAP_WRITE, &destMapped);
    if (error.isError())
    {
        return error;
    }

    D3D11_MAPPED_SUBRESOURCE srcMapped;
    error = src->map(D3D11_MAP_READ, &srcMapped);
    if (error.isError())
    {
        dest->unmap();
        return error;
    }

    const uint8_t *sourceData = reinterpret_cast<const uint8_t*>(srcMapped.pData);
    uint8_t *destData = reinterpret_cast<uint8_t*>(destMapped.pData);

    dxgiFormatInfo.mipGenerationFunction(src->getWidth(), src->getHeight(), src->getDepth(),
                                         sourceData, srcMapped.RowPitch, srcMapped.DepthPitch,
                                         destData, destMapped.RowPitch, destMapped.DepthPitch);

    dest->unmap();
    src->unmap();

    dest->markDirty();

    return gl::Error(GL_NO_ERROR);
}

bool Image11::isDirty() const
{
    // If mDirty is true
    // AND mStagingTexture doesn't exist AND mStagingTexture doesn't need to be recovered from TextureStorage
    // AND the texture doesn't require init data (i.e. a blank new texture will suffice)
    // then isDirty should still return false.
    if (mDirty && !mStagingTexture && !mRecoverFromStorage)
    {
        const Renderer11DeviceCaps &deviceCaps = mRenderer->getRenderer11DeviceCaps();
        const d3d11::TextureFormat &formatInfo = d3d11::GetTextureFormatInfo(mInternalFormat, deviceCaps, false);
        if (formatInfo.dataInitializerFunction == nullptr)
        {
            return false;
        }
    }

    return mDirty;
}

gl::Error Image11::copyToStorage(TextureStorage *storage, const gl::ImageIndex &index, const gl::Box &region)
{
    TextureStorage11 *storage11 = GetAs<TextureStorage11>(storage);

    // If an app's behavior results in an Image11 copying its data to/from to a TextureStorage multiple times,
    // then we should just keep the staging texture around to prevent the copying from impacting perf.
    // We allow the Image11 to copy its data to/from TextureStorage once.
    // This accounts for an app making a late call to glGenerateMipmap.
    bool attemptToReleaseStagingTexture = (mRecoveredFromStorageCount < 2);

    if (attemptToReleaseStagingTexture)
    {
        // If another image is relying on this Storage for its data, then we must let it recover its data before we overwrite it.
        gl::Error error = storage11->releaseAssociatedImage(index, this);
        if (error.isError())
        {
            return error;
        }
    }

    ID3D11Resource *stagingTexture = nullptr;
    unsigned int stagingSubresourceIndex = 0;
    gl::Error error = getStagingTexture(&stagingTexture, &stagingSubresourceIndex);
    if (error.isError())
    {
        return error;
    }

    if (storage11->isRenderTarget() && mUsesAlternateRenderableFormat)
    {
        // This means that the texture storage and the staging texture use different formats
        // We must copy and convert the staging texture's data using a temporary texture
        error = copyAndConvertViaTemporaryTexture(stagingTexture, storage11, stagingSubresourceIndex, index, region, true);
    }
    else
    {
        error = storage11->updateSubresourceLevel(stagingTexture, stagingSubresourceIndex, index, region);
    }

    if (error.isError())
    {
        return error;
    }

    // Once the image data has been copied into the Storage, we can release it locally.
    if (attemptToReleaseStagingTexture)
    {
        storage11->associateImage(this, index);
        releaseStagingTexture();
        mRecoverFromStorage = true;
        mAssociatedStorage = storage11;
        mAssociatedImageIndex = index;
    }

    return gl::Error(GL_NO_ERROR);
}

bool Image11::isAssociatedStorageValid(TextureStorage11* textureStorage) const
{
    return (mAssociatedStorage == textureStorage);
}

gl::Error Image11::recoverFromAssociatedStorage()
{
    if (mRecoverFromStorage)
    {
        gl::Error error = createStagingTexture();
        if (error.isError())
        {
            return error;
        }

        bool textureStorageCorrect = mAssociatedStorage->isAssociatedImageValid(mAssociatedImageIndex, this);

        // This means that the cached TextureStorage has been modified after this Image11 released its copy of its data.
        // This should not have happened. The TextureStorage should have told this Image11 to recover its data before it was overwritten.
        ASSERT(textureStorageCorrect);

        if (textureStorageCorrect)
        {
            gl::Box region(0, 0, 0, mWidth, mHeight, mDepth);

            if (mAssociatedStorage->isRenderTarget() && mUsesAlternateRenderableFormat)
            {
                // This means that the texture storage and the staging texture use different formats
                // We must copy and convert the texture storage's data using a temporary texture
                error = copyAndConvertViaTemporaryTexture(mStagingTexture, mAssociatedStorage, mStagingSubresource, mAssociatedImageIndex, region, false);
            }
            else
            {
                // CopySubResource from the Storage to the Staging texture
                error = mAssociatedStorage->copySubresourceLevel(mStagingTexture, mStagingSubresource, mAssociatedImageIndex, region);
            }

            if (error.isError())
            {
                return error;
            }

            mDirty = true;

            mRecoveredFromStorageCount += 1;
        }

        // Reset all the recovery parameters, even if the texture storage association is broken.
        disassociateStorage();
    }

    return gl::Error(GL_NO_ERROR);
}

void Image11::disassociateStorage()
{
    if (mRecoverFromStorage)
    {
        // Make the texturestorage release the Image11 too
        mAssociatedStorage->disassociateImage(mAssociatedImageIndex, this);

        mRecoverFromStorage = false;
        mAssociatedStorage = nullptr;
        mAssociatedImageIndex = gl::ImageIndex::MakeInvalid();
    }
}

bool Image11::redefine(GLenum target, GLenum internalformat, const gl::Extents &size, bool forceRelease)
{
    if (mWidth != size.width ||
        mHeight != size.height ||
        mInternalFormat != internalformat ||
        forceRelease)
    {
        // End the association with the TextureStorage, since that data will be out of date.
        // Also reset mRecoveredFromStorageCount since this Image is getting completely redefined.
        disassociateStorage();
        mRecoveredFromStorageCount = 0;

        mWidth = size.width;
        mHeight = size.height;
        mDepth = size.depth;
        mInternalFormat = internalformat;
        mTarget = target;

        // compute the d3d format that will be used
        const d3d11::TextureFormat &formatInfo = d3d11::GetTextureFormatInfo(internalformat, mRenderer->getRenderer11DeviceCaps(), false);
        const d3d11::TextureFormat &renderableFormatInfo = d3d11::GetTextureFormatInfo(internalformat, mRenderer->getRenderer11DeviceCaps(), true);
        mDXGIFormat = formatInfo.texFormat;
        mRenderable = (renderableFormatInfo.rtvFormat != DXGI_FORMAT_UNKNOWN);
        mUsesAlternateRenderableFormat = (formatInfo.texFormat != renderableFormatInfo.texFormat);

        releaseStagingTexture();
        mDirty = (formatInfo.dataInitializerFunction != nullptr);

        return true;
    }

    return false;
}

DXGI_FORMAT Image11::getDXGIFormat() const
{
    // this should only happen if the image hasn't been redefined first
    // which would be a bug by the caller
    ASSERT(mDXGIFormat != DXGI_FORMAT_UNKNOWN);

    return mDXGIFormat;
}

// Store the pixel rectangle designated by xoffset,yoffset,width,height with pixels stored as format/type at input
// into the target pixel rectangle.
gl::Error Image11::loadData(const gl::Box &area, const gl::PixelUnpackState &unpack, GLenum type, const void *input)
{
    const gl::InternalFormat &formatInfo = gl::GetInternalFormatInfo(mInternalFormat);
    GLsizei inputRowPitch = formatInfo.computeRowPitch(type, area.width, unpack.alignment, unpack.rowLength);
    GLsizei inputDepthPitch = formatInfo.computeDepthPitch(type, area.width, area.height, unpack.alignment, unpack.rowLength);

    const d3d11::DXGIFormat &dxgiFormatInfo = d3d11::GetDXGIFormatInfo(mDXGIFormat);
    GLuint outputPixelSize = dxgiFormatInfo.pixelBytes;

    const d3d11::TextureFormat &d3dFormatInfo = d3d11::GetTextureFormatInfo(mInternalFormat, mRenderer->getRenderer11DeviceCaps(), false);
    LoadImageFunction loadFunction = d3dFormatInfo.loadFunctions.at(type);

    D3D11_MAPPED_SUBRESOURCE mappedImage;
    gl::Error error = map(D3D11_MAP_WRITE, &mappedImage);
    if (error.isError())
    {
        return error;
    }

    uint8_t *offsetMappedData = (reinterpret_cast<uint8_t*>(mappedImage.pData) + (area.y * mappedImage.RowPitch + area.x * outputPixelSize + area.z * mappedImage.DepthPitch));
    loadFunction(area.width, area.height, area.depth,
                 reinterpret_cast<const uint8_t*>(input), inputRowPitch, inputDepthPitch,
                 offsetMappedData, mappedImage.RowPitch, mappedImage.DepthPitch);

    unmap();

    return gl::Error(GL_NO_ERROR);
}

gl::Error Image11::loadCompressedData(const gl::Box &area, const void *input)
{
    const gl::InternalFormat &formatInfo = gl::GetInternalFormatInfo(mInternalFormat);
    GLsizei inputRowPitch = formatInfo.computeRowPitch(GL_UNSIGNED_BYTE, area.width, 1, 0);
    GLsizei inputDepthPitch = formatInfo.computeDepthPitch(GL_UNSIGNED_BYTE, area.width, area.height, 1, 0);

    const d3d11::DXGIFormat &dxgiFormatInfo = d3d11::GetDXGIFormatInfo(mDXGIFormat);
    GLuint outputPixelSize = dxgiFormatInfo.pixelBytes;
    GLuint outputBlockWidth = dxgiFormatInfo.blockWidth;
    GLuint outputBlockHeight = dxgiFormatInfo.blockHeight;

    ASSERT(area.x % outputBlockWidth == 0);
    ASSERT(area.y % outputBlockHeight == 0);

    const d3d11::TextureFormat &d3dFormatInfo = d3d11::GetTextureFormatInfo(mInternalFormat, mRenderer->getRenderer11DeviceCaps(), false);
    LoadImageFunction loadFunction = d3dFormatInfo.loadFunctions.at(GL_UNSIGNED_BYTE);

    D3D11_MAPPED_SUBRESOURCE mappedImage;
    gl::Error error = map(D3D11_MAP_WRITE, &mappedImage);
    if (error.isError())
    {
        return error;
    }

    uint8_t* offsetMappedData = reinterpret_cast<uint8_t*>(mappedImage.pData) + ((area.y / outputBlockHeight) * mappedImage.RowPitch +
                                                                           (area.x / outputBlockWidth) * outputPixelSize +
                                                                           area.z * mappedImage.DepthPitch);

    loadFunction(area.width, area.height, area.depth,
                 reinterpret_cast<const uint8_t*>(input), inputRowPitch, inputDepthPitch,
                 offsetMappedData, mappedImage.RowPitch, mappedImage.DepthPitch);

    unmap();

    return gl::Error(GL_NO_ERROR);
}

gl::Error Image11::copy(const gl::Offset &destOffset, const gl::Rectangle &sourceArea, RenderTargetD3D *source)
{
    RenderTarget11 *sourceRenderTarget = GetAs<RenderTarget11>(source);
    ASSERT(sourceRenderTarget->getTexture());

    ID3D11Resource *resource = sourceRenderTarget->getTexture();
    UINT subresourceIndex = sourceRenderTarget->getSubresourceIndex();

    gl::Box sourceBox(sourceArea.x, sourceArea.y, 0, sourceArea.width, sourceArea.height, 1);
    gl::Error error = copy(destOffset, sourceBox, resource, subresourceIndex);

    SafeRelease(resource);

    return error;
}

gl::Error Image11::copy(const gl::Offset &destOffset, const gl::Box &sourceArea, const gl::ImageIndex &sourceIndex, TextureStorage *source)
{
    TextureStorage11 *sourceStorage11 = GetAs<TextureStorage11>(source);

    UINT subresourceIndex = sourceStorage11->getSubresourceIndex(sourceIndex);
    ID3D11Resource *resource = nullptr;
    gl::Error error = sourceStorage11->getResource(&resource);
    if (error.isError())
    {
        return error;
    }

    error = copy(destOffset, sourceArea, resource, subresourceIndex);

    SafeRelease(resource);

    return error;
}

gl::Error Image11::copy(const gl::Offset &destOffset, const gl::Box &sourceArea, ID3D11Resource *source, UINT sourceSubResource)
{
    D3D11_RESOURCE_DIMENSION dim;
    source->GetType(&dim);

    DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
    gl::Extents extents;
    UINT sampleCount = 0;

    ID3D11Texture2D *source2D = nullptr;

    if (dim == D3D11_RESOURCE_DIMENSION_TEXTURE2D)
    {
        D3D11_TEXTURE2D_DESC textureDesc2D;
        source2D = d3d11::DynamicCastComObject<ID3D11Texture2D>(source);
        ASSERT(source2D);
        source2D->GetDesc(&textureDesc2D);

        format = textureDesc2D.Format;
        extents = gl::Extents(textureDesc2D.Width, textureDesc2D.Height, 1);
        sampleCount = textureDesc2D.SampleDesc.Count;
    }
    else if (dim == D3D11_RESOURCE_DIMENSION_TEXTURE3D)
    {
        D3D11_TEXTURE3D_DESC textureDesc3D;
        ID3D11Texture3D *source3D = d3d11::DynamicCastComObject<ID3D11Texture3D>(source);
        ASSERT(source3D);
        source3D->GetDesc(&textureDesc3D);

        format = textureDesc3D.Format;
        extents = gl::Extents(textureDesc3D.Width, textureDesc3D.Height, textureDesc3D.Depth);
        sampleCount = 1;
    }
    else
    {
        UNREACHABLE();
    }

    if (format == mDXGIFormat)
    {
        // No conversion needed-- use copyback fastpath
        ID3D11Resource *stagingTexture = nullptr;
        unsigned int stagingSubresourceIndex = 0;
        gl::Error error = getStagingTexture(&stagingTexture, &stagingSubresourceIndex);
        if (error.isError())
        {
            return error;
        }

        ID3D11Device *device = mRenderer->getDevice();
        ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();

        UINT subresourceAfterResolve = sourceSubResource;

        ID3D11Resource *srcTex = nullptr;

        bool needResolve = (dim == D3D11_RESOURCE_DIMENSION_TEXTURE2D && sampleCount > 1);

        if (needResolve)
        {
            D3D11_TEXTURE2D_DESC resolveDesc;
            resolveDesc.Width = extents.width;
            resolveDesc.Height = extents.height;
            resolveDesc.MipLevels = 1;
            resolveDesc.ArraySize = 1;
            resolveDesc.Format = format;
            resolveDesc.SampleDesc.Count = 1;
            resolveDesc.SampleDesc.Quality = 0;
            resolveDesc.Usage = D3D11_USAGE_DEFAULT;
            resolveDesc.BindFlags = 0;
            resolveDesc.CPUAccessFlags = 0;
            resolveDesc.MiscFlags = 0;

            ID3D11Texture2D *srcTex2D = nullptr;
            HRESULT result = device->CreateTexture2D(&resolveDesc, nullptr, &srcTex2D);
            if (FAILED(result))
            {
                return gl::Error(GL_OUT_OF_MEMORY, "Failed to create resolve texture for Image11::copy, HRESULT: 0x%X.", result);
            }
            srcTex = srcTex2D;

            deviceContext->ResolveSubresource(srcTex, 0, source, sourceSubResource, format);
            subresourceAfterResolve = 0;
        }
        else
        {
            srcTex = source;
        }

        D3D11_BOX srcBox;
        srcBox.left = sourceArea.x;
        srcBox.right = sourceArea.x + sourceArea.width;
        srcBox.top = sourceArea.y;
        srcBox.bottom = sourceArea.y + sourceArea.height;
        srcBox.front = sourceArea.z;
        srcBox.back = sourceArea.z + sourceArea.depth;

        deviceContext->CopySubresourceRegion(stagingTexture, stagingSubresourceIndex, destOffset.x, destOffset.y,
                                             destOffset.z, srcTex, subresourceAfterResolve, &srcBox);

        if (needResolve)
        {
            SafeRelease(srcTex);
        }
    }
    else
    {
        // This format requires conversion, so we must copy the texture to staging and manually convert via readPixels
        D3D11_MAPPED_SUBRESOURCE mappedImage;
        gl::Error error = map(D3D11_MAP_WRITE, &mappedImage);
        if (error.isError())
        {
            return error;
        }

        // determine the offset coordinate into the destination buffer
        const d3d11::DXGIFormat &dxgiFormatInfo = d3d11::GetDXGIFormatInfo(mDXGIFormat);
        GLsizei rowOffset = dxgiFormatInfo.pixelBytes * destOffset.x;
        uint8_t *dataOffset = static_cast<uint8_t*>(mappedImage.pData) + mappedImage.RowPitch * destOffset.y + rowOffset + destOffset.z * mappedImage.DepthPitch;

        const gl::InternalFormat &formatInfo = gl::GetInternalFormatInfo(mInternalFormat);

        // Currently in ANGLE, the source data may only need to be converted if the source is the current framebuffer
        // and OpenGL ES framebuffers must be 2D textures therefore we should not need to convert 3D textures between different formats.
        ASSERT(dim == D3D11_RESOURCE_DIMENSION_TEXTURE2D);
        ASSERT(sourceArea.z == 0 && sourceArea.depth == 1);
        gl::Rectangle sourceRect(sourceArea.x, sourceArea.y, sourceArea.width, sourceArea.height);
        error = mRenderer->readTextureData(source2D, sourceSubResource, sourceRect, formatInfo.format, formatInfo.type, mappedImage.RowPitch, gl::PixelPackState(), dataOffset);

        unmap();

        if (error.isError())
        {
            return error;
        }
    }

    mDirty = true;

    return gl::Error(GL_NO_ERROR);
}

gl::Error Image11::getStagingTexture(ID3D11Resource **outStagingTexture, unsigned int *outSubresourceIndex)
{
    gl::Error error = createStagingTexture();
    if (error.isError())
    {
        return error;
    }

    *outStagingTexture = mStagingTexture;
    *outSubresourceIndex = mStagingSubresource;
    return gl::Error(GL_NO_ERROR);
}

void Image11::releaseStagingTexture()
{
    SafeRelease(mStagingTexture);
}

gl::Error Image11::createStagingTexture()
{
    if (mStagingTexture)
    {
        return gl::Error(GL_NO_ERROR);
    }

    ASSERT(mWidth > 0 && mHeight > 0 && mDepth > 0);

    const DXGI_FORMAT dxgiFormat = getDXGIFormat();

    ID3D11Device *device = mRenderer->getDevice();
    HRESULT result;

    int lodOffset = 1;
    GLsizei width = mWidth;
    GLsizei height = mHeight;

    // adjust size if needed for compressed textures
    d3d11::MakeValidSize(false, dxgiFormat, &width, &height, &lodOffset);

    if (mTarget == GL_TEXTURE_3D)
    {
        ID3D11Texture3D *newTexture = nullptr;

        D3D11_TEXTURE3D_DESC desc;
        desc.Width = width;
        desc.Height = height;
        desc.Depth = mDepth;
        desc.MipLevels = lodOffset + 1;
        desc.Format = dxgiFormat;
        desc.Usage = D3D11_USAGE_STAGING;
        desc.BindFlags = 0;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;

        if (d3d11::GetTextureFormatInfo(mInternalFormat, mRenderer->getRenderer11DeviceCaps(), false).dataInitializerFunction != nullptr)
        {
            std::vector<D3D11_SUBRESOURCE_DATA> initialData;
            std::vector< std::vector<BYTE> > textureData;
            d3d11::GenerateInitialTextureData(mInternalFormat, mRenderer->getRenderer11DeviceCaps(), width, height, mDepth,
                                              lodOffset + 1, &initialData, &textureData);

            result = device->CreateTexture3D(&desc, initialData.data(), &newTexture);
        }
        else
        {
            result = device->CreateTexture3D(&desc, nullptr, &newTexture);
        }

        if (FAILED(result))
        {
            ASSERT(result == E_OUTOFMEMORY);
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to create staging texture, result: 0x%X.", result);
        }

        mStagingTexture = newTexture;
        mStagingSubresource = D3D11CalcSubresource(lodOffset, 0, lodOffset + 1);
    }
    else if (mTarget == GL_TEXTURE_2D || mTarget == GL_TEXTURE_2D_ARRAY || mTarget == GL_TEXTURE_CUBE_MAP)
    {
        ID3D11Texture2D *newTexture = nullptr;

        D3D11_TEXTURE2D_DESC desc;
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = lodOffset + 1;
        desc.ArraySize = 1;
        desc.Format = dxgiFormat;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_STAGING;
        desc.BindFlags = 0;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;

        if (d3d11::GetTextureFormatInfo(mInternalFormat, mRenderer->getRenderer11DeviceCaps(), false).dataInitializerFunction != nullptr)
        {
            std::vector<D3D11_SUBRESOURCE_DATA> initialData;
            std::vector< std::vector<BYTE> > textureData;
            d3d11::GenerateInitialTextureData(mInternalFormat, mRenderer->getRenderer11DeviceCaps(), width, height, 1,
                                              lodOffset + 1, &initialData, &textureData);

            result = device->CreateTexture2D(&desc, initialData.data(), &newTexture);
        }
        else
        {
            result = device->CreateTexture2D(&desc, nullptr, &newTexture);
        }

        if (FAILED(result))
        {
            ASSERT(result == E_OUTOFMEMORY);
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to create staging texture, result: 0x%X.", result);
        }

        mStagingTexture = newTexture;
        mStagingSubresource = D3D11CalcSubresource(lodOffset, 0, lodOffset + 1);
    }
    else
    {
        UNREACHABLE();
    }

    mDirty = false;
    return gl::Error(GL_NO_ERROR);
}

gl::Error Image11::map(D3D11_MAP mapType, D3D11_MAPPED_SUBRESOURCE *map)
{
    // We must recover from the TextureStorage if necessary, even for D3D11_MAP_WRITE.
    gl::Error error = recoverFromAssociatedStorage();
    if (error.isError())
    {
        return error;
    }

    ID3D11Resource *stagingTexture = nullptr;
    unsigned int subresourceIndex = 0;
    error = getStagingTexture(&stagingTexture, &subresourceIndex);
    if (error.isError())
    {
        return error;
    }

    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();

    ASSERT(mStagingTexture);
    HRESULT result = deviceContext->Map(stagingTexture, subresourceIndex, mapType, 0, map);

    if (FAILED(result))
    {
        // this can fail if the device is removed (from TDR)
        if (d3d11::isDeviceLostError(result))
        {
            mRenderer->notifyDeviceLost();
        }
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to map staging texture, result: 0x%X.", result);
    }

    mDirty = true;

    return gl::Error(GL_NO_ERROR);
}

void Image11::unmap()
{
    if (mStagingTexture)
    {
        ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();
        deviceContext->Unmap(mStagingTexture, mStagingSubresource);
    }
}

gl::Error Image11::copyAndConvertTexture(ID3D11Resource *input, DXGI_FORMAT inputFormat,
                                         ID3D11Resource *output, DXGI_FORMAT outputFormat,
                                         unsigned int subresourceIndex, size_t width, size_t height, size_t depth)
{
    ASSERT(input);
    ASSERT(output);

    D3D11_MAPPED_SUBRESOURCE inputMap;
    D3D11_MAPPED_SUBRESOURCE outputMap;
    HRESULT hr = S_OK;

    LoadImageFunction loadFunction = nullptr;
    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();

    if (inputFormat == DXGI_FORMAT_B4G4R4A4_UNORM && outputFormat == DXGI_FORMAT_R8G8B8A8_UNORM)
    {
        loadFunction = LoadARGB4ToRGBA8;
    }
    else if (inputFormat == DXGI_FORMAT_R8G8B8A8_UNORM && outputFormat == DXGI_FORMAT_B4G4R4A4_UNORM)
    {
        loadFunction = LoadRGBA8toARGB4;
    }
    else
    {
        // If we want to support conversion between more DXGI formats, then we should
        // add a proper map between DXGI formats instead of hardcoding these functions
        UNREACHABLE();
    }

    hr = deviceContext->Map(input, subresourceIndex, D3D11_MAP_WRITE, 0, &inputMap);
    if (FAILED(hr))
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to map inputted texture, result: 0x%X.", hr);
    }

    hr = deviceContext->Map(output, subresourceIndex, D3D11_MAP_WRITE, 0, &outputMap);
    if (FAILED(hr))
    {
        deviceContext->Unmap(input, subresourceIndex);
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to map output texture, result: 0x%X.", hr);
    }

    loadFunction(width, height, depth,
                 reinterpret_cast<const uint8_t*>(inputMap.pData), inputMap.RowPitch, inputMap.DepthPitch,
                 reinterpret_cast<uint8_t*>(outputMap.pData), outputMap.RowPitch, outputMap.DepthPitch);

    deviceContext->Unmap(input, subresourceIndex);
    deviceContext->Unmap(output, subresourceIndex);

    return gl::Error(GL_NO_ERROR);
}

gl::Error Image11::createTemporaryRenderableFormatImage2D(DXGI_FORMAT format, ID3D11Texture2D **output)
{
    ID3D11Texture2D *staging2D = d3d11::DynamicCastComObject<ID3D11Texture2D>(mStagingTexture);
    ASSERT(staging2D != nullptr);

    // Create a new texture with the same parameters, except in the renderable texture format
    D3D11_TEXTURE2D_DESC tex2Ddesc;
    staging2D->GetDesc(&tex2Ddesc);
    tex2Ddesc.Format = format;

    HRESULT hr = mRenderer->getDevice()->CreateTexture2D(&tex2Ddesc, nullptr, output);
    if (FAILED(hr))
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to create temporary Image2D in the renderable format, result: 0x%X.", hr);
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error Image11::createTemporaryRenderableFormatImage3D(DXGI_FORMAT format, ID3D11Texture3D **output)
{
    ID3D11Texture3D *staging3D = d3d11::DynamicCastComObject<ID3D11Texture3D>(mStagingTexture);
    ASSERT(staging3D != nullptr);

    // Create a new texture with the same parameters, except in the renderable texture format
    D3D11_TEXTURE3D_DESC tex3Ddesc;
    staging3D->GetDesc(&tex3Ddesc);
    tex3Ddesc.Format = format;

    HRESULT hr = mRenderer->getDevice()->CreateTexture3D(&tex3Ddesc, nullptr, output);
    if (FAILED(hr))
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to create temporary Image3D in the renderable format, result: 0x%X.", hr);
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error Image11::copyAndConvertViaTemporaryTexture(ID3D11Resource *stagingTexture, TextureStorage11 *storage11,
                                                     unsigned int stagingSubresourceIndex,
                                                     const gl::ImageIndex &index, const gl::Box &region,
                                                     bool intoStorage)
{
    // This method does the following:
    //  - Creates a temporary 1-level texture in the renderable texture format
    //  - If 'intoStorage' is true then:
    //        - Copies and convert stagingTexture into this temporary texture
    //        - Copies the temporary texture's data into the storage
    //  - Otherwise, if 'intoStorage' is false then:
    //        - Copy the texture storage's data into this temporary texture
    //        - Copy and convert the temporary texture's data back into stagingTexture
    //  - Deletes the temporary texture

    const d3d11::TextureFormat &d3dRenderableFormatInfo = d3d11::GetTextureFormatInfo(mInternalFormat, mRenderer->getRenderer11DeviceCaps(), true);
    gl::Error error(GL_NO_ERROR);

    ID3D11Resource *tempRenderableFormatTexture = nullptr;

    D3D11_RESOURCE_DIMENSION dim;
    stagingTexture->GetType(&dim);
    if (dim == D3D11_RESOURCE_DIMENSION_TEXTURE2D)
    {
        ID3D11Texture2D *tex2d = nullptr;
        createTemporaryRenderableFormatImage2D(d3dRenderableFormatInfo.texFormat, &tex2d);
        tempRenderableFormatTexture = d3d11::DynamicCastComObject<ID3D11Resource>(tex2d);
    }
    else if (dim == D3D11_RESOURCE_DIMENSION_TEXTURE3D)
    {
        ID3D11Texture3D *tex3d = nullptr;
        createTemporaryRenderableFormatImage3D(d3dRenderableFormatInfo.texFormat, &tex3d);
        tempRenderableFormatTexture = d3d11::DynamicCastComObject<ID3D11Resource>(tex3d);
    }
    else
    {
        UNREACHABLE();
        return gl::Error(GL_INVALID_OPERATION, "Invalid texture type detected");
    }

    if (intoStorage)
    {
        // First copy this image's staging texture into the temporary texture
        // Then we copy the temporary texture into the storage
        copyAndConvertTexture(stagingTexture, mDXGIFormat, tempRenderableFormatTexture, d3dRenderableFormatInfo.texFormat, stagingSubresourceIndex, mWidth, mHeight, mDepth);
        if (error.isError())
        {
            SafeRelease(tempRenderableFormatTexture);
            return error;
        }

        error = storage11->updateSubresourceLevel(tempRenderableFormatTexture, stagingSubresourceIndex, index, region);
    }
    else
    {
        // Copy the texture storage's data into the temporary texture
        // Then copy the temporary texture's data into the actual image
        error = mAssociatedStorage->copySubresourceLevel(tempRenderableFormatTexture, mStagingSubresource, mAssociatedImageIndex, region);
        if (error.isError())
        {
            SafeRelease(tempRenderableFormatTexture);
            return error;
        }

        error = copyAndConvertTexture(tempRenderableFormatTexture, d3dRenderableFormatInfo.texFormat, mStagingTexture, mDXGIFormat, mStagingSubresource, mWidth, mHeight, mDepth);
    }

    SafeRelease(tempRenderableFormatTexture);
    if (error.isError())
    {
        return error;
    }

    return error;
}

}
