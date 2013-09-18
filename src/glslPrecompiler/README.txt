How to use the GLSL precompiler:

- open glslPrecompiler.sln with VS2012 for desktop
- convert the existing projects to 2012 if necessary
- in libGLESv2 define these macros in the preprocessor tab of c/c++ in its project properties:
  - COMPILE_SHADER
  - USE_FEATURE_LEVEL_9_1 (or whatever other level desired)
  - ANGLE_ENABLE_D3D11
- the usage for the program is:

glslPrecompiler [-o=[file] -v=[variable]] file1 file2
Where: filename : filename ending in .frag or .vert
    -o=[file] : output file
    -v=[variable] : output to char array named [variable] in header file, omit for binary file

see precompiler.bat for an example.