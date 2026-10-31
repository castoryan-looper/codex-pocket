#pragma once
#include "taiji_port.h"
#include "round_interaction.h"
#include "quadrature.h"
#include "directional_pulse_encoder.h"
#include "round_geometry.h"
#include "stream_gestures.h"
#include "stream_motion.h"
#include "taiji_local_ui.h"
static void taijiAscii(uint8_t *image,int px,int py,const String &text,int scale=1){
  for(unsigned n=0;n<text.length();n++){unsigned char c=text[n];if(c<32||c>126)c='?';
    for(int y=0;y<12;y++)for(int x=0;x<7;x++)if(asciiFont[c-32][y]&(0x40>>x))
      for(int sy=0;sy<scale;sy++)for(int sx=0;sx<scale;sx++){int xx=px+(n*7+x)*scale+sx,yy=py+y*scale+sy;if(xx>=0&&xx<360&&yy>=0&&yy<360)image[yy*45+xx/8]|=0x80>>(xx%8);}
  }
}
static pocket::StreamGestures taijiStreamGestures;
static pocket::StreamMotion taijiMotion;
static char motionOrigin[33]={};
static void taijiPublishMotion(){
 xSemaphoreTake(imageMutex,portMAX_DELAY);streamVisualOffset=taijiMotion.offset;streamVisualPhase=(int)taijiMotion.phase;xSemaphoreGive(imageMutex);
}
static bool taijiStreamEvent(const char *action,const char *token){
 if(!token[0])return false;JsonDocument doc;doc["type"]="event";doc["action"]=action;doc["payload"]["token"]=token;return sendPacket(doc);
}
static pocket::DirectionalPulseEncoder taijiEncoder;
static portMUX_TYPE taijiEncoderLock=portMUX_INITIALIZER_UNLOCKED;
static void ARDUINO_ISR_ATTR taijiEncoderEdge(){
  uint8_t state=(gpio_get_level((gpio_num_t)TAIJI_ENCODER_A)<<1)|gpio_get_level((gpio_num_t)TAIJI_ENCODER_B);
  portENTER_CRITICAL_ISR(&taijiEncoderLock);taijiEncoder.edge(state,micros());taijiEncoderEdges++;portEXIT_CRITICAL_ISR(&taijiEncoderLock);
}
static void screenTask(void*){
  screenPhase=1;esp_err_t initialized=taijiDisplayInit();
  if(initialized!=ESP_OK){Serial.printf("TAIJI DISPLAY ERROR %d\n",initialized);vTaskDelete(nullptr);return;}
  Preferences displayPrefs;displayPrefs.begin("pocket",true);taijiBrightness=constrain(displayPrefs.getUInt("brightness",70),10U,100U);
  unsigned orientation=displayPrefs.getUInt("orientation",3);taijiSetOrientation((orientation&1)?180:0,(orientation&2)?180:0);displayPrefs.end();taijiSetBrightness(taijiBrightness);screenPhase=2;
  uint8_t *snapshot=(uint8_t*)heap_caps_malloc(16200,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT),*scratch=(uint8_t*)heap_caps_malloc(16200,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
  if(!snapshot||!scratch){Serial.println("TAIJI FRAME ALLOCATION FAILED");vTaskDelete(nullptr);return;}
  int previousLocal=-100,previousSeconds=-1,lastOffset=0,lastPhase=0,lastSelection=-9;bool previousConfig=false,denseShown=false,hasMask=false;
  uint32_t batteryAt=0,preparedRevision[2]={};char cachedToken[33]={};bool previousReady=false,nextReady=false;
  for(;;){
    if(localBattery<0||millis()-batteryAt>30000){localBattery=Get_Batterylevel();batteryAt=millis();}
    int ui=localUi;int seconds=recording?(millis()-recordingStartedAt)/1000:-1;
    if(ui==UiIdle&&!configModeActive){if(WiFi.status()!=WL_CONNECTED)ui=8;else if(!bridgeConfigured)ui=10;else if(!netConnected)ui=9;else if(!online)ui=11;}
    if(ui!=previousLocal||seconds!=previousSeconds||previousConfig!=(bool)configModeActive)imageDirty=true;
    int phase=streamVisualPhase;
    bool defer=denseShown&&ui==UiIdle&&!configModeActive&&(phase==1||phase==2||(phase==3&&!strcmp(cachedToken,streamToken)));
    if(imageDirty&&!defer){
      xSemaphoreTake(imageMutex,portMAX_DELAY);
      bool native=roundNative&&online&&netConnected&&ui==UiIdle&&!configModeActive;
      int view=roundView,selected=(view==0||view==4)?roundSelection:-1;
      bool dense=native&&view==4&&streamLayout==2;
      bool same=dense&&denseShown&&hasMask&&lastSelection==selected&&!memcmp(snapshot,roundDesired,16200);
      memcpy(snapshot,native?roundDesired:desiredImage,native?16200:5000);
      String hash=pendingHash;imageDirty=false;char snapshotToken[33];strlcpy(snapshotToken,streamToken,sizeof(snapshotToken));xSemaphoreGive(imageMutex);
      if(configModeActive){
        selected=-1;native=true;bool usb=HWCDC::isPlugged();unsigned index=usb?12:13;pocket::decodeBitmapRle(taijiLocalStates[index],taijiLocalSizes[index],snapshot,16200);
        if(usb){taijiAscii(snapshot,65,164,"https://43.160.215.253");taijiAscii(snapshot,134,184,":8448/setup/");}
        else{taijiAscii(snapshot,75,153,portalSsid);taijiAscii(snapshot,75,175,"KEY: "+portalKey);taijiAscii(snapshot,75,197,"192.168.4.1:80/");}
        taijiAscii(snapshot,145,238,String(taijiBrightness)+"%",2);
      }else if(ui>=0&&ui<12){
        selected=-1;native=true;pocket::decodeBitmapRle(taijiLocalStates[ui],taijiLocalSizes[ui],snapshot,16200);
        if(ui==UiRecording||ui==UiLimit)taijiAscii(snapshot,117,204,String(seconds<0?recordSeconds:seconds)+" / "+String(recordSeconds)+"s",2);
        if(ui==UiSending)taijiAscii(snapshot,145,204,String(uploadProgress)+"%",2);
        if(ui==UiAudioError)taijiAscii(snapshot,65,210,audioFailure);
      }
      esp_err_t result=ESP_OK;
      if(dense){if(!same)taijiCacheFrame(0,snapshot,selected,5);result=taijiPresentCached(0,false,false);}
      else result=taijiPresent(snapshot,native,selected,configModeActive?-2:ui==UiRecording?-3:ui==UiIdle?view:-1);
      screenPhase=result==ESP_OK?4:5;
      if(result==ESP_OK){
        bool changed=strcmp(cachedToken,snapshotToken)!=0;
        denseShown=dense;hasMask=dense;lastSelection=selected;
        if(dense){strlcpy(cachedToken,snapshotToken,33);if(changed){previousReady=nextReady=false;preparedRevision[0]=preparedRevision[1]=0;}}
        else{cachedToken[0]=0;previousReady=nextReady=false;}
        xSemaphoreTake(imageMutex,portMAX_DELAY);
        bool keepDrag=dense&&phase!=3&&streamVisualPhase!=0;
        strlcpy(shownStreamToken,native&&view==4&&!configModeActive&&ui==UiIdle&&!keepDrag?snapshotToken:"",sizeof(shownStreamToken));
        if(!keepDrag){streamVisualOffset=streamVisualPhase=0;}streamCachedPrevious=previousReady;streamCachedNext=nextReady;xSemaphoreGive(imageMutex);
        lastOffset=lastPhase=0;partialRefreshes++;JsonDocument ack;ack["type"]="frame_ack";ack["hash"]=hash;
        if(native&&view==4&&!configModeActive&&ui==UiIdle)ack["streamToken"]=snapshotToken;
        ack["wifiConnected"]=WiFi.status()==WL_CONNECTED;ack["usbConnected"]=HWCDC::isPlugged();ack["configurationMode"]=(bool)configModeActive;sendPacket(ack);
      }
      previousLocal=ui;previousSeconds=seconds;previousConfig=configModeActive;
    }
    if(denseShown&&ui==UiIdle&&!configModeActive&&online){
      for(unsigned i=0;i<2;i++){
        xSemaphoreTake(imageMutex,portMAX_DELAY);uint32_t revision=streamPreviewRevision[i];int selection=streamPreviewSelection[i];
        bool prepare=revision!=preparedRevision[i]&&!strcmp(previewAnchor[i],cachedToken)&&streamPreviewBits[i];
        if(prepare)memcpy(scratch,streamPreviewBits[i],16200);xSemaphoreGive(imageMutex);
        if(prepare){taijiCacheFrame(i+1,scratch,selection,5);preparedRevision[i]=revision;if(i==0)previousReady=true;else nextReady=true;}
      }
      xSemaphoreTake(imageMutex,portMAX_DELAY);streamCachedPrevious=previousReady;streamCachedNext=nextReady;phase=streamVisualPhase;int offset=streamVisualOffset;xSemaphoreGive(imageMutex);
      if((phase||lastPhase)&&(offset!=lastOffset||phase!=lastPhase)){
        esp_err_t result=taijiPresentCached(offset,previousReady,nextReady);screenPhase=result==ESP_OK?4:5;
        if(result==ESP_OK){
          xSemaphoreTake(imageMutex,portMAX_DELAY);
          if(phase)shownStreamToken[0]=0;
          else if(!strcmp(cachedToken,streamToken))strlcpy(shownStreamToken,cachedToken,33);
          xSemaphoreGive(imageMutex);lastOffset=offset;lastPhase=phase;
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(streamVisualPhase?1:12));
  }
}

static void taijiLoop(){
  if(!taijiDisplayReady())return;
  static uint32_t lastSample=0,touchAt=0,lastTap=0,brightnessAt=0,presenceAt=0;static char startStreamToken[33]={},gestureStreamToken[33]={};static bool encoderAttached=false;
  static bool touchDown=false,micGesture=false,micStarted=false;static uint16_t startX=0,startY=0,currentX=0,currentY=0;
  static pocket::RoundHit initialHit=pocket::RoundHit::None;
  uint32_t now=millis();
  if(now-lastSample<5)return;lastSample=now;char activeToken[33],liveToken[33];
  xSemaphoreTake(imageMutex,portMAX_DELAY);strlcpy(activeToken,shownStreamToken,sizeof(activeToken));strlcpy(liveToken,streamToken,sizeof(liveToken));xSemaphoreGive(imageMutex);
  if(taijiMotion.phase!=pocket::StreamMotion::Idle){
    if(configModeActive||localUi!=UiIdle||!online||!netConnected||(taijiMotion.phase==pocket::StreamMotion::Dragging&&strcmp(motionOrigin,liveToken))||(activeToken[0]&&strcmp(motionOrigin,activeToken)))taijiMotion.clear();
    else taijiMotion.tick(now);
    taijiPublishMotion();
  }
  if(roundView==4&&localUi==UiIdle&&!configModeActive&&!recording&&!uploadPending&&online&&netConnected&&taijiMotion.phase==pocket::StreamMotion::Idle&&now-presenceAt>=2000){
   presenceAt=now;JsonDocument presence;presence["type"]="stream_presence";presence["streamToken"]=activeToken;sendPacket(presence);
  }
  if(brightnessAt&&now-brightnessAt>1500){Preferences store;store.begin("pocket",false);store.putUInt("brightness",taijiBrightness);store.end();brightnessAt=0;}
  if(!encoderAttached){taijiEncoder.begin((digitalRead(TAIJI_ENCODER_A)<<1)|digitalRead(TAIJI_ENCODER_B));attachInterrupt(TAIJI_ENCODER_A,taijiEncoderEdge,CHANGE);attachInterrupt(TAIJI_ENCODER_B,taijiEncoderEdge,CHANGE);encoderAttached=true;}
  portENTER_CRITICAL(&taijiEncoderLock);int steps=taijiEncoder.take(now);taijiEncoderMode=taijiEncoder.mode;portEXIT_CRITICAL(&taijiEncoderLock);
  for(int n=0;n<abs(steps);n++){int direction=steps>0?1:-1;
    taijiEncoderCount++;taijiEncoderDirection=direction;
    if(pocket::openRoundSettingsOnTurn(online,netConnected,recording,captureRequested,uploadPending)&&!configModeActive)startConfigPortal(false);
    if(configModeActive&&!recording&&!captureRequested&&!uploadPending){taijiBrightness=constrain((int)taijiBrightness+direction*5,10,100);taijiSetBrightness(taijiBrightness);brightnessAt=now;imageDirty=true;}
    if(!configModeActive&&!recording&&!captureRequested&&!uploadPending&&online&&netConnected){
      if(roundView==4){if(taijiMotion.phase==pocket::StreamMotion::Idle){taijiStreamGestures.clear();taijiStreamEvent(direction>0?"stream_next":"stream_prev",activeToken);}}
      else event(roundView==3?(direction>0?"round_next":"round_prev"):(direction>0?"knob_right":"knob_left"));
    }
  }
  uint16_t x=0,y=0,rawX=0,rawY=0;bool touched=taijiTouch(x,y,&rawX,&rawY);
  if(taijiMotion.phase!=pocket::StreamMotion::Idle&&taijiMotion.phase!=pocket::StreamMotion::Dragging&&!touchDown)return;
  if(taijiOrientationChanged){touchDown=false;micGesture=false;taijiMicHeld=false;if(!touched)taijiOrientationChanged=false;return;}
  if(touched){
    taijiTouchX=x;taijiTouchY=y;
    taijiTouchRawX=rawX;taijiTouchRawY=rawY;
    currentX=x;currentY=y;
    if(!touchDown){touchDown=true;touchAt=now;strlcpy(startStreamToken,activeToken,sizeof(startStreamToken));startX=x;startY=y;
      taijiTouchCount++;
      initialHit=pocket::roundHit(x,y,360,roundView==0?pocket::RoundView::Choices:roundView==1?pocket::RoundView::Confirm:pocket::RoundView::Reader);
      if(!roundNative&&initialHit!=pocket::RoundHit::Mic&&initialHit!=pocket::RoundHit::Tools&&initialHit!=pocket::RoundHit::Back&&initialHit!=pocket::RoundHit::None)initialHit=pocket::RoundHit::Center;
      micStarted=false;micGesture=initialHit==pocket::RoundHit::Mic&&!configModeActive&&roundCanRecord&&!remoteVoiceBusy&&!remoteDraftReady&&!uploadPending&&online&&netConnected;
      if(micGesture){taijiStreamGestures.clear();taijiMicHeld=true;pressStartedAt=now;}
    }
    if(roundView==4&&streamLayout==2&&!micGesture&&startY>=24&&startY<270&&startStreamToken[0]){
      int dy=(int)y-startY,dx=(int)x-startX;
      if(taijiMotion.phase==pocket::StreamMotion::Idle&&abs(dy)>=6&&abs(dy)>abs(dx)){
        strlcpy(motionOrigin,startStreamToken,33);taijiStreamGestures.clear();taijiMotion.begin();
      }
      if(taijiMotion.phase==pocket::StreamMotion::Dragging){taijiMotion.drag(dy,streamCachedPrevious,streamCachedNext);taijiPublishMotion();}
    }
    if(micGesture && !micStarted && now-touchAt>=holdMs){micStarted=true;captureRequested=true;}
    if(micGesture && (abs((int)x-startX)>90||abs((int)y-startY)>90)){
      captureCancelled=true;recording=false;captureRequested=false;taijiMicHeld=false;micGesture=false;setLocalUi(UiCancelled);
    }
    return;
  }
  if(!touchDown)return;touchDown=false;
  if(taijiMotion.phase==pocket::StreamMotion::Dragging){
    int direction=0;
    if(!strcmp(motionOrigin,liveToken))direction=taijiMotion.release(now,streamCanPrevious,streamCanNext,streamCachedPrevious,streamCachedNext);
    else taijiMotion.clear();
    if(direction&&!taijiStreamEvent(direction>0?"stream_next":"stream_prev",motionOrigin))taijiMotion.clear();
    taijiPublishMotion();return;
  }
  if(micGesture){bool captured=recording;taijiMicHeld=false;captureRequested=false;recording=false;micGesture=false;
    if(captured)setLocalUi(UiSending);else if(localUi==UiHold)setLocalUi(UiIdle);return;
  }
  if(configModeActive){taijiStreamGestures.clear();portalCloseAt=now+100;return;}
  if(recording||captureRequested||uploadPending)return;
  int swipeY=(int)currentY-startY,swipeX=(int)currentX-startX;
  if(roundView==4){
    if(!online||!netConnected||strcmp(startStreamToken,activeToken)){taijiStreamGestures.clear();return;}
    if(abs(swipeX)<20&&abs(swipeY)<20&&initialHit==pocket::RoundHit::Tools){taijiStreamGestures.clear();startConfigPortal(false);return;}
    if(strcmp(gestureStreamToken,activeToken))taijiStreamGestures.clear();
    strlcpy(gestureStreamToken,activeToken,sizeof(gestureStreamToken));
    auto intent=taijiStreamGestures.release(startX,startY,swipeX,swipeY,now-touchAt,now,streamLayout==2?62:94);
    if(intent==pocket::StreamIntent::Next)taijiStreamEvent("stream_next",activeToken);
    else if(intent==pocket::StreamIntent::Previous)taijiStreamEvent("stream_prev",activeToken);
    else if(intent==pocket::StreamIntent::Confirm)taijiStreamEvent("stream_confirm",startStreamToken);
    return;
  }
  if(roundView==0&&abs(swipeX)>45&&abs(swipeX)>abs(swipeY)){if(online&&netConnected)event("knob_feed");return;}
  if(roundView==2 && (abs(swipeY)>35||abs(swipeX)>35)){
    if(online&&netConnected)event((abs(swipeY)>abs(swipeX)?swipeY:swipeX)<0?"knob_right":"knob_left");return;
  }
  if(abs(swipeX)>25||abs(swipeY)>25||now-lastTap<220)return;
  lastTap=now;
  if(initialHit==pocket::RoundHit::Tools){startConfigPortal(false);return;}
  if(initialHit==pocket::RoundHit::Back){if(online&&netConnected)event(roundView==3?"b2_click":roundView==0?"knob_feed":"knob_back");return;}
  if(!online||!netConnected)return;
  if(roundView==0 && initialHit==pocket::RoundHit::A)event("rec_a");
  else if(roundView==0 && initialHit==pocket::RoundHit::B)event("rec_b");
  else if(roundView==1 && initialHit==pocket::RoundHit::Details)event("rec_details");
  else if(roundView==3&&startY>=216&&startY<=263)event("knob_click");
  else if(initialHit==pocket::RoundHit::Center||roundView==2)event("knob_click");
}
