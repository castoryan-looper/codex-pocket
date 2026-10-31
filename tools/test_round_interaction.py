"""Compile the actual hardware-independent round-screen interaction policy."""
import pathlib, shutil, subprocess, tempfile, unittest

class RoundInteractionTests(unittest.TestCase):
    def test_touch_encoder_paging_recording_and_busy_guards(self):
        compiler=shutil.which('cl') or shutil.which('g++') or shutil.which('clang++')
        if not compiler:self.skipTest('host C++ compiler required')
        header=(pathlib.Path(__file__).resolve().parents[1]/'firmware/common/round_interaction.h').as_posix()
        rle=(pathlib.Path(__file__).resolve().parents[1]/'firmware/common/bitmap_rle.h').as_posix()
        encoder=(pathlib.Path(__file__).resolve().parents[1]/'firmware/common/quadrature.h').as_posix()
        audio=(pathlib.Path(__file__).resolve().parents[1]/'firmware/common/ram_audio_cache.h').as_posix()
        dual=(pathlib.Path(__file__).resolve().parents[1]/'firmware/common/dual_mode_encoder.h').as_posix()
        geometry=(pathlib.Path(__file__).resolve().parents[1]/'firmware/common/round_geometry.h').as_posix()
        pulse=(pathlib.Path(__file__).resolve().parents[1]/'firmware/common/directional_pulse_encoder.h').as_posix()
        theme=(pathlib.Path(__file__).resolve().parents[1]/'firmware/common/round_theme.h').as_posix()
        gestures=(pathlib.Path(__file__).resolve().parents[1]/'firmware/common/stream_gestures.h').as_posix()
        motion=(pathlib.Path(__file__).resolve().parents[1]/'firmware/common/stream_motion.h').as_posix()
        code=r'''
#include <cassert>
#include "HEADER"
#include "RLE"
#include "ENCODER"
#include "AUDIO"
#include "DUAL"
#include "GEOMETRY"
#include "PULSE"
#include "THEME"
#include "GESTURES"
#include "MOTION"
#include <vector>
using namespace pocket;
int main(){
 RoundInteraction ui;using I=RoundInput;using O=RoundIntent;
 assert(ui.input(I::TapA)==O::None);ui.input(I::OpenTools);
 assert(ui.view==RoundView::Tools);ui.input(I::Back);ui.online=true;
 assert(ui.input(I::TapB)==O::OpenB);assert(ui.view==RoundView::Confirm);
 assert(ui.input(I::TapCenter)==O::Execute);assert(ui.input(I::TapCenter)==O::None);
 ui.busy=false;ui.input(I::TapCenter);ui.input(I::OpenDetails);ui.pages=3;
 assert(ui.input(I::Clockwise)==O::NextPage);assert(ui.page==1);
 assert(ui.input(I::SwipeNext)==O::NextPage);assert(ui.input(I::Clockwise)==O::None);
 assert(ui.input(I::CounterClockwise)==O::PreviousPage);
 assert(ui.input(I::TapCenter)==O::None);assert(ui.view==RoundView::Confirm);
 ui.input(I::Back);assert(ui.input(I::HoldMic)==O::StartCapture);
 assert(ui.input(I::TapA)==O::None);assert(ui.input(I::ReleaseMic)==O::FinishCapture);
 ui.input(I::HoldMic);ui.online=false;assert(ui.input(I::ReleaseMic)==O::CancelCapture);
 ui.online=true;ui.input(I::HoldMic);assert(ui.input(I::Back)==O::CancelCapture);
 ui.input(I::HoldMic);assert(ui.blocked()==O::CancelCapture);assert(!ui.recording);
 ui.online=true;ui.approval=true;assert(ui.input(I::TapA)==O::None);
 ui.approval=false;ui.draft=true;assert(ui.input(I::HoldMic)==O::None);
 assert(roundHit(180,140,360,RoundView::Choices)==RoundHit::A);
 assert(roundHit(180,220,360,RoundView::Choices)==RoundHit::B);
 assert(roundHit(180,290,360,RoundView::Choices)==RoundHit::Mic);
 assert(roundHit(278,290,360,RoundView::Choices)==RoundHit::Tools);
 assert(roundHit(0,0,360,RoundView::Choices)==RoundHit::None);
 assert(roundHit(240,240,480,RoundView::Confirm)==RoundHit::Center);
 assert(roundHit(-1,180,360,RoundView::Choices)==RoundHit::None);
 std::vector<uint8_t> encoded;for(unsigned i=0;i<126;i++){encoded.push_back(255);encoded.push_back(0);}
 encoded.push_back(199);encoded.push_back(0);uint8_t bitmap[16200];
 assert(decodeBitmapRle(encoded.data(),encoded.size(),bitmap,sizeof(bitmap)));
 for(auto value:bitmap)assert(value==0);
 const uint8_t shortRun[]={255};assert(!decodeBitmapRle(shortRun,1,bitmap,128));
 const uint8_t longRun[]={255,0};assert(!decodeBitmapRle(longRun,2,bitmap,127));
 const uint8_t shortLiteral[]={2,1};assert(!decodeBitmapRle(shortLiteral,2,bitmap,3));
 const uint8_t validLiteral[]={2,1,2,3};assert(decodeBitmapRle(validLiteral,4,bitmap,3));
 assert(!decodeBitmapRle(validLiteral,4,bitmap,4));
 Quadrature encoder;int clicks=0;
 for(uint8_t state:{1,0,2,3})clicks+=encoder.step(state);assert(clicks==1);
 clicks=0;for(uint8_t state:{2,0,1,3})clicks+=encoder.step(state);assert(clicks==-1);
 clicks=0;for(uint8_t state:{1,3,1,3,1,0,2,3})clicks+=encoder.step(state);assert(clicks==1);
 encoder.begin(3);assert(encoder.step(0)==0);assert(encoder.partial==0);
 uint8_t memory[16]={};RamAudioStorage store;store.data=memory;store.capacity=sizeof(memory);
 RamAudioFile writer(store,true);uint8_t payload[]={1,2,3,4};
 assert(writer.write(payload,4)==4);assert(writer.write(payload,20)==0);assert(store.length==4);writer.close();
 RamAudioFile reader(store,false);uint8_t readback[4]={};assert(reader.read(readback,3)==3);assert(reader.available()==1);
 assert(reader.read(readback,4)==1);assert(reader.available()==0);reader.close();
 RamAudioFile retry(store,false);assert(retry.read(readback,4)==4);assert(readback[0]==1&&readback[3]==4);
 assert(retry.write(payload,4)==0);retry.close();assert(retry.read(readback,4)==0);
 StreamMotion motion;motion.begin();motion.drag(-70,true,true);assert(motion.offset==-70);
 assert(motion.release(100,true,true,true,true)==1);motion.tick(175);assert(motion.offset< -70&&motion.offset> -StreamHeight);
 motion.tick(250);assert(motion.phase==StreamMotion::Waiting&&motion.offset==-StreamHeight);
 motion.tick(1750);assert(motion.phase==StreamMotion::Settling);motion.tick(1900);assert(motion.phase==StreamMotion::Idle&&motion.offset==0);
 motion.begin();motion.drag(70,false,true);assert(motion.offset==14);assert(motion.release(2000,false,true,false,true)==0);motion.tick(2150);assert(motion.offset==0);
 motion.begin();motion.drag(-80,false,false);assert(motion.release(3000,true,true,false,false)==1);motion.tick(3150);assert(motion.phase==StreamMotion::Waiting&&motion.offset==0);
 motion.clear();assert(motion.phase==StreamMotion::Idle);
 for(int offset=-StreamHeight;offset<=StreamHeight;offset++)for(int y=0;y<360;y++){
  auto sample=streamSample(y,offset,true,true);assert(sample.slot<=2&&sample.row>=0&&sample.row<360);
  if(y<StreamTop||y>=StreamBottom)assert(sample.slot==0&&sample.row==y);
 }
 assert(streamSample(StreamTop,-StreamHeight,true,true).slot==2);
 assert(streamSample(StreamTop,StreamHeight,true,true).slot==1);
 StreamGestures gesture;
 assert(gesture.release(180,180,0,-60,200,100)==StreamIntent::Next);
 assert(gesture.release(180,180,0,60,200,200)==StreamIntent::Previous);
 assert(gesture.release(180,180,0,0,100,1000)==StreamIntent::None);
 assert(gesture.release(181,181,0,0,100,1300)==StreamIntent::Confirm);
 assert(gesture.release(180,180,0,0,100,1500)==StreamIntent::None);
 gesture.clear();assert(gesture.release(180,180,0,0,100,1700)==StreamIntent::None);
 assert(gesture.release(180,180,0,-55,100,1800)==StreamIntent::Next);
 assert(gesture.release(180,180,0,0,100,1950)==StreamIntent::None);
 assert(gesture.release(180,180,0,0,600,2050)==StreamIntent::None);
 assert(gesture.release(180,295,0,0,100,2150)==StreamIntent::None);
 gesture.clear();assert(gesture.release(180,76,0,0,100,3000,62)==StreamIntent::None);
 assert(gesture.release(180,76,0,0,100,3300,62)==StreamIntent::Confirm);
 DirectionalPulseEncoder pulse;pulse.begin(3);
 pulse.edge(1,100);pulse.edge(3,250);assert(pulse.take(250)==1);
 pulse.edge(1,300);pulse.edge(3,450);assert(pulse.take(450)==0); // contact bounce
 pulse.edge(2,10000);pulse.edge(3,10150);assert(pulse.take(10150)==-1);
 pulse.begin(3);int shortSteps=0;
 for(unsigned t=100;t<200100;t+=10000){pulse.edge(1,t);pulse.edge(3,t+100);shortSteps+=pulse.take(t+100);}assert(shortSteps==20);
 pulse.begin(3);pulse.edge(1,100);pulse.edge(3,110);assert(pulse.take(110)==0);
 pulse.begin(3);pulse.edge(1,0xfffffff0U);pulse.edge(3,100);assert(pulse.take(100)==1);
 assert(roundTheme(0,0,false,0,0)==0);
 assert(roundTheme(160,140,false,0,0)!=roundTheme(160,212,false,0,0));
 assert(roundTheme(180,300,false,0,0)!=roundTheme(180,260,false,0,0));
 assert(roundTheme(180,300,true,0,0)!=roundTheme(180,300,false,0,0));
 assert(roundHit(110,295,360,RoundView::Choices)==RoundHit::Mic);
 DualModeEncoder dual;dual.begin(3);
 dual.edge(1,10);dual.edge(3,20);assert(dual.take(21)==0);assert(dual.take(22)==1);assert(dual.mode==1);
 dual.edge(2,40);dual.edge(3,50);assert(dual.take(56)==-1);
 dual.begin(3);dual.edge(1,10);dual.edge(0,12);dual.edge(2,14);dual.edge(3,16);assert(dual.take(22)==1);assert(dual.mode==2);
 dual.edge(2,30);dual.edge(0,32);dual.edge(1,34);dual.edge(3,36);assert(dual.take(42)==-1);
 dual.begin(3);dual.edge(1,10);dual.edge(3,11);assert(dual.take(17)==0);
 dual.edge(1,20);dual.edge(3,21);dual.edge(1,22);dual.edge(0,24);dual.edge(2,26);dual.edge(3,28);assert(dual.take(34)==1);
 dual.begin(3);int fast=0;
 for(unsigned t=10;t<110;t+=10){dual.edge(1,t);dual.edge(3,t+3);fast+=dual.take(t+5);}assert(fast==10);
 dual.edge(2,120);dual.edge(3,123);assert(dual.take(125)==-1);
 dual.begin(3);dual.edge(0,10);dual.edge(3,20);assert(dual.take(30)==0);
 dual.begin(3);dual.edge(1,0xfffffff0U);dual.edge(3,1);assert(dual.take(7)==1);
 for(int y=0;y<360;y++)for(int x=0;x<360;x++){
  auto panel=rotateRound180(x,y,360);assert(panel.x>=0&&panel.x<360&&panel.y>=0&&panel.y<360);
  auto touch=rotateRound180(panel.x,panel.y,360);assert(touch.x==x&&touch.y==y);
 }
 auto settings=rotateRound180(278,290,360);assert(settings.x==81&&settings.y==69);
 auto touched=rotateRound180(settings.x,settings.y,360);
 assert(roundHit(touched.x,touched.y,360,RoundView::Choices)==RoundHit::Tools);
 auto normal=orientRoundPoint(278,290,360,0);assert(normal.x==278&&normal.y==290);
 auto rotated=orientRoundPoint(81,69,360,180);assert(rotated.x==278&&rotated.y==290);
 auto mic=rotateRound180(180,290,360);auto micTouch=rotateRound180(mic.x,mic.y,360);
 assert(roundHit(micTouch.x,micTouch.y,360,RoundView::Choices)==RoundHit::Mic);
 assert(openRoundSettingsOnTurn(false,false,false,false,false));
 assert(openRoundSettingsOnTurn(false,true,false,false,false));
 assert(!openRoundSettingsOnTurn(true,true,false,false,false));
 assert(!openRoundSettingsOnTurn(false,false,true,false,false));
 assert(!openRoundSettingsOnTurn(false,false,false,true,false));
 assert(!openRoundSettingsOnTurn(false,false,false,false,true));
}
'''.replace('HEADER',header).replace('RLE',rle).replace('ENCODER',encoder).replace('AUDIO',audio).replace('DUAL',dual).replace('GEOMETRY',geometry).replace('PULSE',pulse).replace('THEME',theme).replace('GESTURES',gestures).replace('MOTION',motion)
        with tempfile.TemporaryDirectory() as folder:
            src=pathlib.Path(folder)/'round.cpp';src.write_text(code,encoding='utf-8')
            exe=pathlib.Path(folder)/'round.exe'
            if pathlib.Path(compiler).stem=='cl':
                args=[compiler,'/nologo','/EHsc','/std:c++14',str(src),'/Fe:'+str(exe)]
            else:args=[compiler,'-std=c++14',str(src),'-o',str(exe)]
            subprocess.run(args,cwd=folder,check=True,capture_output=True)
            subprocess.run([str(exe)],check=True)

if __name__=='__main__':unittest.main()
