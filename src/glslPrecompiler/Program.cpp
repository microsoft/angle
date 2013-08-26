#include "Program.h"
#include "libGLESv2/BinaryStream.h"
#include "libGLESv2/utilities.h"
#include "common/version.h"

#undef max
#undef min

#include <algorithm>

#if !defined(ANGLE_COMPILE_OPTIMIZATION_LEVEL)
#define ANGLE_COMPILE_OPTIMIZATION_LEVEL D3DCOMPILE_OPTIMIZATION_LEVEL3
#endif

using namespace gl;

const char *g_fakepath = "C:\\";

static std::string str(int i)
{
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%d", i);
    return buffer;
}

// This method needs to match OutputHLSL::decorate
static std::string decorateAttribute(const std::string &name)
{
    if (name.compare(0, 3, "gl_") != 0 && name.compare(0, 3, "dx_") != 0)
    {
        return "_" + name;
    }
    
    return name;
}

AttributeBindings::AttributeBindings()
{
}

AttributeBindings::~AttributeBindings()
{
}

void AttributeBindings::bindAttributeLocation(GLuint index, const char *name)
{
    if (index < MAX_VERTEX_ATTRIBS)
    {
        for (int i = 0; i < MAX_VERTEX_ATTRIBS; i++)
        {
            mAttributeBinding[i].erase(name);
        }

        mAttributeBinding[index].insert(name);
    }
}

int AttributeBindings::getAttributeBinding(const std::string &name) const
{
    for (int location = 0; location < MAX_VERTEX_ATTRIBS; location++)
    {
        if (mAttributeBinding[location].find(name) != mAttributeBinding[location].end())
        {
            return location;
        }
    }

    return -1;
}

InfoLog::InfoLog() : mInfoLog(NULL)
{
}

InfoLog::~InfoLog()
{
    delete[] mInfoLog;
}

int InfoLog::getLength() const
{
    if (!mInfoLog)
    {
        return 0;
    }
    else
    {
       return strlen(mInfoLog) + 1;
    }
}

void InfoLog::getLog(GLsizei bufSize, GLsizei *length, char *infoLog)
{
    int index = 0;

    if (bufSize > 0)
    {
        if (mInfoLog)
        {
            index = std::min(bufSize - 1, (int)strlen(mInfoLog));
            memcpy(infoLog, mInfoLog, index);
        }

        infoLog[index] = '\0';
    }

    if (length)
    {
        *length = index;
    }
}

// append a santized message to the program info log.
// The D3D compiler includes a fake file path in some of the warning or error 
// messages, so lets remove all occurrences of this fake file path from the log.
void InfoLog::appendSanitized(const char *message)
{
    std::string msg(message);

    size_t found;
    do
    {
        found = msg.find(g_fakepath);
        if (found != std::string::npos)
        {
            msg.erase(found, strlen(g_fakepath));
        }
    }
    while (found != std::string::npos);

    append("%s", msg.c_str());
}

void InfoLog::append(const char *format, ...)
{
    if (!format)
    {
        return;
    }

    char info[1024];

    va_list vararg;
    va_start(vararg, format);
    vsnprintf(info, sizeof(info), format, vararg);
    va_end(vararg);

    size_t infoLength = strlen(info);

    if (!mInfoLog)
    {
        mInfoLog = new char[infoLength + 2];
        strcpy(mInfoLog, info);
        strcpy(mInfoLog + infoLength, "\n");
    }
    else
    {
        size_t logLength = strlen(mInfoLog);
        char *newLog = new char[logLength + infoLength + 2];
        strcpy(newLog, mInfoLog);
        strcpy(newLog + logLength, info);
        strcpy(newLog + logLength + infoLength, "\n");

        delete[] mInfoLog;
        mInfoLog = newLog;
    }
}

void InfoLog::reset()
{
    if (mInfoLog)
    {
        delete [] mInfoLog;
        mInfoLog = NULL;
    }
}

struct AttributeSorter
{
    AttributeSorter(const int (&semanticIndices)[MAX_VERTEX_ATTRIBS])
        : originalIndices(semanticIndices)
    {
    }

    bool operator()(int a, int b)
    {
        return originalIndices[a] == -1 ? false : originalIndices[a] < originalIndices[b];
    }

    const int (&originalIndices)[MAX_VERTEX_ATTRIBS];
};

UniformLocation::UniformLocation(const std::string &name, unsigned int element, unsigned int index) 
    : name(name), element(element), index(index)
{
}

Program::Program(Shader *vertexShader, Shader *fragmentShader)
:   mVertexShader(vertexShader),
    mFragmentShader(fragmentShader),
    mUsedVertexSamplerRange(0),
    mUsedPixelSamplerRange(0),
    mUsesPointSize(false)
{
    for (int index = 0; index < MAX_VERTEX_ATTRIBS; index++)
    {
        mSemanticIndex[index] = -1;
    }

    for (int index = 0; index < MAX_TEXTURE_IMAGE_UNITS; index++)
    {
        mSamplersPS[index].active = false;
    }

    for (int index = 0; index < IMPLEMENTATION_MAX_VERTEX_TEXTURE_IMAGE_UNITS; index++)
    {
        mSamplersVS[index].active = false;
    }
}

