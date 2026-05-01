/**
 * @file text_renderer.c
 * @brief 文字渲染器实现
 */

#define STB_TRUETYPE_IMPLEMENTATION
#include "../../third_party/stb/stb_truetype.h"

#include <glad/glad.h>
#include "contourforge/rendering.h"
#include "contourforge/core.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ========== 常量定义 ========== */

#define CF_FONT_ATLAS_WIDTH 512
#define CF_FONT_ATLAS_HEIGHT 512
#define CF_FONT_FIRST_CHAR 32
#define CF_FONT_NUM_CHARS 96

/* ========== 字体结构 ========== */

/**
 * @brief 字形信息
 */
typedef struct {
    float x0, y0, x1, y1;  /**< 纹理坐标 */
    float xoff, yoff;      /**< 偏移 */
    float xadvance;        /**< 前进距离 */
} cf_glyph_t;

/**
 * @brief 字体
 */
typedef struct cf_font {
    stbtt_fontinfo info;           /**< stb字体信息 */
    unsigned char* ttf_buffer;     /**< TTF数据缓冲 */
    unsigned char* atlas_bitmap;   /**< 字形纹理图集位图 */
    GLuint texture;                /**< OpenGL纹理 */
    float scale;                   /**< 缩放因子 */
    float size;                    /**< 字体大小 */
    int ascent;                    /**< 上升高度 */
    int descent;                   /**< 下降高度 */
    int line_gap;                  /**< 行间距 */
    cf_glyph_t glyphs[CF_FONT_NUM_CHARS];  /**< 字形数据 */
} cf_font_t;

/* ========== 文字渲染器结构 ========== */

/**
 * @brief 文字渲染器
 */
struct cf_text_renderer {
    cf_font_t* font;       /**< 字体 */
    cf_shader_t* shader;   /**< 着色器 */
    GLuint vao;            /**< VAO */
    GLuint vbo;            /**< VBO */
    bool initialized;      /**< 是否已初始化 */
};

/* ========== 字体API ========== */

/**
 * @brief 加载字体
 */
