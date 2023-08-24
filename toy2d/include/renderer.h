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
	void DrawTexture(const Rect&, Texture& texture);
	void DrawLine(const Vec& p1, const Vec& p2);
	void SetDrawColor(const Color&);

	void StartRender();
	void EndRender();

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
	// �����������
	void bufferRectData();
	// �������vertex����
	void bufferRectVertexData();
	// �������index����
	void bufferRectIndicesData();
	// �����߶�vertex����
	void bufferLineData(const Vec& p1, const Vec& p2);
	// ����MVP����
	void bufferMVPData();

	/* ������������ */
	// ������������: ��uniform�������󶨵���������
	void updateDescriptorSets();

	/* ���ߺ��� */
	// �����ݴ�srcBuffer������dstBuffer
	void transformBuffer2Device(Buffer& src, Buffer& dst, size_t srcOffset, size_t dstOffset, size_t size);
	// ��ѯ�ڴ���������
	std::uint32_t queryBufferMemTypeIndex(std::uint32_t, vk::MemoryPropertyFlags);
	// ������ɫ����
	void createWhiteTexture();

private:
	int maxFlightCount_;	// ���ͬʱ��Ⱦ֡��
	int curFrame_;			// ��ǰ֡
	uint32_t imageIndex_;	// ��ǰimage����

	std::vector<vk::Fence> fences_;	// ��������Ƿ���õ��ź���

	std::vector<vk::Semaphore> imageAvaliableSems_;	// ͼƬ�Ƿ���õ��ź���
	std::vector<vk::Semaphore> renderFinishSems_;	// ͼƬ�Ƿ������ɵ��ź���
	std::vector<vk::CommandBuffer> cmdBufs_;		// �������

	/* �������� */
	std::unique_ptr<Buffer> rectVerticesBuffer_;	// (����)���㻺����
	std::unique_ptr<Buffer> rectIndicesBuffer_;		// (����)����������
	std::unique_ptr<Buffer> lineVerticesBuffer_;	// (�߶�)���㻺����

	/* �任���� */
	Mat4 projectMat_;
	Mat4 viewMat_;
	
	/* uniform���� */
	std::vector<std::unique_ptr<Buffer>> uniformBuffers_;		// uniform������
	std::vector<std::unique_ptr<Buffer>> colorBuffers_;			// ��ɫ������
	std::vector<std::unique_ptr<Buffer>> deviceUniformBuffers_;	// (GPU)uniform������
	std::vector<std::unique_ptr<Buffer>> deviceColorBuffers_;	// (GPU)��ɫ������

	/* ���� */
	std::vector<DescriptorSetManager::SetInfo> descriptorSets_;	// uniform������Ӧ����������
	vk::Sampler sampler;	// ���������
	Texture* whiteTexture;	// ��ɫ����
};
}