bool Program::link()
{
	std::string pixelHLSL = mFragmentShader->getHlsl();
	std::string vertexHLSL = mVertexShader->getHlsl();
	const Varying *packing[IMPLEMENTATION_MAX_VARYING_VECTORS][4] = {NULL};
	InfoLog infoLog;
    int registers = packVaryings(infoLog, packing);
	if (registers < 0)
    {
        return false;
    }

    if (!linkVaryings(infoLog, registers, packing, pixelHLSL, vertexHLSL, mFragmentShader, mVertexShader))
    {
        return false;
    }

    bool success = true;

    if (!linkAttributes(infoLog, mAttributeBindings, mFragmentShader, mVertexShader))
    {
        success = false;
    }

    if (!linkUniforms(infoLog, mVertexShader->getUniforms(), mFragmentShader->getUniforms()))
    {
        success = false;
    }

    // special case for gl_DepthRange, the only built-in uniform (also a struct)
    if (mVertexShader->mUsesDepthRange || mFragmentShader->mUsesDepthRange)
    {
        mUniforms.push_back(new Uniform(GL_FLOAT, GL_HIGH_FLOAT, "gl_DepthRange.near", 0));
        mUniforms.push_back(new Uniform(GL_FLOAT, GL_HIGH_FLOAT, "gl_DepthRange.far", 0));
        mUniforms.push_back(new Uniform(GL_FLOAT, GL_HIGH_FLOAT, "gl_DepthRange.diff", 0));
    }

    if (success)
    {
		HRESULT result;
		ID3DBlob *errorMessage;

		result = D3DCompile(vertexHLSL.c_str(),
							vertexHLSL.size(),
							"C:\\",
							NULL,
							NULL,
							"main",
							"vs_4_0_level_9_1",
							D3DCOMPILE_OPTIMIZATION_LEVEL3,
							0, 
							&mVertexShaderCompiled,
							&errorMessage);
		if(FAILED(result))
			logErrorMessage(infoLog, errorMessage);

		result = D3DCompile(pixelHLSL.c_str(),
							pixelHLSL.size(),
							"C:\\",
							NULL,
							NULL,
							"main",
							"ps_4_0_level_9_1",
							D3DCOMPILE_OPTIMIZATION_LEVEL3,
							0, 
							&mFragmentShaderCompiled,
							&errorMessage);
		if(FAILED(result))
			logErrorMessage(infoLog, errorMessage);

        if (!mVertexShaderCompiled || !mFragmentShaderCompiled)
        {
            infoLog.append("Failed to create D3D shaders.");
            success = false;
        }
    }

    return success;
}

bool Program::save(const char *file)
{
    BinaryOutputStream stream;

    stream.write(GL_PROGRAM_BINARY_ANGLE);
    stream.write(VERSION_DWORD);
    stream.write(ANGLE_COMPILE_OPTIMIZATION_LEVEL);

    for (unsigned int i = 0; i < MAX_VERTEX_ATTRIBS; ++i)
    {
        stream.write(mLinkedAttribute[i].type);
        stream.write(mLinkedAttribute[i].name);
        stream.write(mSemanticIndex[i]);
    }

    for (unsigned int i = 0; i < MAX_TEXTURE_IMAGE_UNITS; ++i)
    {
        stream.write(mSamplersPS[i].active);
        stream.write(mSamplersPS[i].logicalTextureUnit);
        stream.write((int) mSamplersPS[i].textureType);
    }

    for (unsigned int i = 0; i < IMPLEMENTATION_MAX_VERTEX_TEXTURE_IMAGE_UNITS; ++i)
    {
        stream.write(mSamplersVS[i].active);
        stream.write(mSamplersVS[i].logicalTextureUnit);
        stream.write((int) mSamplersVS[i].textureType);
    }

    stream.write(mUsedVertexSamplerRange);
    stream.write(mUsedPixelSamplerRange);
    stream.write(mUsesPointSize);

    stream.write(mUniforms.size());
    for (unsigned int i = 0; i < mUniforms.size(); ++i)
    {
        stream.write(mUniforms[i]->type);
        stream.write(mUniforms[i]->precision);
        stream.write(mUniforms[i]->name);
        stream.write(mUniforms[i]->arraySize);

        stream.write(mUniforms[i]->psRegisterIndex);
        stream.write(mUniforms[i]->vsRegisterIndex);
        stream.write(mUniforms[i]->registerCount);
    }

    stream.write(mUniformIndex.size());
    for (unsigned int i = 0; i < mUniformIndex.size(); ++i)
    {
        stream.write(mUniformIndex[i].name);
        stream.write(mUniformIndex[i].element);
        stream.write(mUniformIndex[i].index);
    }

    UINT pixelShaderSize = mFragmentShaderCompiled->GetBufferSize();
    stream.write(pixelShaderSize);

    UINT vertexShaderSize = mVertexShaderCompiled->GetBufferSize();
    stream.write(vertexShaderSize);

    UINT geometryShaderSize = 0;
    stream.write(geometryShaderSize);

    GUID identifier = {0};//mRenderer->getAdapterIdentifier();

    GLsizei streamLength = stream.length();
    const void *streamData = stream.data();

    //GLsizei totalLength = streamLength + sizeof(GUID) + pixelShaderSize + vertexShaderSize + geometryShaderSize;
    //if (totalLength > bufSize)
    //{
    //    if (length)
    //    {
    //        *length = 0;
    //    }

    //    return false;
    //}

    //if (binary)
    //{
    //    char *ptr = (char*) binary;

    //    memcpy(ptr, streamData, streamLength);
    //    ptr += streamLength;

    //    memcpy(ptr, &identifier, sizeof(GUID));
    //    ptr += sizeof(GUID);

    //    memcpy(ptr, mFragmentShaderCompiled->GetBufferPointer(), pixelShaderSize);
    //    ptr += pixelShaderSize;

    //    memcpy(ptr, mVertexShaderCompiled->GetBufferPointer(), vertexShaderSize);
    //    ptr += vertexShaderSize;

    //    ASSERT(ptr - totalLength == binary);
    //}
    FILE *fp = fopen(file, "wb");
    fwrite(streamData, streamLength, 1, fp);
    fwrite(&identifier, sizeof(identifier), 1, fp);
    fwrite(mFragmentShaderCompiled->GetBufferPointer(), mFragmentShaderCompiled->GetBufferSize(), 1, fp);
    fwrite(mVertexShaderCompiled->GetBufferPointer(), mVertexShaderCompiled->GetBufferSize(), 1, fp);
    fclose(fp);

    //if (length)
    //{
    //    *length = totalLength;
    //}

    return true;
}

