#pragma once

#include <algorithm>
#include <vector>
#include <functional>
#include "vulkan/vulkan.hpp"

using CreateSurfaceFunc = std::function<vk::SurfaceKHR(vk::Instance)>;
