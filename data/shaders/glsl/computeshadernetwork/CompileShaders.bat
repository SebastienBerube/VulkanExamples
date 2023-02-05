@echo off
@echo:
@echo Compiling shaders...
glslangValidator.exe -V edgedetect.comp  -o edgedetect.comp.spv
glslangValidator.exe -V emboss.comp      -o emboss.comp.spv
glslangValidator.exe -V sharpen.comp     -o sharpen.comp.spv
glslangValidator.exe -V threshold.comp   -o threshold.comp.spv
glslangValidator.exe -V blur.comp        -o blur.comp.spv
glslangValidator.exe -V channelswap.comp -o channelswap.comp.spv
glslangValidator.exe -V texture.frag     -o texture.frag.spv
glslangValidator.exe -V texture.vert     -o texture.vert.spv
@echo:
pause