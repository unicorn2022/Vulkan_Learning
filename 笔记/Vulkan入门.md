[TOC]

# vulkan绘制相关类

- `vk::PhysicalDevice`：物理设备，用于查询物理设备的相关信息
- `vk::Device`：逻辑设备，用于与物理设备交互，创建其他组件
- `vk::Queue`：命令队列，将CPU的命令传递给GPU
- `vk::SwapchainKHR`：交换链，用于将GPU中的图像传递到窗口系统中
- `vk::Framebuffer`：帧缓冲，拥有一个或多个纹理附件，用于写入颜色、进行深度/模板测试

# vulkan渲染管线

- 顶点着色器的调用频率 < 片段着色器，因此尽可能将计算放到顶点着色器中
  - 例如矩阵运算
- 在光栅化之前，会进行一次`viewport`变换：$[-1,1]^3 → [0,w][0,h]$

<img src="AssetMarkdown/image-20230817104711286.png" alt="image-20230817104711286" style="zoom:80%;" />

# 帧缓冲 FrameBuffer

每个帧缓冲有多个纹理附件`attachment`，也就是`vk::Image`

- `color attachment`：颜色附件，至少一个，用于存储颜色
- `stencil & depth attachment`：模板&深度附件，用于记录模板/深度值