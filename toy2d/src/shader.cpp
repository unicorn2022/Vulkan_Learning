#include "shader.h"
#include "context.h"

namespace toy2d {
std::unique_ptr<Shader> Shader::m_instance = nullptr;

void Shader::Init(const std::string& vertexSource, const std::string& fragSource) {
	m_instance.reset(new Shader(vertexSource, fragSource));
}

void Shader::Quit() {
	m_instance.reset();
}

Shader::Shader(const std::string& vertexSource, const std::string& fragSource) {
	vk::ShaderModuleCreateInfo createInfo;

	createInfo.codeSize = vertexSource.size();
	createInfo.pCode = (uint32_t*)vertexSource.data();
	vertexModule = Context::GetInstance().device.createShaderModule(createInfo);

	createInfo.codeSize = fragSource.size();
	createInfo.pCode = (uint32_t*)fragSource.data();
	fragmentModule = Context::GetInstance().device.createShaderModule(createInfo);
	
	initShages();
}

Shader::~Shader() {
	auto& device = Context::GetInstance().device;
	device.destroyShaderModule(vertexModule);
	device.destroyShaderModule(fragmentModule);
}

std::vector<vk::PipelineShaderStageCreateInfo> Shader::GetStage() {
	return stages;
}

void Shader::initShages() {
	stages.resize(2);
	stages[0].setStage(vk::ShaderStageFlagBits::eVertex) // 指定是哪种着色器
		.setModule(vertexModule) // 指定着色器模块
		.setPName("main"); // 指定着色器入口函数

	stages[1].setStage(vk::ShaderStageFlagBits::eFragment)
		.setModule(fragmentModule)
		.setPName("main");
}

}