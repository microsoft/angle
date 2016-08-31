//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Shader.cpp: Implements the gl::Shader class and its  derived classes
// VertexShader and FragmentShader. Implements GL shader objects and related
// functionality. [OpenGL ES 2.0.24] section 2.10 page 24 and section 3.8 page 84.

#include "libANGLE/Shader.h"

#include <sstream>

#include "common/utilities.h"
#include "GLSLANG/ShaderLang.h"
#include "libANGLE/Compiler.h"
#include "libANGLE/Constants.h"
#include "libANGLE/renderer/Renderer.h"
#include "libANGLE/renderer/ShaderImpl.h"
#include "libANGLE/ResourceManager.h"
#include "libANGLE/renderer/d3d/d3d11/winrt/HolographicNativeWindow.h"

namespace gl
{

namespace
{
template <typename VarT>
std::vector<VarT> GetActiveShaderVariables(const std::vector<VarT> *variableList)
{
    ASSERT(variableList);
    std::vector<VarT> result;
    for (size_t varIndex = 0; varIndex < variableList->size(); varIndex++)
    {
        const VarT &var = variableList->at(varIndex);
        if (var.staticUse)
        {
            result.push_back(var);
        }
    }
    return result;
}

template <typename VarT>
const std::vector<VarT> &GetShaderVariables(const std::vector<VarT> *variableList)
{
    ASSERT(variableList);
    return *variableList;
}

}  // anonymous namespace

// true if varying x has a higher priority in packing than y
bool CompareShaderVar(const sh::ShaderVariable &x, const sh::ShaderVariable &y)
{
    if (x.type == y.type)
    {
        return x.arraySize > y.arraySize;
    }

    // Special case for handling structs: we sort these to the end of the list
    if (x.type == GL_STRUCT_ANGLEX)
    {
        return false;
    }

    if (y.type == GL_STRUCT_ANGLEX)
    {
        return true;
    }

    return gl::VariableSortOrder(x.type) < gl::VariableSortOrder(y.type);
}

Shader::Data::Data(GLenum shaderType) : mLabel(), mShaderType(shaderType), mShaderVersion(100)
{
}

Shader::Data::~Data()
{
}

Shader::Shader(ResourceManager *manager,
               rx::ImplFactory *implFactory,
               const gl::Limitations &rendererLimitations,
               GLenum type,
               GLuint handle)
    : mData(type),
      mImplementation(implFactory->createShader(mData)),
      mRendererLimitations(rendererLimitations),
      mHandle(handle),
      mType(type),
      mRefCount(0),
      mDeleteStatus(false),
      mCompiled(false),
      mResourceManager(manager)
{
    ASSERT(mImplementation);
}

Shader::~Shader()
{
    SafeDelete(mImplementation);
}

void Shader::setLabel(const std::string &label)
{
    mData.mLabel = label;
}

const std::string &Shader::getLabel() const
{
    return mData.mLabel;
}

GLuint Shader::getHandle() const
{
    return mHandle;
}

void Shader::setSource(GLsizei count, const char *const *string, const GLint *length)
{
    std::ostringstream stream;

    for (int i = 0; i < count; i++)
    {
        if (length == nullptr || length[i] < 0)
        {
            stream.write(string[i], strlen(string[i]));
        }
        else
        {
            stream.write(string[i], length[i]);
        }
    }

    mData.mSource = stream.str();
}

int Shader::getInfoLogLength() const
{
    if (mInfoLog.empty())
    {
        return 0;
    }

    return (static_cast<int>(mInfoLog.length()) + 1);
}

void Shader::getInfoLog(GLsizei bufSize, GLsizei *length, char *infoLog) const
{
    int index = 0;

    if (bufSize > 0)
    {
        index = std::min(bufSize - 1, static_cast<GLsizei>(mInfoLog.length()));
        memcpy(infoLog, mInfoLog.c_str(), index);

        infoLog[index] = '\0';
    }

    if (length)
    {
        *length = index;
    }
}

int Shader::getSourceLength() const
{
    return mData.mSource.empty() ? 0 : (static_cast<int>(mData.mSource.length()) + 1);
}

int Shader::getTranslatedSourceLength() const
{
    if (mData.mTranslatedSource.empty())
    {
        return 0;
    }

    return (static_cast<int>(mData.mTranslatedSource.length()) + 1);
}

int Shader::getTranslatedSourceWithDebugInfoLength() const
{
    const std::string &debugInfo = mImplementation->getDebugInfo();
    if (debugInfo.empty())
    {
        return 0;
    }

    return (static_cast<int>(debugInfo.length()) + 1);
}

void Shader::getSourceImpl(const std::string &source, GLsizei bufSize, GLsizei *length, char *buffer)
{
    int index = 0;

    if (bufSize > 0)
    {
        index = std::min(bufSize - 1, static_cast<GLsizei>(source.length()));
        memcpy(buffer, source.c_str(), index);

        buffer[index] = '\0';
    }

    if (length)
    {
        *length = index;
    }
}

void Shader::getSource(GLsizei bufSize, GLsizei *length, char *buffer) const
{
    getSourceImpl(mData.mSource, bufSize, length, buffer);
}

void Shader::getTranslatedSource(GLsizei bufSize, GLsizei *length, char *buffer) const
{
    getSourceImpl(mData.mTranslatedSource, bufSize, length, buffer);
}

void Shader::getTranslatedSourceWithDebugInfo(GLsizei bufSize, GLsizei *length, char *buffer) const
{
    const std::string &debugInfo = mImplementation->getDebugInfo();
    getSourceImpl(debugInfo, bufSize, length, buffer);
}

void Shader::compile(Compiler *compiler)
{
    mData.mTranslatedSource.clear();
    mInfoLog.clear();
    mData.mShaderVersion = 100;
    mData.mVaryings.clear();
    mData.mUniforms.clear();
    mData.mInterfaceBlocks.clear();
    mData.mActiveAttributes.clear();
    mData.mActiveOutputVariables.clear();

    ShHandle compilerHandle = compiler->getCompilerHandle(mData.mShaderType);

    std::stringstream sourceStream;

    std::string sourcePath;
    int additionalOptions =
        mImplementation->prepareSourceAndReturnOptions(&sourceStream, &sourcePath);
    int compileOptions    = (SH_OBJECT_CODE | SH_VARIABLES | additionalOptions);

    // Some targets (eg D3D11 Feature Level 9_3 and below) do not support non-constant loop indexes
    // in fragment shaders. Shader compilation will fail. To provide a better error message we can
    // instruct the compiler to pre-validate.
    if (mRendererLimitations.shadersRequireIndexedLoopValidation)
    {
        compileOptions |= SH_VALIDATE_LOOP_INDEXING;
    }

    std::string sourceString  = sourceStream.str();
    std::vector<const char *> sourceCStrings;

    if (!sourcePath.empty())
    {
        sourceCStrings.push_back(sourcePath.c_str());
    }

#ifdef ANGLE_ENABLE_WINDOWS_HOLOGRAPHIC
    if (rx::HolographicNativeWindow::IsInitialized() && rx::HolographicSwapChain11::getIsAutomaticStereoRenderingEnabled())
    {
        // On Windows Holographic, update the shader version to 3, if it is less than that, and 
        // apply our stereo instancing modification - which requires gl_InstanceID, which is only
        // available on shader version 3 or higher.
        bool isGlslesVersion3 = false;
        if (sourceString.find("#version 3") != std::string::npos)
        {
            isGlslesVersion3 = true;
        }

        // Update the vertex shader to apply stereo instancing.
        if (sourceString.find("gl_Position") != std::string::npos)
        {
            if (!isGlslesVersion3)
            {
                // Translate the shader to GLSL version 300 syntax.
                std::string modifiedSourceString = std::string("#version 300 es\n ") + sourceString;
                size_t varyingPos = 0;
                while ((varyingPos = modifiedSourceString.find("varying")) != std::string::npos)
                {
                    const char* outString = "out ";
                    modifiedSourceString.replace(varyingPos, size_t(7), outString);
                }
                while ((varyingPos = modifiedSourceString.find("attribute")) != std::string::npos)
                {
                    const char* inString = "in ";
                    modifiedSourceString.replace(varyingPos, size_t(9), inString);
                }
                sourceString = modifiedSourceString;
            }
        
            std::string firstStringAlwaysStart = "#version 3";
            std::string firstStringAlwaysEnd = "\n";
            size_t index0 = sourceString.find(firstStringAlwaysStart);
            size_t index1 = sourceString.find(firstStringAlwaysEnd, index0 + 1) + firstStringAlwaysEnd.size();
            size_t index2 = sourceString.find_last_of('}');
            std::string modifiedSourceString = 
                sourceString.substr(0, index1) + 
                "uniform mat4 uHolographicViewMatrix[2];\n uniform mat4 uHolographicProjectionMatrix[2];\n uniform mat4 uHolographicViewProjectionMatrix[2];\n uniform mat4 uUndoMidViewMatrix;\n out float vRenderTargetArrayIndex;\n " +
                sourceString.substr(index1, index2-index1) +
                " int index = gl_InstanceIDUnmodified - (gl_InstanceID*2);\n vRenderTargetArrayIndex = float(index);\n gl_Position = uHolographicProjectionMatrix[index] * uHolographicViewMatrix[index] * uUndoMidViewMatrix * gl_Position;\n " +
                sourceString.substr(index2);
            sourceString = modifiedSourceString;
        }
        else
        {
            // Update the pixel shader to accept the render target array index as pass-through. This
            // allows the ANGLE compiler to avoid throwing it away, if it thinks it is not being used.
            if (!isGlslesVersion3)
            {
                // Translate the shader to GLSL version 300 syntax.
                std::string modifiedSourceString = 
                    std::string("#version 300 es\n ") + 
                    "precision mediump float;\n " +
                    sourceString;
                size_t varyingPos = 0;
                while ((varyingPos = modifiedSourceString.find("varying")) != std::string::npos)
                {
                    const char* outString = "in ";
                    modifiedSourceString.replace(varyingPos, size_t(7), outString);
                }
                while ((varyingPos = modifiedSourceString.find("gl_FragColor")) != std::string::npos)
                {
                    const char* inString = "fragColor ";
                    modifiedSourceString.replace(varyingPos, size_t(12), inString);
                }
                while ((varyingPos = modifiedSourceString.find("texture2D")) != std::string::npos)
                {
                    const char* inString = "texture";
                    modifiedSourceString.replace(varyingPos, size_t(9), inString);
                }
                while ((varyingPos = modifiedSourceString.find("textureCube")) != std::string::npos)
                {
                    const char* inString = "texture";
                    modifiedSourceString.replace(varyingPos, size_t(11), inString);
                }
                sourceString = modifiedSourceString;
            }

            size_t index1 = sourceString.find("void");
            size_t index2 = sourceString.find_last_of('}');
            std::string modifiedSourceString = 
                sourceString.substr(0, index1) + 
                (!isGlslesVersion3 ? "out vec4 fragColor;\n " : "") +
                "in float vRenderTargetArrayIndex;\n " +
                sourceString.substr(index1, index2-index1) +
                "    float index = vRenderTargetArrayIndex;\n " +
                sourceString.substr(index2);
            sourceString = modifiedSourceString;
        }
    }
#endif

    sourceCStrings.push_back(sourceString.c_str());

    bool result =
        ShCompile(compilerHandle, &sourceCStrings[0], sourceCStrings.size(), compileOptions);

    if (!result)
    {
        mInfoLog = ShGetInfoLog(compilerHandle);
        TRACE("\n%s", mInfoLog.c_str());
        mCompiled = false;
        return;
    }

    mData.mTranslatedSource = ShGetObjectCode(compilerHandle);

#ifndef NDEBUG
    // Prefix translated shader with commented out un-translated shader.
    // Useful in diagnostics tools which capture the shader source.
    std::ostringstream shaderStream;
    shaderStream << "// GLSL\n";
    shaderStream << "//\n";

    size_t curPos = 0;
    while (curPos != std::string::npos)
    {
        size_t nextLine = mData.mSource.find("\n", curPos);
        size_t len      = (nextLine == std::string::npos) ? std::string::npos : (nextLine - curPos + 1);

        shaderStream << "// " << mData.mSource.substr(curPos, len);

        curPos = (nextLine == std::string::npos) ? std::string::npos : (nextLine + 1);
    }
    shaderStream << "\n\n";
    shaderStream << mData.mTranslatedSource;
    mData.mTranslatedSource = shaderStream.str();
#endif

    // Gather the shader information
    mData.mShaderVersion = ShGetShaderVersion(compilerHandle);

    mData.mVaryings        = GetShaderVariables(ShGetVaryings(compilerHandle));
    mData.mUniforms        = GetShaderVariables(ShGetUniforms(compilerHandle));
    mData.mInterfaceBlocks = GetShaderVariables(ShGetInterfaceBlocks(compilerHandle));

    if (mData.mShaderType == GL_VERTEX_SHADER)
    {
        mData.mActiveAttributes = GetActiveShaderVariables(ShGetAttributes(compilerHandle));
    }
    else
    {
        ASSERT(mData.mShaderType == GL_FRAGMENT_SHADER);

        // TODO(jmadill): Figure out why we only sort in the FS, and if we need to.
        std::sort(mData.mVaryings.begin(), mData.mVaryings.end(), CompareShaderVar);
        mData.mActiveOutputVariables =
            GetActiveShaderVariables(ShGetOutputVariables(compilerHandle));
    }

    ASSERT(!mData.mTranslatedSource.empty());

    mCompiled = mImplementation->postTranslateCompile(compiler, &mInfoLog);
}

void Shader::addRef()
{
    mRefCount++;
}

void Shader::release()
{
    mRefCount--;

    if (mRefCount == 0 && mDeleteStatus)
    {
        mResourceManager->deleteShader(mHandle);
    }
}

unsigned int Shader::getRefCount() const
{
    return mRefCount;
}

bool Shader::isFlaggedForDeletion() const
{
    return mDeleteStatus;
}

void Shader::flagForDeletion()
{
    mDeleteStatus = true;
}

int Shader::getShaderVersion() const
{
    return mData.mShaderVersion;
}

const std::vector<sh::Varying> &Shader::getVaryings() const
{
    return mData.getVaryings();
}

const std::vector<sh::Uniform> &Shader::getUniforms() const
{
    return mData.getUniforms();
}

const std::vector<sh::InterfaceBlock> &Shader::getInterfaceBlocks() const
{
    return mData.getInterfaceBlocks();
}

const std::vector<sh::Attribute> &Shader::getActiveAttributes() const
{
    return mData.getActiveAttributes();
}

const std::vector<sh::OutputVariable> &Shader::getActiveOutputVariables() const
{
    return mData.getActiveOutputVariables();
}

int Shader::getSemanticIndex(const std::string &attributeName) const
{
    if (!attributeName.empty())
    {
        const auto &activeAttributes = mData.getActiveAttributes();

        int semanticIndex = 0;
        for (size_t attributeIndex = 0; attributeIndex < activeAttributes.size(); attributeIndex++)
        {
            const sh::ShaderVariable &attribute = activeAttributes[attributeIndex];

            if (attribute.name == attributeName)
            {
                return semanticIndex;
            }

            semanticIndex += gl::VariableRegisterCount(attribute.type);
        }
    }

    return -1;
}

}
