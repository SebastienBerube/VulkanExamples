#include "vulkan/vulkan.h"
#include "VulkanTexture.h"
#include <string>
#include <vector>

namespace VulkanUtilities
{
    class IUnityComputeShader
    {
        virtual void SetFloat(const std::string& name, float val) = 0;
        virtual void SetInt(const std::string& name, int val) = 0;
        virtual void SetTexture(int kernelIndex, int nameID, vks::Texture2D& texture) = 0;
        virtual void Dispatch(int kernelIndex, int threadGroupsX, int threadGroupsY, int threadGroupsZ) = 0;
    };

    class UnityComputeShader
    {
    public:
        struct BindingInfo
        {
            std::string name;
            VkDescriptorType type;
            VkFormat format;
            uint32_t bindingIndex;
        };

        UnityComputeShader(VkDevice device, VkDescriptorPool descriptorPool, const std::string& shader);
        ~UnityComputeShader();

        void SetFloat(const std::string& name, float val);
        void SetInt(const std::string& name, int val);
        void SetTexture(int kernelIndex, const std::string& name, vks::Texture2D& texture);
        void Dispatch(int kernelIndex, int threadGroupsX, int threadGroupsY, int threadGroupsZ);

    private:
        void Prepare();
        //Not owner
        VkDevice _device = VK_NULL_HANDLE;
        VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;
        std::string _shaderName;

        //Owner
        std::vector<BindingInfo> _bindingInfos;

        VkDescriptorSetLayout _descriptorSetLayout = VK_NULL_HANDLE;
        VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
        VkDescriptorSet _descriptorSet = VK_NULL_HANDLE;
    };
}


