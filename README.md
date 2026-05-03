# M5StickS3 桌面电子宠物

## 项目管理
- [项目计划](docs/PROJECT_PLAN.md)：记录架构决策、阶段路线和验收标准
- [开发进度](docs/PROGRESS.md)：每次开发都更新当前状态、下一步和 git 提交记录
- 版本维护：使用 git 管理；ROM、API Key、构建产物不提交到仓库

## 硬件
- M5StickS3 (ESP32-S3-PICO-1, 双核240MHz, 8MB Flash, 8MB PSRAM)
- 1.14寸 LCD 135x240, 1W喇叭, MEMS麦克风, BMI270 IMU, BLE 5.0
- 250mAh 电池, `power` + `key1` + `key2`, 红外收发
- 官网 $21.50 + 运费 = $29，已下单 2026-04-20

## 核心功能：拓麻歌子（TamaLIB 移植）

基于逆向工程的原版 Tamagotchi P1 ROM 模拟，不是自己写状态机。

技术路线：
- TamaLIB (github.com/jcrona/tamalib) — 硬件无关的 P1 模拟库，跑原版 ROM，100% 还原
- 已有 ESP32 移植版 (github.com/anabolyc/Tamagotchi)，需适配 S3
- TamaTool (github.com/jcrona/tamatool) — PC 端调试/ROM 编辑工具，可自定义角色图片

适配 S3 的工作：
- 屏幕驱动：M5Unified/LovyanGFX，原版 32x16 → 128x64
- 按钮映射：A=`key1`，B=`key2`，C=`key1` 后按 `key2`，A+C=`key2` 后按 `key1`；`power` 只作为系统键
- 音频：原版蜂鸣器 → S3 的 1W I2S 喇叭
- ROM：本地 `data/rom.h`，不提交到 git；无 ROM 时固件仍可启动
- 存储：阶段 3 持久化到 LittleFS，断电不丢失

优势：
- 所有进化路线、隐藏角色、时间逻辑完整保留
- 二次开发在完整游戏基础上叠加，不是从零搭半成品
- ROM 可编辑，想换自定义角色用 TamaTool 改

## 二次开发方向

在原版基础上扩展：
- AI 对话：联网时可通过语音/文字跟宠物互动（Gemini Flash API）
- IMU 互动：摇晃触发特殊事件（喂食、玩耍的快捷方式）
- 自定义角色：用 TamaTool 替换 ROM 里的像素图
- 音效增强：利用 1W 喇叭播放更丰富的音效

## 可选功能：CC Buddy 模式（待定）

通过 BLE 连接 Claude Desktop/Cowork，作为 Claude Code 的物理伴侣。

Anthropic 官方 claude-desktop-buddy 仓库的固件是给 StickC Plus 写的，需要适配 S3：
- BLE 协议：Nordic UART Service，S3 的 BLE 5.0 兼容
- 显示：ASCII species + 表情动画，屏幕分辨率一样，改动不大
- 主要工作：引脚映射、BLE 库从 ESP32 切到 ESP32-S3 (NimBLE)

集成思路：
- 默认跑拓麻歌子模式
- 检测到 BLE 配对请求时自动切换到 Buddy 模式
- Buddy 模式下 Claude 的活动（编码、思考、等待确认）映射为宠物行为
- 断开 BLE 后回到拓麻歌子

开放问题：
- Buddy 需要 Claude Desktop 开启开发者模式，日常是否方便？
- 两个模式的状态是否互通？（比如 Buddy 模式下写代码算"喂食"？）
- 是否值得投入时间适配，还是先专注拓麻歌子做好？

## 关键仓库
- https://github.com/jcrona/tamalib — P1 模拟库（核心）
- https://github.com/jcrona/tamatool — PC 调试/ROM 编辑
- https://github.com/jcrona/mcugotchi — MCU 移植参考
- https://github.com/anabolyc/Tamagotchi — ESP32 移植版（直接参考）
- https://github.com/agg23/tamagotchi-disassembled — P1 ROM 反汇编
- https://github.com/anthropics/claude-desktop-buddy — CC Buddy 固件

## 技术栈
- Arduino + ESP-IDF (PlatformIO)
- TamaLIB（C 库，硬件无关）
- LovyanGFX 或 TFT_eSPI 驱动屏幕
- NimBLE 处理 BLE（Buddy 模式）
- NVS 存储宠物状态

## 里程碑
1. 到手后验证硬件：屏幕、按钮、喇叭、IMU 基本驱动（已完成）
2. 移植 TamaLIB 到 S3，建立本地 ROM 接入路径（已完成到可交付代码状态）
3. 添加 LittleFS 存档、亮度/音量和功耗控制
4. 添加 IMU 互动和音效增强
5. （可选）适配 CC Buddy BLE 协议 + 双模式切换
6. （可选）AI 对话扩展