// Packs varyings into generic varying registers, using the algorithm from [OpenGL ES Shading Language 1.00 rev. 17] appendix A section 7 page 111
// Returns the number of used varying registers, or -1 if unsuccesful
int Program::packVaryings(InfoLog &infoLog, const Varying *packing[][4])
{
    const int maxVaryingVectors = D3D10_VS_OUTPUT_REGISTER_COUNT;//mRenderer->getMaxVaryingVectors();

    mFragmentShader->resetVaryingsRegisterAssignment();

    for (VaryingList::iterator varying = mFragmentShader->mVaryings.begin(); varying != mFragmentShader->mVaryings.end(); varying++)
    {
        int n = VariableRowCount(varying->type) * varying->size;
        int m = VariableColumnCount(varying->type);
        bool success = false;

        if (m == 2 || m == 3 || m == 4)
        {
            for (int r = 0; r <= maxVaryingVectors - n && !success; r++)
            {
                bool available = true;

                for (int y = 0; y < n && available; y++)
                {
                    for (int x = 0; x < m && available; x++)
                    {
                        if (packing[r + y][x])
                        {
                            available = false;
                        }
                    }
                }

                if (available)
                {
                    varying->reg = r;
                    varying->col = 0;

                    for (int y = 0; y < n; y++)
                    {
                        for (int x = 0; x < m; x++)
                        {
                            packing[r + y][x] = &*varying;
                        }
                    }

                    success = true;
                }
            }

            if (!success && m == 2)
            {
                for (int r = maxVaryingVectors - n; r >= 0 && !success; r--)
                {
                    bool available = true;

                    for (int y = 0; y < n && available; y++)
                    {
                        for (int x = 2; x < 4 && available; x++)
                        {
                            if (packing[r + y][x])
                            {
                                available = false;
                            }
                        }
                    }

                    if (available)
                    {
                        varying->reg = r;
                        varying->col = 2;

                        for (int y = 0; y < n; y++)
                        {
                            for (int x = 2; x < 4; x++)
                            {
                                packing[r + y][x] = &*varying;
                            }
                        }

                        success = true;
                    }
                }
            }
        }
        else if (m == 1)
        {
            int space[4] = {0};

            for (int y = 0; y < maxVaryingVectors; y++)
            {
                for (int x = 0; x < 4; x++)
                {
                    space[x] += packing[y][x] ? 0 : 1;
                }
            }

            int column = 0;

            for (int x = 0; x < 4; x++)
            {
                if (space[x] >= n && space[x] < space[column])
                {
                    column = x;
                }
            }

            if (space[column] >= n)
            {
                for (int r = 0; r < maxVaryingVectors; r++)
                {
                    if (!packing[r][column])
                    {
                        varying->reg = r;

                        for (int y = r; y < r + n; y++)
                        {
                            packing[y][column] = &*varying;
                        }

                        break;
                    }
                }

                varying->col = column;

                success = true;
            }
        }
        else UNREACHABLE();

        if (!success)
        {
            infoLog.append("Could not pack varying %s", varying->name.c_str());

            return -1;
        }
    }

    // Return the number of used registers
    int registers = 0;

    for (int r = 0; r < maxVaryingVectors; r++)
    {
        if (packing[r][0] || packing[r][1] || packing[r][2] || packing[r][3])
        {
            registers++;
        }
    }

    return registers;
}

