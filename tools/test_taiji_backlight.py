"""Compile the actual PWM constants and brightness function against a fake pin."""
from pathlib import Path
import shutil,subprocess,tempfile,unittest,re

class BacklightTests(unittest.TestCase):
    def test_xtal_clock_budget_duty_bounds_and_gpio_fallback(self):
        compiler=shutil.which('cl') or shutil.which('g++') or shutil.which('clang++')
        if not compiler:self.skipTest('host C++ compiler required')
        source=(Path(__file__).resolve().parents[1]/'firmware/boards/taiji_knob32/port_display.cpp').read_text()
        constants=re.search(r'static constexpr unsigned backlightHz=.*?\nstatic_assert\([^\n]+',source).group()
        function=re.search(r'void taijiSetBrightness\(unsigned percent\)\{[^\n]+',source).group()
        code=r'''
#include <algorithm>
#include <cassert>
using std::min;
const int TAIJI_BACKLIGHT=47,HIGH=1,LOW=0;
bool backlightPwm=true;unsigned duty=0;int level=-1;
void ledcWrite(int pin,unsigned value){assert(pin==47);duty=value;}
void digitalWrite(int pin,int value){assert(pin==47);level=value;}
CONSTANTS
FUNCTION
int main(){
 taijiSetBrightness(100);assert(duty==511);taijiSetBrightness(70);assert(duty==357);
 taijiSetBrightness(300);assert(duty==511);taijiSetBrightness(0);assert(duty==0);
 backlightPwm=false;taijiSetBrightness(70);assert(level==HIGH);taijiSetBrightness(0);assert(level==LOW);
}
'''.replace('CONSTANTS',constants).replace('FUNCTION',function)
        with tempfile.TemporaryDirectory() as folder:
            src=Path(folder)/'backlight.cpp';src.write_text(code);exe=Path(folder)/'backlight.exe'
            args=[compiler,'/nologo','/EHsc','/std:c++14',str(src),'/Fe:'+str(exe)] if Path(compiler).stem=='cl' else [compiler,'-std=c++14',str(src),'-o',str(exe)]
            subprocess.run(args,cwd=folder,check=True,capture_output=True);subprocess.run([str(exe)],check=True)

if __name__=='__main__':unittest.main()
