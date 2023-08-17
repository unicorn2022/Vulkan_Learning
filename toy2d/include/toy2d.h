#pragma once

#include "vulkan/vulkan.hpp"
#include "context.h"
#include "shader.h"
#include <functional>
#include "tool.h"


namespace toy2d {
	void Init(const std::vector<const char*> extensions, CreateSurfaceFunc func, int w, int h);
	void Quit();
}