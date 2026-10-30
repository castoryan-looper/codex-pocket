#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>
namespace pocket {
struct RamAudioStorage {uint8_t *data=nullptr;size_t capacity=0,length=0;};
class RamAudioFile {
  RamAudioStorage *storage=nullptr;size_t position=0;bool writing=false;
public:
  RamAudioFile()=default;
  RamAudioFile(RamAudioStorage &value,bool write):storage(value.data?&value:nullptr),writing(write){if(storage&&write)storage->length=0;}
  explicit operator bool()const{return storage!=nullptr;}
  void close(){storage=nullptr;position=0;}
  size_t size()const{return storage?storage->length:0;}
  size_t available()const{return storage&&position<=storage->length?storage->length-position:0;}
  size_t write(const uint8_t *data,size_t bytes){
    if(!storage||!writing||!data||bytes>storage->capacity-position)return 0;
    memcpy(storage->data+position,data,bytes);position+=bytes;storage->length=position;return bytes;
  }
  int read(uint8_t *out,size_t bytes){
    if(!storage||writing||!out)return 0;
    size_t count=bytes<available()?bytes:available();memcpy(out,storage->data+position,count);position+=count;return (int)count;
  }
};
}
