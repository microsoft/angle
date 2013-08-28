#include "Shader.h"
#include <assert.h>
#include "common/debug.h"
#include "libGLESv2/utilities.h"

using namespace gl;

Shader::Shader(const char *hlsl)
{
    mHlsl = new char[strlen(hlsl) + 1];
    strcpy(mHlsl, hlsl);
}

Shader::~Shader()
{
    delete [] mHlsl;
}

void Shader::parseUniforms(void *uniformBlob)
{
    mActiveUniforms = *(sh::ActiveUniforms*)uniformBlob;
}

void Shader::parseVaryings()
{
    if (mHlsl)
    {
        const char *input = strstr(mHlsl, "// Varyings") + 12;

        while(true)
        {
            char varyingType[256];
            char varyingName[256];

            int matches = sscanf(input, "static %255s %255s", varyingType, varyingName);

            if (matches != 2)
            {
                break;
            }

            char *array = strstr(varyingName, "[");
            int size = 1;

            if (array)
            {
                size = atoi(array + 1);
                *array = '\0';
            }

            mVaryings.push_back(Varying(parseType(varyingType), varyingName, size, array != NULL));

            input = strstr(input, ";") + 2;
        }

        mUsesMultipleRenderTargets = strstr(mHlsl, "GL_USES_MRT") != NULL;
        mUsesFragColor = strstr(mHlsl, "GL_USES_FRAG_COLOR") != NULL;
        mUsesFragData = strstr(mHlsl, "GL_USES_FRAG_DATA") != NULL;
        mUsesFragCoord = strstr(mHlsl, "GL_USES_FRAG_COORD") != NULL;
        mUsesFrontFacing = strstr(mHlsl, "GL_USES_FRONT_FACING") != NULL;
        mUsesPointSize = strstr(mHlsl, "GL_USES_POINT_SIZE") != NULL;
        mUsesPointCoord = strstr(mHlsl, "GL_USES_POINT_COORD") != NULL;
        mUsesDepthRange = strstr(mHlsl, "GL_USES_DEPTH_RANGE") != NULL;
        mUsesFragDepth = strstr(mHlsl, "GL_USES_FRAG_DEPTH") != NULL;
    }
}

void Shader::parseAttributes()
{
    const char *hlsl = mHlsl;
    if (hlsl)
    {
        const char *input = strstr(hlsl, "// Attributes") + 14;

        while(true)
        {
            char attributeType[256];
            char attributeName[256];

            int matches = sscanf(input, "static %255s _%255s", attributeType, attributeName);

            if (matches != 2)
            {
                break;
            }

            mAttributes.push_back(Attribute(parseType(attributeType), attributeName));

            input = strstr(input, ";") + 2;
        }
    }
}

int Shader::getSemanticIndex(const std::string &attributeName)
{
    if (!attributeName.empty())
    {
        int semanticIndex = 0;
        for (AttributeArray::iterator attribute = mAttributes.begin(); attribute != mAttributes.end(); attribute++)
        {
            if (attribute->name == attributeName)
            {
                return semanticIndex;
            }

            semanticIndex += VariableRowCount(attribute->type);
        }
    }

    return -1;
}

GLenum Shader::parseType(const std::string &type)
{
    if (type == "float")
    {
        return GL_FLOAT;
    }
    else if (type == "float2")
    {
        return GL_FLOAT_VEC2;
    }
    else if (type == "float3")
    {
        return GL_FLOAT_VEC3;
    }
    else if (type == "float4")
    {
        return GL_FLOAT_VEC4;
    }
    else if (type == "float2x2")
    {
        return GL_FLOAT_MAT2;
    }
    else if (type == "float3x3")
    {
        return GL_FLOAT_MAT3;
    }
    else if (type == "float4x4")
    {
        return GL_FLOAT_MAT4;
    }
    else assert(0 && "invalid glsl type");

    return GL_NONE;
}

bool Shader::compareVarying(const Varying &x, const Varying &y)
{
    if(x.type == y.type)
    {
        return x.size > y.size;
    }

    switch (x.type)
    {
      case GL_FLOAT_MAT4: return true;
      case GL_FLOAT_MAT2:
        switch(y.type)
        {
          case GL_FLOAT_MAT4: return false;
          case GL_FLOAT_MAT2: return true;
          case GL_FLOAT_VEC4: return true;
          case GL_FLOAT_MAT3: return true;
          case GL_FLOAT_VEC3: return true;
          case GL_FLOAT_VEC2: return true;
          case GL_FLOAT:      return true;
          default: UNREACHABLE();
        }
        break;
      case GL_FLOAT_VEC4:
        switch(y.type)
        {
          case GL_FLOAT_MAT4: return false;
          case GL_FLOAT_MAT2: return false;
          case GL_FLOAT_VEC4: return true;
          case GL_FLOAT_MAT3: return true;
          case GL_FLOAT_VEC3: return true;
          case GL_FLOAT_VEC2: return true;
          case GL_FLOAT:      return true;
          default: UNREACHABLE();
        }
        break;
      case GL_FLOAT_MAT3:
        switch(y.type)
        {
          case GL_FLOAT_MAT4: return false;
          case GL_FLOAT_MAT2: return false;
          case GL_FLOAT_VEC4: return false;
          case GL_FLOAT_MAT3: return true;
          case GL_FLOAT_VEC3: return true;
          case GL_FLOAT_VEC2: return true;
          case GL_FLOAT:      return true;
          default: UNREACHABLE();
        }
        break;
      case GL_FLOAT_VEC3:
        switch(y.type)
        {
          case GL_FLOAT_MAT4: return false;
          case GL_FLOAT_MAT2: return false;
          case GL_FLOAT_VEC4: return false;
          case GL_FLOAT_MAT3: return false;
          case GL_FLOAT_VEC3: return true;
          case GL_FLOAT_VEC2: return true;
          case GL_FLOAT:      return true;
          default: UNREACHABLE();
        }
        break;
      case GL_FLOAT_VEC2:
        switch(y.type)
        {
          case GL_FLOAT_MAT4: return false;
          case GL_FLOAT_MAT2: return false;
          case GL_FLOAT_VEC4: return false;
          case GL_FLOAT_MAT3: return false;
          case GL_FLOAT_VEC3: return false;
          case GL_FLOAT_VEC2: return true;
          case GL_FLOAT:      return true;
          default: UNREACHABLE();
        }
        break;
      case GL_FLOAT: return false;
      default: UNREACHABLE();
    }

    return false;
}

void Shader::sortVaryings()
{
    mVaryings.sort(compareVarying);
}

const char *Shader::getHlsl() const
{
	return mHlsl;
}

void Shader::resetVaryingsRegisterAssignment()
{
    for (VaryingList::iterator var = mVaryings.begin(); var != mVaryings.end(); var++)
    {
        var->reg = -1;
        var->col = -1;
    }
}

const sh::ActiveUniforms &Shader::getUniforms()
{
    return mActiveUniforms;
}
