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

Shader& Shader::GetInstance() {
	assert(m_instance);
	return *m_instance;
}

Shader::Shader(const std::string& vertexSource, const std::string& fragSource) {
	vk::ShaderModuleCreateInfo createInfo;

	createInfo.codeSize = vertexSource.size();
	createInfo.pCode = (uint32_t*)vertexSource.data();
	vertexModule = Context::GetInstance().device.createShaderModule(createInfo);

	createInfo.codeSize = fragSource.size();
	createInfo.pCode = (uint32_t*)fragSource.data();
	fragmentModule = Context::GetInstance().device.createShaderModule(createInfo);
}

Shader::~Shader() {
	auto& device = Context::GetInstance().device;
	device.destroyShaderModule(vertexModule);
	device.destroyShaderModule(fragmentModule);
}



}