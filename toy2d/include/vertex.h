#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d {

struct Vertex final {
	float x, y;

	// 顶点数据的属性描述
	static vk::VertexInputAttributeDescription GetAttribute() {
		vk::VertexInputAttributeDescription attr;
		attr.setBinding(0)	// 顶点数据的binding
			.setFormat(vk::Format::eR32G32Sfloat)	// 2个32位有符号浮点数
			.setLocation(0)	// 顶点数据的location
			.setOffset(0);	// 顶点数据的偏移量
		return attr;
	}

	// 顶点数据的binding描述
	static vk::VertexInputBindingDescription GetBinding() {
		vk::VertexInputBindingDescription binding;

		binding.setBinding(0)
			.setInputRate(vk::VertexInputRate::eVertex) // 顶点数据的读取频率: 每个顶点传输一次
			.setStride(sizeof(Vertex));					// 顶点数据的步长: 每经过一个Vertex的距离, 可以拿到下一个Vertex的数据
		
		return binding;
	}
};

}