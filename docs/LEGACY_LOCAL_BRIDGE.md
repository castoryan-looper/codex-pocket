# Codex Pocket：Wi-Fi 随身助手

适用：Waveshare ESP32-C6-ePaper-1.54，200×200，16MB Flash。当前版本 0.2.0。

设备日常只通过 Wi-Fi 发送录音、按键和状态，接收画面。电脑运行 Codex 与本地 Whisper，需保持在线。USB 仅用于烧录和启动日志，没有 USB 桥接或 USB 配网功能。

## 设备网页配置

1. 首次升级会自动打开设备配置热点；以后同时按住 BOOT 和 PWR 约 3 秒打开。
2. 墨水屏显示热点名称 `CPOCKET-xxxxxx` 与随机密码 `KEY`。用电脑或手机连接此热点。
3. 浏览器打开 **http://192.168.4.1/**，网页配置密码同屏幕上的 KEY。
4. 填写 2.4GHz 热点名称与密码。电脑连接可稍后配置；同一热点的密码留空会保留原密码。
5. 如需连接 Codex，在电脑控制台点击“下载电脑连接配置”，再在设备网页导入该 JSON，填入地址、端口、证书和设备密钥。
6. 可以设置 BOOT 长按门槛（800–2500ms）、单次录音上限（2–20 秒）及维护全刷最小间隔（5–60 分钟）。
7. 点击“保存并重启设备”。设置保存在 ESP32 的 NVS 中，断电不丢失。

设备连入 Wi-Fi 后，配置网页也可通过其局域网 IP 打开，仍需配置密码。同一电脑给设备开手机热点时，可用电脑或另一台手机连接设备的配置热点；配置完成后再恢复手机热点。

按住 PWR 约 1.2 秒可关闭配置热点，或点击网页上的“关闭配置热点”。未配网或热点连接失败约 30 秒后，配置热点会自动打开，便于恢复。配置模式显示配网信息，暂不执行 AI 按键操作。

配置网页供设备热点或可信局域网使用，不要将其 HTTP 端口映射到公网。密码和连接密钥不会在配置读取接口中返回；连接配置 JSON 含设备密钥，应按私人文件保存。

## 电脑控制台

运行：

```powershell
powershell -ExecutionPolicy Bypass -File .\start.ps1
```

首次安装会建立 Python 环境。离线语音模型需下载一次：

```powershell
.venv/Scripts/python setup-speech.py
```

控制台提供任务输入、回复、审批和设备状态，以及“下载电脑连接配置”；不再打开串口或写入设备配置。旧的 `/api/provision` 接口已删除。连接配置下载接口要求本机访问令牌，不对公网开放。

## 基本交互

| 操作 | 功能 |
|---|---|
| PWR 短按 | 选择菜单 |
| BOOT 短按 | 执行选中项 / 确认发送语音草稿 |
| BOOT 按住达到设定门槛 | 进入录音，看到“录音中”后说话 |
| 松开 BOOT | 停止录音，电脑识别后显示草稿 |
| PWR 长按约 1.2 秒 | 停止当前任务；配置模式下关闭配置热点 |
| BOOT + PWR 同时按住约 3 秒 | 打开设备配置热点 |

语音默认使用本机 Whisper small，CPU int8；录音不上传到语音服务。临时 WAV 在识别后删除。短录音、传输不完整、麦克风错误、识别失败会分别报告原因。默认单次最长 20 秒。

## 连接与显示

- Wi-Fi 图标：是否连接热点；USB 图标：是否检测到电脑 USB 总线。USB 图标不表示有任务桥接，也不表示电池正在充电。
- 图标由设备本地更新。正文分别显示未配网、热点断开、连接电脑中、桥接无响应或电脑连接未配置。
- 订阅额度显示真实剩余百分比，不能换算成一个确定的剩余 token 数。累计 token 来自 Codex 会话事件。
- 电量由 ADC 电压估算，充电时会偏高，不是精密电量计。
- 相同画面不刷屏；电量 1% 抖动不改变画面。维护全刷需至少 80 次变化，且满足设置的最小时间间隔；录音时及结束后 30 秒内不维护全刷。
- 开机和维护全刷会短暂黑白闪烁，之后应恢复正常。彻底断电后墨水屏仍会保留画面。

## 公网连接

电脑和设备在不同网络时，需要可用的固定 WSS 入口，指向电脑的 `http://127.0.0.1:8787`，路径 `/device`。设备使用独立随机密钥鉴权，TLS 验证服务器证书。

开发用 Cloudflare Quick Tunnel 地址会变化，也曾在实测中失效，不能作为长期稳定地址。可配置固定 Cloudflare Tunnel 或现有 Tailscale Funnel；Funnel 首次需在账户中启用。公网入口仍需实际验证，网页配置完成不等于电脑桥接在线。

## 编译、刷写与恢复

```powershell
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core install esp32:esp32@3.3.0 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli lib install ArduinoJson WebSockets
.\build.ps1
```

仅更新程序并保留配置：

```powershell
python -m pip install esptool
python -m esptool --chip esp32c6 --port COM6 --baud 921600 write-flash 0x10000 build/Pocket.ino.bin
```

`flash.ps1` 刷写合并固件，会清空 NVS；日常升级应使用上述仅更新程序方式。
`backup/original-flash.bin` 是本块设备修改前的完整 16MB 备份，按私人文件保管。恢复：

```powershell
python -m esptool --chip esp32c6 --port COM6 write-flash 0 backup/original-flash.bin
```

刷写失败可按住 BOOT 再连接 USB 进入下载模式。若整条 I²C 超时，断开 USB 与电池再接回；固件也会清除 GPIO 保持状态并尝试总线恢复。

软件测试：`.venv/Scripts/python -m unittest discover -s bridge -p test_bridge.py -v`。实际验证范围见 VALIDATION.md。

设备使用独立的 Codex app-server 会话，默认工作目录为 assistant-workspace；不会直接接管当前桌面聊天。权限审批保留在电脑控制台，设备支持拒绝或停止。BLE 戒指和旋钮尚未实现，预留统一动作接口。

## 来源

- [厂商硬件资料](https://docs.waveshare.com/ESP32-C6-ePaper-1.54)
- [厂商驱动与示例](https://github.com/waveshareteam/ESP32-C6-ePaper-1.54)
- [Codex app-server](https://learn.chatgpt.com/docs/app-server)
- [faster-whisper](https://github.com/SYSTRAN/faster-whisper)

第三方版权与许可证见 THIRD_PARTY.md 及各源文件。不要公开 bridge/runtime、backup 或个人连接配置文件。
