#pragma once
namespace pocket {
struct RoundPoint {int x,y;};
// The 180-degree transform is its own inverse. Both framebuffer addressing
// and touch input use it so visible controls and hit regions stay aligned.
inline RoundPoint rotateRound180(int x,int y,int diameter){return {diameter-1-x,diameter-1-y};}
inline RoundPoint orientRoundPoint(int x,int y,int diameter,unsigned degrees){return degrees==180?rotateRound180(x,y,diameter):RoundPoint{x,y};}
inline bool openRoundSettingsOnTurn(bool online,bool connected,bool recording,bool capture,bool upload){return (!online||!connected)&&!recording&&!capture&&!upload;}
}
