我将把系统拆分为两个独立的文件：

1. **键盘系统模块** (`keyboard.c` 和 `keyboard.h`)
2. **卡片管理系统** (`card_system.c` 和 `card_system.h`)

下面是完整的实现：

## 文件1: 键盘系统模块

### keyboard.h
```c
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "stm32f4xx_hal.h"
#include "display.h"
#include "keypad.h"

// 键盘布局定义
typedef enum {
    LAYOUT_LOWER,       // 小写字母布局
    LAYOUT_UPPER,       // 大写字母布局
    LAYOUT_NUMERIC,     // 数字和符号布局
    LAYOUT_SPECIAL      // 特殊字符布局
} KeyboardLayout;

// 键盘状态结构
typedef struct {
    char input_buffer[32];      // 输入缓冲区
    uint8_t input_index;        // 当前输入位置
    KeyboardLayout layout;      // 当前键盘布局
    uint8_t cursor_x;           // 键盘X位置 (0-10)
    uint8_t cursor_y;           // 键盘Y位置 (0-3)
    uint8_t caps_lock;          // 大写锁定
} KeyboardState;

// 图标定义
#define KEY_ICON_SHIFT     0x01
#define KEY_ICON_BACKSPACE 0x02
#define KEY_ICON_123       0x03
#define KEY_ICON_ABC       0x04
#define KEY_ICON_SPACE     0x05
#define KEY_ICON_ENTER     0x06
#define KEY_ICON_ALT       0x07

// 键盘初始化
void Keyboard_Init(void);

// 获取文本输入
uint8_t Get_Text_Input(char *buffer, uint8_t max_len, const char *prompt, const char *default_text);

#endif // KEYBOARD_H
```

### keyboard.c
```c
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
```

## 文件2: 卡片管理系统

### card_system.h
```c
#ifndef CARD_SYSTEM_H
#define CARD_SYSTEM_H

#include "stm32f4xx_hal.h"
#include "display.h"
#include "keypad.h"
#include "storage.h"
#include "rtc.h"
#include "nfc.h"
#include "rfid.h"
#include "keyboard.h"

// 卡片类型定义
typedef enum {
    CARD_UNKNOWN = 0,
    CARD_MEAL,          // 饭卡
    CARD_ACCESS,        // 门禁卡
    CARD_TRANSPORT,     // 公交卡
    CARD_MEMBER,        // 会员卡
    CARD_ID,            // 身份证
    CARD_CREDIT,        // 信用卡
    CARD_OTHER          // 其他卡片
} CardType;

// 卡片信息结构
typedef struct {
    CardType type;          // 卡片类型
    char name[32];          // 自定义名称
    uint8_t uid[8];         // 卡片UID
    uint16_t uid_len;       // UID长度
    uint8_t data[256];      // 卡片原始数据
    uint16_t data_len;      // 数据长度
    uint32_t timestamp;     // 读取时间戳
} CardInfo;

// 应用模式定义
typedef enum {
    MODE_IDLE,          // 空闲模式
    MODE_MAIN_MENU,     // 主菜单
    MODE_CARD_READ,     // 读卡模式
    MODE_CARD_MANAGE,   // 卡片管理
    MODE_CARD_INFO      // 卡片信息查看
} AppMode;

// 卡片系统初始化
void CardSystem_Init(void);

// 卡片系统主循环
void CardSystem_Main_Loop(void);

#endif // CARD_SYSTEM_H
```

