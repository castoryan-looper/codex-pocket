#pragma once
#include <stddef.h>
#include <stdint.h>
namespace pocket {
// PackBits-like encoding: high bit repeats one byte, low bit copies literal
// bytes. Both use (low 7 bits)+1 bytes; every read/write is bounded.
inline bool decodeBitmapRle(const uint8_t *src,size_t size,uint8_t *dst,size_t capacity){
  size_t in=0,out=0;
  while(in<size){uint8_t tag=src[in++];size_t count=(tag&127)+1;
    if(count>capacity-out)return false;
    if(tag&128){if(in==size)return false;uint8_t value=src[in++];for(size_t i=0;i<count;i++)dst[out++]=value;}
    else{if(count>size-in)return false;for(size_t i=0;i<count;i++)dst[out++]=src[in++];}
  }return out==capacity;
}
}
