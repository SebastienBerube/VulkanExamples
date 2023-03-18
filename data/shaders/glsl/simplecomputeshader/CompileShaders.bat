@echo off
@echo:
@echo Compiling shaders...
glslangValidator.exe -V Advect.comp      -o Advect.comp.spv
glslangValidator.exe -V blur.comp        -o blur.comp.spv
glslangValidator.exe -V threshold.comp   -o threshold.comp.spv
glslangValidator.exe -V channelswap.comp -o channelswap.comp.spv
glslangValidator.exe -V multikerneltest.comp -o multikerneltest.comp.spv
@echo:
pause