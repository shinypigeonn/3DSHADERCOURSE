#version 330

in vec2 vUV;
out vec4 colour;

uniform sampler2D sceneTexture; // the image of your rendered pyramids
uniform float strength;         // how far the channels are pushed apart

void main()
{
    vec2 offset = vec2(strength, 0.0);

    float r = texture(sceneTexture, vUV + offset).r; // red from slightly right
    float g = texture(sceneTexture, vUV).g;          // green from the true position
    float b = texture(sceneTexture, vUV - offset).b; // blue from slightly left

    colour = vec4(r, g, b, 1.0);
}