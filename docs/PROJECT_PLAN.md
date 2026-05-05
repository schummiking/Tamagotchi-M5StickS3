# M5StickS3 桌面电子宠物项目计划

更新时间：2026-05-05

## 项目目标

在 M5StickS3 上实现一个可随身/桌面使用的电子宠物。核心体验优先还原原版 Tamagotchi P1，后续叠加 AI 对话、IMU 互动、语音和可选 Buddy 模式。

## 核心决策

### 1. TamaLIB 是核心，AI 是外挂层

使用 TamaLIB 跑原版 P1 ROM，保留进化、时间、隐藏角色和原版状态机。AI 层不直接修改 ROM 或内部状态，只做两件事：

- 显示层：在游戏画面外显示对话气泡、日记、心情提示
- 输入层：把 AI 意图转成玩家本来也能做的按键操作，例如喂食、玩耍、治疗、查看状态

这样能保持原版游戏规则稳定，也方便后续逐步扩展。

### 2. 小智优先走独立实验线

阶段 3 交付后，AI 路线调整为先探索小智语音框架，而不是先把 Gemini 文字对话并入主固件。理由：

- 小智已经覆盖语音交互所需的大块能力：录音、VAD/OPUS、流式对话、播放和会话管理
- 语音链路会影响音频、网络、功耗和 UI 主循环，适合在分支/fork 里大胆验证
- 先确认它能否接入更智能的模型、是否支持工具调用/agent 行为，再决定抽哪些能力回主线
- `main` 继续保持可构建、可玩、可烧录的 Tamagotchi/M5StickS3 主线

Gemini 仍保留为候选模型/文字对话路径，但不再是下一步的唯一优先项。

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

### 6. 联网校时补偿走独立分支

联网校时不并入当前 `main` 主线，先在 `codex/network-time-catchup` 分支完成验证。`main` 继续保持 `phase3-lowpower-006` 的可构建、可烧录、可玩基线；联网、Wi-Fi、NTP、时间补偿和 ROM 时钟识别都作为实验功能推进，稳定后再评估是否拆模块回接主线。

校时补偿的核心原则：

- 不直接改宠物状态，不写饥饿、开心、年龄、生病、睡眠等 ROM 内部变量。
- 联网只提供真实当前时间；游戏逻辑仍由 TamaLIB/P1 ROM 自己推进。
- 补偿方式是计算 ROM 当前时钟与 NTP 本地时间的一天内正向差值，再调用 TamaLIB 快速补跑。
- 原版 P1 时钟只暴露小时、分钟、AM/PM，没有日期；因此超过 24 小时的离线时间天然无法识别，这一点保留为产品策略，避免多日离线后一联网直接把宠物推死。
- 离线启动后用户玩了一段时间再联网，逻辑仍然相同：ROM 时钟已经在离线游玩期间前进，联网时只补 ROM 当前时钟落后真实时钟的那部分。
- 自动 true deep sleep 继续关闭；联网补偿解决的是硬关机、断电、没电或刷机回来后的时间追赶，不重新启用 deep sleep 省电路线。

## 系统分层

