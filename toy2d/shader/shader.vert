#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec2 Position;

void main(){
    gl_Position = vec4(Position, 0.0, 1.0);
}