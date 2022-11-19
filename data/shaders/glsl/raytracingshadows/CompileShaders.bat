@echo off
@echo:
@echo Compiling shaders...
C:\VulkanSDK\1.2.198.1\Bin\glslangValidator.exe -V C:\Dev\Perso\GitHub\VulkanExamples\data\shaders\glsl\raytracingshadows\miss.rmiss       -o C:\Dev\Perso\GitHub\VulkanExamples\data\shaders\glsl\raytracingshadows\miss.rmiss.spv       --target-env spirv1.4
C:\VulkanSDK\1.2.198.1\Bin\glslangValidator.exe -V C:\Dev\Perso\GitHub\VulkanExamples\data\shaders\glsl\raytracingshadows\closesthit.rchit -o C:\Dev\Perso\GitHub\VulkanExamples\data\shaders\glsl\raytracingshadows\closesthit.rchit.spv --target-env spirv1.4
C:\VulkanSDK\1.2.198.1\Bin\glslangValidator.exe -V C:\Dev\Perso\GitHub\VulkanExamples\data\shaders\glsl\raytracingshadows\raygen.rgen      -o C:\Dev\Perso\GitHub\VulkanExamples\data\shaders\glsl\raytracingshadows\raygen.rgen.spv      --target-env spirv1.4
C:\VulkanSDK\1.2.198.1\Bin\glslangValidator.exe -V C:\Dev\Perso\GitHub\VulkanExamples\data\shaders\glsl\raytracingshadows\shadow.rmiss     -o C:\Dev\Perso\GitHub\VulkanExamples\data\shaders\glsl\raytracingshadows\shadow.rmiss.spv     --target-env spirv1.4
@echo:
pause
