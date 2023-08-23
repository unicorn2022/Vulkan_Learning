#pragma once

#include "context.h"
#include "render_process.h"
#include "renderer.h"
#include "descriptor_manager.h"
#include <memory>

namespace toy2d {
	void Init(const std::vector<const char*> extensions, Context::GetSurfaceCallback func, int w, int h);
	void Quit();

	Texture* LoadTexture(const std::string& filename);
	void DestroyTexture(Texture* texture);

	Renderer* GetRenderer();
}