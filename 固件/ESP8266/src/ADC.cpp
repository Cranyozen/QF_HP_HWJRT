#include "ADC.h"
#include <Ticker.h>
#include "EC11.h"

ADC adc;
Ticker adc_max_temp_tic;


ADC::ADC()
{
    pinMode(switch_io, OUTPUT);
    digitalWrite(switch_io, channel_low_temp);
}

void ADC::get()
{
    uint8_t i;
    if (adc_mode_state == channel_low_temp)
    {
        for (i = 0; i < 7; i++)
            adc_buf[i] = adc_buf[i + 1];
        adc_buf[i] = analogRead(A0);
    }
    else
    {
        for (i = 0; i < 7; i++)
            adc_buf_high[i] = adc_buf_high[i + 1];
        adc_buf_high[i] = analogRead(A0);
    }

    set_channel(!adc_mode_state);
}

void ADC::get_voltage()
{
    uint8_t i;
    vol_low = 0;
    for (i = 0; i < 8; i++)
        vol_low += adc_buf[i];
    vol_low >>= 3;
    vol_low = vol_low * 1000 / 1024;

    vol_high = 0;
    for (i = 0; i < 8; i++)
        vol_high += adc_buf_high[i];
    vol_high >>= 3;
    vol_high = vol_high * 1000 / 1024;
}
// 51  150`
void ADC::get_temp_task()
{
    int16_t rt;
    get_voltage();
    int16_t tt;
    int16_t temp_buf = 0;

    double buf;

    rt = vol_low * 1000 / ((3300 - vol_low) / 13);
    now_temp = (100 / (log(rt / 10000.0) / 3950 + 1 / 298.15) - 27315) / 100;
    // Serial.print("low:");
    // Serial.println(now_temp);
    if (now_temp < 151)
        return;

    if (now_temp < 160 && !adc_error)
    {
        if (pwm.power || adc_max_temp_auto_flg == 0)
            temp_buf = now_temp;
    }

    vol_high = vol_high * 1000 / 21;

    buf = vol_high * 1000 / ((3300000 - vol_high) / 13.0);
    tt = (100 / (log(buf / 10000) / 3950 + 1 / 298.15) - 27315) / 100;

    // Serial.print("high:");
    // Serial.println(tt);

    if (temp_buf)
        adc_error = temp_buf - tt;

    if (adc_max_temp_auto_flg)
    {
        int16_t tmp = 150 - adc_error;
        rt = user_datas.adc_hotbed_max_temp - adc_error;
        int16_t tmp1 = user_datas.adc_adc_max_temp - rt;
        buf = double(tmp1) / (double)(user_datas.adc_adc_max_temp - tmp);
        tt = (double)tt - ((double)(tt - tmp) * buf);
    }
    if (adc_max_temp_auto_flg)
        now_temp = tt + adc_error;
    else
        now_temp = tt;
}

void ADC::set_channel(bool channel)
{
    digitalWrite(switch_io, channel);
    adc_mode_state = channel;
}

void adc_max_temp_auto_feed()
{
    static uint16_t last_temp = 0;
    if (last_temp != adc.now_temp)
    {
        last_temp = adc.now_temp;
    }
    else
    {
        adc.adc_max_temp_auto_flg = 1;
        if (adc.now_temp != 38)
        {
            user_datas.adc_adc_max_temp = adc.now_temp;
            ec11.int_close();
            eeprom.write_flg = 2;
            eeprom.write_task();
            ec11.int_work();
        }
        adc_max_temp_tic.detach();
    }
}

bool ADC::adc_max_temp_auto()
{
    return adc_max_temp_auto_flg;
}
