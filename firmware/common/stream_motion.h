#pragma once
#include <stdint.h>
namespace pocket {
static constexpr int StreamTop=62,StreamBottom=270,StreamHeight=StreamBottom-StreamTop;
struct StreamSample {unsigned slot;int row;};
inline StreamSample streamSample(int y,int offset,bool previous,bool next){
 if(y<StreamTop||y>=StreamBottom)return {0,y};
 int row=y-offset;
 if(row<StreamTop)return {previous?1U:3U,row+StreamHeight};
 if(row>=StreamBottom)return {next?2U:3U,row-StreamHeight};
 return {0,row};
}
class StreamMotion {
 int from=0,target=0,raw=0;uint32_t began=0,waitingAt=0;bool committed=false;
public:
 enum Phase {Idle=0,Dragging=1,Settling=2,Waiting=3};Phase phase=Idle;int offset=0;
 void clear(){phase=Idle;offset=raw=0;committed=false;}
 void begin(){if(phase==Idle){phase=Dragging;offset=raw=0;committed=false;}}
 void drag(int dy,bool previous,bool next){
  if(phase!=Dragging)return;raw=dy;
  int value=dy;if(value>StreamHeight)value=StreamHeight;if(value< -StreamHeight)value=-StreamHeight;
  if((value>0&&!previous)||(value<0&&!next))value/=5;
  offset=value;
 }
 int release(uint32_t now,bool canPrevious,bool canNext,bool previous,bool next){
  if(phase!=Dragging)return 0;
  int direction=raw<=-40&&canNext?1:raw>=40&&canPrevious?-1:0;
  from=offset;target=direction==1&&next?-StreamHeight:direction==-1&&previous?StreamHeight:0;
  committed=direction!=0;began=now;phase=Settling;return direction;
 }
 void tick(uint32_t now){
  if(phase==Settling){
   uint32_t elapsed=now-began;
   if(elapsed>=150){offset=target;if(committed){phase=Waiting;waitingAt=now;}else phase=Idle;return;}
   int64_t remaining=150-elapsed;int64_t cube=remaining*remaining*remaining;
   offset=target+(int)((int64_t(from-target)*cube)/(150LL*150*150));
  }else if(phase==Waiting&&uint32_t(now-waitingAt)>=1500){from=offset;target=0;began=now;committed=false;phase=Settling;}
 }
};
}
