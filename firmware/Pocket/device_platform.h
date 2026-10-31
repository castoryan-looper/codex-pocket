#pragma once
#include <Arduino.h>
#ifdef POCKET_TAIJI_KNOB32
#include <esp_heap_caps.h>
#include "taiji_port.h"
#define POCKET_FIRMWARE "codex-pocket-taiji-0.15.1"
static volatile unsigned taijiBrightness=70;
static volatile uint32_t taijiEncoderCount=0,taijiTouchCount=0;
static volatile uint32_t taijiEncoderEdges=0;
static volatile unsigned taijiEncoderMode=0;
static volatile int taijiEncoderDirection=0;
static volatile uint16_t taijiTouchX=0,taijiTouchY=0;
static volatile uint16_t taijiTouchRawX=0,taijiTouchRawY=0;
static volatile bool taijiOrientationChanged=false;
static uint8_t *roundDesired=nullptr,*roundPacked=nullptr,*roundDecoded=nullptr;
static uint8_t *streamPreviewBits[2]={};
static bool initRoundBuffers(){
  uint8_t *buffer=(uint8_t*)heap_caps_calloc(74800,1,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
  if(!buffer)return false;
  roundDesired=buffer;roundPacked=buffer+16200;roundDecoded=buffer+26200;streamPreviewBits[0]=buffer+42400;streamPreviewBits[1]=buffer+58600;return true;
}
static volatile bool roundNative=false,roundExpanded=false,roundBusy=false;
static volatile bool roundCanRecord=true;
static volatile int roundView=0,roundSelection=0;
static char streamToken[33]={},shownStreamToken[33]={},previewAnchor[2][33]={};
static uint32_t streamPreviewRevision[2]={};static int streamPreviewSelection[2]={};
static volatile bool streamCachedPrevious=false,streamCachedNext=false,streamCanPrevious=false,streamCanNext=true;
static volatile int streamVisualOffset=0,streamVisualPhase=0,streamLayout=1;
static volatile bool taijiMicHeld=false,taijiCancelHeld=false;
static int pocketBootLevel(){return taijiMicHeld?LOW:HIGH;}
static int pocketPowerLevel(){return taijiCancelHeld?LOW:HIGH;}
#else
#define POCKET_FIRMWARE "codex-pocket-0.11.1"
static int pocketBootLevel(){return digitalRead(BOOT_BUTTON_PIN);}
static int pocketPowerLevel(){return digitalRead(PWR_BUTTON_PIN);}
#endif
#define POCKET_FRAME_LIMIT 15000