### card_system.c
```c
#include "card_system.h"
#include "display.h"
#include "keypad.h"
#include "storage.h"
#include "rtc.h"
#include "nfc.h"
#include "keyboard.h"

// 卡片类型名称
const char* card_type_names[] = {
    "未知卡片",
    "饭卡",
    "门禁卡",
    "公交卡",
    "会员卡",
    "身份证",
    "信用卡",
    "其他卡片"
};

// 全局状态
static AppMode current_mode = MODE_IDLE;
static uint8_t menu_selection = 0;
static uint8_t card_data_ready = 0;
static CardInfo current_card;
static uint8_t card_list_selection = 0;
static uint16_t card_count = 0;

// 卡片系统初始化
void CardSystem_Init(void) {
    current_mode = MODE_IDLE;
    menu_selection = 0;
    card_data_ready = 0;
    card_list_selection = 0;
    card_count = 0;
    memset(&current_card, 0, sizeof(CardInfo));
}

// 检测卡片类型
static CardType Detect_Card_Type(uint8_t* data, uint16_t len) {
    // 简单检测逻辑 - 实际应用中应更复杂
    if (len < 4) return CARD_UNKNOWN;
    
    // Mifare Classic 卡
    if (data[0] == 0x04 && data[1] == 0x00) {
        return CARD_ACCESS;
    }
    
    // FeliCa 卡 (公交卡常见)
    if (data[0] == 0x00 && data[1] == 0x0F) {
        return CARD_TRANSPORT;
    }
    
    // ISO14443-4 卡 (身份证、信用卡)
    if (data[0] == 0x02 && data[1] == 0x00) {
        return CARD_ID;
    }
    
    // 其他检测逻辑...
    
    // 默认返回饭卡
    return CARD_MEAL;
}

// 保存卡片数据
static void Save_Card_Data(CardInfo *card) {
    // 生成默认名称
    char default_name[32];
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;
    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
    
    snprintf(default_name, sizeof(default_name), "%s_%02d%02d%02d", 
             card_type_names[card->type], date.Date, time.Hours, time.Minutes);
    
    // 获取用户输入的文件名
    char filename[32];
    char prompt[40];
    snprintf(prompt, sizeof(prompt), "输入名称 (%s):", card_type_names[card->type]);
    
    if (Get_Text_Input(filename, sizeof(filename), prompt, default_name)) {
        // 保存卡片数据
        Save_Card_To_File(filename, card);
        Display_Message("卡片已保存!", 2000);
    } else {
        Display_Message("已取消保存", 1000);
    }
}

// 保存卡片到文件
static void Save_Card_To_File(const char* name, CardInfo *card) {
    char full_path[64];
    
    // 按类型创建目录
    const char* dir_path = Get_Card_Directory(card->type);
    Create_Directory(dir_path);
    
    snprintf(full_path, sizeof(full_path), "%s/%s.card", dir_path, name);
    
    FIL file;
    FRESULT res = f_open(&file, full_path, FA_WRITE | FA_CREATE_ALWAYS);
    if (res == FR_OK) {
        UINT bytes_written;
        
        // 写入卡片信息头
        f_write(&file, card, sizeof(CardInfo), &bytes_written);
        
        f_close(&file);
    }
}

// 获取卡片类型对应的目录
static const char* Get_Card_Directory(CardType type) {
    switch (type) {
        case CARD_MEAL:     return "/Cards/Meal";
        case CARD_ACCESS:   return "/Cards/Access";
        case CARD_TRANSPORT: return "/Cards/Transport";
        case CARD_MEMBER:   return "/Cards/Member";
        case CARD_ID:       return "/Cards/ID";
        case CARD_CREDIT:   return "/Cards/Credit";
        default:            return "/Cards/Other";
    }
}

// 创建目录（如果不存在）
static void Create_Directory(const char *path) {
    FRESULT res = f_stat(path, NULL);
    if (res == FR_NO_FILE || res == FR_NO_PATH) {
        f_mkdir(path);
    }
}

// 加载卡片列表
static uint16_t Load_Card_List(CardInfo *cards, uint16_t max_count) {
    DIR dir;
    FILINFO fno;
    FRESULT res;
    uint16_t count = 0;
    
    // 遍历所有卡片目录
    const char* dirs[] = {
        "/Cards/Meal",
        "/Cards/Access",
        "/Cards/Transport",
        "/Cards/Member",
        "/Cards/ID",
        "/Cards/Credit",
        "/Cards/Other"
    };
    
    for (int i = 0; i < sizeof(dirs)/sizeof(dirs[0]); i++) {
        res = f_opendir(&dir, dirs[i]);
        if (res == FR_OK) {
            while (1) {
                res = f_readdir(&dir, &fno);
                if (res != FR_OK || fno.fname[0] == 0) break;
                
                // 只处理.card文件
                if (strstr(fno.fname, ".card")) {
                    char full_path[128];
                    snprintf(full_path, sizeof(full_path), "%s/%s", dirs[i], fno.fname);
                    
                    // 读取卡片信息
                    FIL file;
                    if (f_open(&file, full_path, FA_READ) == FR_OK) {
                        if (f_read(&file, &cards[count], sizeof(CardInfo), NULL) == FR_OK) {
                            count++;
                            if (count >= max_count) break;
                        }
                        f_close(&file);
                    }
                }
            }
            f_closedir(&dir);
        }
    }
    
    return count;
}

// 显示卡片列表
static void Show_Card_List(void) {
    CardInfo cards[50];
    card_count = Load_Card_List(cards, 50);
    
    Display_Clear();
    Display_Print(10, 5, "卡片列表", FONT_LARGE);
    
    if (card_count == 0) {
        Display_Print(10, 30, "无保存的卡片", FONT_MEDIUM);
        Display_Print(10, 60, "按BACK返回", FONT_SMALL);
        return;
    }
    
    // 显示最多5个卡片项
    uint8_t start_index = (card_list_selection / 5) * 5;
    uint8_t end_index = (start_index + 5 < card_count) ? start_index + 5 : card_count;
    
    for (int i = start_index; i < end_index; i++) {
        uint8_t y_pos = 30 + (i % 5) * 20;
        
        if (i == card_list_selection) {
            Display_DrawRect(5, y_pos - 2, DISPLAY_WIDTH - 10, 18, COLOR_BLUE);
        }
        
        char line[40];
        snprintf(line, sizeof(line), "%s: %s", card_type_names[cards[i].type], cards[i].name);
        Display_Print(10, y_pos, line, FONT_SMALL);
    }
    
    // 翻页指示器
    if (card_count > 5) {
        char page_info[20];
        snprintf(page_info, sizeof(page_info), "第%d/%d页", 
                (start_index / 5) + 1, (card_count + 4) / 5);
        Display_Print(DISPLAY_WIDTH - 60, DISPLAY_HEIGHT - 15, page_info, FONT_SMALL);
    }
    
    // 操作提示
    Display_Print(5, DISPLAY_HEIGHT - 15, "OK:选择  BACK:返回", FONT_SMALL);
}

// 处理卡片列表输入
static void Handle_Card_List(KeyCode key) {
    switch (key) {
        case KEY_UP:
            if (card_list_selection > 0) {
                card_list_selection--;
                Show_Card_List();
            }
            break;
            
        case KEY_DOWN:
            if (card_list_selection < card_count - 1) {
                card_list_selection++;
                Show_Card_List();
            }
            break;
            
        case KEY_OK:
            if (card_count > 0) {
                // 加载选中的卡片
                CardInfo cards[50];
                Load_Card_List(cards, 50);
                current_card = cards[card_list_selection];
                current_mode = MODE_CARD_INFO;
                Show_Card_Info();
            }
            break;
            
        case KEY_BACK:
            current_mode = MODE_MAIN_MENU;
            Show_Main_Menu();
            break;
    }
}

// 显示卡片详细信息
static void Show_Card_Info(void) {
    Display_Clear();
    
    // 显示卡片类型和名称
    char title[50];
    snprintf(title, sizeof(title), "%s: %s", card_type_names[current_card.type], current_card.name);
    Display_Print(10, 5, title, FONT_LARGE);
    
    // 显示UID
    char uid_str[32] = "";
    for (int i = 0; i < current_card.uid_len; i++) {
        char byte_str[4];
        snprintf(byte_str, sizeof(byte_str), "%02X ", current_card.uid[i]);
        strcat(uid_str, byte_str);
    }
    
    Display_Print(10, 30, "UID:", FONT_MEDIUM);
    Display_Print(50, 30, uid_str, FONT_MEDIUM);
    
    // 显示读取时间
    RTC_DateTypeDef date;
    RTC_TimeTypeDef time;
    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
    
    char time_str[30];
    snprintf(time_str, sizeof(time_str), "读取时间: %04d-%02d-%02d %02d:%02d",
             2000 + date.Year, date.Month, date.Date, 
             time.Hours, time.Minutes);
    Display_Print(10, 50, time_str, FONT_SMALL);
    
    // 显示数据长度
    char data_len_str[20];
    snprintf(data_len_str, sizeof(data_len_str), "数据长度: %d字节", current_card.data_len);
    Display_Print(10, 70, data_len_str, FONT_SMALL);
    
    // 操作选项
    Display_Print(10, DISPLAY_HEIGHT - 30, "A:模拟卡片  B:删除卡片", FONT_SMALL);
    Display_Print(10, DISPLAY_HEIGHT - 15, "BACK:返回", FONT_SMALL);
}

// 处理卡片信息界面输入
static void Handle_Card_Info(KeyCode key) {
    switch (key) {
        case KEY_A: // 模拟卡片
            Emulate_Card(&current_card);
            Display_Message("模拟中... 按BACK停止", 0);
            break;
            
        case KEY_B: // 删除卡片
            if (Confirm_Action("确认删除卡片?")) {
                Delete_Card(&current_card);
                Display_Message("卡片已删除", 2000);
                current_mode = MODE_CARD_MANAGE;
                Show_Card_List();
            }
            break;
            
        case KEY_BACK:
            current_mode = MODE_CARD_MANAGE;
            Show_Card_List();
            break;
    }
}

// 确认操作
static uint8_t Confirm_Action(const char* message) {
    Display_Clear();
    Display_Print(10, 20, message, FONT_MEDIUM);
    Display_Print(10, 50, "OK:确认  BACK:取消", FONT_MEDIUM);
    
    while (1) {
        KeyCode key = Keypad_Scan();
        if (key == KEY_OK) return 1;
        if (key == KEY_BACK) return 0;
        HAL_Delay(10);
    }
}

// 删除卡片
static void Delete_Card(CardInfo *card) {
    char filename[64];
    snprintf(filename, sizeof(filename), "%s/%s.card", 
             Get_Card_Directory(card->type), card->name);
    
    f_unlink(filename);
}

// 模拟卡片
static void Emulate_Card(CardInfo *card) {
    // 根据卡片类型选择模拟协议
    switch (card->type) {
        case CARD_MEAL:
        case CARD_ACCESS:
            NFC_Emulate(card->uid, card->uid_len);
            break;
            
        case CARD_TRANSPORT:
            FeliCa_Emulate(card->data, card->data_len);
            break;
            
        // 其他卡片类型的模拟...
    }
}

// 主菜单显示
static void Show_Main_Menu(void) {
    Display_Clear();
    Display_Print(10, 10, "多卡片管理系统", FONT_LARGE);
    
    const char *menu_items[] = {
        "读取卡片",
        "管理卡片",
        "模拟卡片",
        "设置"
    };
    
    for (int i = 0; i < 4; i++) {
        if (i == menu_selection) {
            Display_DrawRect(5, 40 + i*30, DISPLAY_WIDTH-10, 25, COLOR_BLUE);
        }
        Display_Print(20, 50 + i*30, menu_items[i], FONT_MEDIUM);
    }
}

// 处理主菜单输入
static void Handle_Main_Menu(KeyCode key) {
    switch (key) {
        case KEY_UP:
            menu_selection = (menu_selection - 1 + 4) % 4;
            Show_Main_Menu();
            break;
            
        case KEY_DOWN:
            menu_selection = (menu_selection + 1) % 4;
            Show_Main_Menu();
            break;
            
        case KEY_OK:
            switch (menu_selection) {
                case 0: // 读取卡片
                    current_mode = MODE_CARD_READ;
                    Start_Card_Read();
                    break;
                    
                case 1: // 管理卡片
                    current_mode = MODE_CARD_MANAGE;
                    card_list_selection = 0;
                    Show_Card_List();
                    break;
                    
                case 2: // 模拟卡片
                    current_mode = MODE_CARD_MANAGE;
                    card_list_selection = 0;
                    Show_Card_List();
                    Display_Message("选择要模拟的卡片", 1000);
                    break;
                    
                // 其他菜单项处理...
            }
            break;
            
        case KEY_BACK:
            current_mode = MODE_IDLE;
            Display_Clear();
            Display_Print(10, 10, "按OK键唤醒", FONT_MEDIUM);
            break;
    }
}

// 进入低功耗模式
static void Enter_Low_Power_Mode(void) {
    // 保存状态
    Save_System_State();
    
    // 配置唤醒源
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
    
    // 设置GPIO状态
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_All;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    // 保留唤醒引脚
    GPIO_InitStruct.Pin = KEY_OK_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(KEY_OK_GPIO_Port, &GPIO_InitStruct);
    
    // 进入STOP模式
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
    
    // 唤醒后重新初始化
    SystemClock_Config();
    Peripherals_Reinit();
    Restore_System_State();
    
    // 更新显示
    if (current_mode == MODE_IDLE) {
        Display_Clear();
        Display_Print(10, 10, "按OK键唤醒", FONT_MEDIUM);
    } else {
        Show_Current_Screen();
    }
}

// NFC读取回调
static void NFC_Callback(uint8_t* uid, uint8_t uid_len, uint8_t* data, uint16_t data_len) {
    memcpy(current_card.uid, uid, uid_len);
    current_card.uid_len = uid_len;
    memcpy(current_card.data, data, data_len);
    current_card.data_len = data_len;
    current_card.timestamp = HAL_GetTick();
    current_card.type = Detect_Card_Type(data, data_len);
    
    card_data_ready = 1;
}

// 卡片系统主循环
void CardSystem_Main_Loop(void) {
    static uint32_t last_activity = HAL_GetTick();
    
    // 检查按键活动
    KeyCode key = Keypad_Scan();
    if (key != KEY_NONE) {
        last_activity = HAL_GetTick();
    }
    
    // 检查低功耗超时
    if (HAL_GetTick() - last_activity > 300000) { // 5分钟
        Enter_Low_Power_Mode();
        last_activity = HAL_GetTick();
    }
    
    // 处理当前模式
    switch (current_mode) {
        case MODE_IDLE:
            if (key == KEY_OK) {
                current_mode = MODE_MAIN_MENU;
                Show_Main_Menu();
            }
            break;
            
        case MODE_MAIN_MENU:
            Handle_Main_Menu(key);
            break;
            
        case MODE_CARD_READ:
            if (card_data_ready) {
                Save_Card_Data(&current_card);
                card_data_ready = 0;
                current_mode = MODE_MAIN_MENU;
                Show_Main_Menu();
            }
            break;
            
        case MODE_CARD_MANAGE:
            Handle_Card_List(key);
            break;
            
        case MODE_CARD_INFO:
            Handle_Card_Info(key);
            break;
            
        // 其他模式处理...
    }
}
```

