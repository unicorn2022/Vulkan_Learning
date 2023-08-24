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
	vk::DescriptorSetLayoutCreateInfo createInfo;
	/* set = 0 �� layout */
	// 2 �� Binding: 
	//	vertex shader �� uniform buffer: view, project
	//	fragment shader �� uniform buffer: color
	std::vector<vk::DescriptorSetLayoutBinding> bindings(1);
	bindings[0].setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex);
	createInfo.setBindings(bindings);
	// ����������������
	layouts_.push_back(Context::Instance().device.createDescriptorSetLayout(createInfo));

	/* set = 1 �� layout */
	// 1 �� Binding: 
	//	fragment shader �� sampler2D
	bindings.resize(1);
	bindings[0].setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);
	createInfo.setBindings(bindings);
	// ����������������
	layouts_.push_back(Context::Instance().device.createDescriptorSetLayout(createInfo));
}

std::vector<vk::PushConstantRange> Shader::GetPushConstantRange() const {
	std::vector<vk::PushConstantRange> ranges(2);
	ranges[0].setOffset(0)		// ƫ����
		.setSize(sizeof(Mat4))	// ��С
		.setStageFlags(vk::ShaderStageFlagBits::eVertex); // ���ý׶�
	ranges[1].setOffset(sizeof(Mat4))
		.setSize(sizeof(Color))
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);
	return ranges;
}

}