#include "User.h"

/*
  启凡科创QF-HP物联网加热台固件源码
  版本:V1.62
  日期:2023-2-18
*/

/**
 * @brief EC11旋转方向
 *
 * @param 0 反转
 * @param 1 正转
 */
#define EC11_rotation 1

void setup()
{
  user_datas_init();
  delay(100);

#ifdef DEBUG
  {
    Serial.begin(115200);
  }
#endif

  oled.begin();

  eeprom.read_all_data();

  Ticker_init();

  miot.begin();

#if EC11_rotation
  ec11.begin(5, 4, 2, ui_key_callb);
#else
  ec11.begin(5, 2, 4, ui_key_callb);
#endif
  ec11.speed_up(true);
  ec11.speed_up_max(20);

  ui.page_switch_flg = true;
}

void loop()
{
  delay(1);
  if (system_get_cpu_freq() != SYS_CPU_160MHZ)
    system_update_cpu_freq(SYS_CPU_160MHZ);
  ui.run_task();
  eeprom.write_task();
  miot.run_task();
}
