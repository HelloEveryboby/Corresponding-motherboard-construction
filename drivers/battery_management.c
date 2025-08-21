// battery_management.c
#include "battery_management.h"

void Battery_Init(void) {
    // ADC初始化用于电池电压检测
    ADC_ChannelConfTypeDef sConfig = {0};
    hadc.Instance = ADC1;
    hadc.Init.Resolution = ADC_RESOLUTION_12B;
    hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc.Init.ContinuousConvMode = ENABLE;
    hadc.Init.DiscontinuousConvMode = DISABLE;
    hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    HAL_ADC_Init(&hadc);
    
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc, &sConfig);
    
    // 启动ADC
    HAL_ADC_Start(&hadc);
}

float Get_Battery_Voltage(void) {
    // 读取ADC值并转换为电压
    HAL_ADC_PollForConversion(&hadc, 10);
    uint32_t adc_value = HAL_ADC_GetValue(&hadc);
    
    // 分压比例: R1=100k, R2=220k
    return (adc_value * 3.3f / 4096.0f) * (100 + 220) / 100;
}

ChargingState Get_Charging_State(void) {
    GPIO_PinState stat_pin = HAL_GPIO_ReadPin(CHG_STAT_GPIO_Port, CHG_STAT_Pin);
    
    if(stat_pin == GPIO_PIN_RESET) {
        return CHARGING_IN_PROGRESS;
    } else {
        float voltage = Get_Battery_Voltage();
        if(voltage > 4.1f) {
            return CHARGING_COMPLETE;
        } else {
            return CHARGING_ERROR;
        }
    }
}

void Update_Charging_LED(void) {
    static uint32_t last_update = 0;
    if(HAL_GetTick() - last_update < 500) return;
    last_update = HAL_GetTick();
    
    switch(Get_Charging_State()) {
        case CHARGING_IN_PROGRESS:
            HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
            break;
            
        case CHARGING_COMPLETE:
            HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
            break;
            
        case CHARGING_ERROR:
            // 错误状态闪烁
            static uint8_t blink_state = 0;
            blink_state = !blink_state;
            HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, blink_state ? GPIO_PIN_SET : GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
            break;
    }
}
