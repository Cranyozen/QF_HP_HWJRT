#include "user_datas.h"

user_datas_t user_datas;

void user_datas_init()
{
    user_datas.pwm_temp_buf = 100;
    user_datas.pwm_temp_mode = 0;
    user_datas.miot_miot_able = 0;
    user_datas.pwm_temp_mode1_time = 10;
    user_datas.ui_oled_light = 127;
    user_datas.adc_adc_max_temp = 255;
    user_datas.adc_hotbed_max_temp = 255;
    user_datas.fan_auto_flg = 1;
    memset(user_datas.blinker_id,0,sizeof(user_datas.blinker_id));
    user_datas.curve_temp_buf[0] = 140;
    user_datas.curve_temp_buf[1] = 90;
    user_datas.curve_temp_buf[2] = 255;
    user_datas.curve_temp_buf[3] = 60;

    
}
