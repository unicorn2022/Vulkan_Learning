#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d {
class Swapchain final {
public:
	Swapchain(int w, int h);
	~Swapchain();
	// 查询交换链的相关信息
	void queryInfo(int w, int h);
	// 获取所有的图像
	void getImages();
	// 创建图像视图
	void createImageViews();
	// 创建帧缓冲
	void createFramebuffers(int w, int h);

public:
	struct SwapchainInfo {
		vk::SurfaceFormatKHR format;// 图像颜色格式
		uint32_t imageCount;		// 图像个数
		vk::Extent2D imageExtent;	// 图像大小
		vk::SurfaceTransformFlagsKHR transform; // 将图像贴到屏幕上之前, 对图像做的变换
		vk::PresentModeKHR present;	// 显示信息
	};

	
	SwapchainInfo info;			// 交换链信息
	vk::SwapchainKHR swapchain;	// 交换链

	std::vector<vk::Image> images;				// GPU中的图像
	std::vector<vk::ImageView> imageViews;		// 图像对应的视图
	std::vector<vk::Framebuffer> framebuffers;	// 图像对应的帧缓冲
};
}