bool Program::linkVaryings(InfoLog &infoLog, int registers, const Varying *packing[][4],
                                 std::string& pixelHLSL, std::string& vertexHLSL,
								 Shader *fragmentShader, Shader *vertexShader)
{
    if (pixelHLSL.empty() || vertexHLSL.empty())
    {
        return false;
    }

#if defined(ANGLE_PLATFORM_WINRT)
    bool isWinRT = true;
#else
    bool isWinRT = false;
#endif // ANGLE_PLATFORM_WINRT

    bool usesMRT = fragmentShader->mUsesMultipleRenderTargets;
    bool usesFragColor = fragmentShader->mUsesFragColor;
    bool usesFragData = fragmentShader->mUsesFragData;
    if (usesFragColor && usesFragData)
    {
        infoLog.append("Cannot use both gl_FragColor and gl_FragData in the same fragment shader.");
        return false;
    }

    // Write the HLSL input/output declarations
    const int shaderModel = 2;//mRenderer->getMajorShaderModel();
    const int maxVaryingVectors = D3D10_VS_OUTPUT_REGISTER_COUNT;//mRenderer->getMaxVaryingVectors();

    const int registersNeeded = registers + (fragmentShader->mUsesFragCoord ? 1 : 0) + (fragmentShader->mUsesPointCoord ? 1 : 0);

    // The output color is broadcast to all enabled draw buffers when writing to gl_FragColor 
    const bool broadcast = fragmentShader->mUsesFragColor;
    const unsigned int numRenderTargets = (broadcast || usesMRT ? D3D_FL9_1_SIMULTANEOUS_RENDER_TARGET_COUNT : 1);

    if (registersNeeded > maxVaryingVectors)
    {
        infoLog.append("No varying registers left to support gl_FragCoord/gl_PointCoord");

        return false;
    }

    vertexShader->resetVaryingsRegisterAssignment();

    for (VaryingList::iterator input = fragmentShader->mVaryings.begin(); input != fragmentShader->mVaryings.end(); input++)
    {
        bool matched = false;

        for (VaryingList::iterator output = vertexShader->mVaryings.begin(); output != vertexShader->mVaryings.end(); output++)
        {
            if (output->name == input->name)
            {
                if (output->type != input->type || output->size != input->size)
                {
                    infoLog.append("Type of vertex varying %s does not match that of the fragment varying", output->name.c_str());

                    return false;
                }

                output->reg = input->reg;
                output->col = input->col;

                matched = true;
                break;
            }
        }

        if (!matched)
        {
            infoLog.append("Fragment varying %s does not match any vertex varying", input->name.c_str());

            return false;
        }
    }

    mUsesPointSize = vertexShader->mUsesPointSize;
    std::string varyingSemantic = /*(mUsesPointSize && shaderModel == 3) ? "COLOR" : */"TEXCOORD";
    std::string targetSemantic = /*(shaderModel >= 4 || isWinRT) ? */"SV_Target"/* : "COLOR"*/;
    std::string positionSemantic = /*(shaderModel >= 4 || isWinRT) ? */"SV_Position"/* : "POSITION"*/;
    std::string depthSemantic = /*(shaderModel >= 4 || isWinRT) ? */"SV_Depth"/* : "DEPTH"*/;

    // special varyings that use reserved registers
    int reservedRegisterIndex = registers;
    std::string fragCoordSemantic;
    std::string pointCoordSemantic;

    if (fragmentShader->mUsesFragCoord)
    {
        fragCoordSemantic = varyingSemantic + str(reservedRegisterIndex++);
    }

    if (fragmentShader->mUsesPointCoord)
    {
        // Shader model 3 uses a special TEXCOORD semantic for point sprite texcoords.
        // In DX11 we compute this in the GS.
        if (shaderModel == 3)
        {
            pointCoordSemantic = "TEXCOORD0";
        }
        else if (shaderModel >= 4)
        {
            pointCoordSemantic = varyingSemantic + str(reservedRegisterIndex++); 
        }
    }

    vertexHLSL += "struct VS_INPUT\n"
                  "{\n";

    int semanticIndex = 0;
    for (AttributeArray::iterator attribute = vertexShader->mAttributes.begin(); attribute != vertexShader->mAttributes.end(); attribute++)
    {
        switch (attribute->type)
        {
          case GL_FLOAT:      vertexHLSL += "    float ";    break;
          case GL_FLOAT_VEC2: vertexHLSL += "    float2 ";   break;
          case GL_FLOAT_VEC3: vertexHLSL += "    float3 ";   break;
          case GL_FLOAT_VEC4: vertexHLSL += "    float4 ";   break;
          case GL_FLOAT_MAT2: vertexHLSL += "    float2x2 "; break;
          case GL_FLOAT_MAT3: vertexHLSL += "    float3x3 "; break;
          case GL_FLOAT_MAT4: vertexHLSL += "    float4x4 "; break;
          default:  UNREACHABLE();
        }

        vertexHLSL += decorateAttribute(attribute->name) + " : TEXCOORD" + str(semanticIndex) + ";\n";

        semanticIndex += VariableRowCount(attribute->type);
    }

    vertexHLSL += "};\n"
                  "\n"
                  "struct VS_OUTPUT\n"
                  "{\n";

    if (shaderModel < 4 && !isWinRT)
    {
        vertexHLSL += "    float4 gl_Position : " + positionSemantic + ";\n";
    }

    for (int r = 0; r < registers; r++)
    {
        int registerSize = packing[r][3] ? 4 : (packing[r][2] ? 3 : (packing[r][1] ? 2 : 1));

        vertexHLSL += "    float" + str(registerSize) + " v" + str(r) + " : " + varyingSemantic + str(r) + ";\n";
    }

    if (fragmentShader->mUsesFragCoord)
    {
        vertexHLSL += "    float4 gl_FragCoord : " + fragCoordSemantic + ";\n";
    }

    if (vertexShader->mUsesPointSize && shaderModel >= 3)
    {
        vertexHLSL += "    float gl_PointSize : PSIZE;\n";
    }

    if (shaderModel >= 4 || isWinRT)
    {
        vertexHLSL += "    float4 gl_Position : " + positionSemantic + ";\n";
    }

    vertexHLSL += "};\n"
                  "\n"
                  "VS_OUTPUT main(VS_INPUT input)\n"
                  "{\n";

    for (AttributeArray::iterator attribute = vertexShader->mAttributes.begin(); attribute != vertexShader->mAttributes.end(); attribute++)
    {
        vertexHLSL += "    " + decorateAttribute(attribute->name) + " = ";

        if (VariableRowCount(attribute->type) > 1)   // Matrix
        {
            vertexHLSL += "transpose";
        }

        vertexHLSL += "(input." + decorateAttribute(attribute->name) + ");\n";
    }

    //if (shaderModel >= 4 || isWinRT)
    {
        vertexHLSL += "\n"
                      "    gl_main();\n"
                      "\n"
                      "    VS_OUTPUT output;\n"
                      "    output.gl_Position.x = gl_Position.x;\n"
                      "    output.gl_Position.y = -gl_Position.y;\n"
                      "    output.gl_Position.z = (gl_Position.z + gl_Position.w) * 0.5;\n"
                      "    output.gl_Position.w = gl_Position.w;\n";
    }
    //else
    //{
    //    vertexHLSL += "\n"
    //                  "    gl_main();\n"
    //                  "\n"
    //                  "    VS_OUTPUT output;\n"
    //                  "    output.gl_Position.x = gl_Position.x * dx_ViewAdjust.z + dx_ViewAdjust.x * gl_Position.w;\n"
    //                  "    output.gl_Position.y = -(gl_Position.y * dx_ViewAdjust.w + dx_ViewAdjust.y * gl_Position.w);\n"
    //                  "    output.gl_Position.z = (gl_Position.z + gl_Position.w) * 0.5;\n"
    //                  "    output.gl_Position.w = gl_Position.w;\n";
    //}

    if (vertexShader->mUsesPointSize && shaderModel >= 3)
    {
        vertexHLSL += "    output.gl_PointSize = gl_PointSize;\n";
    }

    if (fragmentShader->mUsesFragCoord)
    {
        vertexHLSL += "    output.gl_FragCoord = gl_Position;\n";
    }

    for (VaryingList::iterator varying = vertexShader->mVaryings.begin(); varying != vertexShader->mVaryings.end(); varying++)
    {
        if (varying->reg >= 0)
        {
            for (int i = 0; i < varying->size; i++)
            {
                int rows = VariableRowCount(varying->type);

                for (int j = 0; j < rows; j++)
                {
                    int r = varying->reg + i * rows + j;
                    vertexHLSL += "    output.v" + str(r);

                    bool sharedRegister = false;   // Register used by multiple varyings
                    
                    for (int x = 0; x < 4; x++)
                    {
                        if (packing[r][x] && packing[r][x] != packing[r][0])
                        {
                            sharedRegister = true;
                            break;
                        }
                    }

                    if (sharedRegister)
                    {
                        vertexHLSL += ".";

                        for (int x = 0; x < 4; x++)
                        {
                            if (packing[r][x] == &*varying)
                            {
                                switch(x)
                                {
                                  case 0: vertexHLSL += "x"; break;
                                  case 1: vertexHLSL += "y"; break;
                                  case 2: vertexHLSL += "z"; break;
                                  case 3: vertexHLSL += "w"; break;
                                }
                            }
                        }
                    }

                    vertexHLSL += " = " + varying->name;
                    
                    if (varying->array)
                    {
                        vertexHLSL += "[" + str(i) + "]";
                    }

                    if (rows > 1)
                    {
                        vertexHLSL += "[" + str(j) + "]";
                    }
                    
                    vertexHLSL += ";\n";
                }
            }
        }
    }

    vertexHLSL += "\n"
                  "    return output;\n"
                  "}\n";

    pixelHLSL += "struct PS_INPUT\n"
                 "{\n";
    
    for (VaryingList::iterator varying = fragmentShader->mVaryings.begin(); varying != fragmentShader->mVaryings.end(); varying++)
    {
        if (varying->reg >= 0)
        {
            for (int i = 0; i < varying->size; i++)
            {
                int rows = VariableRowCount(varying->type);
                for (int j = 0; j < rows; j++)
                {
                    std::string n = str(varying->reg + i * rows + j);
                    pixelHLSL += "    float" + str(VariableColumnCount(varying->type)) + " v" + n + " : " + varyingSemantic + n + ";\n";
                }
            }
        }
        else UNREACHABLE();
    }

    if (fragmentShader->mUsesFragCoord)
    {
        pixelHLSL += "    float4 gl_FragCoord : " + fragCoordSemantic + ";\n";
    }
        
    if (fragmentShader->mUsesPointCoord && shaderModel >= 3)
    {
        pixelHLSL += "    float2 gl_PointCoord : " + pointCoordSemantic + ";\n";
    }

    // Must consume the PSIZE element if the geometry shader is not active
    // We won't know if we use a GS until we draw
    if (vertexShader->mUsesPointSize && shaderModel >= 4)
    {
        pixelHLSL += "    float gl_PointSize : PSIZE;\n";
    }

    if (fragmentShader->mUsesFragCoord)
    {
        if (shaderModel >= 4 || isWinRT)
        {
            pixelHLSL += "    float4 dx_VPos : SV_Position;\n";
        }
        else if (shaderModel >= 3)
        {
            pixelHLSL += "    float2 dx_VPos : VPOS;\n";
        }
    }

    pixelHLSL += "};\n"
                 "\n"
                 "struct PS_OUTPUT\n"
                 "{\n";

    for (unsigned int renderTargetIndex = 0; renderTargetIndex < numRenderTargets; renderTargetIndex++)
    {
        pixelHLSL += "    float4 gl_Color" + str(renderTargetIndex) + " : " + targetSemantic + str(renderTargetIndex) + ";\n";
    }

    if (fragmentShader->mUsesFragDepth)
    {
        pixelHLSL += "    float gl_Depth : " + depthSemantic + ";\n";
    }

    pixelHLSL += "};\n"
                 "\n";

    if (fragmentShader->mUsesFrontFacing)
    {
        if (shaderModel >= 4 || isWinRT)
        {
            pixelHLSL += "PS_OUTPUT main(PS_INPUT input, bool isFrontFace : SV_IsFrontFace)\n"
                         "{\n";
        }
        else
        {
            pixelHLSL += "PS_OUTPUT main(PS_INPUT input, float vFace : VFACE)\n"
                         "{\n";
        }
    }
    else
    {
        pixelHLSL += "PS_OUTPUT main(PS_INPUT input)\n"
                     "{\n";
    }

    if (fragmentShader->mUsesFragCoord)
    {
        pixelHLSL += "    float rhw = 1.0 / input.gl_FragCoord.w;\n";
        
        if (shaderModel >= 4 || isWinRT)
        {
            pixelHLSL += "    gl_FragCoord.x = input.dx_VPos.x;\n"
                         "    gl_FragCoord.y = input.dx_VPos.y;\n";
        }
        else if (shaderModel >= 3)
        {
            pixelHLSL += "    gl_FragCoord.x = input.dx_VPos.x + 0.5;\n"
                         "    gl_FragCoord.y = input.dx_VPos.y + 0.5;\n";
        }
        else
        {
            // dx_ViewCoords contains the viewport width/2, height/2, center.x and center.y. See Renderer::setViewport()
            pixelHLSL += "    gl_FragCoord.x = (input.gl_FragCoord.x * rhw) * dx_ViewCoords.x + dx_ViewCoords.z;\n"
                         "    gl_FragCoord.y = (input.gl_FragCoord.y * rhw) * dx_ViewCoords.y + dx_ViewCoords.w;\n";
        }
        
        pixelHLSL += "    gl_FragCoord.z = (input.gl_FragCoord.z * rhw) * dx_DepthFront.x + dx_DepthFront.y;\n"
                     "    gl_FragCoord.w = rhw;\n";
    }

    if (fragmentShader->mUsesPointCoord && shaderModel >= 3)
    {
        pixelHLSL += "    gl_PointCoord.x = input.gl_PointCoord.x;\n";
        pixelHLSL += "    gl_PointCoord.y = 1.0 - input.gl_PointCoord.y;\n";
    }

    if (fragmentShader->mUsesFrontFacing)
    {
        if (shaderModel <= 3 && !isWinRT)
        {
            pixelHLSL += "    gl_FrontFacing = (vFace * dx_DepthFront.z >= 0.0);\n";
        }
        else
        {
            pixelHLSL += "    gl_FrontFacing = isFrontFace;\n";
        }
    }

    for (VaryingList::iterator varying = fragmentShader->mVaryings.begin(); varying != fragmentShader->mVaryings.end(); varying++)
    {
        if (varying->reg >= 0)
        {
            for (int i = 0; i < varying->size; i++)
            {
                int rows = VariableRowCount(varying->type);
                for (int j = 0; j < rows; j++)
                {
                    std::string n = str(varying->reg + i * rows + j);
                    pixelHLSL += "    " + varying->name;

                    if (varying->array)
                    {
                        pixelHLSL += "[" + str(i) + "]";
                    }

                    if (rows > 1)
                    {
                        pixelHLSL += "[" + str(j) + "]";
                    }

                    switch (VariableColumnCount(varying->type))
                    {
                      case 1: pixelHLSL += " = input.v" + n + ".x;\n";   break;
                      case 2: pixelHLSL += " = input.v" + n + ".xy;\n";  break;
                      case 3: pixelHLSL += " = input.v" + n + ".xyz;\n"; break;
                      case 4: pixelHLSL += " = input.v" + n + ";\n";     break;
                      default: UNREACHABLE();
                    }
                }
            }
        }
        else UNREACHABLE();
    }

    pixelHLSL += "\n"
                 "    gl_main();\n"
                 "\n"
                 "    PS_OUTPUT output;\n";

    for (unsigned int renderTargetIndex = 0; renderTargetIndex < numRenderTargets; renderTargetIndex++)
    {
        unsigned int sourceColorIndex = broadcast ? 0 : renderTargetIndex;

        pixelHLSL += "    output.gl_Color" + str(renderTargetIndex) + " = gl_Color[" + str(sourceColorIndex) + "];\n";
    }

    if (fragmentShader->mUsesFragDepth)
    {
        pixelHLSL += "    output.gl_Depth = gl_Depth;\n";
    }

    pixelHLSL += "\n"
                 "    return output;\n"
                 "}\n";

    return true;
}

