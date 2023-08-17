#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d {
class Shader final {
public:
	static void Init(const std::string& vertexSource, const std::string& fragSource);
	static void Quit();
	static Shader& GetInstance() {
		assert(m_instance);
		return *m_instance;
	}

	~Shader();
	std::vector<vk::PipelineShaderStageCreateInfo> GetStage();

public:
	vk::ShaderModule vertexModule;
	vk::ShaderModule fragmentModule;

private:
	Shader(const std::string& vertexSource, const std::string& fragSource);
	void initShages();

private:
	static std::unique_ptr<Shader> m_instance;
	std::vector<vk::PipelineShaderStageCreateInfo> stages;
};
}