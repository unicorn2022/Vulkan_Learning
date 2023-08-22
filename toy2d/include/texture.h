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
	// ����Image
	void createImage(uint32_t w, uint32_t h);
	// ����ImageView
	void createImageView();
	// �����ڴ�
	void allocateMemory();
	// ��ѯImage�ڴ�����
	uint32_t queryImageMemoryIndex();
	// ��Image�Ĳ��ִ�Undefinedת��ΪTransferDstOptimal
	void transitionImageLayoutFromUndifine2Dst();
	// CPU���� => GPU Image
	void transformData2Image(Buffer& buffer, uint32_t w, uint32_t h);
	// ��Image�Ĳ��ִ�TransferDstOptimalת��ΪShaderReadOnlyOptimal
	void transitionImageLayoutFromDst2Optimal();
};

}