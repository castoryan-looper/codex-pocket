#pragma once
#include <Arduino.h>
#include <esp_err.h>
esp_err_t taijiDisplayInit();
esp_err_t taijiPresent(const uint8_t *bitmap,bool native,int selection,int view);
bool taijiTouch(uint16_t &x,uint16_t &y,uint16_t *rawX=nullptr,uint16_t *rawY=nullptr);
void taijiSetBrightness(unsigned percent);
bool taijiDisplayReady();
void taijiSetOrientation(unsigned displayDegrees,unsigned touchDegrees);
unsigned taijiDisplayRotation();
unsigned taijiTouchRotation();

void taijiCacheFrame(unsigned slot,const uint8_t *bits,int selection,int view);
esp_err_t taijiPresentCached(int offset,bool previous,bool next);
unsigned taijiLastRenderMicros();
unsigned taijiMotionFrames();

unsigned taijiCachedRotation();
unsigned taijiComposeMicros();
unsigned taijiTransferMicros();
