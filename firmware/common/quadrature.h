#pragma once
#include <stdint.h>
namespace pocket {
struct Quadrature {
  uint8_t previous=3;int8_t partial=0;
  void begin(uint8_t state){previous=state&3;partial=0;}
  inline int step(uint8_t state){
    state&=3;uint8_t transition=(previous<<2)|state;
    if((previous^state)==3){partial=0;previous=state;return 0;}
    previous=state;
    if(transition==1||transition==7||transition==8||transition==14)partial--;
    else if(transition==2||transition==4||transition==11||transition==13)partial++;
    if(partial>=4){partial=0;return 1;}if(partial<=-4){partial=0;return -1;}return 0;
  }
};
}
