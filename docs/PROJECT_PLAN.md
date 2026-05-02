# M5StickS3 桌面电子宠物项目计划

更新时间：2026-05-02

## 项目目标

在 M5StickS3 上实现一个可随身/桌面使用的电子宠物。核心体验优先还原原版 Tamagotchi P1，后续叠加 AI 对话、IMU 互动、语音和可选 Buddy 模式。

## 核心决策

### 1. TamaLIB 是核心，AI 是外挂层

使用 TamaLIB 跑原版 P1 ROM，保留进化、时间、隐藏角色和原版状态机。AI 层不直接修改 ROM 或内部状态，只做两件事：

- 显示层：在游戏画面外显示对话气泡、日记、心情提示
- 输入层：把 AI 意图转成玩家本来也能做的按键操作，例如喂食、玩耍、治疗、查看状态

这样能保持原版游戏规则稳定，也方便后续逐步扩展。

### 2. 先 Gemini 3 Flash，后小智/语音

第一版 AI 交流选 Gemini 3 Flash API 的文字对话。理由：

- 集成轻，只需要 Wi-Fi + HTTPS + JSON
- 不抢麦克风/I2S/界面主控权
- 适合先验证“宠物能聊天、能触发动作”的体验

小智路线放到后期，主要参考它的音频采集、OPUS、流式 ASR/LLM/TTS 管线，不把完整小智固件作为主应用并入。M5StickS3 的主应用仍然是拓麻歌子。

### 3. ROM 本地化

Tamagotchi P1 ROM 不提交到仓库。固件支持两种状态：

- 没有 `data/rom.h`：固件仍可编译、烧录、启动，并在屏幕上显示 ROM 准备页和硬件调试信息
- 存在 `data/rom.h`：编译时自动接入 `kTamaRom[8192]`，启动时进入 TamaLIB 主循环

生成工具放在 `tools/rom_to_header.py`，支持文本 12-bit word、16-bit word、packed12 等本地 dump 格式。

### 4. 状态读取后置

MVP 和第二阶段不解析宠物内部饥饿/开心状态。不要从 LCD 像素或 segment 反推状态，成本高且脆弱。

后期如需精确状态，优先从 `tamalib_get_state()` 获取 CPU/RAM 快照，再结合反汇编项目里的 RAM 地址映射读取年龄、饥饿、开心、生病等变量。

### 5. 存储策略

- TamaLIB 存档：LittleFS 文件，例如 `/tama_state.bin`
- 配置项：NVS，例如音量、亮度、Wi-Fi 凭证、API Key
- ROM：本地自行准备，不提交到 git
- 对话/日记：LittleFS，后期再加入

存档采用事件驱动：

- 玩家操作结束并 idle 一段时间后保存
- 进入 sleep 前保存
- 低电压或关机前尽力保存

避免每几分钟固定写入 flash。

## 系统分层

```text
UI Overlay
  对话气泡、状态栏、日记、AI 心情提示

AI Bridge
  Gemini 3 Flash / 后续小智语音
  意图解析、动作冷却、按键序列生成

Event Router
  按钮、IMU、AI 自动动作、定时器

M5StickS3 TamaLIB HAL
  LCD、音频、时钟、日志、本地 ROM 接入

TamaLIB Core
  E0C6S46 模拟器 + P1 ROM
```

## 屏幕与交互设计

M5StickS3 采用竖屏 135x240。

- 原版 LCD：32x16
- 当前缩放：4x，显示为 128x64
- 横向居中：左约 4px，右约 3px
- 游戏画面放在中上方
- 下方保留给按钮状态、AI 对话和后续状态提示

按钮映射：

| 操作 | 映射 |
| --- | --- |
| 原版 A | `key1`，蓝色正面键 |
| 原版 B | `key2`，侧键 |
| 原版 C | `key1` + `key2` 同时按 |
| 唤起 AI | `key2` 长按 |
| 设置/模式菜单 | `key1` 长按 |

统一命名：

- `power`：电源/系统键。不要作为游戏输入使用；实测短按会触发系统级黑屏/重启，长按会进入绿灯闪烁的下载模式，双击会关机。
- `key1`：蓝色正面键，GPIO11。
- `key2`：侧键，GPIO12。

IMU 互动后置：

- 摇晃：触发玩耍快捷动作
- 轻拍：触发呼唤/回应
- 翻转：可选静音或睡眠

## 当前阶段 3 实现

已完成：

- 以 git submodule 方式引入 `jcrona/tamalib`
- 在 `lib/hal_types.h` 提供 TamaLIB 目标类型
- 实现 `hal_t` 到 M5StickS3：
  - LCD matrix/icon 缓冲
  - 30fps 渲染到 135x240 屏幕
  - key1/key2/key1+key2 到 A/B/C
  - 蜂鸣器频率到 M5Unified Speaker
  - `micros()` 时间戳和 `sleep_until`
  - 串口错误日志
