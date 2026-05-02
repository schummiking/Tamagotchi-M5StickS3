# 开发进度

更新时间：2026-05-02

## 当前状态

项目已完成阶段 3 到可交付状态：带本地 P1 ROM 的固件可启动、可恢复 LittleFS 存档、可从 NVS 恢复亮度/音量配置，空闲 30 秒后会降低屏幕亮度。串口确认 `storage: restored 648 bytes`、`boot ok: M5StickS3 phase3-storage-power-001` 和 `power: display idle brightness`。

已完成：

- 用 Kiro CLI + `claude-opus-4.6` 讨论总体开发方案
- 明确路线：TamaLIB 原版核心优先，AI 作为 overlay/输入外挂层
- 明确 AI 顺序：先 Gemini 3 Flash 文字交互，后小智/语音
- 明确存档策略：TamaLIB 状态用 LittleFS，配置用 NVS
- 完成阶段 1：屏幕、按钮、喇叭、IMU 实机验证
- 完成阶段 2：TamaLIB submodule、HAL、ROM 本地生成入口、无 ROM 启动页
- 完成阶段 3：LittleFS 存档/恢复、NVS 亮度/音量配置、idle 降亮和低电压/睡眠前强制保存入口

下一步：

- 进入阶段 4：Gemini 文字对话
- 准备 Wi-Fi/API Key 的本地配置入口，继续保持 secrets 不提交
- 设计 AI overlay 和结构化动作到 A/B/C 按键序列的桥接

## 进度维护规则

后续每一步开发都要维护本文件：

1. 开始一个任务前，在“当前任务”写明目标和验收标准。
2. 开发过程中如发现方案变化，更新“决策记录”。
3. 完成后更新“进度日志”和“下一步”。
4. 提交 git 前确认本文件已经同步。
5. 每个提交尽量对应一个可说明的工作单元。

## 当前任务

| 字段 | 内容 |
| --- | --- |
| 任务 | 阶段 4：Gemini 文字对话准备 |
| 状态 | 未开始 |
| 验收标准 | `key2` 长按可触发一次文本对话；回复显示在 overlay；Wi-Fi/API Key 不进入 git；AI 动作有冷却且不影响手动操作 |

## 里程碑进度

| 阶段 | 状态 | 验收标准 |
| --- | --- | --- |
| 阶段 0：项目基线 | 已完成 | 计划、进度、git 基线完成 |
| 阶段 1：硬件验证 | 已完成 | 屏幕/按钮/喇叭/IMU 可用 |
| 阶段 2：TamaLIB 移植 | 已完成到带本地 ROM 可交付状态 | TamaLIB 可编译、可烧录；ROM 本地接入；S3 HAL 已实现；本地 P1 ROM 固件启动成功 |
| 阶段 3：存档和功耗 | 已完成到可交付状态 | 重启可恢复 LittleFS 存档；亮度/音量 NVS 持久化；空闲降亮；低电压/睡眠前尽力保存 |
| 阶段 4：Gemini 文字对话 | 未开始 | `key2` 长按触发 AI 对话并显示 overlay |
| 阶段 5：语音/小智参考 | 未开始 | 按键录音、云端处理、端侧播放 |
| 阶段 6：可选扩展 | 未开始 | Buddy/日记/自定义角色等 |

## 决策记录

