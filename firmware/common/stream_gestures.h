#pragma once
#include <stdint.h>
#include <stdlib.h>
namespace pocket {
enum class StreamIntent {None,Next,Previous,Confirm};
class StreamGestures {
 bool armed=false;uint32_t at=0;int px=0,py=0;
public:
 void clear(){armed=false;}
 StreamIntent release(int x,int y,int dx,int dy,uint32_t held,uint32_t now){
  if(abs(dy)>=40&&abs(dy)>abs(dx)){clear();return dy<0?StreamIntent::Next:StreamIntent::Previous;}
  if(abs(dx)>20||abs(dy)>20||held>400||x<48||x>312||y<94||y>260){clear();return StreamIntent::None;}
  uint32_t gap=now-at;
  if(armed&&gap>=60&&gap<=450&&abs(x-px)<=28&&abs(y-py)<=28){clear();return StreamIntent::Confirm;}
  armed=true;at=now;px=x;py=y;return StreamIntent::None;
 }
};
}
