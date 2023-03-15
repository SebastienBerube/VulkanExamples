/*
* Vulkan Example - Compute shader image processing
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include "SimpleComputeShaderTest1.h"
#include "SimpleComputeShaderTest2.h"
#include "VulkanFramework.h"
#include "SimpleComputeShader.h"

VulkanExampleBase* vulkanExample;
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (vulkanExample != NULL)
    {
        vulkanExample->handleMessages(hWnd, uMsg, wParam, lParam);
    }
    return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    for (int32_t i = 0; i < __argc; i++)
    {
        VulkanExampleBase::args.push_back(__argv[i]);
    };

    //TODO : Add switch here
    vulkanExample = new SimpleComputeShaderTest1();
    vulkanExample->initVulkan();
    vulkanExample->setupWindow(hInstance, WndProc);
    vulkanExample->prepare();
    vulkanExample->renderLoop();
    delete(vulkanExample);
    return 0;
}
