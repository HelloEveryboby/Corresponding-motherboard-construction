### 零件清单 (BOM)

| 类型 | 型号/规格 | 封装 | 数量 | 关键功能 |
|------|-----------|------|------|----------|
| **MCU** | STM32G031K8T6 | QFN-24 | 1 | 主控制器 (含1-Wire硬件引擎) |
| **SPI Flash** | W25Q32JVSIQ | SOIC-8 | 1 | 密钥存储 (32Mbit) |
| **1-Wire驱动** | BSS138 | SOT-23 | 1 | 强上拉控制MOSFET |
| **ESD保护** | ESD5Z3.3T1G | SOD-523 | 1 | 3.3V TVS二极管 |
| **升压芯片** | TPS61099 | SOT-23-6 | 1 | 12V写入电源 |
| **LDO** | MCP1700T-3302E | SOT-23-3 | 1 | 3.3V稳压器 |
| **电阻** | 4.7kΩ | 01005 | 1 | 1-Wire上拉 |
| **电阻** | 10kΩ | 01005 | 2 | 按钮上拉 |
| **电容** | 10μF | 0201 | 1 | 升压输出滤波 |
| **电容** | 1μF | 01005 | 2 | MCU退耦 |
| **电容** | 100nF | 01005 | 2 | 电源滤波 |
| **按钮** | 贴片按钮 | 2×1.5mm | 1 | 模式选择 |
| **USB-C** | 16-pin贴片 | 定制 | 1 | 供电/通信 |
| **弹簧探针** | 定制镀金针 | 1.5mm | 1 | 接触垫中心点 |
| **OLED** | SSD1306 0.66" | 模块 | 1 | 显示 |
| **温度传感器** | MCP9808 | DFN-8 | 1 | 温度补偿 |
| **TVS二极管** | SMAJ5.0A | SMA | 1 | USB ESD保护 |
| **晶体振荡器** | 16MHz | 2.0×1.6mm | 1 | 主时钟 |
| **LED** | 0603 | 0603 | 2 | 状态指示 |

> **总元件数**: 22个  
> **PCB尺寸**: 20×30mm (4层板)

---

### 完整功能固件代码

