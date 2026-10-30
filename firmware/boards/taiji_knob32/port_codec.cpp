#include <Arduino.h>
#include <driver/i2s_pdm.h>
#include <driver/i2s_std.h>
#include "port_codec.h"
#include "taiji_pins.h"
static i2s_chan_handle_t mic=nullptr,speaker=nullptr;
void Codec_StartInit(){
  // Vendor board routes the PCM5100A to the S3 while GPIO0 is HIGH.
  // GPIO0 must remain an output: recording uses the touchscreen, not BOOT.
  pinMode(TAIJI_AUDIO_OWNER,OUTPUT);digitalWrite(TAIJI_AUDIO_OWNER,HIGH);
  i2s_chan_config_t rx=I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0,I2S_ROLE_MASTER);
  ESP_ERROR_CHECK(i2s_new_channel(&rx,nullptr,&mic));
  i2s_pdm_rx_config_t pdm={};
  pdm.clk_cfg=I2S_PDM_RX_CLK_DEFAULT_CONFIG(16000);
  pdm.slot_cfg=I2S_PDM_RX_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT,I2S_SLOT_MODE_MONO);
  pdm.gpio_cfg.clk=TAIJI_MIC_CLOCK;pdm.gpio_cfg.din=TAIJI_MIC_DATA;
  ESP_ERROR_CHECK(i2s_channel_init_pdm_rx_mode(mic,&pdm));
  ESP_ERROR_CHECK(i2s_channel_enable(mic));
  i2s_chan_config_t tx=I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_1,I2S_ROLE_MASTER);
  ESP_ERROR_CHECK(i2s_new_channel(&tx,&speaker,nullptr));
  i2s_std_config_t dac={};dac.clk_cfg=I2S_STD_CLK_DEFAULT_CONFIG(16000);
  dac.slot_cfg=I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT,I2S_SLOT_MODE_STEREO);
  dac.gpio_cfg.mclk=I2S_GPIO_UNUSED;dac.gpio_cfg.bclk=TAIJI_DAC_BCLK;
  dac.gpio_cfg.ws=TAIJI_DAC_WS;dac.gpio_cfg.dout=TAIJI_DAC_DATA;dac.gpio_cfg.din=I2S_GPIO_UNUSED;
  ESP_ERROR_CHECK(i2s_channel_init_std_mode(speaker,&dac));ESP_ERROR_CHECK(i2s_channel_enable(speaker));
}
esp_err_t Codec_Init(const char *){return mic&&speaker?ESP_OK:ESP_ERR_INVALID_STATE;}
esp_err_t Codec_PlaybackData(uint8_t *buffer,size_t bytes){
  if(!speaker)return ESP_ERR_INVALID_STATE;
  size_t written=0;esp_err_t result=i2s_channel_write(speaker,buffer,bytes,&written,500);
  return result==ESP_OK&&written!=bytes?ESP_ERR_TIMEOUT:result;
}
esp_err_t Codec_RecordData(uint8_t *buffer,size_t bytes){
  if(!mic || bytes%4)return ESP_ERR_INVALID_ARG;
  // Existing capture contract is interleaved stereo 16 kHz. Duplicate the
  // PDM mono channel so common cache/upload logic stays byte-for-byte shared.
  int16_t mono[480];int16_t *out=(int16_t*)buffer;size_t frames=bytes/4,offset=0;
  while(offset<frames){size_t count=min((size_t)480,frames-offset),read=0;
    esp_err_t result=i2s_channel_read(mic,mono,count*2,&read,500);
    if(result!=ESP_OK)return result;if(read!=count*2)return ESP_ERR_TIMEOUT;
    for(size_t i=0;i<count;i++)out[2*(offset+i)]=out[2*(offset+i)+1]=mono[i];
    offset+=count;
  }return ESP_OK;
}
