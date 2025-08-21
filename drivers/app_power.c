// app_power.c
void Power_Manage_Sleep(void) {
    static uint32_t last_activity = HAL_GetTick();
    const uint32_t sleep_timeout = 300000; // 5分钟
    
    // 检测活动
    if(Keypad_Scan() != KEY_NONE || Get_Charging_State() == CHARGING_IN_PROGRESS) {
        last_activity = HAL_GetTick();
    }
    
    // 检查超时
    if(HAL_GetTick() - last_activity > sleep_timeout) {
        Power_Enter_Sleep();
    }
}

void Power_Enter_Sleep(void) {
    // 保存当前状态
    Save_Current_State();
    
    // 配置唤醒源（后退按钮和充电状态变化）
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1); // KEY_BACK
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN2); // CHG_STAT
    
    // 设置所有GPIO为模拟输入（最小功耗）
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_All;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    // 保留必要的唤醒引脚
    GPIO_InitStruct.Pin = KEY_BACK_Pin | CHG_STAT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(KEY_BACK_GPIO_Port, &GPIO_InitStruct);
    
    // 进入STOP模式
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
    
    // 唤醒后重新初始化
    SystemClock_Config();
    Peripherals_Reinit();
    App_Restore_State();
}
