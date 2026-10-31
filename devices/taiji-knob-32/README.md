# Taiji Knob 32 适配工作区

版本：codex-pocket-taiji-0.15.1。按用户确认的 ESP32-S3、1.8 英寸 360×360 规格，以 Waveshare ESP32-S3-Knob-Touch-LCD-1.8 原厂 Demo 为板级参考。屏幕、触摸、旋钮、PDM 麦克风和 I2S 输出已接入共享固件，构建目标为 taiji-knob32。S3 已实机刷入，USB、PSRAM 缓存、Wi-Fi 与云端连接、屏幕方向和触摸命中已验证。128 KB 可取消诊断上传完成且无掉线；真实语音识别及外接音频播放仍需听测验收。其他同名硬件修订版需核对原理图。

## 已实现的硬件无关部分

firmware/common/round_interaction.h 定义交互策略与圆形命中区域，板级 taiji_runtime.h 使用相同触摸区域并通过现有队列发送明确操作。firmware/common/directional_pulse_encoder.h 按本板原厂方向脉冲接线分别计数，使用微秒计时与 8 ms 去重，不再等待两路一起回到空闲，旋钮中断只计数，不进行网络或屏幕操作；翻页和提交仍由现有云端操作锁保护。

- 主屏同时放 A/B，短行动词优先；目标最多一行，完整内容按需查看。
- 旋钮浏览，触摸 A/B 或中心打开单屏确认，再触摸“开始执行”提交；不假设旋钮有按压开关。
- 详情旋转前后翻页，也支持上/下滑动；到边界停止。轻触返回确认，末页不执行。
- 明确的话筒区域按住录音，松开结束；返回、断线或触摸取消必须终止采集。
- 网络未连接时仍允许进入设置；任务、审批、草稿和录音进行时禁止发出新的推荐执行。任务执行中可用语音补充要求，沿用已有 turn/steer 流程。
- 触摸区域以 360 基准坐标归一化，并做圆形边界检查；坐标归一化不表示已确认屏幕分辨率。

## 驱动接入前必须核对

1. 购买链接、厂家型号、PCB/屏幕版本与原厂 Demo。
2. 主芯片、Flash/PSRAM、USB 接到主芯片还是音频伴随芯片。
3. LCD 控制器、初始化表和 QSPI 接线；触摸芯片、坐标方向、I2C 地址和复位。
4. 旋钮 A/B 与按压能力；不要把 BOOT 当作旋钮按压。
5. 麦克风是 PDM 还是标准 I2S；音频 DAC 是否由另一颗 MCU 控制；输出是内置扬声器还是耳机接口。
6. 电池检测、背光、震动与电源管理。

参考资料存在差异：Waveshare 的 ESP32-S3-Knob-Touch-LCD-1.8 使用双 MCU；Guition JC3636K518 厂家组件列出 ST77916，但部分其他版本采用不同显示初始化。不能只凭 TAIJI KNOB 32 名称选择驱动。

厂家资料：

- https://www.waveshare.com/wiki/ESP32-S3-Knob-Touch-LCD-1.8
- https://components.espressif.com/components/guition-jc/guition_boards/versions/0.1.3/readme

设备认证、8 个 Wi-Fi 记忆、USB/AP 配置、蓝牙外接旋钮、声音和取消型录音上传均复用现有应用。S3 用 PSRAM 缓存一段最长 20 秒录音，不格式化原厂 Flash 数据；断电后不保留未发送录音。PCM5100A 输出按原厂接线走 3.5 mm 音频口，需要耳机或有源音箱，不假设有内置扬声器。GPIO0 用于音频总线控制，不当作用户按压键。

云端按 hello.displayProfile=taiji360 协商 360×360 中文原生布局，兼容 C6 的旧 200×200 协议。圆屏位图使用有界 PackBits 编码，单条消息保持在原 WebSocket 限制内；旧网关可回退到居中旧画面。圆屏正文、A/B、确认与任务菜单不是简单裁切。原生布局需要部署带 bridge/round_display.py 的 cloud 网关。

## 编译、配置和烧录