```text
UI Overlay
  对话气泡、状态栏、日记、AI 心情提示

AI Bridge
  XiaoZhi voice path / Gemini or stronger model backend
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
| 原版 C | 按住 `key1` 后按 `key2` |
| 原版 A+C | 按住 `key2` 后按 `key1` |
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
  - key1/key2/顺序组合键到 A/B/C/A+C
  - 蜂鸣器频率到 M5Unified Speaker
  - `micros()` 时间戳和 `sleep_until`
  - 串口错误日志
- 通过 `tamalib_step()` 接入 Arduino `loop()`，避免阻塞后续 overlay/AI
- 无 ROM 时显示 setup/debug 页面，不影响硬件验证
- `platformio run` 编译通过，`COM4` 实机烧录通过
- 使用本地 ignored P1 ROM 完成实机启动验证
- P1 画面从全屏重绘改为局部刷新，解决插 USB 测试时的闪屏
- P1 关灯时的全亮 LCD 矩阵在显示层渲染为黑房间，避免整块绿色屏幕
- 竖屏运行界面删除常驻按键说明，改为 8 个原版照顾菜单图标和短标签
- 顶部状态栏显示亮度/音量档位，不再显示瞬时蜂鸣 `sound:on/off`；音量包含静音档，`0` 档会静音 boot 音、按键音和 ROM 蜂鸣
- `src/tama_storage.*` 用 LittleFS 保存/恢复 TamaLIB CPU/RAM/timer/interrupt 快照
- `src/settings.*` 用 NVS 保存亮度、音量和 idle 阈值
- `src/power_manager.*` 实现 idle 降亮、暗屏夜间亮度、显示待机、外部供电诊断、自动 true deep sleep 关闭、串口 `nap` 手动睡眠诊断、按键活动恢复亮度、低电压/睡眠前尽力保存
- `key1` 短按松开触发 A，`key2` 短按松开触发 B；组合键按下顺序会区分语义：`key1 -> key2` 触发 C/退出，`key2 -> key1` 触发原版 A+C/SET，避免组合键误触 A/B
- `key1` 长按松开循环亮度，`key2` 长按松开循环音量
- `key1 -> key2` 保持原版 C/退出，不复用为睡眠入口；`key2 -> key1` 用于原版 A+C，例如时钟页进入 SET
- M5Unified fallback board 固定为 `board_M5StickS3`，避免自动识别异常时影响 I2C/IMU

限制：

- ROM 仍然只保存在用户本地 ignored 文件里，不随仓库分发
- `power` 仍作为系统键，不作为应用输入；sleep 前保存通过统一 flush 入口预留给后续电源流程
- 黑屏待机会保留绿色 LED，提示设备仍在运行；自动 true deep sleep 当前关闭，保证电池和 USB 场景下 TamaLIB 只要还有电就实时运行。串口 `nap` 仍可手动触发短 deep sleep 诊断；按键唤醒则回到可操作状态
- 手动 `power` 硬关机仍无法准确知道离线 elapsed；当前通过周期 checkpoint 降低丢档风险，准确硬关机补偿需后续联网/NTP 或额外 RTC
- 阶段 3 暂不解析内部宠物状态
- 阶段 3 暂不包含 AI 对话，阶段 4 改为小智/agent 实验线

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
- 无操作降亮度或关屏；自动 true deep sleep 关闭，普通 idle 待机和宠物关灯黑房间都只进入显示待机，显示待机时保留绿色 LED
- 关灯房间识别需要覆盖打呼动画：房间背景反色为黑，打呼/动画保留为绿色，并继续触发夜间降亮和显示睡眠
- true deep sleep 不再自动进入；串口 `nap [ms]` 保留为诊断入口，会在进入真实低功耗前保存状态、关闭绿色 LED，定时唤醒后快速补跑睡眠时间
- 串口临时诊断支持 `diag` 和 `nap [ms]`，用于查看存档/功耗状态和短睡眠补偿测试；稳定后可隐藏
- USB 调试口支持原版按键注入和屏幕帧 dump，便于调试；物理按键现在可通过 `key2 -> key1` 表达原版 `A+C`
- 竖屏下显示 FOOD、LIGHT、GAME、MED、CLEAN、STAT、DISC、CALL 菜单区
- 顶部显示当前亮度和音量档位；长按 `key1` 调亮度，长按 `key2` 在 `0/32/96/160` 四档音量间切换
- 重启后恢复宠物状态

验收标准：

- 重启不丢档
- 基础电池使用体验可接受
- 手动操作流畅，不误触

### 阶段 4：小智语音与模型替换实验

状态：未开始，计划在独立分支或 fork 推进。

目标：验证小智路线是否适合 M5StickS3，并确认它能否接入更智能的模型后端。

任务：

- 拉取/阅读小智相关固件和协议实现，确认 ESP32-S3、I2S/麦克风、喇叭、屏幕主循环的适配成本
- 在独立分支建立最小语音链路：按键触发录音、连接服务、播放回复
- 分离模型后端接口，评估 Gemini、OpenAI/兼容 API 或本地网关是否可替换默认模型
- 评估工具调用/agent 能力：状态观察、计划、动作序列、记忆、失败回滚和人工确认
- 明确哪些能力可抽回 Tamagotchi 主线，哪些只保留在小智 fork

验收标准：

- 小智实验分支可构建，或明确列出当前不可构建阻塞
- 能跑通最小语音交互或形成不可行原因清单
- 输出模型替换方案和 agent 能力边界
- 不影响 `main` 上现有 Tamagotchi 可玩固件

### 实验分支：联网校时补偿

状态：已开分支 `codex/network-time-catchup`，先做计划和验证工具，不直接改 `main`。

目标：设备重新刷回 Tamagotchi 固件、硬关机/没电后开机、或离线启动后稍后联网时，能把 ROM 当前时钟追到真实时间的一天内位置，并让原版 ROM 自己处理这段时间内发生的饥饿、睡觉、成长、生病等逻辑。

核心公式：

```text
real_minutes = NTP 本地时间分钟数，0..1439
rom_minutes  = ROM 当前时钟分钟数，0..1439
delta_minutes = (real_minutes - rom_minutes + 1440) % 1440
```

补偿规则：

- `delta_minutes < 2`：忽略，避免微小抖动。
- `2 <= delta_minutes < 1440`：允许补跑这段时间。
- 不识别、不补偿完整天数；超过 24 小时只补 modulo 24h 的剩余差值。
- 不倒退 ROM 时钟；如果真实时间看起来早于 ROM，只按正向跨日差值计算。
- 补跑只调用 `tamaAppFastForward(delta_ms)`，不直接写 ROM 变量。

计划模块：

- `src/network_time.*`：Wi-Fi 连接、SNTP/NTP、时区配置、联网状态。
- `src/rom_clock.*`：读取 ROM 当前小时、分钟、AM/PM。
- `src/time_catchup.*`：计算差值、预览补偿、执行补跑、保存结果。
- `src/time_journal.*`：记录最近一次 NTP、最近一次补偿、失败原因和策略版本。
- `serial_console` 扩展命令：`time diag`、`time sync`、`clock read`、`catchup preview`、`catchup apply`。

ROM 时钟读取路线：

- 首选：通过 TamaLIB state memory 定位 P1 clock 字段。用几个已知时间点做 memory diff，例如 `1:23 AM`、`1:24 AM`、`2:24 AM`、`2:24 PM`，确认小时、分钟、AM/PM 地址。
- 备选：通过时钟画面帧解析 32x16 LCD 数字。该方案会切换界面，较脆弱，只做兜底或调试。

实施顺序：

1. 建立分支、写入计划和进度规则。
2. 新增 Wi-Fi/NTP 最小实现，只输出 `time diag`，不补偿。
3. 定位并验证 ROM 时钟读取。
4. 实现 `catchup preview`，串口打印 NTP 时间、ROM 时间、delta。
5. 实现手动 `catchup apply`，调用 `tamaAppFastForward()` 后保存。
6. 长时间和跨日测试通过后，再考虑启动时自动小额补偿。

验收标准：

- 分支可构建，不影响 `main`。
- 没有 Wi-Fi/没有 NTP 时，固件仍可正常玩，不阻塞启动。
- 能读取 ROM 当前时钟，或明确记录无法可靠读取的阻塞。
- `catchup preview` 的 delta 与手算一致。
- 离线启动后玩一段时间再联网，不会重复补已经运行过的时间。
- 超过 24 小时只补一天内差值，不做多日死亡式补偿。
- 所有 Wi-Fi 密码、API key、本地 ROM 继续不提交。

### 阶段 5：AI 回接 Tamagotchi 主线

目标：把阶段 4 验证过的 AI 能力以低风险方式接回电子宠物体验。

建议路线：

- 先接文字/overlay，再接语音
- AI 动作必须转成玩家本来也能执行的 A/B/C/A+C 按键序列
- agent 动作需要冷却、可取消、可观察结果，避免破坏原版状态机
- 电池供电默认不做持续唤醒词；USB 供电再实验常驻语音
- API Key、Wi-Fi、服务端地址继续只走本地配置，不进入 git

### 阶段 6：可选扩展

- CC Buddy BLE 模式
- Claude Desktop/Cowork 物理伴侣
- 日记系统
- 自定义 ROM 角色图
- 红外互动
- 桌面底座/充电模式

## Git 维护规则

- `main` 保持可构建、文档完整、以 Tamagotchi 核心和彩屏/界面稳定改进为主
- 小智、模型替换、agent、联网校时补偿、Buddy 等探索性方向走独立分支或 fork，默认不直接推 `main`
- 实验线合回 `main` 前必须拆出可维护模块，去掉 secrets/ROM/不可复现依赖，并重新验证构建
- 每个阶段至少一个提交
- 每个可验证小功能一个提交
- 提交前更新 `docs/PROGRESS.md`
- 不提交 ROM、API Key、Wi-Fi 凭证、构建产物
- 提交信息使用简洁中文或英文动词短语，例如：
  - `docs: add project plan and progress log`
  - `feat: add display hardware smoke test`
  - `feat: port tamalib lcd hal`
  - `fix: debounce button mapping`
