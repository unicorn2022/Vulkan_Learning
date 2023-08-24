#pragma once

#include "vulkan/vulkan.hpp"
#include "buffer.h"
#include "descriptor_manager.h"
#include <string_view>
#include <string>

namespace toy2d {

class TextureManager;

class Texture final {
public:
	friend class TextureManager;
	// ���ļ��м�������
	Texture(std::string_view filename);
	// ���ڴ��м�������
	Texture(void* data, uint32_t w, uint32_t h);
	~Texture();

public:
	vk::Image image;
	vk::DeviceMemory memory;
	vk::ImageView view;
	DescriptorSetManager::SetInfo set;	// �����Ӧ����������

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
	// ������������
	void updateDescriptorSet();
	// ��������
	void init(void* data, uint32_t w, uint32_t h);
};

class TextureManager final {
public:
	static TextureManager& Instance() {
		if (!instance_) instance_.reset(new TextureManager);
		return *instance_;
	}

	// ���ļ��м�������
	Texture* Load(const std::string& filename);
	// ���ڴ��д�������
	Texture* Create(void* data, uint32_t w, uint32_t h);
	void Destroy(Texture* texture);
	void Clear();


private:
	static std::unique_ptr<TextureManager> instance_;

	std::vector<std::unique_ptr<Texture>> datas_;		// ����Texture
};

}