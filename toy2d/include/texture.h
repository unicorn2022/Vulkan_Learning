#pragma once

#include "vulkan/vulkan.hpp"
#include "buffer.h"
#include <string_view>

namespace toy2d {

class Texture final {
public:
	Texture(std::string_view filename);
	~Texture();

public:
	vk::Image image;
	vk::DeviceMemory memory;
	vk::ImageView view;

private:
	// 创建Image
	void createImage(uint32_t w, uint32_t h);
	// 创建ImageView
	void createImageView();
	// 分配内存
	void allocateMemory();
	// 查询Image内存索引
	uint32_t queryImageMemoryIndex();
	// 将Image的布局从Undefined转换为TransferDstOptimal
	void transitionImageLayoutFromUndifine2Dst();
	// CPU数据 => GPU Image
	void transformData2Image(Buffer& buffer, uint32_t w, uint32_t h);
	// 将Image的布局从TransferDstOptimal转换为ShaderReadOnlyOptimal
	void transitionImageLayoutFromDst2Optimal();
};

}