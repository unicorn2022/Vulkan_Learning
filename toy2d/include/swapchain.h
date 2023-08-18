#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d {
class Swapchain final {
public:
	Swapchain(int w, int h);
	~Swapchain();
	// ��ѯ�������������Ϣ
	void queryInfo(int w, int h);
	// ��ȡ���е�ͼ��
	void getImages();
	// ����ͼ����ͼ
	void createImageViews();
	// ����֡����
	void createFramebuffers(int w, int h);

public:
	struct SwapchainInfo {
		vk::SurfaceFormatKHR format;// ͼ����ɫ��ʽ
		uint32_t imageCount;		// ͼ�����
		vk::Extent2D imageExtent;	// ͼ���С
		vk::SurfaceTransformFlagsKHR transform; // ��ͼ��������Ļ��֮ǰ, ��ͼ�����ı任
		vk::PresentModeKHR present;	// ��ʾ��Ϣ
	};

	
	SwapchainInfo info;			// ��������Ϣ
	vk::SwapchainKHR swapchain;	// ������

	std::vector<vk::Image> images;				// GPU�е�ͼ��
	std::vector<vk::ImageView> imageViews;		// ͼ���Ӧ����ͼ
	std::vector<vk::Framebuffer> framebuffers;	// ͼ���Ӧ��֡����
};
}