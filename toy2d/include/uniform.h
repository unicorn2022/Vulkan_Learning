#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d {

struct Color final {
	float r, g, b;
};

struct Uniform final {
	Color color;

	static vk::DescriptorSetLayoutBinding GetBinding() {
		vk::DescriptorSetLayoutBinding binding;
		binding.setBinding(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)	// uniform������
			.setStageFlags(vk::ShaderStageFlagBits::eFragment)		// uniform����������һ��shader��
			.setDescriptorCount(1);									// uniform������

		return binding;
	}
};

}