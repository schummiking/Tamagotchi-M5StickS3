# Tamagotchi-M5StickS3

中文 | [English](#english)

M5StickS3 上的桌面/随身电子宠物项目。当前版本使用 TamaLIB 运行原版 Tamagotchi P1 ROM，并已经接入 M5StickS3 的屏幕、按钮、喇叭、LittleFS 存档、亮度/音量设置和基础功耗策略。

> ROM 不随仓库分发。你需要准备自己的 Tamagotchi P1 ROM dump，并在本地生成 `data/rom.h`。

## 当前状态

- 阶段 1：硬件验证已完成，屏幕、`key1`、`key2`、喇叭、IMU 均可用。
- 阶段 2：TamaLIB 已移植到 M5StickS3，本地 P1 ROM 固件可编译、烧录、启动。
- 阶段 3：已完成 LittleFS 存档、NVS 配置、4 档音量含静音、亮度调节、空闲降亮、关灯黑房间、显示睡眠和绿色系统 LED 睡眠熄灭。
- 当前固件版本：`phase3-combo-order-001`。
- 下一阶段：在独立分支探索小智语音路线、可替换更强模型的接口，以及轻量 agent 能力。

## 硬件

- M5StickS3，ESP32-S3-PICO-1
- 8MB Flash，8MB PSRAM
- 1.14 inch 135x240 IPS LCD
- 1W speaker，MEMS microphone，BMI270 IMU
- 250mAh battery
- `power` + `key1` + `key2`

## 操作方式

| 操作 | 功能 |
| --- | --- |
| 短按 `key1` | 原版 A |
| 短按 `key2` | 原版 B |
| 按住 `key1` 后按 `key2` | 原版 C / 退出 |
| 按住 `key2` 后按 `key1` | 原版 A+C / SET，用于时钟页改时间 |
| 长按 `key1` 后松开 | 循环亮度档位 |
| 长按 `key2` 后松开 | 循环音量档位，包含静音 |
| `power` | 系统电源键，不作为游戏输入 |

## 本地 ROM

仓库不会提交 ROM、API Key、Wi-Fi 凭证或构建产物。

推荐本地路径：

```text
data/tama.zip
data/tama.bin
data/rom.h
```

生成 `data/rom.h`：

```powershell
.\.venv\Scripts\platformio.exe run
python tools\rom_to_header.py data\tama.bin data\rom.h
```

`tools/rom_to_header.py` 支持 MAME P1/P2 常见的 16-bit big-endian word dump，并会把 6144 words 自动补齐到 TamaLIB 需要的 8192 words。

## 构建与烧录

```powershell
.\.venv\Scripts\platformio.exe run
.\.venv\Scripts\platformio.exe run --target upload
```

串口调试命令：

```text
help
dump
save
tap A|B|C|AC|AB|BC|ABC [ms]
```

## 项目文档

- [项目计划](docs/PROJECT_PLAN.md)：架构决策、阶段路线和验收标准
- [开发进度](docs/PROGRESS.md)：每次开发的状态、验证记录和 git 记录

## 技术路线

- Arduino + ESP-IDF via PlatformIO
- M5Unified / LovyanGFX
- TamaLIB
- LittleFS 保存 TamaLIB 状态
- Preferences / NVS 保存亮度、音量和后续联网配置

## 后续方向

- 小智语音路线探索，独立分支推进
- 可替换更强模型的对话/工具调用接口
- 轻量 agent 能力，例如状态观察、计划、动作序列和记忆
- AI overlay 与结构化动作到 A/B/C 按键序列的桥接
- IMU 互动，例如摇晃、轻拍、翻转
- 彩屏和核心游戏体验稳定后再进入 `main`
- 可选 Claude Desktop Buddy / BLE 模式

## 参考项目

- [jcrona/tamalib](https://github.com/jcrona/tamalib)
- [jcrona/tamatool](https://github.com/jcrona/tamatool)
- [jcrona/mcugotchi](https://github.com/jcrona/mcugotchi)
- [anabolyc/Tamagotchi](https://github.com/anabolyc/Tamagotchi)
- [agg23/tamagotchi-disassembled](https://github.com/agg23/tamagotchi-disassembled)
- [anthropics/claude-desktop-buddy](https://github.com/anthropics/claude-desktop-buddy)

---

## English

A desktop / pocket virtual pet project for the M5StickS3. The current firmware runs an original Tamagotchi P1 ROM through TamaLIB and integrates the M5StickS3 display, buttons, speaker, LittleFS save data, brightness / volume settings, and basic power behavior.

> The ROM is not distributed in this repository. You need to provide your own Tamagotchi P1 ROM dump and generate `data/rom.h` locally.

## Current Status

- Phase 1: Hardware validation is complete. Display, `key1`, `key2`, speaker, and IMU are working.
- Phase 2: TamaLIB is ported to M5StickS3. The local P1 ROM firmware builds, flashes, and boots.
- Phase 3: LittleFS save / restore, NVS settings, 4-level volume including mute, brightness control, idle dimming, dark-room rendering, display sleep, and green system LED sleep-off behavior are implemented.
- Current firmware version: `phase3-combo-order-001`.
- Next phase: explore the XiaoZhi voice path, stronger replaceable model backends, and lightweight agent behavior on a separate branch.

## Hardware

- M5StickS3, ESP32-S3-PICO-1
- 8MB Flash, 8MB PSRAM
- 1.14 inch 135x240 IPS LCD
- 1W speaker, MEMS microphone, BMI270 IMU
- 250mAh battery
- `power` + `key1` + `key2`

## Controls

| Action | Function |
| --- | --- |
| Short press `key1` | Original A |
| Short press `key2` | Original B |
| Hold `key1`, then press `key2` | Original C / back |
| Hold `key2`, then press `key1` | Original A+C / SET, used for clock setting |
| Hold and release `key1` | Cycle brightness |
| Hold and release `key2` | Cycle volume, including mute |
| `power` | System power key, not used as a game input |

## Local ROM

This repository does not commit ROM files, API keys, Wi-Fi credentials, or build outputs.

Recommended local paths:

```text
data/tama.zip
data/tama.bin
data/rom.h
```

Generate `data/rom.h`:

```powershell
.\.venv\Scripts\platformio.exe run
python tools\rom_to_header.py data\tama.bin data\rom.h
```

`tools/rom_to_header.py` supports common MAME P1/P2 16-bit big-endian word dumps and pads 6144 source words to the 8192 words expected by TamaLIB.

## Build And Flash

```powershell
.\.venv\Scripts\platformio.exe run
.\.venv\Scripts\platformio.exe run --target upload
```

Serial debug commands:

```text
help
dump
save
tap A|B|C|AC|AB|BC|ABC [ms]
```

## Project Docs

- [Project plan](docs/PROJECT_PLAN.md): architecture decisions, phase roadmap, and acceptance criteria
- [Progress log](docs/PROGRESS.md): development status, verification records, and git history notes

## Stack

- Arduino + ESP-IDF via PlatformIO
- M5Unified / LovyanGFX
- TamaLIB
- LittleFS for TamaLIB save data
- Preferences / NVS for brightness, volume, and later network configuration

## Roadmap

- XiaoZhi voice-path exploration on a separate branch
- Replaceable stronger model backends for conversation and tool calls
- Lightweight agent behavior such as observation, planning, action sequences, and memory
- AI overlay and structured action bridge to A/B/C button sequences
- IMU interactions such as shake, tap, and flip
- Color-screen and core gameplay improvements can later land on `main`
- Optional Claude Desktop Buddy / BLE mode