// Determines the mapping between GL attributes and Direct3D 9 vertex stream usage indices
bool Program::linkAttributes(InfoLog &infoLog, const AttributeBindings &attributeBindings, Shader *, Shader *vertexShader)
{
    unsigned int usedLocations = 0;

    // Link attributes that have a binding location
    for (AttributeArray::iterator attribute = vertexShader->mAttributes.begin(); attribute != vertexShader->mAttributes.end(); attribute++)
    {
        int location = attributeBindings.getAttributeBinding(attribute->name);

        if (location != -1)   // Set by glBindAttribLocation
        {
            if (!mLinkedAttribute[location].name.empty())
            {
                // Multiple active attributes bound to the same location; not an error
            }

            mLinkedAttribute[location] = *attribute;

            int rows = VariableRowCount(attribute->type);

            if (rows + location > MAX_VERTEX_ATTRIBS)
            {
                infoLog.append("Active attribute (%s) at location %d is too big to fit", attribute->name.c_str(), location);

                return false;
            }

            for (int i = 0; i < rows; i++)
            {
                usedLocations |= 1 << (location + i);
            }
        }
    }

    // Link attributes that don't have a binding location
    for (AttributeArray::iterator attribute = vertexShader->mAttributes.begin(); attribute != vertexShader->mAttributes.end(); attribute++)
    {
        int location = attributeBindings.getAttributeBinding(attribute->name);

        if (location == -1)   // Not set by glBindAttribLocation
        {
            int rows = VariableRowCount(attribute->type);
            int availableIndex = AllocateFirstFreeBits(&usedLocations, rows, MAX_VERTEX_ATTRIBS);

            if (availableIndex == -1 || availableIndex + rows > MAX_VERTEX_ATTRIBS)
            {
                infoLog.append("Too many active attributes (%s)", attribute->name.c_str());

                return false;   // Fail to link
            }

            mLinkedAttribute[availableIndex] = *attribute;
        }
    }

    for (int attributeIndex = 0; attributeIndex < MAX_VERTEX_ATTRIBS; )
    {
        int index = vertexShader->getSemanticIndex(mLinkedAttribute[attributeIndex].name);
        int rows = std::max(VariableRowCount(mLinkedAttribute[attributeIndex].type), 1);

        for (int r = 0; r < rows; r++)
        {
            mSemanticIndex[attributeIndex++] = index++;
        }
    }

    initAttributesByLayout();

    return true;
}

