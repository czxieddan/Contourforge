#version 330 core

layout (location = 0) in vec2 vertex_pos;  // 顶点位置（2D）
layout (location = 1) in vec2 tex_coord;   // 纹理坐标

out vec2 TexCoord;

uniform mat4 view;
uniform mat4 projection;
uniform float position_x;
uniform float position_y;
uniform float position_z;

void main() {
    // 提取相机的右向量和上向量（Billboard效果）
    vec3 camera_right = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 camera_up = vec3(view[0][1], view[1][1], view[2][1]);
    
    // 标注的3D位置
    vec3 label_position = vec3(position_x, position_y, position_z);
    
    // 缩放因子（根据字体大小调整）
    float scale = 0.01;
    
    // 计算世界空间位置（Billboard变换）
    vec3 world_pos = label_position + 
                     camera_right * vertex_pos.x * scale +
                     camera_up * vertex_pos.y * scale;
    
    // 投影到屏幕空间
    gl_Position = projection * view * vec4(world_pos, 1.0);
    
    TexCoord = tex_coord;
}
