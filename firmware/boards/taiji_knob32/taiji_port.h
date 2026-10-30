#pragma once
#include <Arduino.h>
#include <esp_err.h>
esp_err_t taijiDisplayInit();
esp_err_t taijiPresent(const uint8_t *bitmap,bool native,int selection);
bool taijiTouch(uint16_t &x,uint16_t &y);
void taijiSetBrightness(unsigned percent);
bool taijiDisplayReady();