bool Program::linkUniforms(InfoLog &infoLog, const sh::ActiveUniforms &vertexUniforms, const sh::ActiveUniforms &fragmentUniforms)
{
    for (sh::ActiveUniforms::const_iterator uniform = vertexUniforms.begin(); uniform != vertexUniforms.end(); uniform++)
    {
        if (!defineUniform(GL_VERTEX_SHADER, *uniform, infoLog))
        {
            return false;
        }
    }

    for (sh::ActiveUniforms::const_iterator uniform = fragmentUniforms.begin(); uniform != fragmentUniforms.end(); uniform++)
    {
        if (!defineUniform(GL_FRAGMENT_SHADER, *uniform, infoLog))
        {
            return false;
        }
    }

    return true;
}

bool Program::defineUniform(GLenum shader, const sh::Uniform &constant, InfoLog &infoLog)
{
    if (constant.type == GL_SAMPLER_2D ||
        constant.type == GL_SAMPLER_CUBE)
    {
        unsigned int samplerIndex = constant.registerIndex;
            
        do
        {
            if (shader == GL_VERTEX_SHADER)
            {
                if (samplerIndex < 16)
                {
                    mSamplersVS[samplerIndex].active = true;
                    mSamplersVS[samplerIndex].textureType = (constant.type == GL_SAMPLER_CUBE) ? TEXTURE_CUBE : TEXTURE_2D;
                    mSamplersVS[samplerIndex].logicalTextureUnit = 0;
                    mUsedVertexSamplerRange = std::max(samplerIndex + 1, mUsedVertexSamplerRange);
                }
                else
                {
                    infoLog.append("Vertex shader sampler count exceeds the maximum vertex texture units (%d).", 16);
                    return false;
                }
            }
            else if (shader == GL_FRAGMENT_SHADER)
            {
                if (samplerIndex < MAX_TEXTURE_IMAGE_UNITS)
                {
                    mSamplersPS[samplerIndex].active = true;
                    mSamplersPS[samplerIndex].textureType = (constant.type == GL_SAMPLER_CUBE) ? TEXTURE_CUBE : TEXTURE_2D;
                    mSamplersPS[samplerIndex].logicalTextureUnit = 0;
                    mUsedPixelSamplerRange = std::max(samplerIndex + 1, mUsedPixelSamplerRange);
                }
                else
                {
                    infoLog.append("Pixel shader sampler count exceeds MAX_TEXTURE_IMAGE_UNITS (%d).", MAX_TEXTURE_IMAGE_UNITS);
                    return false;
                }
            }
            else UNREACHABLE();

            samplerIndex++;
        }
        while (samplerIndex < constant.registerIndex + constant.arraySize);
    }

    Uniform *uniform = NULL;
    GLint location = getUniformLocation(constant.name);

    if (location >= 0)   // Previously defined, type and precision must match
    {
        uniform = mUniforms[mUniformIndex[location].index];

        if (uniform->type != constant.type)
        {
            infoLog.append("Types for uniform %s do not match between the vertex and fragment shader", uniform->name.c_str());
            return false;
        }

        if (uniform->precision != constant.precision)
        {
            infoLog.append("Precisions for uniform %s do not match between the vertex and fragment shader", uniform->name.c_str());
            return false;
        }
    }
    else
    {
        uniform = new Uniform(constant.type, constant.precision, constant.name, constant.arraySize);
    }

    if (!uniform)
    {
        return false;
    }

    if (shader == GL_FRAGMENT_SHADER)
    {
        uniform->psRegisterIndex = constant.registerIndex;
    }
    else if (shader == GL_VERTEX_SHADER)
    {
        uniform->vsRegisterIndex = constant.registerIndex;
    }
    else UNREACHABLE();

    if (location >= 0)
    {
        return uniform->type == constant.type;
    }

    mUniforms.push_back(uniform);
    unsigned int uniformIndex = mUniforms.size() - 1;

    for (unsigned int i = 0; i < uniform->elementCount(); i++)
    {
        mUniformIndex.push_back(UniformLocation(constant.name, i, uniformIndex));
    }

    if (shader == GL_VERTEX_SHADER)
    {
        if (constant.registerIndex + uniform->registerCount > 1024)
        {
            infoLog.append("Vertex shader active uniforms exceed GL_MAX_VERTEX_UNIFORM_VECTORS (%u)", 1024);
            return false;
        }
    }
    else if (shader == GL_FRAGMENT_SHADER)
    {
        if (constant.registerIndex + uniform->registerCount > 1024)
        {
            infoLog.append("Fragment shader active uniforms exceed GL_MAX_FRAGMENT_UNIFORM_VECTORS (%u)", 1024);
            return false;
        }
    }
    else UNREACHABLE();

    return true;
}

