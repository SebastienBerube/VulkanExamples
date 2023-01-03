@echo off
@echo:
@echo Compiling shaders...
glslangValidator.exe -V phongpass.frag -o phongpass.frag.spv
@echo:
pause
