#pragma once

#include "vulkan/vulkan.hpp"
#include "context.h"
#include "shader.h"


namespace toy2d {
	void Init(const std::vector<const char*> extensions, CreateSurfaceFunc func, int w, int h);
	void Quit();

	inline Renderer& GetRenderer() {
		return *Context::GetInstance().renderer;
	}
}