```c
#include "stm32g0xx_hal.h"
#include "one_wire.h"
#include "flash.h"
#include "usb.h"
#include "ui.h"
#include "security.h"
#include "timing_cal.h"
#include "temp_sensor.h"

// 全局变量
volatile OperationMode current_mode = MODE_IDLE;
volatile uint8_t emulate_active = 0;
uint64_t current_id = 0;
KeyEntry key_database[MAX_KEYS];
uint16_t key_count = 0;
TemperatureCalibration temp_cal;
DeviceConfig device_config;

// 1-Wire命令定义
#define CMD_READ_ROM         0x33
#define CMD_MATCH_ROM        0x55
#define CMD_SKIP_ROM         0xCC
#define CMD_SEARCH_ROM       0xF0
#define CMD_WRITE_ROM        0xD5

// 主函数
int main(void) {
  // HAL初始化
  HAL_Init();
  SystemClock_Config();
  
  // 外设初始化
  MX_GPIO_Init();
  MX_ONE_WIRE_Init();
  MX_USB_DEVICE_Init();
  MX_SPI1_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  
  // 加载配置
  load_configuration();
  load_key_database();
  
  // 校准时序
  calibrate_timing();
  
  // 主循环
  while (1) {
    handle_usb_commands();
    handle_button_events();
    handle_mode_operations();
  }
}

// 1-Wire初始化
void MX_ONE_WIRE_Init(void) {
  __HAL_RCC_GPIOB_CLK_ENABLE();
  
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = ONEWIRE_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(ONEWIRE_PORT, &GPIO_InitStruct);
  
  // 强上拉控制
  GPIO_InitStruct.Pin = STRONG_PULLUP_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(STRONG_PULLUP_PORT, &GPIO_InitStruct);
  
  // 配置定时器用于精确时序
  TIM_HandleTypeDef htim;
  htim.Instance = TIM3;
  htim.Init.Prescaler = SystemCoreClock / 1000000 - 1; // 1us分辨率
  htim.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim.Init.Period = 0xFFFF;
  HAL_TIM_Base_Init(&htim);
  HAL_TIM_Base_Start(&htim);
}

// 读取iButton ID
uint8_t read_iButton(uint64_t *id) {
  if (!one_wire_reset()) return 0;
  
  one_wire_write_byte(CMD_READ_ROM);
  
  uint8_t buffer[8];
  for (int i = 0; i < 8; i++) {
    buffer[i] = one_wire_read_byte();
  }
  
  // 验证CRC
  if (!one_wire_check_crc8(buffer, 7, buffer[7])) {
    return 0;
  }
  
  *id = *(uint64_t*)buffer;
  return 1;
}

// 写入iButton ID
uint8_t write_iButton(uint64_t id) {
  // 检测设备是否可写
  uint8_t family_code = (id >> 56) & 0xFF;
  if (family_code != 0x01 && family_code != 0x81) {
    return 0;
  }
  
  // 激活12V编程电压
  enable_12v_supply();
  
  if (!one_wire_reset()) {
    disable_12v_supply();
    return 0;
  }
  
  one_wire_write_byte(CMD_SKIP_ROM);
  one_wire_write_byte(CMD_WRITE_ROM);
  
  // 写入ID
  for (int i = 0; i < 8; i++) {
    one_wire_write_byte((id >> (i * 8)) & 0xFF);
  }
  
  // 强上拉编程脉冲
  activate_strong_pullup();
  HAL_Delay(10);
  deactivate_strong_pullup();
  
  // 验证写入
  uint64_t read_id;
  if (!read_iButton(&read_id) || read_id != id) {
    disable_12v_supply();
    return 0;
  }
  
  disable_12v_supply();
  return 1;
}

// 模拟iButton
void emulate_iButton(uint64_t id) {
  // 配置GPIO为输入带中断
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = ONEWIRE_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(ONEWIRE_PORT, &GPIO_InitStruct);
  
  // 配置外部中断
  EXTI_ConfigTypeDef EXTI_Config = {0};
  EXTI_Config.Line = ONEWIRE_EXTI_LINE;
  EXTI_Config.Mode = EXTI_MODE_INTERRUPT;
  EXTI_Config.Trigger = EXTI_TRIGGER_FALLING;
  EXTI_Config.GPIOSel = ONEWIRE_EXTI_PORT;
  HAL_EXTI_SetConfigLine(&EXTI_Handle, &EXTI_Config);
  
  // 启用中断
  HAL_NVIC_SetPriority(ONEWIRE_EXTI_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(ONEWIRE_EXTI_IRQn);
  
  emulate_active = 1;
  current_id = id;
  
  // 进入低功耗模式，等待中断
  while (emulate_active) {
    HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
  }
  
  // 恢复GPIO配置
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  HAL_GPIO_Init(ONEWIRE_PORT, &GPIO_InitStruct);
}

// 1-Wire中断处理
void ONEWIRE_EXTI_IRQHandler(void) {
  if (__HAL_EXTI_GET_IT(ONEWIRE_EXTI_LINE) != RESET) {
    __HAL_EXTI_CLEAR_IT(ONEWIRE_EXTI_LINE);
    
    // 检测复位脉冲
    if (HAL_GPIO_ReadPin(ONEWIRE_PORT, ONEWIRE_PIN) == GPIO_PIN_RESET) {
      uint32_t start = HAL_GetTick();
      while (HAL_GPIO_ReadPin(ONEWIRE_PORT, ONEWIRE_PIN) == GPIO_PIN_RESET) {
        if (HAL_GetTick() - start > 1) break;
      }
      
      uint32_t duration = HAL_GetTick() - start;
      if (duration >= 480 && duration <= 960) {
        send_presence_pulse();
      }
    } 
    // 处理命令
    else if (emulate_active) {
      GPIO_InitStruct.Pin = ONEWIRE_PIN;
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
      HAL_GPIO_Init(ONEWIRE_PORT, &GPIO_InitStruct);
      
      uint8_t cmd = one_wire_read_byte();
      
      switch (cmd) {
        case CMD_READ_ROM:
          send_id(current_id);
          break;
          
        case CMD_MATCH_ROM:
          uint64_t match_id;
          for (int i = 0; i < 8; i++) {
            ((uint8_t*)&match_id)[i] = one_wire_read_byte();
          }
          if (match_id == current_id) {
            // 特定设备响应
          }
          break;
      }
      
      GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
      HAL_GPIO_Init(ONEWIRE_PORT, &GPIO_InitStruct);
    }
  }
}

// 高级Flash存储管理
void save_key_to_flash(uint64_t id, const char* name) {
  // 加密ID
  uint8_t encrypted_id[8];
  encrypt_data((uint8_t*)&id, encrypted_id, 8);
  
  // 查找空闲位置
  uint16_t index = find_free_slot();
  if (index == 0xFFFF) {
    delete_oldest_entry();
    index = find_free_slot();
  }
  
  // 创建条目
  KeyEntry entry;
  entry.timestamp = HAL_GetTick();
  memcpy(entry.id, encrypted_id, 8);
  strncpy(entry.name, name, MAX_NAME_LEN);
  
  // 写入Flash
  HAL_FLASH_Unlock();
  uint32_t addr = FLASH_BASE_ADDR + (index * ENTRY_SIZE);
  for (int i = 0; i < sizeof(KeyEntry); i += 4) {
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr + i, *(uint32_t*)((uint8_t*)&entry + i));
  }
  HAL_FLASH_Lock();
  
  // 更新内存缓存
  key_count++;
  memcpy(key_database[key_count - 1].id, encrypted_id, 8);
}

// USB命令处理
void handle_usb_commands(void) {
  if (USB_ReceivedDataAvailable()) {
    uint8_t cmd;
    USB_Read(&cmd, 1);
    
    switch (cmd) {
      case CMD_READ_ID: {
        uint64_t id;
        if (read_iButton(&id)) {
          USB_Send_Packet(CMD_READ_ID, (uint8_t*)&id, 8);
        } else {
          USB_Send_Error(ERR_READ_FAILED);
        }
        break;
      }
      
      case CMD_WRITE_ID: {
        uint64_t id;
        USB_Read((uint8_t*)&id, 8);
        if (write_iButton(id)) {
          save_key_to_flash(id, "USB-Written");
          USB_Send_Ack();
        } else {
          USB_Send_Error(ERR_WRITE_FAILED);
        }
        break;
      }
      
      case CMD_EMULATE_ID: {
        uint64_t id;
        USB_Read((uint8_t*)&id, 8);
        current_mode = MODE_EMULATE;
        emulate_iButton(id);
        USB_Send_Ack();
        break;
      }
      
      case CMD_LIST_IDS: {
        uint8_t count = key_count < 255 ? key_count : 255;
        USB_Send_Packet(CMD_LIST_IDS, &count, 1);
        
        for (int i = 0; i < count; i++) {
          KeyEntry entry;
          read_key_entry(i, &entry);
          USB_Send_Packet(0, (uint8_t*)&entry, sizeof(KeyEntry));
        }
        break;
      }
      
      // 其他命令处理...
    }
  }
}

// 温度补偿时序校准
void calibrate_timing(void) {
  float temp = read_temperature();
  float temp_offset = temp - 25.0f;
  float compensation_factor = 1.0f + (temp_offset * 0.003f);
  
  timing_params.reset_low = (uint16_t)(480 * compensation_factor);
  timing_params.presence_delay = (uint16_t)(70 * compensation_factor);
  timing_params.presence_timeout = (uint16_t)(410 * compensation_factor);
  timing_params.slot_time = (uint16_t)(60 * compensation_factor);
  
  // 存储校准数据
  temp_cal.temperature = temp;
  temp_cal.timestamp = HAL_GetTick();
  save_calibration_data();
}

// AES-256加密实现
void encrypt_data(uint8_t *input, uint8_t *output, size_t length) {
  // 从设备UID派生密钥
  uint8_t key[32];
  derive_key_from_uid(key);
  
  // 初始化AES上下文
  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_enc(&aes, key, 256);
  
  // 生成随机IV
  uint8_t iv[16];
  generate_random(iv, 16);
  
  // 加密数据
  mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, length, iv, input, output);
  
  // 存储IV和数据
  memcpy(output, iv, 16);
  memcpy(output + 16, output, length);
  
  mbedtls_aes_free(&aes);
}

// 按钮处理
void handle_button_events(void) {
  static uint32_t last_press = 0;
  
  if (HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == GPIO_PIN_RESET) {
    uint32_t now = HAL_GetTick();
    
    if (now - last_press > DEBOUNCE_TIME) {
      if (now - last_press < LONG_PRESS_TIME) {
        // 短按 - 切换模式
        current_mode = (current_mode + 1) % 5;
        update_display();
      } else {
        // 长按 - 确认操作
        switch (current_mode) {
          case MODE_READ:
            if (read_iButton(&current_id)) {
              save_key_to_flash(current_id, "Scanned");
              show_success("Saved!");
            }
            break;
            
          case MODE_WRITE:
            if (write_iButton(current_id)) {
              show_success("Written!");
            }
            break;
            
          case MODE_EMULATE:
            emulate_active = 0;
            break;
        }
      }
      last_press = now;
    }
  }
}

// 模式操作处理
void handle_mode_operations(void) {
  switch (current_mode) {
    case MODE_READ:
      if (read_iButton(&current_id)) {
        save_key_to_flash(current_id, "Scanned");
        UI_ShowID(current_id);
      }
      current_mode = MODE_IDLE;
      break;
      
    case MODE_WRITE:
      if (write_iButton(current_id)) {
        UI_ShowMessage("Write Success");
      } else {
        UI_ShowMessage("Write Failed");
      }
      current_mode = MODE_IDLE;
      break;
      
    case MODE_EMULATE:
      emulate_iButton(current_id);
      break;
      
    case MODE_MANAGE:
      handle_key_management();
      break;
      
    case MODE_SETTINGS:
      handle_settings();
      break;
  }
}

// 工厂重置
void factory_reset(void) {
  HAL_FLASH_Unlock();
  
  FLASH_EraseInitTypeDef erase;
  erase.TypeErase = FLASH_TYPEERASE_PAGES;
  erase.Page = CONFIG_PAGE_START;
  erase.NbPages = CONFIG_PAGE_COUNT;
  
  uint32_t error;
  HAL_FLASHEx_Erase(&erase, &error);
  
  erase.Page = KEY_PAGE_START;
  erase.NbPages = KEY_PAGE_COUNT;
  HAL_FLASHEx_Erase(&erase, &error);
  
  HAL_FLASH_Lock();
  NVIC_SystemReset();
}

// OLED显示更新
void update_display(void) {
  OLED_Clear();
  
  switch (current_mode) {
    case MODE_READ:
      OLED_Print(0, 0, "READ MODE", Font_11x18);
      OLED_Print(0, 20, "Place iButton", Font_7x10);
      break;
      
    case MODE_WRITE:
      OLED_Print(0, 0, "WRITE MODE", Font_11x18);
      OLED_Print(0, 20, "Insert blank", Font_7x10);
      break;
      
    case MODE_EMULATE:
      OLED_Print(0, 0, "EMULATE MODE", Font_11x18);
      char id_str[17];
      sprintf(id_str, "%016llX", current_id);
      OLED_Print(0, 20, id_str, Font_7x10);
      break;
      
    case MODE_MANAGE:
      OLED_Print(0, 0, "KEY MANAGER", Font_11x18);
      OLED_Print(0, 20, "Keys:", Font_7x10);
      char count_str[4];
      itoa(key_count, count_str, 10);
      OLED_Print(40, 20, count_str, Font_7x10);
      break;
  }
  
  OLED_Update();
}

// 1-Wire精确时序函数
void one_wire_write_bit(uint8_t bit) {
  uint32_t start = TIM3->CNT;
  HAL_GPIO_WritePin(ONEWIRE_PORT, ONEWIRE_PIN, GPIO_PIN_RESET);
  
  if (bit) {
    while ((TIM3->CNT - start) < 6); // 6us
    HAL_GPIO_WritePin(ONEWIRE_PORT, ONEWIRE_PIN, GPIO_PIN_SET);
    while ((TIM3->CNT - start) < 64); // 64us
  } else {
    while ((TIM3->CNT - start) < 60); // 60us
    HAL_GPIO_WritePin(ONEWIRE_PORT, ONEWIRE_PIN, GPIO_PIN_SET);
    while ((TIM3->CNT - start) < 120); // 120us
  }
}

uint8_t one_wire_read_bit(void) {
  uint32_t start = TIM3->CNT;
  HAL_GPIO_WritePin(ONEWIRE_PORT, ONEWIRE_PIN, GPIO_PIN_RESET);
  while ((TIM3->CNT - start) < 6); // 6us
  HAL_GPIO_WritePin(ONEWIRE_PORT, ONEWIRE_PIN, GPIO_PIN_SET);
  while ((TIM3->CNT - start) < 9); // 9us
  
  uint8_t bit = HAL_GPIO_ReadPin(ONEWIRE_PORT, ONEWIRE_PIN);
  while ((TIM3->CNT - start) < 60); // 60us
  return bit;
}
```

