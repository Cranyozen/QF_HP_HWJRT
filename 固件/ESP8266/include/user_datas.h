#pragma once

#include <Arduino.h>

typedef struct
{
    int16_t pwm_temp_buf;
    int16_t pwm_temp_mode1_time;
    int16_t curve_temp_buf[4];
    uint16_t adc_hotbed_max_temp;
    uint16_t adc_adc_max_temp;
    uint8_t ui_oled_light;
    uint8_t fan_auto_flg;
    uint8_t pwm_temp_mode;
    bool miot_miot_able;
    char blinker_id[13];
} user_datas_t;

void user_datas_init();

extern user_datas_t user_datas;
