#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d {

struct Vertex final {
	float x, y;

	// �������ݵ���������
	static vk::VertexInputAttributeDescription GetAttribute() {
		vk::VertexInputAttributeDescription attr;
		attr.setBinding(0)	// �������ݵ�binding
			.setFormat(vk::Format::eR32G32Sfloat)	// 2��32λ�з��Ÿ�����
			.setLocation(0)	// �������ݵ�location
			.setOffset(0);	// �������ݵ�ƫ����
		return attr;
	}

	// �������ݵ�binding����
	static vk::VertexInputBindingDescription GetBinding() {
		vk::VertexInputBindingDescription binding;

		binding.setBinding(0)
			.setInputRate(vk::VertexInputRate::eVertex) // �������ݵĶ�ȡƵ��: ÿ�����㴫��һ��
			.setStride(sizeof(Vertex));					// �������ݵĲ���: ÿ����һ��Vertex�ľ���, �����õ���һ��Vertex������
		
		return binding;
	}
};

}