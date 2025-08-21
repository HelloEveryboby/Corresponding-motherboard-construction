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
