#version 330 core

in vec3 vertexColor;
in float vertexDepth;
out vec4 FragColor;

uniform vec4 lineColor;
uniform float alpha;
uniform bool useDepthFade;
uniform float depthFadeStart;
uniform float depthFadeEnd;

void main()
{
    // 基础颜色
    vec3 finalColor = (lineColor.a > 0.0) ? lineColor.rgb : vertexColor;
    float finalAlpha = alpha;
    
    // 深度淡化效果（远处的线更透明）
    if (useDepthFade) {
        float fade = smoothstep(depthFadeStart, depthFadeEnd, vertexDepth);
        finalAlpha *= (1.0 - fade);
    }
    
    FragColor = vec4(finalColor, finalAlpha);
}
