# Taiji Knob 32 适配工作区

版本：codex-pocket-taiji-0.12.0。按用户确认的 ESP32-S3、1.8 英寸 360×360 规格，以 Waveshare ESP32-S3-Knob-Touch-LCD-1.8 原厂 Demo 为板级参考。屏幕、触摸、旋钮、PDM 麦克风和 I2S 输出已接入共享固件，构建目标为 taiji-knob32。编译通过不代表已完成用户这台设备的实机验收；其他同名硬件修订版需核对原理图。

## 已实现的硬件无关部分

firmware/common/round_interaction.h 定义交互策略与圆形命中区域，板级 taiji_runtime.h 使用相同触摸区域并通过现有队列发送明确操作。firmware/common/quadrature.h 解码完整四相步进，旋钮中断只计数，不进行网络或屏幕操作；翻页和提交仍由现有云端操作锁保护。

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

进入设置后屏幕显示 USB 公网管理页或 AP 地址、KEY，旋钮调节亮度，停止旋转 1.5 秒后保存，轻触退出。录音、断网和识别中的本地画面同样针对圆屏生成，不出现 BOOT/PWR 的旧按钮提示。

任务页旋转查看全文；左下“选项”切换任务或审批菜单，触摸当前菜单项确认。阅读页旋转/滑动翻页，轻触正文或左下返回确认。所有审批沿用明确选择流程。

## 主机验证

在有 cl/g++/clang++ 的环境运行：

```sh
python -m unittest discover -s tools -p test_round_interaction.py -v
```

测试直接编译实际头文件，覆盖确认与防重复执行、旋钮/滑动翻页及边界、录音结束/取消、离线配置、审批/草稿保护和触摸命中。主机测试不代替新设备驱动编译或实机验收。
