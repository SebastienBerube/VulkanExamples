@echo off
@echo:
@echo Compiling shaders...
glslangValidator.exe -V miss.rmiss       -o miss.rmiss.spv       --target-env spirv1.4
glslangValidator.exe -V closesthit.rchit -o closesthit.rchit.spv --target-env spirv1.4
glslangValidator.exe -V raygen.rgen      -o raygen.rgen.spv      --target-env spirv1.4
glslangValidator.exe -V shadow.rmiss     -o shadow.rmiss.spv     --target-env spirv1.4
@echo:
pause
