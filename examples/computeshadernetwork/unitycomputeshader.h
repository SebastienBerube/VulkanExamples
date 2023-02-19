#include "vulkanexamplebase.h"

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

        std::vector<BindingInfo> bindingInfos;

        UnityComputeShader(const std::string& shader);
        ~UnityComputeShader();

        void SetFloat(const std::string& name, float val);
        void SetInt(const std::string& name, int val);
        void SetTexture(int kernelIndex, const std::string& name, vks::Texture2D& texture);
        void Dispatch(int kernelIndex, int threadGroupsX, int threadGroupsY, int threadGroupsZ);
    };
}


