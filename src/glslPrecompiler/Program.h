#ifndef LIBGLESV2_PROGRAM_H_
#define LIBGLESV2_PROGRAM_H_

#include <set>
#include <string>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "libGLESv2/Constants.h"
#include "libGLESv2/angletypes.h"
#include "Shader.h"
#include "Uniform.h"
#include <d3dcompiler.h>
#include <d3d11.h>

using namespace gl;

class Shader;

// Struct used for correlating uniforms/elements of uniform arrays to handles
struct UniformLocation
{
    UniformLocation()
    {
    }

    UniformLocation(const std::string &name, unsigned int element, unsigned int index);

    std::string name;
    unsigned int element;
    unsigned int index;
};

class AttributeBindings
{
  public:
    AttributeBindings();
    ~AttributeBindings();

    void bindAttributeLocation(GLuint index, const char *name);
    int getAttributeBinding(const std::string &name) const;

  private:
    std::set<std::string> mAttributeBinding[MAX_VERTEX_ATTRIBS];
};

class InfoLog
{
  public:
    InfoLog();
    ~InfoLog();

    int getLength() const;
    void getLog(GLsizei bufSize, GLsizei *length, char *infoLog);

    void appendSanitized(const char *message);
    void append(const char *info, ...);
    void reset();
  private:
    char *mInfoLog;
};

class Program
{
public:
    Program(Shader *vertexShader, Shader *fragmentShader);
    bool link();
	bool save(const char *file);

private:
    int packVaryings(InfoLog &infoLog, const Varying *packing[][4]);
	bool linkVaryings(InfoLog &infoLog, int registers, const Varying *packing[][4],
                      std::string& pixelHLSL, std::string& vertexHLSL,
                      Shader *fragmentShader, Shader *vertexShader);
    bool linkAttributes(InfoLog &infoLog, const AttributeBindings &attributeBindings, Shader *fragmentShader, Shader *vertexShader);
	bool linkUniforms(InfoLog &infoLog, const sh::ActiveUniforms &vertexUniforms, const sh::ActiveUniforms &fragmentUniforms);
    bool defineUniform(GLenum shader, const sh::Uniform &constant, InfoLog &infoLog);
	int getSemanticIndex(int attributeIndex);
	void initAttributesByLayout();
	void logErrorMessage(InfoLog &infoLog, ID3DBlob *errorBlob);
	GLint getUniformLocation(std::string name);

	struct Sampler
    {
        Sampler();

        bool active;
        GLint logicalTextureUnit;
        TextureType textureType;
    };

	Shader *mVertexShader;
	Shader *mFragmentShader;
	ID3DBlob *mVertexShaderCompiled;
	ID3DBlob *mFragmentShaderCompiled;
	
    AttributeBindings mAttributeBindings;
    Attribute mLinkedAttribute[MAX_VERTEX_ATTRIBS];
    int mSemanticIndex[MAX_VERTEX_ATTRIBS];
    int mAttributesByLayout[MAX_VERTEX_ATTRIBS];

    Sampler mSamplersPS[MAX_TEXTURE_IMAGE_UNITS];
    Sampler mSamplersVS[IMPLEMENTATION_MAX_VERTEX_TEXTURE_IMAGE_UNITS];
    GLuint mUsedVertexSamplerRange;
    GLuint mUsedPixelSamplerRange;
    bool mUsesPointSize;

    UniformArray mUniforms;
    typedef std::vector<UniformLocation> UniformIndex;
    UniformIndex mUniformIndex;
};

#endif