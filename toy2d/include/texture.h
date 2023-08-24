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
	// 从文件中加载纹理
	Texture(std::string_view filename);
	// 从内存中加载纹理
	Texture(void* data, uint32_t w, uint32_t h);
	~Texture();

public:
	vk::Image image;
	vk::DeviceMemory memory;
	vk::ImageView view;
	DescriptorSetManager::SetInfo set;	// 纹理对应的描述符集

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
	// 更新描述符集
	void updateDescriptorSet();
	// 创建纹理
	void init(void* data, uint32_t w, uint32_t h);
};

class TextureManager final {
public:
	static TextureManager& Instance() {
		if (!instance_) instance_.reset(new TextureManager);
		return *instance_;
	}

	// 从文件中加载纹理
	Texture* Load(const std::string& filename);
	// 从内存中创建纹理
	Texture* Create(void* data, uint32_t w, uint32_t h);
	void Destroy(Texture* texture);
	void Clear();


private:
	static std::unique_ptr<TextureManager> instance_;

	std::vector<std::unique_ptr<Texture>> datas_;		// 所有Texture
};

}