```powershell
.\build.ps1 -Device taiji-knob32
```

依赖 ESP32 Arduino 3.3.0、ArduinoJson 7.3.2、WebSockets 2.7.2 和随仓库提供的 OneButton。无需 LVGL、SensorLib 或额外图形库；屏幕采用原厂 SH8601 QSPI 驱动及原厂初始化表，来源与哈希在 firmware/boards/taiji_knob32/UPSTREAM.json。位图资源可用 tools/generate_taiji_ui.py 重新生成。

该参考板的 USB-C 插头方向可能连接不同 MCU。必须先确认 esptool 检测结果为 ESP32-S3；CH340 串口可能连接的是音频伴随 ESP32，不能向其写入 S3 固件。烧录脚本固定指定 esp32s3，会拒绝芯片不匹配。首次改写原厂固件前，私下备份原主芯片完整 Flash；不改写伴随芯片或 TF 卡。

```powershell
# 将 COMx 换成已确认的 S3 串口；不要直接使用音频芯片串口。
python -m esptool --chip esp32s3 --port COMx flash-id
New-Item -ItemType Directory -Force backup
python -m esptool --chip esp32s3 --port COMx read-flash 0 0x1000000 backup/taiji-s3-before-pocket.bin
.\flash.ps1 -Device taiji-knob32 -Port COMx
```

应用文件在 build/taiji-knob32/app/Pocket.ino.bin，完整安装镜像为同目录 Pocket.ino.merged.bin。完整安装会写入该目标的分区表；已有 Pocket 升级需先核对实际活动应用分区，再决定只写应用分区。备份含私密信息，不提交 GitHub。

0.12.5 包含首次未配网时先初始化网络栈再启动网页服务的修复。S3 圆屏缓存移至 PSRAM，背光采用适合默认 40 MHz XTAL 的 50 kHz / 9 位 PWM，并提供 GPIO 全亮回退。flash.ps1 在 S3 写入后调用 tools/start_s3_application.py 清除易失下载锁存并用看门狗启动，避免停留在 ROM 下载模式；只操作易失寄存器，不改写 eFuse。

USB status 另有 nativeEncoderEdges、nativeEncoderMode（0 待识别 / 1 方向脉冲 / 2 四相）、nativeEncoderCount、nativeEncoderDirection、nativeTouchCount、nativeTouchX/Y、displayBrightness、backlightHz、freePsram 和 maxFreeBlock 诊断字段。未配网时不能执行推荐或录音提交，应先通过屏幕右下“设置”进入配置，并在电脑的 USB 网页完成连接配置。

进入设置后屏幕显示 USB 公网管理页或 AP 地址、KEY，旋钮调节亮度，停止旋转 1.5 秒后保存，轻触退出。录音、断网和识别中的本地画面同样针对圆屏生成，不出现 BOOT/PWR 的旧按钮提示。

任务页旋转查看全文；左下“选项”切换任务或审批菜单，触摸当前菜单项确认。阅读页旋转/滑动翻页，轻触正文或左下返回确认。所有审批沿用明确选择流程。

## 主机验证

在有 cl/g++/clang++ 的环境运行：

```sh
python -m unittest discover -s tools -p test_round_interaction.py -v
```

测试直接编译实际头文件，覆盖确认与防重复执行、旋钮/滑动翻页及边界、录音结束/取消、离线配置、审批/草稿保护和触摸命中。主机测试不代替新设备驱动编译或实机验收。

## 方向与离线输入（0.12.5）

画面默认旋转 180°，触摸使用对应逻辑坐标；原生圆屏、旧画面兼容回退和高亮一起旋转。未联网时转动旋钮会进入配置并调整亮度，不再静默忽略。录音或上传期间保留输入保护。

USB RPC display_orientation 接受 displayRotation / touchRotation（各为 0 或 180）。两者以同一个 NVS 值保存并立即应用；拒绝 90° 等无效参数。status 提供两项角度以及原始和逻辑触摸坐标。分别设置可用于不同触摸层方向的硬件校准。

## 旋钮与配网诊断（0.12.6）

