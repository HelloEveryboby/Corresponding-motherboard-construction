#include "keyboard.h"
#include "display.h"
#include "keypad.h"

// Flipper Zero风格键盘布局
const char keyboard_lower[4][11] = {
    {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 0},
    {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 0, 0},
    {KEY_ICON_SHIFT, 'z', 'x', 'c', 'v', 'b', 'n', 'm', KEY_ICON_BACKSPACE, 0, 0},
    {KEY_ICON_123, KEY_ICON_SPACE, KEY_ICON_ENTER, 0, 0, 0, 0, 0, 0, 0, 0}
};

const char keyboard_upper[4][11] = {
    {'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', 0},
    {'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 0, 0},
    {KEY_ICON_SHIFT, 'Z', 'X', 'C', 'V', 'B', 'N', 'M', KEY_ICON_BACKSPACE, 0, 0},
    {KEY_ICON_123, KEY_ICON_SPACE, KEY_ICON_ENTER, 0, 0, 0, 0, 0, 0, 0, 0}
};

const char keyboard_numeric[4][11] = {
    {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 0},
    {'!', '@', '#', '$', '%', '^', '&', '*', '(', ')', 0},
    {KEY_ICON_ABC, '-', '+', '=', '[', ']', '{', '}', KEY_ICON_BACKSPACE, 0, 0},
    {KEY_ICON_ALT, ',', '.', '?', ':', ';', '"', '\'', KEY_ICON_ENTER, 0, 0}
};

const char keyboard_special[4][11] = {
    {'_', '/', '|', '\\', '~', '`', '<', '>', 0, 0, 0},
    {KEY_ICON_ABC, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {'α', 'β', 'γ', 'δ', 'ε', '€', '£', '¥', KEY_ICON_BACKSPACE, 0, 0},
    {KEY_ICON_123, KEY_ICON_SPACE, KEY_ICON_ENTER, 0, 0, 0, 0, 0, 0, 0, 0}
};

// 全局键盘状态
static KeyboardState kb_state;

// 初始化键盘
void Keyboard_Init(void) {
    memset(kb_state.input_buffer, 0, sizeof(kb_state.input_buffer));
    kb_state.input_index = 0;
    kb_state.cursor_x = 0;
    kb_state.cursor_y = 0;
    kb_state.layout = LAYOUT_LOWER;
    kb_state.caps_lock = 0;
}

// 检查当前位置是否为空键
static uint8_t Keyboard_IsPositionEmpty(void) {
    const char *key_row = NULL;
    
    switch (kb_state.layout) {
        case LAYOUT_LOWER: key_row = keyboard_lower[kb_state.cursor_y]; break;
        case LAYOUT_UPPER: key_row = keyboard_upper[kb_state.cursor_y]; break;
        case LAYOUT_NUMERIC: key_row = keyboard_numeric[kb_state.cursor_y]; break;
        case LAYOUT_SPECIAL: key_row = keyboard_special[kb_state.cursor_y]; break;
    }
    
    return key_row[kb_state.cursor_x] == 0;
}

// 绘制键盘界面
static void Keyboard_Draw(const char *prompt) {
    Display_Clear();
    
    // 显示提示信息
    if (prompt != NULL) {
        Display_Print(10, 5, prompt, FONT_MEDIUM);
    }
    
    // 显示输入缓冲区
    Display_DrawRect(5, 25, DISPLAY_WIDTH-10, 25, COLOR_WHITE);
    Display_Print(10, 30, kb_state.input_buffer, FONT_MEDIUM);
    
    // 绘制键盘网格
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 11; x++) {
            int pos_x = 5 + x * 20;
            int pos_y = 60 + y * 30;
            
            // 获取当前键值
            char key = 0;
            switch (kb_state.layout) {
                case LAYOUT_LOWER: key = keyboard_lower[y][x]; break;
                case LAYOUT_UPPER: key = keyboard_upper[y][x]; break;
                case LAYOUT_NUMERIC: key = keyboard_numeric[y][x]; break;
                case LAYOUT_SPECIAL: key = keyboard_special[y][x]; break;
            }
            
            if (key == 0) continue; // 跳过空键位
            
            // 绘制键位背景
            uint16_t color = COLOR_DARK_GRAY;
            if (x == kb_state.cursor_x && y == kb_state.cursor_y) {
                color = COLOR_BLUE; // 高亮当前选中的键
            }
            
            Display_FillRect(pos_x, pos_y, 18, 18, color);
            
            // 绘制键位内容
            if (key < 0x10) {
                // 绘制图标
                switch (key) {
                    case KEY_ICON_SHIFT:
                        Display_DrawIcon(pos_x+4, pos_y+4, kb_state.caps_lock ? ICON_SHIFT_LOCKED : ICON_SHIFT);
                        break;
                    case KEY_ICON_BACKSPACE:
                        Display_DrawIcon(pos_x+4, pos_y+4, ICON_BACKSPACE);
                        break;
                    case KEY_ICON_123:
                        Display_DrawIcon(pos_x+4, pos_y+4, ICON_NUMERIC);
                        break;
                    case KEY_ICON_ABC:
                        Display_DrawIcon(pos_x+4, pos_y+4, ICON_ALPHA);
                        break;
                    case KEY_ICON_SPACE:
                        Display_DrawIcon(pos_x+4, pos_y+4, ICON_SPACE);
                        break;
                    case KEY_ICON_ENTER:
                        Display_DrawIcon(pos_x+4, pos_y+4, ICON_ENTER);
                        break;
                    case KEY_ICON_ALT:
                        Display_DrawIcon(pos_x+4, pos_y+4, ICON_ALT);
                        break;
                }
            } else {
                // 绘制字符
                char str[2] = {key, '\0'};
                Display_Print(pos_x+5, pos_y+5, str, FONT_SMALL);
            }
        }
    }
    
    // 布局指示器
    const char* layout_names[] = {"ABC", "ABC", "123", "ALT"};
    Display_Print(5, DISPLAY_HEIGHT-15, layout_names[kb_state.layout], FONT_SMALL);
    
    // 操作提示
    Display_Print(50, DISPLAY_HEIGHT-15, "OK:选择  BACK:退出", FONT_SMALL);
}