| 日期 | 决策 | 理由 |
| --- | --- | --- |
| 2026-05-02 | TamaLIB/P1 ROM 作为核心 | 保留原版状态机、进化路线和游戏规则 |
| 2026-05-02 | AI 不直接改 ROM/状态 | 避免破坏原版逻辑，降低调试难度 |
| 2026-05-02 | 先 Gemini 3 Flash 文字对话 | 最小集成成本，适合先验证体验 |
| 2026-05-02 | 小智后置 | 小智是完整语音助手框架，早期会抢占主应用复杂度 |
| 2026-05-02 | MVP 不解析 LCD/内部状态 | 状态读取后置，先保证可玩和可交流 |
| 2026-05-02 | TamaLIB 存档用 LittleFS | 状态数据更适合文件存储，NVS 留给小配置 |
| 2026-05-02 | ROM 不提交到 git | 避免版权和分发风险 |
| 2026-05-02 | `power` 不作为游戏输入 | 实测短按也会触发系统级黑屏/重启，长按进下载模式、双击关机 |
| 2026-05-02 | 统一按钮命名：`power`、`key1`、`key2` | 避免“左键/侧键/电源键”混用导致误操作 |
| 2026-05-02 | 第二阶段使用 `tamalib_step()` 而不是阻塞式 `tamalib_mainloop()` | 保留 Arduino loop 给显示 overlay、AI、IMU 和后续功耗管理 |
| 2026-05-02 | 无 ROM 时也必须可编译可启动 | 让仓库保持可交付，同时不分发 ROM |
| 2026-05-02 | 首轮 ROM 验证优先使用 MAME `tama` / P1 World | 与当前 3 键输入、E0C6S46/P1 目标和社区参考最匹配 |
| 2026-05-02 | MAME P1/P2 ROM 应按 16-bit big-endian word 转换，并补零到 8192 words | TamaTool 说明和源码均显示每条 12-bit 指令存为 16-bit big-endian；12288-byte P1/P2 文件是 6144 words，不是 packed12 |
| 2026-05-02 | TamaLIB 存档写入采用 idle/dirty 策略 | 只在状态变化后且玩家松手空闲时写入，降低 flash 写入频率 |
| 2026-05-02 | 亮度/音量配置使用 NVS | 配置项小而稳定，适合 Preferences/NVS，和 LittleFS 存档职责分离 |
| 2026-05-02 | 阶段 3 不拦截 `power` 键 | StickS3 的 `power` 是系统键，继续避免把它纳入应用输入；sleep 前保存先通过统一保存入口给后续电源流程复用 |
| 2026-05-02 | M5Unified 初始化固定 StickS3 fallback board | 避免自动识别偶发读到异常缓存时影响 I2C 引脚、IMU 或显示 |

## 进度日志

