#ifndef LIBGLESV2_SHADER_H_
#define LIBGLESV2_SHADER_H_

#include <GLES2/gl2.h>
#include <list>
#include <string>
#include <vector>
#include "Uniform.h"
#include "compiler/Uniform.h"

struct Varying
{
    Varying(GLenum type, const std::string &name, int size, bool array)
        : type(type), name(name), size(size), array(array), reg(-1), col(-1)
    {
    }

    GLenum type;
    std::string name;
    int size;   // Number of 'type' elements
    bool array;

    int reg;    // First varying register, assigned during link
    int col;    // First register element, assigned during link
};

typedef std::list<Varying> VaryingList;

struct Attribute
{
    Attribute() : type(GL_NONE), name("")
    {
    }

    Attribute(GLenum type, const std::string &name) : type(type), name(name)
    {
    }

    GLenum type;
    std::string name;
};

typedef std::vector<Attribute> AttributeArray;

class Shader
{
	friend class Program;
public:
    Shader(const char *hlsl);
    ~Shader();
    void parseUniforms(void *uniformBlob);
    void parseVaryings();
    void parseAttributes();
    void sortVaryings();
	const char *getHlsl() const;
	void resetVaryingsRegisterAssignment();
    const sh::ActiveUniforms &getUniforms();
    int getSemanticIndex(const std::string &attributeName);
    
    static GLenum parseType(const std::string &type);
    static bool compareVarying(const Varying &x, const Varying &y);

private:
    char *mHlsl;
    VaryingList mVaryings;
    AttributeArray mAttributes;
    sh::ActiveUniforms mActiveUniforms;
    bool mUsesMultipleRenderTargets;
    bool mUsesFragColor;
    bool mUsesFragData;
    bool mUsesFragCoord;
    bool mUsesFrontFacing;
    bool mUsesPointSize;
    bool mUsesPointCoord;
    bool mUsesDepthRange;
    bool mUsesFragDepth;
};

#endif
