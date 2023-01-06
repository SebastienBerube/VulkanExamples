@echo off
@echo:
@echo Compiling shaders...
glslangValidator.exe -V gaussblur.frag -o gaussblur.frag.spv
glslangValidator.exe -V gaussblur.vert -o gaussblur.vert.spv
glslangValidator.exe -V phongpass.frag -o phongpass.frag.spv
@echo:
pause