| 日期 | 类型 | 内容 | Git |
| --- | --- | --- | --- |
| 2026-05-02 | 规划 | Kiro Opus 4.6 参与讨论并形成阶段方案 | `6a6561a` |
| 2026-05-02 | 文档 | 新增项目计划和进度维护规则 | `6a6561a` |
| 2026-05-02 | 版本 | 初始化 git 仓库并添加忽略规则 | `6a6561a` |
| 2026-05-02 | 开发 | 新增 PlatformIO/M5Unified smoke test 工程，覆盖屏幕、按钮、喇叭、IMU | `f854205` |
| 2026-05-02 | 验证 | `platformio run` 编译通过 | `f854205` |
| 2026-05-02 | 验证 | StickS3 下载模式枚举为 `COM4`，esptool 成功识别 ESP32-S3-PICO-1、8MB Flash、8MB PSRAM | `2efab9d` |
| 2026-05-02 | 验证 | smoke test 固件成功烧录到 `COM4` | `2efab9d` |
| 2026-05-02 | 修正 | 根据 StickS3 实物与实测，统一按钮命名并确认 `power` 不作为游戏输入 | `e5fb9b9` / `a33f098` |
| 2026-05-02 | 验收 | `key1`、`key2`、`key1+key2`、`key1` 长按、`key2` 长按均已实机确认可用 | `6e20ffa` |
| 2026-05-02 | 开发 | 引入 TamaLIB submodule，并在 submodule 外提供 `lib/hal_types.h` | 本次提交 |
| 2026-05-02 | 开发 | 新增 M5StickS3 TamaLIB HAL：LCD、icons、音频、时间戳、按钮桥接、日志 | 本次提交 |
| 2026-05-02 | 开发 | 新增 `tools/rom_to_header.py` 和 `data/README.md`，建立本地 ROM 生成和 ignored 接入路径 | 本次提交 |
| 2026-05-02 | 开发 | 主循环切换为 `tamalib_step()` 集成；无 ROM 时显示 ROM 准备页 | 本次提交 |
| 2026-05-02 | 验证 | `platformio run` 编译通过，固件大小约 512901 bytes，RAM 使用约 7.1% | 本次提交 |
| 2026-05-02 | 验证 | `platformio run --target upload` 成功烧录到 `COM4`，串口可见 IMU 数据持续输出 | 本次提交 |
| 2026-05-02 | 调研 | 使用 Kiro CLI + `claude-opus-4.6` 复核 ROM 候选，确定首选 MAME `tama`，并发现 converter 需要支持 `words-be16` 6144→8192 padding | `8c16fe8` |
| 2026-05-02 | 修正 | `tools/rom_to_header.py` 默认将 0x3000/0x4000 MAME 文件识别为 `words-be16`，并支持 6144 source words 自动补零到 8192 words | 本次提交 |
| 2026-05-02 | 验证 | dummy 12288-byte P1 形状文件可通过 `auto` 转成 `data/rom.h`，前两条 word 正确读为 `0xFA2, 0xC87`，并可编译 | 本次提交 |
| 2026-05-02 | 验证 | 删除测试 `data/rom.h` 后，无 ROM 默认固件再次 `platformio run` 编译通过 | 本次提交 |
| 2026-05-02 | 验证 | 用户本地 `data/tama.zip` 已包含匹配首选 ROM 的 `tama.bin`：12288 bytes、CRC32 `5c864cb1`、SHA1 `4b4979cf92dc9d2fb6d7295a38f209f3da144f72` | 本次提交 |
| 2026-05-02 | 保护 | `.gitignore` 增加 `data/*.zip`，避免用户本地 ROM 包进入 git | 本次提交 |
| 2026-05-02 | 开发 | 从 `data/tama.zip` 提取 ignored 的 `data/tama.bin`，生成 ignored 的 `data/rom.h`：6144 source words padded to 8192 words | 本地文件，不提交 |
| 2026-05-02 | 验证 | 带 ROM 固件 `platformio run` 编译通过，Flash 使用约 530645 bytes，RAM 使用约 23384 bytes | 本次提交 |
| 2026-05-02 | 验证 | 带 ROM 固件成功烧录到 `COM4`，esptool 确认 ESP32-S3-PICO-1、8MB Flash、8MB PSRAM | 本次提交 |
| 2026-05-02 | 验证 | 串口软复位后确认启动日志：`tamalib: initialized with local ROM` 和 `boot ok: M5StickS3 phase2-tamalib-001` | 本次提交 |
| 2026-05-02 | 发现 | 插 USB 测试时屏幕持续闪烁，定位为 P1 渲染路径 30fps 全屏 `fillScreen` 重画 | 本次提交 |
| 2026-05-02 | 修正 | P1 渲染改为局部刷新：启动时绘制静态框架，运行时只更新变化的 32x16 像素、icons 和状态栏 | 本次提交 |
| 2026-05-02 | 验证 | 防闪烁修正版 `platformio run` 编译通过，Flash 使用约 531057 bytes，RAM 使用约 23904 bytes | 本次提交 |
| 2026-05-02 | 验证 | 防闪烁修正版成功烧录到 `COM4`，串口软复位后仍确认 `tamalib: initialized with local ROM` | 本次提交 |
| 2026-05-02 | 验收 | 用户实机确认插 USB 测试时屏幕已不再闪烁 | 本次提交 |
| 2026-05-02 | 开发 | 开始阶段 3 实现：LittleFS 存档/恢复、NVS 音量亮度配置、idle 降亮度 | 本次提交 |
| 2026-05-02 | 开发 | 新增 `src/tama_storage.*`，保存 CPU 寄存器、timer、interrupts 和 TamaLIB memory，启动后校验并恢复快照 | 本次提交 |
| 2026-05-02 | 开发 | 新增 `src/settings.*`，用 Preferences/NVS 持久化 active brightness、idle brightness、volume 和 idle dim 阈值 | 本次提交 |
| 2026-05-02 | 开发 | 新增 `src/power_manager.*`，空闲 30 秒降亮，按键活动后恢复亮度，低电压/睡眠前尽力保存 TamaLIB 状态 | 本次提交 |
| 2026-05-02 | 开发 | `key1` 长按循环亮度，`key2` 长按循环音量；`key1`/`key2`/组合键仍映射原版 A/B/C | 本次提交 |
| 2026-05-02 | 修正 | `M5.begin()` 改为带配置初始化，并将 fallback board 固定为 `board_M5StickS3` | 本次提交 |
| 2026-05-02 | 验证 | 阶段 3 固件 `platformio run` 编译通过，Flash 使用约 587797 bytes，RAM 使用约 24304 bytes | 本次提交 |
| 2026-05-02 | 验证 | 阶段 3 固件成功烧录到 `COM4`，esptool 确认 ESP32-S3-PICO-1、8MB Flash、8MB PSRAM | 本次提交 |
| 2026-05-02 | 验证 | 串口复位后确认 `settings: brightness=128 idle=32 volume=96`、`storage: LittleFS mounted`、`storage: restored 648 bytes` | 本次提交 |
| 2026-05-02 | 验证 | 空闲约 30 秒后串口确认 `power: display idle brightness`，阶段 3 功耗策略闭环 | 本次提交 |

