#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D text_atlas;
uniform float text_color_r;
uniform float text_color_g;
uniform float text_color_b;
uniform float text_color_a;

void main() {
    // 从纹理图集采样（单通道）
    float alpha = texture(text_atlas, TexCoord).r;
    
    // 应用颜色
    vec4 color = vec4(text_color_r, text_color_g, text_color_b, text_color_a);
    FragColor = vec4(color.rgb, color.a * alpha);
    
    // 丢弃完全透明的片段
    if (FragColor.a < 0.01) {
        discard;
    }
}