// 添加字符到输入缓冲区
static void Keyboard_Add_Char(char c) {
    if (kb_state.input_index < sizeof(kb_state.input_buffer)) {
        kb_state.input_buffer[kb_state.input_index++] = c;
        kb_state.input_buffer[kb_state.input_index] = '\0';
    }
}

// 删除最后一个字符
static void Keyboard_Backspace(void) {
    if (kb_state.input_index > 0) {
        kb_state.input_buffer[--kb_state.input_index] = '\0';
    }
}

// 处理键盘输入
static uint8_t Keyboard_Process(KeyCode key) {
    char current_key = 0;
    
    // 获取当前光标位置的键值
    switch (kb_state.layout) {
        case LAYOUT_LOWER: current_key = keyboard_lower[kb_state.cursor_y][kb_state.cursor_x]; break;
        case LAYOUT_UPPER: current_key = keyboard_upper[kb_state.cursor_y][kb_state.cursor_x]; break;
        case LAYOUT_NUMERIC: current_key = keyboard_numeric[kb_state.cursor_y][kb_state.cursor_x]; break;
        case LAYOUT_SPECIAL: current_key = keyboard_special[kb_state.cursor_y][kb_state.cursor_x]; break;
    }
    
    switch (key) {
        case KEY_UP:
            do {
                kb_state.cursor_y = (kb_state.cursor_y - 1 + 4) % 4;
            } while (Keyboard_IsPositionEmpty());
            break;
            
        case KEY_DOWN:
            do {
                kb_state.cursor_y = (kb_state.cursor_y + 1) % 4;
            } while (Keyboard_IsPositionEmpty());
            break;
            
        case KEY_LEFT:
            do {
                kb_state.cursor_x = (kb_state.cursor_x - 1 + 11) % 11;
            } while (Keyboard_IsPositionEmpty());
            break;
            
        case KEY_RIGHT:
            do {
                kb_state.cursor_x = (kb_state.cursor_x + 1) % 11;
            } while (Keyboard_IsPositionEmpty());
            break;
            
        case KEY_OK:
            // 处理功能键
            if (current_key < 0x10) {
                switch (current_key) {
                    case KEY_ICON_SHIFT:
                        if (kb_state.layout == LAYOUT_LOWER || kb_state.layout == LAYOUT_UPPER) {
                            kb_state.caps_lock = !kb_state.caps_lock;
                            kb_state.layout = kb_state.caps_lock ? LAYOUT_UPPER : LAYOUT_LOWER;
                        }
                        break;
                        
                    case KEY_ICON_BACKSPACE:
                        Keyboard_Backspace();
                        break;
                        
                    case KEY_ICON_123:
                        kb_state.layout = LAYOUT_NUMERIC;
                        kb_state.cursor_x = 0;
                        kb_state.cursor_y = 0;
                        while (Keyboard_IsPositionEmpty()) {
                            kb_state.cursor_x++;
                        }
                        break;
                        
                    case KEY_ICON_ABC:
                        kb_state.layout = kb_state.caps_lock ? LAYOUT_UPPER : LAYOUT_LOWER;
                        kb_state.cursor_x = 0;
                        kb_state.cursor_y = 0;
                        while (Keyboard_IsPositionEmpty()) {
                            kb_state.cursor_x++;
                        }
                        break;
                        
                    case KEY_ICON_SPACE:
                        Keyboard_Add_Char(' ');
                        break;
                        
                    case KEY_ICON_ENTER:
                        return 1; // 完成输入
                        
                    case KEY_ICON_ALT:
                        kb_state.layout = LAYOUT_SPECIAL;
                        kb_state.cursor_x = 0;
                        kb_state.cursor_y = 0;
                        while (Keyboard_IsPositionEmpty()) {
                            kb_state.cursor_x++;
                        }
                        break;
                }
            } else {
                // 添加字符
                Keyboard_Add_Char(current_key);
                
                // 自动切换回小写
                if (kb_state.layout == LAYOUT_UPPER && !kb_state.caps_lock) {
                    kb_state.layout = LAYOUT_LOWER;
                }
            }
            break;
            
        case KEY_BACK:
            return 2; // 退出键盘
    }
    
    return 0; // 继续输入
}

// 获取文本输入
uint8_t Get_Text_Input(char *buffer, uint8_t max_len, const char *prompt, const char *default_text) {
    Keyboard_Init();
    
    if (default_text) {
        strncpy(kb_state.input_buffer, default_text, sizeof(kb_state.input_buffer));
        kb_state.input_index = strlen(default_text);
    }
    
    Keyboard_Draw(prompt);
    
    uint8_t result = 0;
    uint8_t done = 0;
    
    while (!done) {
        KeyCode key = Keypad_Scan();
        
        if (key != KEY_NONE) {
            uint8_t action = Keyboard_Process(key);
            
            if (action > 0) {
                Keyboard_Draw(prompt); // 更新显示
            }
            
            if (action == 1) { // 完成
                strncpy(buffer, kb_state.input_buffer, max_len);
                result = 1;
                done = 1;
            } else if (action == 2) { // 退出
                done = 1;
            }
        }
        
        HAL_Delay(10);
    }
    
    return result;
}
