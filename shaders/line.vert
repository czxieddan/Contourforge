#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vertexColor;
out float vertexDepth;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float lineWidth;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    vec4 viewPos = view * worldPos;
    gl_Position = projection * viewPos;
    
    vertexColor = aColor;
    vertexDepth = -viewPos.z; // 深度用于线宽调整
}