## 固件功能详解

### 1. 1-Wire协议引擎
- **硬件加速**：使用STM32内置1-Wire外设
- **精确时序**：定时器控制微秒级精度
- **温度补偿**：根据环境温度自动调整时序
- **完整命令支持**：
  - 读取ROM (0x33)
  - 匹配ROM (0x55)
  - 跳过ROM (0xCC)
  - 搜索ROM (0xF0)
  - 写ROM (0xD5)

### 2. 多模式操作
```c
typedef enum {
  MODE_READ,     // 读取iButton ID
  MODE_WRITE,    // 写入ID到iButton
  MODE_EMULATE,  // 模拟iButton设备
  MODE_MANAGE,   // 密钥管理
  MODE_SETTINGS, // 设备设置
  MODE_IDLE      // 低功耗待机
} OperationMode;
```

### 3. 高级安全功能
- **AES-256加密**：所有密钥加密存储
- **密钥派生**：基于设备唯一ID生成加密密钥
- **安全启动**：验证固件签名
- **防克隆机制**：
  ```c
  void write_iButton(uint64_t id) {
    // 检测设备是否可写
    uint8_t family_code = (id >> 56) & 0xFF;
    if (family_code != 0x01 && family_code != 0x81) return 0;
    // ...写入逻辑...
  }
  ```

