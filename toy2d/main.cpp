#include "toy2d.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_vulkan.h"
#include <SDL2/SDL_video.h>


void MainLoop();

int main(int argc, char* argv[]) {
	// SDL初始化
	SDL_Init(SDL_INIT_EVERYTHING);

	// 创建SDL窗口
	SDL_Window* window = SDL_CreateWindow("test",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		1024, 720,
		SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);

	if (!window) {
		SDL_Log("create window failed");
		exit(2);
	}

	// 获取当前系统支持的窗口扩展
	unsigned int count;
	SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);
	std::vector<const char*> extensions(count);
	SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data());
	std::cout << "当前系统支持的窗口扩展: ";
	for (auto& extension : extensions) 
		std::cout << extension << ", ";
	std::cout << "\n";


	// 开启渲染循环
	toy2d::Init(extensions, 
		[&](vk::Instance instance) {
			VkSurfaceKHR surface;
			if (!SDL_Vulkan_CreateSurface(window, instance, &surface)) {
				throw std::runtime_error("cannot create surface");
			}
			return surface;
		}, 1024, 720);
	MainLoop();
	toy2d::Quit();

	// 销毁SDL窗口, 并推出SDL应用
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}

void MainLoop() {
	auto renderer = toy2d::GetRenderer();

    bool shouldClose = false;
    SDL_Event event;

    while (!shouldClose) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                shouldClose = true;
            }
        }
		renderer->DrawTriangle();
    }
}