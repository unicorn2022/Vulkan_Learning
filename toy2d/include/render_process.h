#pragma once

#include "vulkan/vulkan.hpp"

namespace toy2d {

class RenderProcess final {
public:
	void InitPipeline(int width, int height);
	void DestoryPipline();

public:
	vk::Pipeline pipeline;
};
}