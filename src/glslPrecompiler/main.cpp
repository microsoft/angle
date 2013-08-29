//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "../samples/gles2_book/Common/esUtil.h"
#include <stdio.h>

static void usage();

int main(int argc, char* argv[])
{
    ESContext esContext;
    esInitContext(&esContext);
    esCreateWindow(&esContext, TEXT("GLSL Precompiler"), 1, 1, ES_WINDOW_RGB);

    GLuint vertexShader = 0;
    GLuint fragmentShader = 0;
    bool usageFail = false;
    char outputFile[1024] = "shader.file";

    argc--;
    argv++;
    for (; (argc >= 1) && !usageFail; argc--, argv++) {
        if (argv[0][0] == '-') {
            switch (argv[0][1]) {
            case 'o':
                if (argv[0][2] == '=' && strlen(argv[0] + 3)) {
                    strcpy(outputFile, argv[0] + 3);
                }
                break;
            default: usageFail = true;
            }
        } else {
            GLuint shader;
            if(strstr(argv[0], ".vert"))
                shader = vertexShader = glCreateShader(GL_VERTEX_SHADER);
            else
                shader = fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
            FILE *fp = fopen(argv[0], "r");
            if(!fp)
            {
                printf("Can't open %s, aborting\n", argv[0]);
                return 0;
            }
            fseek(fp, 0, SEEK_END);
            int fileSize = ftell(fp);
            rewind(fp);
            char *sourceBuffer = new char[fileSize + 1];
            fileSize = fread(sourceBuffer, 1, fileSize, fp);
            fclose(fp);
            sourceBuffer[fileSize] = 0;
            glShaderSource(shader, 1, (const char**)&sourceBuffer, NULL);
            delete [] sourceBuffer;
            glCompileShader(shader);
            int compileStatus;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
            if(compileStatus == GL_FALSE)
            {
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &compileStatus);
                sourceBuffer = new char[compileStatus];
                glGetShaderInfoLog(shader, compileStatus, NULL, sourceBuffer);
                printf("Failed to compile %s due to:\n%s\n", argv[0], sourceBuffer);
                delete [] sourceBuffer;
                glDeleteShader(shader);
                return 0;
            }
        }
    }

    if (usageFail)
        usage();

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    int linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if(linkStatus == GL_FALSE)
    {
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &linkStatus);
        char *infoLog = new char[linkStatus];
        glGetProgramInfoLog(program, linkStatus, NULL, infoLog);
        printf("Failed to link the shaders into a program due to:\n%s\n", infoLog);
        delete [] infoLog;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }
    glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH_OES, &linkStatus);
    char *binary = new char[linkStatus];
    GLenum binaryFormat;
    glGetProgramBinaryOES(program, linkStatus, NULL, &binaryFormat, binary);
    FILE *fp = fopen(outputFile, "wb");
    fwrite(binary, linkStatus, 1, fp);
    fclose(fp);
    delete [] binary;
    printf("Compilation successful\n");
    return 0;
}

//
//   print usage to stdout
//
void usage()
{
    printf("Usage: translate [-i -m -o -u -l -e -b=e -b=g -b=h -x=i -x=d] file1 file2 ...\n"
        "Where: filename : filename ending in .frag or .vert\n"
        "       -o=[file] : output file\n");
}