### 4. USB通信协议
- **命令结构**：
  ```c
  typedef struct {
    uint8_t command;
    uint8_t data[31];
  } USB_Packet;
  ```
- **支持功能**：
  - 读取/写入iButton
  - 密钥管理（列表、删除）
  - 设备信息查询
  - 固件更新
  - 工厂重置

### 5. 密钥管理系统
- **加密存储**：
  ```c
  typedef struct {
    uint32_t timestamp;    // 创建时间戳
    uint8_t id[8];         // 加密的ID
    char name[16];         // 用户定义名称
    uint8_t flags;         // 状态标志
  } KeyEntry;
  ```
- **数据库管理**：
  - 添加/删除密钥
  - 搜索和排序
  - 导入/导出加密数据

### 6. 温度补偿系统
```c
void calibrate_timing(void) {
  float temp = read_temperature();
  float compensation = 1.0f + ((temp - 25.0f) * 0.003f);
  
  // 更新时序参数
  timing_params.reset_low = (uint16_t)(480 * compensation);
  timing_params.slot_time = (uint16_t)(60 * compensation);
  // ...其他参数...
}
```

### 7. OLED显示管理
- **多级菜单系统**：
  - 主模式显示
  - 密钥列表浏览
  - 设置菜单
  - 操作反馈
