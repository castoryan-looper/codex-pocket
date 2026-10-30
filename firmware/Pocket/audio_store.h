#pragma once
#ifdef POCKET_TAIJI_KNOB32
#include <esp_heap_caps.h>
#include "ram_audio_cache.h"
using PocketAudioFile=pocket::RamAudioFile;
static pocket::RamAudioStorage pocketAudioCache;
static bool initRamAudioStore(){
  pocketAudioCache.capacity=20*32000;
  pocketAudioCache.data=(uint8_t*)heap_caps_malloc(pocketAudioCache.capacity,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
  return pocketAudioCache.data!=nullptr;
}
static PocketAudioFile audioOpen(const char*,const char *mode){return PocketAudioFile(pocketAudioCache,mode&&mode[0]=='w');}
static void audioRemove(const char*){pocketAudioCache.length=0;}
static size_t audioFreeBytes(){return pocketAudioCache.data?pocketAudioCache.capacity-pocketAudioCache.length:0;}
#else
using PocketAudioFile=File;
static PocketAudioFile audioOpen(const char *path,const char *mode){return FFat.open(path,mode);}
static void audioRemove(const char *path){FFat.remove(path);}
static size_t audioFreeBytes(){return FFat.freeBytes();}
#endif
