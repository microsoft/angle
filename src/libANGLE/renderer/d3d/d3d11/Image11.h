//
// Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Image11.h: Defines the rx::Image11 class, which acts as the interface to
// the actual underlying resources of a Texture

#ifndef LIBANGLE_RENDERER_D3D_D3D11_IMAGE11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_IMAGE11_H_

#include "common/debug.h"
#include "libANGLE/ImageIndex.h"
#include "libANGLE/renderer/d3d/formatutilsD3D.h"
#include "libANGLE/renderer/d3d/ImageD3D.h"

namespace gl
{
class Framebuffer;
}

namespace rx
{
class Renderer11;
class TextureStorage11;

class Image11 : public ImageD3D
{
  public:
    Image11(Renderer11 *renderer);
    virtual ~Image11();

    static gl::Error generateMipmap(Image11 *dest, Image11 *src);

    virtual bool isDirty() const;

    virtual gl::Error copyToStorage(TextureStorage *storage, const gl::ImageIndex &index, const gl::Box &region);

    bool redefine(GLenum target, GLenum internalformat, const gl::Extents &size, bool forceRelease) override;

    DXGI_FORMAT getDXGIFormat() const;

    virtual gl::Error loadData(const gl::Box &area, const gl::PixelUnpackState &unpack, GLenum type, const void *input);
    virtual gl::Error loadCompressedData(const gl::Box &area, const void *input);

    virtual gl::Error copy(const gl::Offset &destOffset, const gl::Rectangle &sourceArea, RenderTargetD3D *source);
    virtual gl::Error copy(const gl::Offset &destOffset, const gl::Box &sourceArea,
                           const gl::ImageIndex &sourceIndex, TextureStorage *source);

    gl::Error recoverFromAssociatedStorage();
    bool isAssociatedStorageValid(TextureStorage11* textureStorage) const;
    void disassociateStorage();

  protected:
    gl::Error map(D3D11_MAP mapType, D3D11_MAPPED_SUBRESOURCE *map);
    void unmap();

  private:
    gl::Error copyToStorageImpl(TextureStorage11 *storage11, const gl::ImageIndex &index, const gl::Box &region);
    gl::Error copy(const gl::Offset &destOffset, const gl::Box &sourceArea, ID3D11Resource *source, UINT sourceSubResource);

    gl::Error copyAndConvertTexture(ID3D11Resource *input, DXGI_FORMAT inputFormat,
                                    ID3D11Resource *output, DXGI_FORMAT outputFormat,
                                    unsigned int subresourceIndex, size_t width, size_t height, size_t depth);

    gl::Error createTemporaryRenderableFormatImage2D(DXGI_FORMAT format, ID3D11Texture2D **output);
    gl::Error createTemporaryRenderableFormatImage3D(DXGI_FORMAT format, ID3D11Texture3D **output);

    gl::Error getStagingTexture(ID3D11Resource **outStagingTexture, unsigned int *outSubresourceIndex);
    gl::Error createStagingTexture();
    gl::Error copyAndConvertViaTemporaryTexture(ID3D11Resource *stagingTexture, TextureStorage11 *storage11,
                                                unsigned int stagingSubresourceIndex,
                                                const gl::ImageIndex &index, const gl::Box &region,
                                                bool intoStorage);
    void releaseStagingTexture();

    Renderer11 *mRenderer;

    DXGI_FORMAT mDXGIFormat;
    ID3D11Resource *mStagingTexture;
    unsigned int mStagingSubresource;

    bool mRecoverFromStorage;
    TextureStorage11 *mAssociatedStorage;
    gl::ImageIndex mAssociatedImageIndex;
    unsigned int mRecoveredFromStorageCount;

    bool mUsesAlternateRenderableFormat;
};

}

#endif // LIBANGLE_RENDERER_D3D_D3D11_IMAGE11_H_
