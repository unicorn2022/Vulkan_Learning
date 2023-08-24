#pragma once

#include "vulkan/vulkan.hpp"
#include <vector>

namespace toy2d {

class Shader {
public:
    Shader(const std::vector<char>& vertexSource, const std::vector<char>& fragSource);
    ~Shader();

    vk::ShaderModule GetVertexModule() const { return vertexModule_; }
    vk::ShaderModule GetFragModule() const { return fragModule_; }

    // 获取shader的描述符集布局
    const std::vector<vk::DescriptorSetLayout>& GetDescriptorSetLayouts() const { return layouts_; }
    // 获取shader的PushConstant配置
    std::vector<vk::PushConstantRange> GetPushConstantRange() const;

private:
    vk::ShaderModule vertexModule_; // 顶点着色器模块
    vk::ShaderModule fragModule_;   // 片段着色器模块
    std::vector<vk::DescriptorSetLayout> layouts_;  // 描述符集布局

    // 初始化描述符集布局
    void initDescriptorSetLayouts();
};

}
