#include "eeprom_flash.h"
#include "User.h"

eeprom_flash eeprom;

void eeprom_flash::data_init()
{
    // EEPROM初始化数据

    user_datas.pwm_temp_buf = 100;
    user_datas.pwm_temp_mode = 0;
    user_datas.miot_miot_able = 0;
    user_datas.pwm_temp_mode1_time = 10;
    user_datas.ui_oled_light = 127;
    user_datas.adc_adc_max_temp = 255;
    user_datas.adc_hotbed_max_temp = 255;
    user_datas.fan_auto_flg = 1;
    memset(user_datas.blinker_id, 0, sizeof(user_datas.blinker_id));
    user_datas.curve_temp_buf[0] = 140;
    user_datas.curve_temp_buf[1] = 90;
    user_datas.curve_temp_buf[2] = 255;
    user_datas.curve_temp_buf[3] = 60;

    write_flg = 2;

    EEPROM.write(eeprom_write_add, ee_head_0);
    EEPROM.write(eeprom_write_add + 1, ee_head_1);
}

void eeprom_flash::resume_factory()
{
    EEPROM.begin(eeprom_size);
    data_init();
    EEPROM.commit();
    EEPROM.end();
    write_flg = 2;
    write_task();
    ESP.restart();
}

void eeprom_flash::read_all_data()
{

    ec11.int_close();
    EEPROM.begin(eeprom_size);
    if (EEPROM.read(eeprom_write_add) != ee_head_0 || EEPROM.read(eeprom_write_add + 1) != ee_head_1)
    {
        data_init();
    }
    else
    {
        read_bytes(eeprom_write_add + 2, &user_datas, sizeof(user_datas));
        Serial.print("Blinker ID:");
        Serial.println(user_datas.blinker_id);
    }

    EEPROM.commit();
    EEPROM.end();

    ec11.int_work();
    oled.light(user_datas.ui_oled_light);
}

void eeprom_flash::read_bytes(int add, void *buf, size_t size)
{
    uint8_t *p = (uint8_t *)buf;
    while (size--)
        *p++ = EEPROM.read(add++);
}

void eeprom_flash::write_bytes(int add, void *buf, size_t size)
{
    uint8_t *p = (uint8_t *)buf;
    while (size--)
        EEPROM.write(add++, *p++);
}

void eeprom_flash::write_task()
{
    if (write_flg == 2)
    {
        write_flg = 0;
        ec11.int_close();
#ifdef DEBUG
        Serial.println("EE WRITE");
#endif
        EEPROM.begin(eeprom_size);
        write_bytes(eeprom_write_add + 2, &user_datas, sizeof(user_datas));
        EEPROM.commit();
        EEPROM.end();
#ifdef DEBUG
        Serial.println("END");
#endif
        ec11.int_work();
    }
}
