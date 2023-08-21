#pragma once

#include "context.h"
#include "render_process.h"
#include "renderer.h"
#include <memory>

namespace toy2d {
	void Init(const std::vector<const char*> extensions, Context::GetSurfaceCallback func, int w, int h);
	void Quit();

	Renderer* GetRenderer();
}