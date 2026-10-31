#pragma once
#include <stdint.h>
namespace pocket {
inline uint16_t rgb565(unsigned r,unsigned g,unsigned b){return ((r>>3)<<11)|((g>>2)<<5)|(b>>3);}
inline bool roundRect(int x,int y,int l,int t,int r,int b,int radius){
 if(x<l||x>r||y<t||y>b)return false;
 int cx=x<l+radius?l+radius:x>r-radius?r-radius:x;
 int cy=y<t+radius?t+radius:y>b-radius?b-radius:y;
 return (x-cx)*(x-cx)+(y-cy)*(y-cy)<=radius*radius;
}
inline uint16_t roundTheme(int x,int y,bool ink,int view,int selected){
 int dx=x-180,dy=y-180,d2=dx*dx+dy*dy;if(d2>178*178)return 0;
 uint16_t bg=rgb565(9,17+y/40,28+y/24),fg=rgb565(235,242,252);
 if(d2>174*174)bg=rgb565(27,48,62);
 if(view==0){for(int i=0;i<2;i++)if(roundRect(x,y,54,114+i*72,306,174+i*72,17)){
   bool active=i==selected;bg=i==0?(active?rgb565(128,240,191):rgb565(22,57,53)):(active?rgb565(157,177,255):rgb565(34,42,71));
   fg=active?rgb565(10,30,35):rgb565(216,230,245);
 }}
 if(view==4&&roundRect(x,y,42,99,318,256,23)){
  bg=selected==1?rgb565(66,44,30):selected==2?rgb565(27,43,67):rgb565(18,47,48);
  fg=rgb565(233,247,242);
 }
 if(view==1&&roundRect(x,y,78,185,282,215,15)){bg=rgb565(128,240,191);fg=rgb565(10,30,35);}
 if(view>=0&&roundRect(x,y,108,270,252,320,23)){bg=rgb565(255,155,108);fg=rgb565(39,26,24);}
 if(view>=0&&(roundRect(x,y,52,279,101,314,14)||roundRect(x,y,258,279,307,314,14))){bg=rgb565(30,43,58);fg=rgb565(201,214,230);}
 if(view==-3){if(d2>161*161&&d2<168*168)bg=rgb565(255,112,104);}
 if(y>=24&&y<=48&&x>=104&&x<=131){bg=rgb565(128,240,191);fg=rgb565(10,30,35);}
 return ink?fg:bg;
}
}
