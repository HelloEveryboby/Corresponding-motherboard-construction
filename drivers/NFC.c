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

// 保存卡片到文件（统一目录）
static void Save_Card_To_File(const char* name, CardInfo *card) {
    char full_path[64];
    
    // 统一存储目录
    const char* dir_path = "/NFC_Cards";
    Create_Directory(dir_path);
    
    snprintf(full_path, sizeof(full_path), "%s/%s.card", dir_path, name);
    
    FIL file;
    FRESULT res = f_open(&file, full_path, FA_WRITE | FA_CREATE_ALWAYS);
    if (res == FR_OK) {
        UINT bytes_written;
        
        // 写入卡片信息
        f_write(&file, card, sizeof(CardInfo), &bytes_written);
        
        f_close(&file);
    }
}

// 创建目录（如果不存在）
static void Create_Directory(const char *path) {
    FRESULT res = f_stat(path, NULL);
    if (res == FR_NO_FILE || res == FR_NO_PATH) {
        f_mkdir(path);
    }
}

// 加载卡片列表（从统一目录）
static uint16_t Load_Card_List(CardInfo *cards, uint16_t max_count) {
    DIR dir;
    FILINFO fno;
    FRESULT res;
    uint16_t count = 0;
    
    // 统一目录路径
    const char* dir_path = "/NFC_Cards";
    
    res = f_opendir(&dir, dir_path);
    if (res == FR_OK) {
        while (count < max_count) {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0) break;
            
            // 只处理.card文件
            if (strstr(fno.fname, ".card")) {
                char full_path[128];
                snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, fno.fname);
                
                // 读取卡片信息
                FIL file;
                if (f_open(&file, full_path, FA_READ) == FR_OK) {
                    if (f_read(&file, &cards[count], sizeof(CardInfo), NULL) == FR_OK) {
                        count++;
                    }
                    f_close(&file);
                }
            }
        }
        f_closedir(&dir);
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
    Display_Print(10, DISPLAY_HEIGHT - 30, "A:模拟卡片  B:删除  C:重命名", FONT_SMALL);
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
            
        case KEY_C: // 重命名卡片
            Rename_Card(&current_card);
            break;
            
        case KEY_BACK:
            current_mode = MODE_CARD_MANAGE;
            Show_Card_List();
            break;
    }
}

// 重命名卡片
static void Rename_Card(CardInfo *card) {
    char new_name[32];
    char prompt[40];
    snprintf(prompt, sizeof(prompt), "重命名卡片:");
    
    // 使用键盘输入新名称
    if (Get_Text_Input(new_name, sizeof(new_name), prompt, card->name)) {
        // 删除旧文件
        char old_path[64];
        snprintf(old_path, sizeof(old_path), "/NFC_Cards/%s.card", card->name);
        f_unlink(old_path);
        
        // 更新卡片名称
        strncpy(card->name, new_name, sizeof(card->name));
        
        // 保存为新文件
        Save_Card_To_File(new_name, card);
        
        Display_Message("卡片已重命名", 2000);
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
    snprintf(filename, sizeof(filename), "/NFC_Cards/%s.card", card->name);
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
            
        default:
            NFC_Emulate(card->uid, card->uid_len);
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
            }
            break;
            
        case KEY_BACK:
            current_mode = MODE_IDLE;
            Display_Clear();
            Display_Print(10, 10, "按OK键唤醒", FONT_MEDIUM);
            break;
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
    }
}