- 通过 `tamalib_step()` 接入 Arduino `loop()`，避免阻塞后续 overlay/AI
- 无 ROM 时显示 setup/debug 页面，不影响硬件验证
- `platformio run` 编译通过，`COM4` 实机烧录通过
- 使用本地 ignored P1 ROM 完成实机启动验证
- P1 画面从全屏重绘改为局部刷新，解决插 USB 测试时的闪屏
- `src/tama_storage.*` 用 LittleFS 保存/恢复 TamaLIB CPU/RAM/timer/interrupt 快照
- `src/settings.*` 用 NVS 保存亮度、音量和 idle 阈值
- `src/power_manager.*` 实现 idle 降亮、按键活动恢复亮度、低电压/睡眠前尽力保存
- `key1` 长按循环亮度，`key2` 长按循环音量，短按/组合键仍作为原版 A/B/C 输入
- M5Unified fallback board 固定为 `board_M5StickS3`，避免自动识别异常时影响 I2C/IMU

限制：

- ROM 仍然只保存在用户本地 ignored 文件里，不随仓库分发
- `power` 仍作为系统键，不作为应用输入；sleep 前保存通过统一 flush 入口预留给后续电源流程
- 阶段 3 暂不解析内部宠物状态
- 阶段 3 暂不包含 AI 对话，阶段 4 开始实现

## 阶段计划

### 阶段 0：项目基线

状态：已完成。

验收标准：

- `docs/PROJECT_PLAN.md` 存在并记录总体方案
- `docs/PROGRESS.md` 存在并记录开发进度规则
- git 仓库初始化并完成初始提交

### 阶段 1：硬件验证

状态：已完成。

验收标准：

- 可编译、可烧录、可串口监视
- 屏幕亮并能刷新
- `key1`、`key2`、组合键和长按事件可靠
- 喇叭能发声
- IMU 有合理读数

### 阶段 2：TamaLIB 移植

状态：已完成到可交付代码状态。

验收标准：

- TamaLIB 已集成并可编译
- 本地 ROM 文件可被构建系统加载但不提交
- S3 HAL 已接入屏幕、按钮、音频、时钟和日志
- 没有 ROM 时固件可启动并提示本地 ROM 准备方式
- 有 ROM 时可进入 TamaLIB step 主循环

### 阶段 3：存档、功耗和可玩性

状态：已完成到可交付状态。

目标：让设备日常可用。

任务：

- LittleFS 保存/恢复 TamaLIB 状态
- idle 保存、sleep 前保存、低电压保存
- 基础亮度/音量设置
- 无操作降亮度或关屏
- 重启后恢复宠物状态

验收标准：

- 重启不丢档
- 基础电池使用体验可接受
- 手动操作流畅，不误触

### 阶段 4：Gemini 3 Flash 文字对话

目标：宠物能以文字形式和用户交流。

任务：

- Wi-Fi 按需连接
- Gemini API Key 配置和保存
- HTTPS 请求封装
- 短 prompt 让模型扮演电子宠物
- 回复显示在下方 overlay
- AI 可输出结构化动作，例如 `none`、`feed`、`play`、`care`
- 动作层把结构化动作转成按键序列

验收标准：

- 长按 `key2` 触发一次对话
- 收到回复后显示对话气泡
- Wi-Fi 用完关闭
- AI 动作有冷却，不影响手动操作

### 阶段 5：语音和小智参考

目标：加入更自然的语音交互。

建议路线：

- 电池供电时不做持续唤醒词
- 按住 `key2` 录音 3 秒
- 云端 ASR + LLM + TTS
- 端侧只负责录音、上传、播放
- USB 供电时可实验唤醒词

小智仅作为参考：

- I2S/PDM 音频采集
- VAD/分包/OPUS
- 流式 ASR/LLM/TTS 协议

### 阶段 6：可选扩展

- CC Buddy BLE 模式
- Claude Desktop/Cowork 物理伴侣
- 日记系统
- 自定义 ROM 角色图
- 红外互动
- 桌面底座/充电模式

## Git 维护规则

- `main` 保持可构建或文档完整状态
- 每个阶段至少一个提交
- 每个可验证小功能一个提交
- 提交前更新 `docs/PROGRESS.md`
- 不提交 ROM、API Key、Wi-Fi 凭证、构建产物
- 提交信息使用简洁中文或英文动词短语，例如：
  - `docs: add project plan and progress log`
  - `feat: add display hardware smoke test`
  - `feat: port tamalib lcd hal`
  - `fix: debounce button mapping`
g`
