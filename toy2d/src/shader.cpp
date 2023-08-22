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
	/* Binding 配置 */
	std::vector<vk::DescriptorSetLayoutBinding> bindings(2);
	bindings[0].setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex);
	bindings[1].setBinding(1)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);
	
	/* 描述符集布局配置 */
	vk::DescriptorSetLayoutCreateInfo createInfo;
	createInfo.setBindings(bindings);

	/* 创建描述符集布局 */
	layouts_.push_back(Context::Instance().device.createDescriptorSetLayout(createInfo));
}

vk::PushConstantRange Shader::GetPushConstantRange() const {
	vk::PushConstantRange range;
	range.setOffset(0)			// 偏移量
		.setSize(sizeof(Mat4))	// 大小
		.setStageFlags(vk::ShaderStageFlagBits::eVertex);	// 作用阶段
	return range;
}

}