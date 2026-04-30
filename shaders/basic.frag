#version 330 core

in vec3 vertexColor;
out vec4 FragColor;

uniform vec4 color;
uniform float alpha;

void main()
{
    // 如果提供了uniform颜色，使用它；否则使用顶点颜色
    vec3 finalColor = (color.a > 0.0) ? color.rgb : vertexColor;
    FragColor = vec4(finalColor, alpha);
}