int Program::getSemanticIndex(int attributeIndex)
{
    ASSERT(attributeIndex >= 0 && attributeIndex < MAX_VERTEX_ATTRIBS);
    
    return mSemanticIndex[attributeIndex];
}

void Program::initAttributesByLayout()
{
    for (int i = 0; i < MAX_VERTEX_ATTRIBS; i++)
    {
        mAttributesByLayout[i] = i;
    }

    std::sort(&mAttributesByLayout[0], &mAttributesByLayout[MAX_VERTEX_ATTRIBS], AttributeSorter(mSemanticIndex));
}

void Program::logErrorMessage(InfoLog &infoLog, ID3DBlob *errorBlob)
{
	const char *message = (const char*)errorBlob->GetBufferPointer();
    infoLog.appendSanitized(message);
    errorBlob->Release();
}

GLint Program::getUniformLocation(std::string name)
{
    unsigned int subscript = 0;

    // Strip any trailing array operator and retrieve the subscript
    size_t open = name.find_last_of('[');
    size_t close = name.find_last_of(']');
    if (open != std::string::npos && close == name.length() - 1)
    {
        subscript = atoi(name.substr(open + 1).c_str());
        name.erase(open);
    }

    unsigned int numUniforms = mUniformIndex.size();
    for (unsigned int location = 0; location < numUniforms; location++)
    {
        if (mUniformIndex[location].name == name &&
            mUniformIndex[location].element == subscript)
        {
            return location;
        }
    }

    return -1;
}

