#pragma once
#include <stdint.h>
#include "quadrature.h"
namespace pocket {
// Supports the independent directional pulses used by the vendor demo and
// full quadrature revisions. Commit after 2 ms stable idle; accept short deliberate pulses at normal turning speed.
class DualModeEncoder {
  Quadrature quad;uint8_t previous=3,channels=0;
  bool active=false,bothLow=false;uint32_t began=0,lastEdge=0;
  int direction=0,queued=0;
  void finish(uint32_t now){
    if(!active||previous!=3||uint32_t(now-lastEdge)<2)return;
    int result=0;
    if(bothLow&&direction){result=direction;mode=2;}
    else if(!bothLow&&uint32_t(lastEdge-began)>=2&&(channels==1||channels==2)){
      result=channels==2?1:-1;mode=1;
    }
    queued+=result;if(queued>8)queued=8;if(queued< -8)queued=-8;
    active=false;channels=0;bothLow=false;direction=0;
  }
public:
  unsigned mode=0; // 0 undetermined, 1 directional pulse, 2 quadrature
  void begin(uint8_t state){previous=state&3;quad.begin(previous);active=false;channels=0;bothLow=false;direction=queued=0;mode=0;}
  void edge(uint8_t state,uint32_t now){
    state&=3;if(state==previous)return;finish(now);
    if(!active&&previous==3&&state!=3){active=true;began=now;channels=0;bothLow=false;direction=0;}
    if(active){channels|=(~state)&3;bothLow|=state==0;lastEdge=now;}
    int step=quad.step(state);if(step)direction=step;previous=state;
  }
  int take(uint32_t now){finish(now);int value=queued;queued=0;return value;}
};
}
