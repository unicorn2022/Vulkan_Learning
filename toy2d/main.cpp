#include "toy2d.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_vulkan.h"
#include <SDL2/SDL_video.h>

struct MyContext {
	bool shouldClose = false;
	float x, y; 
	toy2d::Renderer* renderer;
};
void MainLoop();
void HandleInput(MyContext& context);

int main(int argc, char* argv[]) {
	// SDL��ʼ��
	SDL_Init(SDL_INIT_EVERYTHING);

	// ����SDL����
	SDL_Window* window = SDL_CreateWindow("test",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		1024, 720,
		SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);

	if (!window) {
		SDL_Log("create window failed");
		exit(2);
	}

	// ��ȡ��ǰϵͳ֧�ֵĴ�����չ
	unsigned int count;
	SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);
	std::vector<const char*> extensions(count);
	SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data());

	// ��ʼ��toy2d
	toy2d::Init(extensions, 
		[&](vk::Instance instance) {
			VkSurfaceKHR surface;
			if (!SDL_Vulkan_CreateSurface(window, instance, &surface)) {
				throw std::runtime_error("cannot create surface");
			}
			return surface;
		}, 1024, 720);

	// ������Ⱦѭ��
	MainLoop();

	// ����SDL����, ���Ƴ�SDLӦ��
	toy2d::Quit();
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}

void MainLoop() {
	MyContext context;
	context.shouldClose = false;
	context.x = 100;
	context.y = 100;
	context.renderer = toy2d::GetRenderer();
	context.renderer->SetDrawColor(toy2d::Color{ 1, 1, 1 });

    while (!context.shouldClose) {
		// ����SDL�¼�
		HandleInput(context);
       
		// ���ƾ���
		context.renderer->DrawRect(toy2d::Rect{
			toy2d::Vec{context.x, context.y},
			toy2d::Size{250, 272} 
		});
    }
}

void HandleInput(MyContext& context) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		// �����˳��¼�
		if (event.type == SDL_QUIT) {
			context.shouldClose = true;
		}

		// ���������¼�
		if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.sym == SDLK_a) {
				context.x -= 10;
			}
			if (event.key.keysym.sym == SDLK_d) {
				context.x += 10;
			}
			if (event.key.keysym.sym == SDLK_w) {
				context.y -= 10;
			}
			if (event.key.keysym.sym == SDLK_s) {
				context.y += 10;
			}
			if (event.key.keysym.sym == SDLK_1) {
				context.renderer->SetDrawColor(toy2d::Color{ 0, 0, 1 });
			}
			if (event.key.keysym.sym == SDLK_2) {
				context.renderer->SetDrawColor(toy2d::Color{ 0, 1, 0 });
			}
			if (event.key.keysym.sym == SDLK_3) {
				context.renderer->SetDrawColor(toy2d::Color{ 0, 1, 1 });
			}
			if (event.key.keysym.sym == SDLK_4) {
				context.renderer->SetDrawColor(toy2d::Color{ 1, 0, 0 });
			}
			if (event.key.keysym.sym == SDLK_5) {
				context.renderer->SetDrawColor(toy2d::Color{ 1, 0, 1 });
			}
			if (event.key.keysym.sym == SDLK_6) {
				context.renderer->SetDrawColor(toy2d::Color{ 1, 1, 0 });
			}
			if (event.key.keysym.sym == SDLK_7) {
				context.renderer->SetDrawColor(toy2d::Color{ 1, 1, 1 });
			}
			if (event.key.keysym.sym == SDLK_ESCAPE) {
				context.shouldClose = true;
			}
		}
	}
}