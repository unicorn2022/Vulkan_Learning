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
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)	// uniform的类型
			.setStageFlags(vk::ShaderStageFlagBits::eFragment)		// uniform可以用在哪一个shader中
			.setDescriptorCount(1);									// uniform的数量

		return binding;
	}
};

}