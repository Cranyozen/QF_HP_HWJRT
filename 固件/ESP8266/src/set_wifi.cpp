#include "set_wifi.h"
#include "ui.h"
set_wifi setwifi;

#if compile_style
const char conect_wifi0[] = "啟動";
const char conect_wifi1[] = "AT";
const char conect_wifi2[] = "力場";
#else
const char conect_wifi[] = "连接至网络";
#endif

bool set_wifi::power_on_conect()
{
  int8_t x;
  uint8_t buf = 0;
  uint8_t count = 0;
  WiFi.setAutoReconnect(true);
  for (x = 32; x > 7; x--)
  {
    oled.choose_clr(12, 0, 104, 4);
#if compile_style
    oled.chinese(12, x, conect_wifi0, 16, 1, 1);
    oled.str(44, x, conect_wifi1, 16, 1, 1);
    oled.chinese(60, x, conect_wifi2, 16, 1, 0);
#else
    oled.chinese(12, x, conect_wifi, 16, 1, 0);
#endif

    oled.choose_refresh(12, 0, 104, 4);
    yield();
  }
  WiFi.mode(WIFI_STA);
  Serial.println("Conect to WiFi:");
  Serial.println(WiFi.SSID());
  Serial.println(WiFi.psk());
  WiFi.begin();
  x = 92;

  for (;;)
  {
    if (WiFi.SSID() == "")
      break;
    delay(100);
    count++;
    if (WiFi.status() == WL_CONNECTED)
    {
      buf = 1;
      break;
    }
    if (count == 5)
    {
      count = 0;
      buf++;
      if (buf == 20)
      {
        buf = 0;
        break;
      }
      oled.str(x, 7, ".", 16, 1, 0);
      oled.choose_refresh(92, 0, 24, 4);
      x += 8;
      if (x == 116)
      {
        x = 92;
        oled.choose_clr(92, 0, 24, 4);
      }
    }
  }
  for (x = 32; x > 7; x--)
  {
    oled.choose_clr(8, 0, 120, 4);
#if compile_style
    oled.chinese(12, x - 24, conect_wifi0, 16, 1, 1);
    oled.str(44, x - 24, conect_wifi1, 16, 1, 1);
    oled.chinese(60, x - 24, conect_wifi2, 16, 1, 0);
#else
    oled.chinese(12, x - 24, conect_wifi, 16, 1, 0);
#endif
    if (buf)
    {
#if compile_style
      oled.str(24, x, "AT", 16, 1, 1);
      oled.chinese(40, x, "力場全开", 16, 1, 0);
#else
      oled.chinese(48, x, "搞定", 16, 1, 0);
#endif
    }

    else
    {
#if compile_style
      oled.str(8, x, "AT", 16, 1, 1);
      oled.chinese(24, x, "力場啟動失败", 16, 1, 0);
#else
      oled.chinese(48, x, "失败", 16, 1, 0);
#endif
    }

    oled.choose_refresh(8, 0, 120, 4);
    yield();
  }
  delay(500);
  oled.roll(8, 0, 120, 4, 1, UP, 32);
  return buf;
}
