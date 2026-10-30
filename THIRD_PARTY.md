# Third party sources

- `firmware/Pocket/port_*`, `epaper_config.h`, and `firmware/Pocket/src/`: copied from Waveshare's ESP32-C6-ePaper-1.54 Arduino 3.3.0 Audio/ADC examples. Source: https://github.com/waveshareteam/ESP32-C6-ePaper-1.54 . Original notices remain in individual files. The upstream repository has no top-level license; review redistribution rights before public release.
- `firmware/Pocket/src/esp_codec_dev` and parts of `codec_board`: Espressif audio codec components as distributed by Waveshare. License notices remain in source.
- `firmware/libraries/OneButton`: upstream OneButton library as distributed by Waveshare; retain bundled license.
- ArduinoJson and WebSockets: downloaded separately through Arduino Library Manager; governed by their respective upstream licenses.
- Python packages: aiohttp, pyserial, Pillow; installed through pip and governed by their respective licenses.
- Local ASR: faster-whisper 1.2.1, CTranslate2 and dependencies, plus the public Systran/faster-whisper-small model weights from Hugging Face. Upstream: https://github.com/SYSTRAN/faster-whisper . Model: https://huggingface.co/Systran/faster-whisper-small . Model weights are downloaded separately under models/.
- Root CA PEM files: downloaded from https://pki.goog/repo/certs/gtsr4.pem and https://letsencrypt.org/certs/isrgrootx1.pem . These are public trust anchors, not credentials. Verify that the actual server chain uses the selected root.
- cloudflared is an optional separately downloaded Cloudflare executable; it is not bundled in the source deliverable.
- `firmware/boards/taiji_knob32/esp_lcd_sh8601.c/.h`: Espressif Apache-2.0 panel driver distributed in the Waveshare ESP32-S3-Knob-Touch-LCD-1.8 Arduino demo. Original SPDX/copyright notices and Apache license retained. The board initialization table in `taiji_lcd_init.c` is extracted from that demo's `lcd_bsp.c`, unchanged. Source URL and archive SHA256 are recorded in `UPSTREAM.json`; no factory firmware images are included.
