#pragma once

#include "vulkan/vulkan.hpp"
#include "context.h"
#include "command_manager.h"
#include "swapchain.h"
#include "vertex.h"
#include "buffer.h"
#include <limits>

namespace toy2d {

class Renderer final {
public:
	Renderer(int maxFlightCount = 2);
	~Renderer();

	// ����ÿһ֡����Ⱦ
	void DrawTriangle();


private:
	void createFence();			// ������������Ƿ���õ��ź���
	void createSemaphore();		// �����ź���
	void createCmdBuffers();	// �����������
	void createVertexBuffer();	// �������㻺����
	void bufferVertexData();	// �����ݴ��䵽���㻺����

private:
	int maxFlightCount_;	// ���ͬʱ��Ⱦ֡��
	int curFrame_;			// ��ǰ֡

	std::vector<vk::Fence> fences_;	// ��������Ƿ���õ��ź���

	std::vector<vk::Semaphore> imageAvaliableSems_;	// ͼƬ�Ƿ���õ��ź���
	std::vector<vk::Semaphore> renderFinishSems_;	// ͼƬ�Ƿ������ɵ��ź���
	std::vector<vk::CommandBuffer> cmdBufs_;		// �������

	std::unique_ptr<Buffer> hostVertexBuffer_;		// CPU���㻺����
	std::unique_ptr<Buffer> deviceVertexBuffer_;	// GPU���㻺����
};
}