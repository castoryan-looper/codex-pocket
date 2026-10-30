"""Leave S3 USB download mode after flashing without retaining its boot latch.

Only changes volatile RTC/watchdog registers. No flash or eFuse writes.
Requires esptool 5.x and the S3 port used by the successful flash command.
"""
import argparse
import serial
from esptool.targets.esp32s3 import ESP32S3ROM

def start(port):
    connection=serial.Serial();connection.port=port;connection.baudrate=115200
    connection.timeout=.1;connection.dtr=False;connection.rts=False
    try:
        connection.open();chip=ESP32S3ROM(connection);chip.connect(mode='no-reset')
        chip.write_reg(chip.RTC_CNTL_OPTION1_REG,0,chip.RTC_CNTL_FORCE_DOWNLOAD_BOOT_MASK)
        connection.dtr=False;connection.rts=False
        if not chip.read_reg(0x6000403c)&1:
            raise RuntimeError('请先松开 BOOT，再重新启动应用；不会写入 Flash。')
        chip.watchdog_reset()
    finally:
        connection.close()

if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--port',required=True)
    start(parser.parse_args().port)
