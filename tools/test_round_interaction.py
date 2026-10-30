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
        code=r'''
#include <cassert>
#include "HEADER"
#include "RLE"
#include "ENCODER"
#include "AUDIO"
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
}
'''.replace('HEADER',header).replace('RLE',rle).replace('ENCODER',encoder).replace('AUDIO',audio)
        with tempfile.TemporaryDirectory() as folder:
            src=pathlib.Path(folder)/'round.cpp';src.write_text(code,encoding='utf-8')
            exe=pathlib.Path(folder)/'round.exe'
            if pathlib.Path(compiler).stem=='cl':
                args=[compiler,'/nologo','/EHsc','/std:c++14',str(src),'/Fe:'+str(exe)]
            else:args=[compiler,'-std=c++14',str(src),'-o',str(exe)]
            subprocess.run(args,cwd=folder,check=True,capture_output=True)
            subprocess.run([str(exe)],check=True)

if __name__=='__main__':unittest.main()
