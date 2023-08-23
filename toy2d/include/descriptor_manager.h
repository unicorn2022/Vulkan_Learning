#pragma once

#include "vulkan/vulkan.hpp"
#include <vector>
#include <memory>

namespace toy2d {

class DescriptorSetManager final {
public:
	struct SetInfo {
		vk::DescriptorSet set;
		vk::DescriptorPool pool;
	};

	static void Init(uint32_t maxFlight) {
		instance_.reset(new DescriptorSetManager(maxFlight));
	}
	static void Quit() {
		instance_.reset();
	}
	static DescriptorSetManager& Instance() {
		return *instance_;
	}

public:
	DescriptorSetManager(uint32_t maxFlight);
	~DescriptorSetManager();

	// 申请num个uniform变量的描述符集
	std::vector<SetInfo> AllocBufferSets(uint32_t num);
	// 申请一个纹理的描述符集
	SetInfo AllocImageSet();

	void FreeImageSet(const SetInfo& set);

private:
	struct PoolInfo {
		vk::DescriptorPool pool_;
		uint32_t remainNum_;
	};

	PoolInfo bufferSetPool_;	// uniform变量的DescriptorSetPool

	std::vector<PoolInfo> fulledImageSetPool_;		// 纹理的DescriptorSetPool
	std::vector<PoolInfo> avalibleImageSetPool_;	// 纹理的DescriptorSetPool

	void addImageSetPool();
	PoolInfo& getAvalibleImagePoolInfo();

	uint32_t maxFlight_;
	static std::unique_ptr<DescriptorSetManager> instance_;
};
}