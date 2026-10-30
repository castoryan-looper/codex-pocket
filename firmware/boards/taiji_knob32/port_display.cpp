#include <Arduino.h>
#include <Wire.h>
#include <driver/spi_master.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <esp_heap_caps.h>
#include "esp_lcd_sh8601.h"
#include "taiji_pins.h"
#include "taiji_port.h"
extern "C" {extern const sh8601_lcd_init_cmd_t taiji_lcd_init_cmds[];extern const unsigned taiji_lcd_init_count;}
static esp_lcd_panel_handle_t panel=nullptr;
static SemaphoreHandle_t transferDone=nullptr;
static uint16_t *stripe=nullptr;
static bool ready=false;
static bool backlightPwm=false;
static constexpr unsigned backlightHz=50000,backlightBits=9;
static_assert(backlightHz*(1U<<backlightBits)<=40000000U,"Backlight exceeds S3 XTAL clock budget");
static bool IRAM_ATTR onTransfer(esp_lcd_panel_io_handle_t,esp_lcd_panel_io_event_data_t*,void*){
  BaseType_t wake=pdFALSE;xSemaphoreGiveFromISR(transferDone,&wake);return wake==pdTRUE;
}
esp_err_t taijiDisplayInit(){
  pinMode(TAIJI_ENCODER_A,INPUT_PULLUP);pinMode(TAIJI_ENCODER_B,INPUT_PULLUP);
  Wire.begin(ESP32_I2C_SDA_PIN,ESP32_I2C_SCL_PIN,300000);Wire.setTimeOut(20);
  Wire.beginTransmission(TAIJI_TOUCH_ADDRESS);Wire.write(0x00);Wire.write(0x00);Wire.endTransmission();
  transferDone=xSemaphoreCreateBinary();
  stripe=(uint16_t*)heap_caps_malloc(360*12*2,MALLOC_CAP_DMA|MALLOC_CAP_INTERNAL);
  if(!transferDone||!stripe)return ESP_ERR_NO_MEM;
  spi_bus_config_t bus=SH8601_PANEL_BUS_QSPI_CONFIG(TAIJI_LCD_CLK,TAIJI_LCD_D0,TAIJI_LCD_D1,TAIJI_LCD_D2,TAIJI_LCD_D3,360*12*2);
  esp_err_t result=spi_bus_initialize(SPI2_HOST,&bus,SPI_DMA_CH_AUTO);if(result!=ESP_OK)return result;
  esp_lcd_panel_io_handle_t io=nullptr;
  esp_lcd_panel_io_spi_config_t config=SH8601_PANEL_IO_QSPI_CONFIG(TAIJI_LCD_CS,onTransfer,nullptr);
  config.pclk_hz=40000000;config.trans_queue_depth=1;
  result=esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST,&config,&io);if(result!=ESP_OK)return result;
  sh8601_vendor_config_t vendor={};vendor.init_cmds=taiji_lcd_init_cmds;vendor.init_cmds_size=taiji_lcd_init_count;vendor.flags.use_qspi_interface=1;
  esp_lcd_panel_dev_config_t device={};device.reset_gpio_num=TAIJI_LCD_RESET;device.rgb_ele_order=LCD_RGB_ELEMENT_ORDER_RGB;device.bits_per_pixel=16;device.vendor_config=&vendor;
  result=esp_lcd_new_panel_sh8601(io,&device,&panel);if(result!=ESP_OK)return result;
  result=esp_lcd_panel_reset(panel);if(result!=ESP_OK)return result;
  result=esp_lcd_panel_init(panel);if(result!=ESP_OK)return result;
  backlightPwm=ledcAttach(TAIJI_BACKLIGHT,backlightHz,backlightBits);
  if(!backlightPwm){pinMode(TAIJI_BACKLIGHT,OUTPUT);digitalWrite(TAIJI_BACKLIGHT,HIGH);Serial.println("TAIJI BACKLIGHT PWM FAILED: full brightness fallback");}
  taijiSetBrightness(70);ready=true;return ESP_OK;
}
bool taijiDisplayReady(){return ready;}
void taijiSetBrightness(unsigned percent){if(backlightPwm)ledcWrite(TAIJI_BACKLIGHT,min(percent,100U)*((1U<<backlightBits)-1)/100);else digitalWrite(TAIJI_BACKLIGHT,percent?HIGH:LOW);}
bool taijiTouch(uint16_t &x,uint16_t &y){
  Wire.beginTransmission(TAIJI_TOUCH_ADDRESS);Wire.write(0x02);
  if(Wire.endTransmission(false)!=0)return false;
  if(Wire.requestFrom((uint8_t)TAIJI_TOUCH_ADDRESS,(uint8_t)5)!=(size_t)5)return false;
  uint8_t data[5];for(unsigned i=0;i<5;i++)data[i]=Wire.read();
  if(!(data[0]&0x0f))return false;
  x=((data[1]&0x0f)<<8)|data[2];y=((data[3]&0x0f)<<8)|data[4];
  return x<360&&y<360;
}
esp_err_t taijiPresent(const uint8_t *bits,bool native,int selection){
  if(!ready||!bits)return ESP_ERR_INVALID_STATE;
  for(int top=0;top<360;top+=12){
    for(int row=0;row<12;row++)for(int x=0;x<360;x++){
      int y=top+row,dx=x-180,dy=y-180;
      uint16_t color=0x0843;
      if(dx*dx+dy*dy>178*178)color=0;
      else if(native){
        bool ink=bits[y*45+x/8]&(0x80>>(x%8));
        bool selected=selection>=0&&x>=54&&x<=306&&y>=114+selection*72&&y<174+selection*72;
        color=ink?(selected?0xbff8:0xe7fc):0x0843;
      }else if(x>=60&&x<300&&y>=60&&y<300){
        int sx=(x-60)*200/240,sy=(y-60)*200/240;
        bool white=bits[sy*25+sx/8]&(0x80>>(sx%8));color=white?0x0843:0xe7fc;
      }
      stripe[row*360+x]=(color>>8)|(color<<8);
    }
    while(xSemaphoreTake(transferDone,0)==pdTRUE){}
    esp_err_t result=esp_lcd_panel_draw_bitmap(panel,0,top,360,top+12,stripe);
    if(result!=ESP_OK)return result;
    if(xSemaphoreTake(transferDone,pdMS_TO_TICKS(1000))!=pdTRUE){ready=false;return ESP_ERR_TIMEOUT;}
  }return ESP_OK;
}
