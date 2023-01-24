@echo off
@echo:
@echo Compiling shaders...
glslangValidator.exe -V composition.frag -o composition.frag.spv
glslangValidator.exe -V composition.vert -o composition.vert.spv
glslangValidator.exe -V gbuffer.frag -o gbuffer.frag.spv
glslangValidator.exe -V gbuffer.vert -o gbuffer.vert.spv
glslangValidator.exe -V composition.frag -o composition.frag.spv
glslangValidator.exe -V composition.vert -o composition.vert.spv
glslangValidator.exe -V transparent.frag -o transparent.frag.spv
glslangValidator.exe -V transparent.vert -o transparent.vert.spv
@echo:
pause