Program::Sampler::Sampler() : active(false), logicalTextureUnit(0), textureType(TEXTURE_2D)
{
}

namespace gl
{

int VariableRowCount(GLenum type)
{
    switch (type)
    {
      case GL_NONE:
        return 0;
      case GL_BOOL:
      case GL_FLOAT:
      case GL_INT:
      case GL_BOOL_VEC2:
      case GL_FLOAT_VEC2:
      case GL_INT_VEC2:
      case GL_INT_VEC3:
      case GL_FLOAT_VEC3:
      case GL_BOOL_VEC3:
      case GL_BOOL_VEC4:
      case GL_FLOAT_VEC4:
      case GL_INT_VEC4:
      case GL_SAMPLER_2D:
      case GL_SAMPLER_CUBE:
        return 1;
      case GL_FLOAT_MAT2:
        return 2;
      case GL_FLOAT_MAT3:
        return 3;
      case GL_FLOAT_MAT4:
        return 4;
      default:
        UNREACHABLE();
    }

    return 0;
}

int VariableColumnCount(GLenum type)
{
    switch (type)
    {
      case GL_NONE:
        return 0;
      case GL_BOOL:
      case GL_FLOAT:
      case GL_INT:
      case GL_SAMPLER_2D:
      case GL_SAMPLER_CUBE:
        return 1;
      case GL_BOOL_VEC2:
      case GL_FLOAT_VEC2:
      case GL_INT_VEC2:
      case GL_FLOAT_MAT2:
        return 2;
      case GL_INT_VEC3:
      case GL_FLOAT_VEC3:
      case GL_BOOL_VEC3:
      case GL_FLOAT_MAT3:
        return 3;
      case GL_BOOL_VEC4:
      case GL_FLOAT_VEC4:
      case GL_INT_VEC4:
      case GL_FLOAT_MAT4:
        return 4;
      default:
        UNREACHABLE();
    }

    return 0;
}

int AllocateFirstFreeBits(unsigned int *bits, unsigned int allocationSize, unsigned int bitsSize)
{
    ASSERT(allocationSize <= bitsSize);

    unsigned int mask = std::numeric_limits<unsigned int>::max() >> (std::numeric_limits<unsigned int>::digits - allocationSize);

    for (unsigned int i = 0; i < bitsSize - allocationSize + 1; i++)
    {
        if ((*bits & mask) == 0)
        {
            *bits |= mask;
            return i;
        }

        mask <<= 1;
    }

    return -1;
}

GLenum UniformComponentType(GLenum type)
{
    switch(type)
    {
      case GL_BOOL:
      case GL_BOOL_VEC2:
      case GL_BOOL_VEC3:
      case GL_BOOL_VEC4:
          return GL_BOOL;
      case GL_FLOAT:
      case GL_FLOAT_VEC2:
      case GL_FLOAT_VEC3:
      case GL_FLOAT_VEC4:
      case GL_FLOAT_MAT2:
      case GL_FLOAT_MAT3:
      case GL_FLOAT_MAT4:
          return GL_FLOAT;
      case GL_INT:
      case GL_SAMPLER_2D:
      case GL_SAMPLER_CUBE:
      case GL_INT_VEC2:
      case GL_INT_VEC3:
      case GL_INT_VEC4:
          return GL_INT;
      default:
          UNREACHABLE();
    }

    return GL_NONE;
}

size_t UniformComponentSize(GLenum type)
{
    switch(type)
    {
      case GL_BOOL:  return sizeof(GLint);
      case GL_FLOAT: return sizeof(GLfloat);
      case GL_INT:   return sizeof(GLint);
      default:       UNREACHABLE();
    }

    return 0;
}

size_t UniformInternalSize(GLenum type)
{
    // Expanded to 4-element vectors
    return UniformComponentSize(UniformComponentType(type)) * VariableRowCount(type) * 4;
}

static void output(bool traceFileDebugOnly, void *, const char *format, va_list vararg)
{

#if !defined(ANGLE_DISABLE_TRACE)
#if defined(NDEBUG)
    if (traceFileDebugOnly)
    {
        return;
    }
#else
	(void)traceFileDebugOnly;
#endif

    FILE* file = fopen(TRACE_OUTPUT_FILE, "a");
    if (file)
    {
        vfprintf(file, format, vararg);
        fclose(file);
    }
#endif
}

void trace(bool traceFileDebugOnly, const char *format, ...)
{
    va_list vararg;
    va_start(vararg, format);
    output(traceFileDebugOnly, NULL, format, vararg);
    va_end(vararg);
}

} //gl
