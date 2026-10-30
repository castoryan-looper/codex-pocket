"""Compile the actual network setup prefix with empty and configured NVS."""
from pathlib import Path
import shutil,subprocess,tempfile,unittest

class FirstBootTests(unittest.TestCase):
    def test_listener_always_has_an_initialized_network_stack(self):
        compiler=shutil.which('cl') or shutil.which('g++') or shutil.which('clang++')
        if not compiler:self.skipTest('host C++ compiler required')
        source=(Path(__file__).resolve().parents[1]/'firmware/Pocket/Pocket.ino').read_text(encoding='utf-8')
        start=source.index('void setupNetwork() {');end=source.index('  WiFi.onEvent(',start)
        prefix=source[start:end]+'}\n'
        code=r'''
#include <cassert>
#include <string>
using String=std::string;
const int WIFI_STA=1;
bool initialized=false,wifiConfigured=false,bridgeConfigured=false,tlsEnabled=true;
String wifiSsid,wifiPass,remoteHost,remotePath,remoteToken,rootCa;unsigned remotePort;
struct {
 void begin(const char*,bool){} void end(){}
 String getString(const char*,const char *fallback){return fallback;}
 bool getBool(const char*,bool fallback){return fallback;}
 unsigned getUInt(const char*,unsigned fallback){return fallback;}
} prefs;
struct {bool persistentState=true;
 void persistent(bool value){persistentState=value;}
 void mode(int value){assert(value==WIFI_STA);assert(!persistentState);initialized=true;}
} WiFi;
String normalizeRootCa(const String &text){return text;}
void loadKnownWifi(){}
struct {void begin(){assert(initialized);}} WebServer;
PREFIX
int main(){
 wifiConfigured=false;setupNetwork();WebServer.begin();assert(initialized);
 initialized=false;wifiConfigured=true;setupNetwork();WebServer.begin();assert(initialized);
}
'''.replace('PREFIX',prefix)
        with tempfile.TemporaryDirectory() as folder:
            src=Path(folder)/'first_boot.cpp';src.write_text(code,encoding='utf-8');exe=Path(folder)/'first_boot.exe'
            args=[compiler,'/nologo','/EHsc','/std:c++14',str(src),'/Fe:'+str(exe)] if Path(compiler).stem=='cl' else [compiler,'-std=c++14',str(src),'-o',str(exe)]
            subprocess.run(args,cwd=folder,check=True,capture_output=True);subprocess.run([str(exe)],check=True)

if __name__=='__main__':unittest.main()