## 系统集成说明

### 模块职责划分

| 模块 | 职责 | 依赖 |
|------|------|------|
| **键盘系统** | 提供文本输入功能 | 显示驱动、按键驱动 |
| **卡片系统** | 管理多种卡片数据 | 键盘系统、存储系统、NFC驱动 |

### 集成方式

1. **初始化顺序**:
```c
int main(void) {
    // 硬件初始化
    HAL_Init();
    SystemClock_Config();
    Display_Init();
    Keypad_Init();
    Storage_Init();
    RTC_Init();
    NFC_Init(NFC_Callback);
    
    // 系统初始化
    CardSystem_Init();
    
    // 主循环
    while (1) {
        CardSystem_Main_Loop();
        HAL_Delay(10);
    }
}
```

2. **键盘系统调用**:
```c
// 在卡片系统中调用键盘
if (Get_Text_Input(filename, sizeof(filename), prompt, default_name)) {
    // 处理用户输入
}
```

### 文件结构

```
项目目录/
├── Drivers/
│   ├── STM32F4xx_HAL_Driver/
│   ├── BSP/
│   └── .../
├── Core/
│   ├── Src/
│   │   ├── main.c
│   │   ├── keyboard.c
│   │   └── card_system.c
│   ├── Inc/
│   │   ├── keyboard.h
│   │   └── card_system.h
│   └── .../
├── Middlewares/
│   ├── FatFs/
│   └── .../
└── .../
```

### 扩展接口

1. **添加新卡片类型**:
```c
// 在card_system.h中添加
typedef enum {
    // ... 现有类型
    CARD_GYM,       // 健身卡
    CARD_LIBRARY    // 图书馆卡
} CardType;

// 在Get_Card_Directory()中添加
static const char* Get_Card_Directory(CardType type) {
    switch (type) {
        // ... 现有类型
        case CARD_GYM:      return "/Cards/Gym";
        case CARD_LIBRARY:  return "/Cards/Library";
        default:            return "/Cards/Other";
    }
}
```

2. **添加新键盘布局**:
```c
// 在keyboard.c中添加
const char keyboard_custom[4][11] = {
    // 自定义布局
};

// 在Keyboard_Draw()中添加
switch (kb_state.layout) {
    // ... 现有布局
    case LAYOUT_CUSTOM: key_row = keyboard_custom[kb_state.cursor_y]; break;
}
```

这种模块化设计使得系统更容易维护和扩展。键盘系统作为独立模块，可以在其他需要文本输入的场合复用，而卡片管理系统则专注于多卡片类型的存储、管理和模拟功能。
