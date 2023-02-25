#ifndef _PWM_H_
#define _PWM_H_

#include <Arduino.h>
#include <Ticker.h>
#include "ADC.h"
#include "Tick_IRQ.h"

#define PWM_IO 14
#define FAN_IO 15

#define kp 4.5
#define ki 1.0
#define ki_high 3.0
#define kd 30.0

#define OFF 0
#define ON 1

#define Re_So 0
#define Co_Temp 1

class PWM
{
public:
    PWM();
    void begin();
    void end();
    void temp_set();
    void fan(bool sta);
    bool get_fan_sta();
    void fan_chek();
    bool power = OFF;
    bool sleep_flg = OFF;
    uint8_t percent = 0;
    bool temp_reached_flg = false;
    uint8_t backflow_working_state = 0;
    int16_t backflow_temp_tmp = 0;
    bool fan_state = OFF;

private:
    
    int16_t need_set_temp = 50;
    uint8_t high_time = 0;
    int16_t pwm_buf = 0;
    float pwm_buf_f = 0;
    friend void pwm_irq();
};

extern PWM pwm;

#endif
