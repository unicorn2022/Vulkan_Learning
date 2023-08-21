#pragma once

#include "vulkan/vulkan.hpp"
#include "context.h"
#include "command_manager.h"
#include "swapchain.h"
#include "vertex.h"
#include "buffer.h"
#include "uniform.h"
#include <limits>

namespace toy2d {

class Renderer final {
public:
	Renderer(int maxFlightCount = 2);
	~Renderer();

	// ����ÿһ֡����Ⱦ
	void DrawTriangle();


private:
	/* ���� */
	// ������������Ƿ���õ��ź���
	void createFence();
	// �����ź���
	void createSemaphore();
	// �����������
	void createCmdBuffers();
	
	/* �������� */
	// �������㻺����
	void createVertexBuffer();	
	// �����ݴ��䵽���㻺����
	void bufferVertexData();	
	
	/* uniform���� */
	// ������������
	void createDescriptorPool();
	// ������������
	void allocateSets();
	// ������������: ��uniform�������󶨵���������
	void updateSets();
	// ����uniform������
	void createUniformBuffer();	
	// �����ݴ��䵽uniform������
	void bufferUniformData();

	/* ���ߺ��� */
	// �����ݴ�srcBuffer������dstBuffer
	void copyBuffer(vk::Buffer& src, vk::Buffer& dst, size_t size, size_t srcOffset, size_t dstOffset);

private:
	int maxFlightCount_;	// ���ͬʱ��Ⱦ֡��
	int curFrame_;			// ��ǰ֡

	std::vector<vk::Fence> fences_;	// ��������Ƿ���õ��ź���

	std::vector<vk::Semaphore> imageAvaliableSems_;	// ͼƬ�Ƿ���õ��ź���
	std::vector<vk::Semaphore> renderFinishSems_;	// ͼƬ�Ƿ������ɵ��ź���
	std::vector<vk::CommandBuffer> cmdBufs_;		// �������

	/* �������� */
	std::unique_ptr<Buffer> hostVertexBuffer_;		// CPU���㻺����
	std::unique_ptr<Buffer> deviceVertexBuffer_;	// GPU���㻺����
	
	/* uniform���� */
	vk::DescriptorPool descriptorPool_;
	std::vector<vk::DescriptorSet> sets_;
	std::vector<std::unique_ptr<Buffer>> hostUniformBuffer_;	// CPU uniform������
	std::vector<std::unique_ptr<Buffer>> deviceUniformBuffer_;	// GPU uniform������
};
}