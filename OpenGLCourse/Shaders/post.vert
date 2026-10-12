#version 330

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 uv;

out vec2 vUV;

void main()
{
    vUV = uv;                          // pass UVs on; they get interpolated across the quad
    gl_Position = vec4(pos, 0.0, 1.0); // already in clip space, so no matrices needed
}