#include "shader.h"
#include "context.h"
#include "mymath.h"

namespace toy2d {

Shader::Shader(const std::vector<char>& vertexSource, const std::vector<char>& fragSource) {
	/* 着色器模块的配置 */
	vk::ShaderModuleCreateInfo vertexModuleCreateInfo, fragModuleCreateInfo;
	vertexModuleCreateInfo.setCodeSize(vertexSource.size())
		.setPCode((std::uint32_t*)vertexSource.data());
	fragModuleCreateInfo.setCodeSize(fragSource.size())
		.setPCode((std::uint32_t*)fragSource.data());
	
	/* 创建着色器模块 */ 
	vertexModule_ = Context::Instance().device.createShaderModule(vertexModuleCreateInfo);
	fragModule_ = Context::Instance().device.createShaderModule(fragModuleCreateInfo);

	/* 初始化描述符集layout */
	initDescriptorSetLayouts();
}

Shader::~Shader() {
	auto& device = Context::Instance().device;

	for (auto& layout : layouts_) 
		device.destroyDescriptorSetLayout(layout);
	layouts_.clear();

	device.destroyShaderModule(vertexModule_);
	device.destroyShaderModule(fragModule_);
}

void Shader::initDescriptorSetLayouts() {
	vk::DescriptorSetLayoutCreateInfo createInfo;
	/* set = 0 的 layout */
	// 2 个 Binding: 
	//	vertex shader 的 uniform buffer: view, project
	//	fragment shader 的 uniform buffer: color
	std::vector<vk::DescriptorSetLayoutBinding> bindings(1);
	bindings[0].setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex);
	createInfo.setBindings(bindings);
	// 创建描述符集布局
	layouts_.push_back(Context::Instance().device.createDescriptorSetLayout(createInfo));

	/* set = 1 的 layout */
	// 1 个 Binding: 
	//	fragment shader 的 sampler2D
	bindings.resize(1);
	bindings[0].setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);
	createInfo.setBindings(bindings);
	// 创建描述符集布局
	layouts_.push_back(Context::Instance().device.createDescriptorSetLayout(createInfo));
}

std::vector<vk::PushConstantRange> Shader::GetPushConstantRange() const {
	std::vector<vk::PushConstantRange> ranges(2);
	ranges[0].setOffset(0)		// 偏移量
		.setSize(sizeof(Mat4))	// 大小
		.setStageFlags(vk::ShaderStageFlagBits::eVertex); // 作用阶段
	ranges[1].setOffset(sizeof(Mat4))
		.setSize(sizeof(Color))
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);
	return ranges;
}

}