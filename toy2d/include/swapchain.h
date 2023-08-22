#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d {
class Swapchain final {
public:
	struct Image final {
		vk::Image image;		// ͼ��
		vk::ImageView view;		// ͼ����ͼ
	};
	vk::SurfaceKHR surface = nullptr;			// ����
	vk::SwapchainKHR swapchain = nullptr;		// ������
	std::vector<Image> images;					// ͼ��		
	std::vector<vk::Framebuffer> framebuffers;	// ͼ���Ӧ��֡����

	const auto& GetExtent() const { return surfaceInfo_.extent; }
	const auto& GetFormat() const { return surfaceInfo_.format; }

public:
	Swapchain(vk::SurfaceKHR surface, int w, int h);
	~Swapchain();
	// ��ʼ��֡����
	void InitFramebuffers();

private:
	struct SwapchainInfo {
		vk::SurfaceFormatKHR format;// ͼ����ɫ��ʽ
		vk::Extent2D extent;		// ͼ���С
		uint32_t count;				// ͼ�����
		vk::SurfaceTransformFlagBitsKHR transform; // ��ͼ��������Ļ��֮ǰ, ��ͼ�����ı任
		vk::PresentModeKHR present; // �������ĳ���ģʽ
	} surfaceInfo_;

private:
	// ����������
	vk::SwapchainKHR createSwapchain();
	// ��ѯ�������������Ϣ
	void querySurfaceInfo(int w, int h);
	// ��ѯsurface֧�ֵĸ�ʽ
	vk::SurfaceFormatKHR querySurfaceFormat();
	// ��ѯsurface֧�ֵ�ͼ���С
	vk::Extent2D querySurfaceExtent(const vk::SurfaceCapabilitiesKHR& capability, int w, int h);
	// ����ͼ����ͼ
	void createImageAndViews();
	// ����֡����
	void createFramebuffers();

};
}