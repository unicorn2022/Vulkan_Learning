#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d {
class Swapchain final {
public:
	struct Image final {
		vk::Image image;		// 图像
		vk::ImageView view;		// 图像视图
	};
	vk::SurfaceKHR surface = nullptr;			// 表面
	vk::SwapchainKHR swapchain = nullptr;		// 交换链
	std::vector<Image> images;					// 图像		
	std::vector<vk::Framebuffer> framebuffers;	// 图像对应的帧缓冲

	const auto& GetExtent() const { return surfaceInfo_.extent; }
	const auto& GetFormat() const { return surfaceInfo_.format; }

public:
	Swapchain(vk::SurfaceKHR surface, int w, int h);
	~Swapchain();
	// 初始化帧缓冲
	void InitFramebuffers();

private:
	struct SwapchainInfo {
		vk::SurfaceFormatKHR format;// 图像颜色格式
		vk::Extent2D extent;		// 图像大小
		uint32_t count;				// 图像个数
		vk::SurfaceTransformFlagBitsKHR transform; // 将图像贴到屏幕上之前, 对图像做的变换
		vk::PresentModeKHR present; // 交换链的呈现模式
	} surfaceInfo_;

private:
	// 创建交换链
	vk::SwapchainKHR createSwapchain();
	// 查询交换链的相关信息
	void querySurfaceInfo(int w, int h);
	// 查询surface支持的格式
	vk::SurfaceFormatKHR querySurfaceFormat();
	// 查询surface支持的图像大小
	vk::Extent2D querySurfaceExtent(const vk::SurfaceCapabilitiesKHR& capability, int w, int h);
	// 创建图像视图
	void createImageAndViews();
	// 创建帧缓冲
	void createFramebuffers();

};
}