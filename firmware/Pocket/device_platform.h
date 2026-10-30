#pragma once
#include <Arduino.h>
#ifdef POCKET_TAIJI_KNOB32
#define POCKET_FIRMWARE "codex-pocket-taiji-0.12.0"
static uint8_t roundDesired[16200];
static volatile bool roundNative=false,roundExpanded=false,roundBusy=false;
static volatile bool roundCanRecord=true;
static volatile int roundView=0,roundSelection=0;
static volatile bool taijiMicHeld=false,taijiCancelHeld=false;
static int pocketBootLevel(){return taijiMicHeld?LOW:HIGH;}
static int pocketPowerLevel(){return taijiCancelHeld?LOW:HIGH;}
#else
#define POCKET_FIRMWARE "codex-pocket-0.11.1"
static int pocketBootLevel(){return digitalRead(BOOT_BUTTON_PIN);}
static int pocketPowerLevel(){return digitalRead(PWR_BUTTON_PIN);}
#endif
#define POCKET_FRAME_LIMIT 15000
