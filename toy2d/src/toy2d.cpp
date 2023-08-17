#include "toy2d.h"

namespace toy2d {
	void Init(const std::vector<const char*> extensions, CreateSurfaceFunc func, int w, int h) {
		Context::Init(extensions, func);
		Context::GetInstance().InitSwapchain(w, h);
		Shader::Init(ReadWholeFile("./shader/vert.spv"), ReadWholeFile("./shader/frag.spv"));
	}

	void Quit() {
		Shader::Quit();
		Context::GetInstance().DestroySwapchain();
		Context::Quit();
	}
}

