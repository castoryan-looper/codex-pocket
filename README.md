# Codex Pocket：ESP32 随身终端

支持两个独立构建目标：Waveshare ESP32-C6-ePaper-1.54（200×200 墨水屏，固件 **0.11.1**），以及 [Taiji Knob 32 / ESP32-S3 1.8 英寸旋钮圆屏](devices/taiji-knob-32/README.md)（360×360，固件 **0.15.0**）。S3 已实机刷入并验证 USB、Wi-Fi/云端连接和录音上传；圆屏提供彩色 A/B 卡片、CODEX 标识、扩大语音按钮与方向脉冲旋钮输入。

本仓库保存设备固件、USB/AP 管理页面和设备端测试。云端网关、目标推荐及部署由 [codex-pocket-cloud](https://github.com/castoryan-looper/codex-pocket-cloud) 维护。本次将此前集中在 cloud 仓库中的设备更新同步回来；来源提交及固件文件哈希记录在 firmware-source.json。两个仓库目前不是自动双向同步关系。

## 管理设备

USB 数据线连接电脑，在 Chrome 或 Edge 打开 [云端 USB 管理页](https://43.160.215.253:8448/setup/)，使用受信任的 HTTPS，点击连接 USB 设备。配置数据由浏览器直接通过 Web Serial 传给设备；刷写前点击“断开连接”释放串口。

USB 主机数据连接优先于设备配置热点；仅充电不算主机连接。没有 USB 主机时，同时按住 BOOT + PWR 约 3 秒，连接屏幕显示的 CPOCKET 热点，用屏幕 KEY 登录 http://192.168.4.1:80/。设备屏幕分别显示 USB 公网地址或 AP 地址。

配置页支持 Wi-Fi 扫描与保存后重启、最多 8 个已保存网络自动重连、蓝牙扫描/选择配对/忘记绑定，以及按键、屏幕和声音参数。详见 [Wi-Fi 记忆](WIFI_MEMORY.md)、[蓝牙旋钮](BLUETOOTH_KNOB.md)、[声音反馈](SOUND_FEEDBACK.md)。私人连接配置、设备密钥和 Flash 备份不能提交到仓库。

需要本地 USB 配置工具时运行 USB配置.cmd；本机 127.0.0.1:8788 只有该工具正在运行时可访问。日常直接使用上述公网管理页。

## 当前交互

- 云端推荐页 A/B 同屏显示短选项，旋转选择、按下进入一屏确认，再按下开始执行；不用先翻长说明。
- 主动打开详情后，旋转直接前后翻页，按下返回确认；按住逆时针逐层返回，主页按住顺时针换组。
- BOOT 按住录音，松开结束；任务和录音通过 Wi-Fi 连接网关，USB 用于管理及烧录。
- ANTICATER_MINI BLE 旋钮、连接/确认/录音/错误等提示音已支持。0.11.1 减小上传缓冲，改善蓝牙与 TLS 并存时的录音上传内存压力。
- 城市、当地时间与天气使用设备联网 IP 的大致定位；不代表精确 Wi-Fi 物理地址。

A/B 排序、目标进度和页面渲染在 cloud 网关实现；单独刷写固件不会升级云端逻辑。

## 编译与升级

使用 Arduino CLI、Espressif ESP32 核心 **3.3.0**、ArduinoJson **7.3.2**、WebSockets **2.7.2**；OneButton 随仓库提供。执行：

```powershell
.\build.ps1 -ArduinoCli arduino-cli
.\build.ps1 -Device taiji-knob32 -ArduinoCli arduino-cli
```

默认构建 C6；输出在 build/c6-epaper。S3 输出在 build/taiji-knob32/app；共享应用目录仍为 firmware/Pocket，构建时组合对应板级端口，不手工复制两套业务代码。S3 为 16 MB Flash、8 MB OPI PSRAM，使用硬件 USB CDC。

flash.ps1 使用 merged 镜像从地址 0 写入，适用于完整安装。已有设备升级前应备份 Flash 并核对分区与活动应用，仅写对应应用分区，避免覆盖设备配置；不要直接套用其他设备的偏移。此次只是同步代码，没有再次刷写设备。

## 测试与历史

GitHub Actions 执行固件策略、HID/GATT、Wi-Fi、上传帧、声音及配置网页测试。C++ 主机测试需要 cl、g++ 或 clang++，USB 工具测试需要 pyserial 3.5；主机测试不等于实机 BLE 或语音端到端验收。详见 VALIDATION.md。

原 bridge/ 与 start.ps1 等为历史本地网关实现，保留兼容参考，不是最新云端网关。旧使用方法移至 [历史本地桥接说明](docs/LEGACY_LOCAL_BRIDGE.md)；新部署使用 cloud 仓库。