方向脉冲最短有效宽度与空闲稳定等待均为 2 ms；仍过滤 1 ms 毛刺，连续 3 ms 脉冲与反转有主机测试，避免较快转动时漏步。每个有效刻度保持一个操作，不通过倍增跳过 A/B 或正文页面。

USB status 的 wifiProfiles 增加 passwordSaved 布尔值，不返回密码；扫描结果增加 authMode，便于区分空密码、安全方式不兼容与认证失败。Wi-Fi 返回 210 时先核对热点安全方式及已保存密码；云端连接还需要导入对应服务器连接配置，单独保存 Wi-Fi 不代表云端已绑定。

## 彩色 A/B 主屏（0.13.0）

原生圆屏采用深蓝背景、薄荷绿/蓝紫 A/B 卡片和橙色语音按钮；CODEX 终端标识与字标位于顶部。颜色在 S3 的 RGB565 渲染层生成，沿用有界单色文字掩码协议，不扩大网络消息与帧缓存。触摸坐标和显示旋转继续共用既有校准。

首页转动选择 A/B、轻触打开确认；确认后再点“开始执行”。首页左滑或点左下“换一组”触发现有推荐换方向逻辑，阅读详情时旋转/滑动仍只翻页。扩大语音触摸区域到 (108,270)–(252,320)，持续按住开始录音，松开结束，滑出取消。忙碌状态仍遵循已有云端锁与审批流程。

亮度每个有效方向脉冲调整 5%，不靠倍增跳过正常界面的选项。方向脉冲解码只用于已核对的 Taiji 板；原 C6 的外接蓝牙旋钮保持原处理流程。

## 纵向任务流（0.14.0）

hello.taskStream=1 协商新版 roundView=4；旧服务仍可回退到 A/B。上滑下一张、下滑上一张，旋钮等价浏览；卡片正文内双击确认，单击、滑动和停留均不提交任务。双击间隔 60–450 ms，偏移不超过 28 px；滑动、长按或卡片变化会清除待确认手势。语音与设置仍保留原有独立触摸区。

streamToken 绑定真正完成 LCD 刷新的画面，不使用网络刚收到但尚未显示的卡片。换卡/状态改变后旧 token 不能确认新卡；每 2 秒上报实际可见心跳供服务器计停留，配置、录音和上传时暂停。颜色、180° 方向与亮度设置沿用 0.13.0。

推荐排序、队列、并行会话、决策和偏好数据由 cloud 的 task_stream.py 处理。状态通过任务卡显示；开放问题可在对应问题卡按住说话，识别后再双击提交答案。固件不会根据停留或滑动自动批准任何请求。

## 正文排版与跟手滑动（0.15.0）

当前及前后两张卡片缓存为 PSRAM RGB565 画面；新增缓存约 760 KiB，文字掩码另增加两个 16200-byte 预览缓冲。底层以 DMA 条带合成中间 y=62..269 的移动区域，品牌栏和语音/设置按钮固定。大段标题支持两行，顶部压缩并保留小时间；正文改为 280 px / 19 pt，并加入具体说明。

上下拖动在本地更新位移，松手 150 ms 吸附；无上一页时有阻尼回弹。前后预览不包含可执行 token，移动中清空 shownStreamToken，等待权威画面实际刷新后才允许双击。超时回弹，离线/配置/录音时取消动画。streamLayout=1 继续使用旧布局与手势，C6 不加载这些缓冲。

USB status 的 streamRenderMicros 是最近一次缓存帧 DMA 刷新耗时，streamMotionFrames 是缓存刷新累计数；它们是诊断值，不等同于端到端输入延迟或稳定帧率。

0.15.1 将 RGB 缓存按物理方向预旋转，绘制改为按行拷贝和圆形行边界裁切；拖动仅传输中间区域，避免每个像素重复求坐标。显示方向改变时会使缓存重新生成。实机完整帧一次测得 35.3 ms（合成 11.7 ms、传输 23.6 ms），而先前逐像素版本约 85.7 ms；这不是固定帧率或端到端触摸延迟承诺。
