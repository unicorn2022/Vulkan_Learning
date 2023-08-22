#pragma once

#include "vulkan/vulkan.hpp"
#include "context.h"
#include "command_manager.h"
#include "swapchain.h"
#include "mymath.h"
#include "buffer.h"
#include "texture.h"
#include <limits>

namespace toy2d {

class Renderer final {
public:
	Renderer(int maxFlightCount = 2);
	~Renderer();

	void SetProject(int right, int left, int bottom, int top, int far, int near);
	void DrawRect(const Rect&);
	void SetDrawColor(const Color&);


private:
	// fence: CPU �� GPU ֮���ͬ��
	// semaphore: GPU �� Command Queue ֮���ͬ��
	// event: CPU �� GPU ֮�䷢���źŽ���ͬ��
	// barrier: ���ڸ��������ִ��
	
	/* ���� */
	// ������������Ƿ���õ��ź���
	void createFence();
	// �����ź���
	void createSemaphore();
	// �����������
	void createCmdBuffers();
	
	/* ���������� */
	// ����vertex&index������
	void createBuffers();
	// ����uniform������
	void createUniformBuffer(int flightCount);

	/* �����ݴ��͸�GPU */
	// ��ʼ��MVP����
	void initMats();
	// ��������
	void bufferData();
	// ����vertex����
	void bufferVertexData();
	// ����index����
	void bufferIndicesData();
	// ����MVP����
	void bufferMVPData();

	/* ������������ */
	// ������������
	void createDescriptorPool(int flightCount);
	// ������������
	std::vector<vk::DescriptorSet> allocDescriptorSet(int flightCount);
	// ������������
	void allocDescriptorSets(int flightCount);
	// ������������: ��uniform�������󶨵���������
	void updateDescriptorSets();
	
	/* �������� */
	// �������������
	void createSampler();
	// ��������
	void createTexture();

	/* ���ߺ��� */
	// �����ݴ�srcBuffer������dstBuffer
	void transformBuffer2Device(Buffer& src, Buffer& dst, size_t srcOffset, size_t dstOffset, size_t size);
	// ��ѯ�ڴ���������
	std::uint32_t queryBufferMemTypeIndex(std::uint32_t, vk::MemoryPropertyFlags);

private:
	int maxFlightCount_;	// ���ͬʱ��Ⱦ֡��
	int curFrame_;			// ��ǰ֡

	std::vector<vk::Fence> fences_;	// ��������Ƿ���õ��ź���

	std::vector<vk::Semaphore> imageAvaliableSems_;	// ͼƬ�Ƿ���õ��ź���
	std::vector<vk::Semaphore> renderFinishSems_;	// ͼƬ�Ƿ������ɵ��ź���
	std::vector<vk::CommandBuffer> cmdBufs_;		// �������

	/* �������� */
	std::unique_ptr<Buffer> verticesBuffer_;	// ���㻺����
	std::unique_ptr<Buffer> indicesBuffer_;		// ����������

	/* �任���� */
	Mat4 projectMat_;
	Mat4 viewMat_;
	
	/* uniform���� */
	vk::DescriptorPool descriptorPool_;
	std::vector<vk::DescriptorSet> descriptorSets_;
	std::vector<std::unique_ptr<Buffer>> uniformBuffers_;		// uniform������
	std::vector<std::unique_ptr<Buffer>> colorBuffers_;			// ��ɫ������
	std::vector<std::unique_ptr<Buffer>> deviceUniformBuffers_;	// (GPU)uniform������
	std::vector<std::unique_ptr<Buffer>> deviceColorBuffers_;	// (GPU)��ɫ������

	/* ���� */
	std::unique_ptr<Texture> texture;
	vk::Sampler sampler;
};
}