//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BufferD3D.cpp Defines common functionality between the Buffer9 and Buffer11 classes.

#include "libANGLE/renderer/d3d/BufferD3D.h"
#include "libANGLE/renderer/d3d/VertexBuffer.h"
#include "libANGLE/renderer/d3d/IndexBuffer.h"
#include "libANGLE/vertexattribute.h"

namespace rx
{

unsigned int BufferD3D::mNextSerial = 1;

BufferD3D::BufferD3D()
    : BufferImpl(),
      mStaticIndexBuffer(NULL),
      mUseStaticBuffers(false)
{
    updateSerial();
}

BufferD3D::~BufferD3D()
{
    SafeDelete(mStaticIndexBuffer);

    // Clean up the static buffers for each attribute too.
    for (StaticBufferIteratorType i = mStaticVertexBufferForAttributeMap.begin(); i != mStaticVertexBufferForAttributeMap.end(); i++)
    {
        SafeDelete(i->second);
    }
}

BufferD3D::AttribElement BufferD3D::CreateAttribElementFromAttrib(const gl::VertexAttribute &attrib)
{
    size_t attributeOffset = static_cast<size_t>(attrib.offset) % ComputeVertexAttributeStride(attrib);
    return { attrib.type, attrib.size, static_cast<GLuint>(ComputeVertexAttributeStride(attrib)), attrib.normalized, attrib.pureInteger, attributeOffset };
}

void BufferD3D::updateSerial()
{
    mSerial = mNextSerial++;
}

StaticVertexBufferInterface *BufferD3D::findStaticVertexBufferForAttribute(const gl::VertexAttribute &attrib)
{
    AttribElement element = CreateAttribElementFromAttrib(attrib);

    StaticBufferIteratorType mapElement = mStaticVertexBufferForAttributeMap.find(element);

    // If we've already created a StaticVertexBufferInterface for this attribute, then we return it
    if (mapElement != mStaticVertexBufferForAttributeMap.end())
    {
        return mapElement->second;
    }

    return NULL;
}

StaticVertexBufferInterface *BufferD3D::getStaticVertexBufferForAttribute(const gl::VertexAttribute &attrib)
{
    if (!mUseStaticBuffers)
        return NULL;

    if (attrib.type == GL_NONE)
        return NULL;

    StaticVertexBufferInterface *bufferForAttribute = findStaticVertexBufferForAttribute(attrib);
    if (bufferForAttribute != NULL)
    {
        return bufferForAttribute;
    }

    // If we have too many static buffers (using up a lot of memory) then we should revert to streaming buffers.
    // The exact memory usage of the static buffers is difficult to measure.
    // Each static vertex buffer will say they are of size 0 until they are first used (which is when they are populated with data)
    // We therefore use two metrics to try to limit the memory usage of these static buffers:
    //      - If some of the static vertex buffers are populated with data, then we sum up their total size, and we don't create another
    //        static buffer if their total size is 3x larger than the original buffer's size. "3x" could be made lower to save memory.
    //      - We also set a high upper bound for the number of static vertex buffers that each buffer can create, to cover the case when
    //        a lot of static buffers are created before many of them are populated with data.
    if (mStaticVertexBufferForAttributeMap.size() <= 100)
    {
        size_t totalStaticBufferSize = 0;
        for (StaticBufferIteratorType i = mStaticVertexBufferForAttributeMap.begin(); i != mStaticVertexBufferForAttributeMap.end(); i++)
        {
            // ASSERT against overflow
            ASSERT(totalStaticBufferSize + i->second->getBufferSize() >= totalStaticBufferSize);
            totalStaticBufferSize += i->second->getBufferSize();
        }

        if (totalStaticBufferSize <= 3 * getSize())
        {
            AttribElement element = CreateAttribElementFromAttrib(attrib);
            bufferForAttribute = new StaticVertexBufferInterface(getRenderer());
            mStaticVertexBufferForAttributeMap[element] = bufferForAttribute;
        }
    }

    return bufferForAttribute;
}

void BufferD3D::enableStaticData()
{
    mUseStaticBuffers = true;

    if (!mStaticIndexBuffer)
    {
        mStaticIndexBuffer = new StaticIndexBufferInterface(getRenderer());
    }
}

void BufferD3D::invalidateStaticIndexData()
{
    if (mStaticIndexBuffer && mStaticIndexBuffer->getBufferSize() != 0)
    {
        SafeDelete(mStaticIndexBuffer);

        // Re-init static data to track that we're in a static buffer
        enableStaticData();
    }

    mUnmodifiedIndexDataUse = 0;
}

// Creates static buffers if sufficient used data has been left unmodified
void BufferD3D::promoteStaticIndexUsage(int dataSize)
{
    if (!mStaticIndexBuffer)
    {
        mUnmodifiedIndexDataUse += dataSize;

        if (mUnmodifiedIndexDataUse > 3 * getSize())
        {
            mStaticIndexBuffer = new StaticIndexBufferInterface(getRenderer());
        }
    }
}

void BufferD3D::promoteStaticVertexUsageForAttrib(const gl::VertexAttribute &attrib, int dataSize)
{
    StaticVertexBufferInterface* bufferForAttribute = findStaticVertexBufferForAttribute(attrib);

    if (bufferForAttribute == NULL)
    {
        AttribElement element = CreateAttribElementFromAttrib(attrib);

        if (mUnmodifiedVertexDataUseMap.find(element) == mUnmodifiedVertexDataUseMap.end())
        {
            mUnmodifiedVertexDataUseMap[element] = 0;
        }

        mUnmodifiedVertexDataUseMap[element] += dataSize;

        if (mUnmodifiedVertexDataUseMap[element] > 3 * getSize())
        {
            getStaticVertexBufferForAttribute(attrib);
        }
    }
}

}