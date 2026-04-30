/**
 * @file input.c
 * @brief 输入处理模块实现
 */

#include "../../include/contourforge/control.h"
#include "../../include/contourforge/rendering.h"
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief 更新输入状态
 */
void cf_input_update(cf_renderer_t* renderer, cf_input_state_t* input) {
    if (!renderer || !input) {
        return;
    }
    
    // 获取GLFW窗口句柄
    GLFWwindow* window = cf_renderer_get_window(renderer);
    if (!window) {
        return;
    }
    
    // 保存上一帧的鼠标位置
    double prev_x = input->mouse_x;
    double prev_y = input->mouse_y;
    
    // 获取当前鼠标位置
    glfwGetCursorPos(window, &input->mouse_x, &input->mouse_y);
    
    // 计算鼠标偏移
    input->mouse_delta_x = input->mouse_x - prev_x;
    input->mouse_delta_y = input->mouse_y - prev_y;
    
    // 更新鼠标按钮状态
    input->mouse_buttons[CF_MOUSE_BUTTON_LEFT] = 
        glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    input->mouse_buttons[CF_MOUSE_BUTTON_RIGHT] = 
        glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    input->mouse_buttons[CF_MOUSE_BUTTON_MIDDLE] = 
        glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
    
    // 更新键盘状态
    input->keys[CF_KEY_W] = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    input->keys[CF_KEY_A] = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
    input->keys[CF_KEY_S] = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    input->keys[CF_KEY_D] = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
    input->keys[CF_KEY_Z] = glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS;
    input->keys[CF_KEY_Y] = glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS;
    input->keys[CF_KEY_SPACE] = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
    input->keys[CF_KEY_SHIFT] = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                                 glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
    input->keys[CF_KEY_CTRL] = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
                                glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
    input->keys[CF_KEY_ESC] = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
    input->keys[CF_KEY_DELETE] = glfwGetKey(window, GLFW_KEY_DELETE) == GLFW_PRESS;
}
