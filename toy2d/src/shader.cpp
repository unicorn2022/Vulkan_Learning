#include "shader.h"
#include "context.h"
#include "mymath.h"

namespace toy2d {

Shader::Shader(const std::vector<char>& vertexSource, const std::vector<char>& fragSource) {
	/* ��ɫ��ģ������� */
	vk::ShaderModuleCreateInfo vertexModuleCreateInfo, fragModuleCreateInfo;
	vertexModuleCreateInfo.setCodeSize(vertexSource.size())
		.setPCode((std::uint32_t*)vertexSource.data());
	fragModuleCreateInfo.setCodeSize(fragSource.size())
		.setPCode((std::uint32_t*)fragSource.data());
	
	/* ������ɫ��ģ�� */ 
	vertexModule_ = Context::Instance().device.createShaderModule(vertexModuleCreateInfo);
	fragModule_ = Context::Instance().device.createShaderModule(fragModuleCreateInfo);

	/* ��ʼ����������layout */
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
	/* Binding ���� */
	std::vector<vk::DescriptorSetLayoutBinding> bindings(2);
	bindings[0].setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex);
	bindings[1].setBinding(1)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);
	
	/* ���������������� */
	vk::DescriptorSetLayoutCreateInfo createInfo;
	createInfo.setBindings(bindings);

	/* ���������������� */
	layouts_.push_back(Context::Instance().device.createDescriptorSetLayout(createInfo));
}

vk::PushConstantRange Shader::GetPushConstantRange() const {
	vk::PushConstantRange range;
	range.setOffset(0)			// ƫ����
		.setSize(sizeof(Mat4))	// ��С
		.setStageFlags(vk::ShaderStageFlagBits::eVertex);	// ���ý׶�
	return range;
}

}