## 阶段 2 交付物

- `.gitmodules` / `lib/tamalib`：TamaLIB submodule
- `lib/hal_types.h`：TamaLIB 目标类型定义
- `src/tama_app.*`：TamaLIB 应用桥接和 HAL
- `src/display.*`：无 ROM setup 页面和 32x16 到 128x64 渲染
- `src/buttons.*`：`key1`/`key2`/组合键到 A/B/C 的实时 mask
- `data/README.md`：本地 ROM 放置说明
- `tools/rom_to_header.py`：本地 ROM dump 转 `data/rom.h`

## 下一步详情

### 阶段 3 交付物

- `src/tama_storage.*`：LittleFS 存档/恢复，保存 TamaLIB CPU/RAM/timer/interruption 快照
- `src/settings.*`：NVS 亮度、音量、idle 阈值配置
- `src/power_manager.*`：idle 降亮、按键唤醒、低电压/睡眠前保存入口
- `src/main.cpp`：接入设置快捷键和功耗更新
- `src/tama_app.cpp`：接入存档恢复、输入后 dirty 标记和 idle 保存
- `include/pins.h`：固件版本更新为 `phase3-storage-power-001`

本地 ROM 仍然只存在于 ignored 文件中，不提交：

- `data/tama.zip`
- `data/tama.bin`
- `data/rom.h`

阶段 3 实机验收：

```powershell
.\.venv\Scripts\platformio.exe run
.\.venv\Scripts\platformio.exe run --target upload
```

已确认：

- 启动日志出现 `tamalib: initialized with local ROM`
- 启动日志出现 `storage: restored 648 bytes`
- 启动日志出现 `boot ok: M5StickS3 phase3-storage-power-001`
- 空闲约 30 秒后出现 `power: display idle brightness`
- Flash/RAM 占用保持安全：约 17.6% Flash、7.4% RAM

### 阶段 4 预备

预计文件：

- `src/network_config.*`
- `src/ai_client.*`
- `src/ai_overlay.*`
- `src/ai_actions.*`

最小验收：

- `include/secrets_local.h` 或 NVS 提供本地 Wi-Fi/API Key，不进入 git
- 长按 `key2` 触发一次 Gemini 文字请求
- 收到回复后显示在 P1 画面下方 overlay
- AI 可选输出结构化动作，动作层把它转换为 A/B/C 按键序列
- Wi-Fi 用完后关闭或进入低功耗状态
