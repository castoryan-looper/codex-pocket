import importlib.util,sys,types,unittest
from pathlib import Path
from unittest.mock import patch

class StartApplicationTests(unittest.TestCase):
    def run_start(self,released):
        events=[]
        class Port:
            def open(self):events.append(('open',self.dtr,self.rts))
            def close(self):events.append(('close',))
        class Chip:
            RTC_CNTL_OPTION1_REG=0x6000812c;RTC_CNTL_FORCE_DOWNLOAD_BOOT_MASK=1
            def __init__(self,port):self.port=port
            def connect(self,mode):events.append(('connect',mode))
            def write_reg(self,address,value,mask):events.append(('volatile',address,value,mask))
            def read_reg(self,address):events.append(('gpio',address));return int(released)
            def watchdog_reset(self):events.append(('start',))
        package=types.ModuleType('esptool');package.__path__=[]
        targets=types.ModuleType('esptool.targets');targets.__path__=[]
        target=types.ModuleType('esptool.targets.esp32s3');target.ESP32S3ROM=Chip
        serial=types.ModuleType('serial');serial.Serial=Port
        with patch.dict(sys.modules,{'serial':serial,'esptool':package,'esptool.targets':targets,'esptool.targets.esp32s3':target}):
            spec=importlib.util.spec_from_file_location('start_under_test',Path(__file__).with_name('start_s3_application.py'))
            module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
            if released:module.start('COM-test')
            else:
                with self.assertRaises(RuntimeError):module.start('COM-test')
        return events

    def test_clear_volatile_download_latch_then_start_and_close(self):
        events=self.run_start(True)
        self.assertEqual(events[0],('open',False,False));self.assertEqual(events[1],('connect','no-reset'))
        self.assertEqual(events[2],('volatile',0x6000812c,0,1))
        self.assertEqual(events[-2:],[('start',),('close',)])

    def test_boot_button_held_never_runs_and_always_closes(self):
        events=self.run_start(False);self.assertNotIn(('start',),events);self.assertEqual(events[-1],('close',))

if __name__=='__main__':unittest.main()