cf_result_t cf_font_load(const char* font_path, float size, cf_font_t** font) {
    if (font_path == NULL || font == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    /* 分配字体结构 */
    cf_font_t* f = (cf_font_t*)malloc(sizeof(cf_font_t));
    if (f == NULL) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    memset(f, 0, sizeof(cf_font_t));
    
    /* 读取TTF文件 */
    FILE* file = fopen(font_path, "rb");
    if (file == NULL) {
        free(f);
        fprintf(stderr, "Failed to open font file: %s\n", font_path);
        return CF_ERROR_FILE_NOT_FOUND;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    f->ttf_buffer = (unsigned char*)malloc(file_size);
    if (f->ttf_buffer == NULL) {
        fclose(file);
        free(f);
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    fread(f->ttf_buffer, 1, file_size, file);
    fclose(file);
    
    /* 初始化字体 */
    if (!stbtt_InitFont(&f->info, f->ttf_buffer, 0)) {
        free(f->ttf_buffer);
        free(f);
        fprintf(stderr, "Failed to initialize font\n");
        return CF_ERROR_INVALID_FORMAT;
    }
    
    f->size = size;
    f->scale = stbtt_ScaleForPixelHeight(&f->info, size);
    
    /* 获取字体度量 */
    stbtt_GetFontVMetrics(&f->info, &f->ascent, &f->descent, &f->line_gap);
    
    /* 创建字形纹理图集 */
    f->atlas_bitmap = (unsigned char*)calloc(CF_FONT_ATLAS_WIDTH * CF_FONT_ATLAS_HEIGHT, 1);
    if (f->atlas_bitmap == NULL) {
        free(f->ttf_buffer);
        free(f);
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    /* 烘焙字形到纹理图集 */
    int x = 0, y = 0;
    int row_height = 0;
    
    for (int i = 0; i < CF_FONT_NUM_CHARS; i++) {
        int codepoint = CF_FONT_FIRST_CHAR + i;
        
        /* 获取字形边界框 */
        int x0, y0, x1, y1;
        stbtt_GetCodepointBitmapBox(&f->info, codepoint, f->scale, f->scale, &x0, &y0, &x1, &y1);
        
        int glyph_width = x1 - x0;
        int glyph_height = y1 - y0;
        
        /* 换行 */
        if (x + glyph_width >= CF_FONT_ATLAS_WIDTH) {
            x = 0;
            y += row_height + 1;
            row_height = 0;
        }
        
        if (y + glyph_height >= CF_FONT_ATLAS_HEIGHT) {
            fprintf(stderr, "Font atlas too small\n");
            break;
        }
        
        /* 渲染字形到图集 */
        stbtt_MakeCodepointBitmap(&f->info, 
            f->atlas_bitmap + y * CF_FONT_ATLAS_WIDTH + x,
            glyph_width, glyph_height, CF_FONT_ATLAS_WIDTH,
            f->scale, f->scale, codepoint);
        
        /* 保存字形信息 */
        f->glyphs[i].x0 = (float)x / CF_FONT_ATLAS_WIDTH;
        f->glyphs[i].y0 = (float)y / CF_FONT_ATLAS_HEIGHT;
        f->glyphs[i].x1 = (float)(x + glyph_width) / CF_FONT_ATLAS_WIDTH;
        f->glyphs[i].y1 = (float)(y + glyph_height) / CF_FONT_ATLAS_HEIGHT;
        f->glyphs[i].xoff = (float)x0;
        f->glyphs[i].yoff = (float)y0;
        
        int advance, lsb;
        stbtt_GetCodepointHMetrics(&f->info, codepoint, &advance, &lsb);
        f->glyphs[i].xadvance = (float)advance * f->scale;
        
        x += glyph_width + 1;
        if (glyph_height > row_height) {
            row_height = glyph_height;
        }
    }
    
    /* 创建OpenGL纹理 */
    glGenTextures(1, &f->texture);
    glBindTexture(GL_TEXTURE_2D, f->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, CF_FONT_ATLAS_WIDTH, CF_FONT_ATLAS_HEIGHT,
                 0, GL_RED, GL_UNSIGNED_BYTE, f->atlas_bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    *font = f;
    return CF_SUCCESS;
}

/**
 * @brief 销毁字体
 */
void cf_font_destroy(cf_font_t* font) {
    if (font == NULL) {
        return;
    }
    
    if (font->texture != 0) {
        glDeleteTextures(1, &font->texture);
    }
    
    free(font->atlas_bitmap);
    free(font->ttf_buffer);
    free(font);
}

/* ========== 文字渲染器API ========== */

/**
 * @brief 创建文字渲染器
 */
cf_result_t cf_text_renderer_create(
    cf_font_t* font,
    cf_shader_t* shader,
    cf_text_renderer_t** renderer
) {
    if (font == NULL || shader == NULL || renderer == NULL) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    cf_text_renderer_t* r = (cf_text_renderer_t*)malloc(sizeof(cf_text_renderer_t));
    if (r == NULL) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    memset(r, 0, sizeof(cf_text_renderer_t));
    
    r->font = font;
    r->shader = shader;
    
    /* 创建VAO和VBO */
    glGenVertexArrays(1, &r->vao);
    glGenBuffers(1, &r->vbo);
    
    glBindVertexArray(r->vao);
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    
    /* 预分配缓冲区（每个字符6个顶点） */
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4 * 256, NULL, GL_DYNAMIC_DRAW);
    
    /* 位置属性 */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    
    /* 纹理坐标属性 */
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    r->initialized = true;
    *renderer = r;
    return CF_SUCCESS;
}

/**
 * @brief 渲染文字（3D空间，Billboard效果）
 */
cf_result_t cf_text_renderer_render_3d(
    cf_text_renderer_t* renderer,
    const char* text,
    cf_point3_t position,
    cf_color_t color,
    const float* view_matrix,
    const float* projection_matrix
) {
    if (renderer == NULL || text == NULL || !renderer->initialized) {
        return CF_ERROR_INVALID_PARAM;
    }
    
    size_t text_len = strlen(text);
    if (text_len == 0) {
        return CF_SUCCESS;
    }
    
    /* 准备顶点数据 */
    float* vertices = (float*)malloc(sizeof(float) * 6 * 4 * text_len);
    if (vertices == NULL) {
        return CF_ERROR_OUT_OF_MEMORY;
    }
    
    float x = 0.0f;
    float y = 0.0f;
    int vertex_index = 0;
    
    for (size_t i = 0; i < text_len; i++) {
        char c = text[i];
        if (c < CF_FONT_FIRST_CHAR || c >= CF_FONT_FIRST_CHAR + CF_FONT_NUM_CHARS) {
            continue;
        }
        
        int glyph_index = c - CF_FONT_FIRST_CHAR;
        cf_glyph_t* glyph = &renderer->font->glyphs[glyph_index];
        
        float xpos = x + glyph->xoff;
        float ypos = y + glyph->yoff;
        float w = (glyph->x1 - glyph->x0) * CF_FONT_ATLAS_WIDTH;
        float h = (glyph->y1 - glyph->y0) * CF_FONT_ATLAS_HEIGHT;
        
        /* 两个三角形组成一个四边形 */
        float quad[6][4] = {
            { xpos,     ypos + h, glyph->x0, glyph->y1 },
            { xpos,     ypos,     glyph->x0, glyph->y0 },
            { xpos + w, ypos,     glyph->x1, glyph->y0 },
            
            { xpos,     ypos + h, glyph->x0, glyph->y1 },
            { xpos + w, ypos,     glyph->x1, glyph->y0 },
            { xpos + w, ypos + h, glyph->x1, glyph->y1 }
        };
        
        memcpy(vertices + vertex_index, quad, sizeof(quad));
        vertex_index += 24;
        
        x += glyph->xadvance;
    }
    
    /* 上传顶点数据 */
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * vertex_index, vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    free(vertices);
    
    /* 渲染 */
    cf_shader_use(renderer->shader);
    
    /* 设置uniform */
    cf_shader_set_mat4(renderer->shader, "view", view_matrix);
    cf_shader_set_mat4(renderer->shader, "projection", projection_matrix);
    cf_shader_set_float(renderer->shader, "position_x", position.x);
    cf_shader_set_float(renderer->shader, "position_y", position.y);
    cf_shader_set_float(renderer->shader, "position_z", position.z);
    cf_shader_set_float(renderer->shader, "text_color_r", color.r);
    cf_shader_set_float(renderer->shader, "text_color_g", color.g);
    cf_shader_set_float(renderer->shader, "text_color_b", color.b);
    cf_shader_set_float(renderer->shader, "text_color_a", color.a);
    
    /* 绑定纹理 */
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderer->font->texture);
    cf_shader_set_int(renderer->shader, "text_atlas", 0);
    
    /* 启用混合 */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    /* 绘制 */
    glBindVertexArray(renderer->vao);
    glDrawArrays(GL_TRIANGLES, 0, vertex_index / 4);
    glBindVertexArray(0);
    
    glDisable(GL_BLEND);
    
    return CF_SUCCESS;
}

/**
 * @brief 计算文字宽度
 */
float cf_text_renderer_measure_width(cf_text_renderer_t* renderer, const char* text) {
    if (renderer == NULL || text == NULL) {
        return 0.0f;
    }
    
    float width = 0.0f;
    size_t text_len = strlen(text);
    
    for (size_t i = 0; i < text_len; i++) {
        char c = text[i];
        if (c < CF_FONT_FIRST_CHAR || c >= CF_FONT_FIRST_CHAR + CF_FONT_NUM_CHARS) {
            continue;
        }
        
        int glyph_index = c - CF_FONT_FIRST_CHAR;
        width += renderer->font->glyphs[glyph_index].xadvance;
    }
    
    return width;
}

/**
 * @brief 销毁文字渲染器
 */
void cf_text_renderer_destroy(cf_text_renderer_t* renderer) {
    if (renderer == NULL) {
        return;
    }
    
    if (renderer->vao != 0) {
        glDeleteVertexArrays(1, &renderer->vao);
    }
    if (renderer->vbo != 0) {
        glDeleteBuffers(1, &renderer->vbo);
    }
    
    free(renderer);
}