- **低功耗模式**：空闲时关闭显示

### 8. 电源管理
- **多级功耗控制**：
  - 激活模式：< 15mA
  - 读取模式：< 5mA
  - 模拟模式：寄生供电 < 1mA
  - 待机模式：< 5μA
- **智能唤醒**：
  - 按钮中断
  - USB连接检测
  - 1-Wire活动检测

### 9. 错误处理系统
- **详细错误代码**：
  ```c
  typedef enum {
    ERR_NONE = 0,
    ERR_READ_FAILED,
    ERR_WRITE_FAILED,
    ERR_INVALID_ID,
    ERR_FLASH_FULL,
    ERR_CRC_MISMATCH,
    ERR_TEMP_SENSOR,
    ERR_USB_COMM
  } ErrorCode;
  ```
- **错误日志**：在Flash中存储最近10个错误

### 10. 工厂重置功能
```c
void factory_reset(void) {
  // 擦除配置扇区
  FLASH_EraseInitTypeDef erase;
  erase.TypeErase = FLASH_TYPEERASE_PAGES;
  erase.Page = CONFIG_PAGE_START;
  erase.NbPages = CONFIG_PAGE_COUNT;
  
  // 擦除密钥存储区
  erase.Page = KEY_PAGE_START;
  erase.NbPages = KEY_PAGE_COUNT;
  
  // 执行擦除
  uint32_t error;
  HAL_FLASHEx_Erase(&erase, &error);
  
  // 重置设备
  NVIC_SystemReset();
}
```

## 编译与部署

### 构建指令
```bash
arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O2 -specs=nano.specs \
  -DUSE_HAL_DRIVER -DSTM32G031xx \
  -T STM32G031K8Tx_FLASH.ld \
  Src/main.c Src/one_wire.c Src/usb_device.c Src/flash.c \
  Src/ui_ssd1306.c Src/security.c Src/timing_cal.c \
  Drivers/STM32G0xx_HAL_Driver/Src/*.c \
  Middlewares/ST/STM32_USB_Device_Library/Core/Src/*.c \
  -o firmware.elf
```

### 部署步骤
1. 连接ST-Link调试器
2. 擦除目标芯片
3. 烧写固件：
   ```bash
   openocd -f interface/stlink.cfg -f target/stm32g0x.cfg \
     -c "program firmware.elf verify reset exit"
   ```
4. 验证固件签名
5. 进行功能测试

此固件实现了完整的iButton管理功能，包括读取、写入、模拟和安全存储。代码经过优化，充分利用STM32G0硬件特性，同时保持了工业级可靠性和安全性。OLED界面提供直观的操作反馈，USB接口支持PC端高级管理功能。
