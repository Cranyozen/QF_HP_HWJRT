#ifndef _User_H_
#define _User_H_

#define DEBUG

#include <Arduino.h>
#include <Ticker.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>

#include "Oled.h"
#include "ui.h"
#include "EC11.h"
#include "Tick_IRQ.h"
#include "set_wifi.h"
#include "eeprom_flash.h"
#include "adc.h"
#include "PWM.h"
#include "MIOT.h"
#include "WiFiManager.h"
#include "user_datas.h"

#define HP_VERSION 162 // 1.62
#define JUST_OTA 1     // 0:重装所有数据升级 1:仅OTA更新保留数据

#endif
