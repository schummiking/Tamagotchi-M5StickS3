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

### 3. 状态读取后置

MVP 和第二阶段不解析宠物内部饥饿/开心状态。不要从 LCD 像素或 segment 反推状态，成本高且脆弱。

后期如需精确状态，优先从 `tamalib_get_state()` 获取 CPU/RAM 快照，再结合反汇编项目里的 RAM 地址映射读取年龄、饥饿、开心、生病等变量。

### 4. 存储策略

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

TamaLIB HAL
  LCD、音频、时钟、存档、日志

TamaLIB Core
  E0C6S46 模拟器 + P1 ROM
```

## 屏幕与交互设计

M5StickS3 采用竖屏 135x240。

- 原版 LCD：32x16
- 推荐缩放：4x，显示为 128x64
- 横向居中：左约 4px，右约 3px
- 游戏画面放在中上方
- 下方保留约 100px 给 AI 对话和状态提示

按钮映射：

| 操作 | 映射 |
| --- | --- |
| 原版 A | 左键短按 |
| 原版 B | 右键短按 |
| 原版 C | 左键长按 |
| 唤起 AI | 双键同时按 |
| 设置/模式菜单 | 右键长按 |

IMU 互动后置：

- 摇晃：触发玩耍快捷动作
- 轻拍：触发呼唤/回应
- 翻转：可选静音或睡眠

## 阶段计划

### 阶段 0：项目基线

目标：建立计划、进度和 git 管理。

验收标准：

- `docs/PROJECT_PLAN.md` 存在并记录总体方案
- `docs/PROGRESS.md` 存在并记录开发进度规则
- git 仓库初始化并完成初始提交

### 阶段 1：硬件验证

目标：确认 M5StickS3 的屏幕、按钮、喇叭、麦克风、IMU、供电基础可用。

任务：

- 建立 PlatformIO/Arduino 工程
- 用 M5Unified 初始化设备
- 屏幕显示 boot/debug 信息
- 按钮短按/长按事件串口输出
- 喇叭播放 440Hz 测试音
- IMU 输出加速度/姿态数据

验收标准：

- 可编译、可烧录、可串口监视
- 屏幕亮并能刷新
- 两个按钮事件可靠
- 喇叭能发声
- IMU 有合理读数

### 阶段 2：TamaLIB 移植

目标：在 S3 上跑通原版拓麻歌子。

任务：

- 引入 TamaLIB 源码
- 创建 `hal_types.h`
- 实现 `hal_t`：
  - `set_lcd_matrix`
  - `set_lcd_icon`
  - `update_screen`
  - `set_frequency`
  - `play_frequency`
  - `get_timestamp`
  - `sleep_until`
  - `handler`
- 加入本地 ROM 数据，但不提交 ROM
- 按键映射到 `tamalib_set_button()`

验收标准：

- 屏幕出现拓麻歌子初始画面或蛋
- A/B/C 操作可用
- 原版蜂鸣器声音可播放

### 阶段 3：存档、功耗和可玩性

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

- 双键触发一次对话
- 收到回复后显示对话气泡
- Wi-Fi 用完关闭
- AI 动作有冷却，不影响手动操作

### 阶段 5：语音和小智参考

目标：加入更自然的语音交互。

建议路线：

- 电池供电时不做持续唤醒词
- 双键按住录音 3 秒
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
  - `fix: debounce button long press`

