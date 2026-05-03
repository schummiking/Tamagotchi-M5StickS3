# 开发进度

更新时间：2026-05-02

## 当前状态

项目已完成阶段 3 到可交付状态：带本地 P1 ROM 的固件可启动、可恢复 LittleFS 存档、可从 NVS 恢复亮度/音量配置，空闲 30 秒后会降低屏幕亮度。当前静音档修正版串口确认 `storage: restored 648 bytes` 和 `boot ok: M5StickS3 phase3-volume-mute-001`。

已完成：

- 用 Kiro CLI + `claude-opus-4.6` 讨论总体开发方案
- 明确路线：TamaLIB 原版核心优先，AI 作为 overlay/输入外挂层
- 明确 AI 顺序：先 Gemini 3 Flash 文字交互，后小智/语音
- 明确存档策略：TamaLIB 状态用 LittleFS，配置用 NVS
- 完成阶段 1：屏幕、按钮、喇叭、IMU 实机验证
- 完成阶段 2：TamaLIB submodule、HAL、ROM 本地生成入口、无 ROM 启动页
- 完成阶段 3：LittleFS 存档/恢复、NVS 亮度/音量配置、4 档音量含静音、状态栏亮度/音量档位、idle 降亮、黑房间渲染、竖屏菜单图标、显示睡眠和低电压/睡眠前强制保存入口

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
| 任务 | 音量修正：增加静音档 |
| 状态 | 已完成并验证 |
| 验收标准 | 音量从 3 档改为 4 档 `0 -> 32 -> 96 -> 160`；音量 0 时 boot 音、按键音和 ROM 蜂鸣都静音；状态栏显示静音/音量档位；编译、烧录、串口启动验证通过 |

## 里程碑进度

| 阶段 | 状态 | 验收标准 |
| --- | --- | --- |
| 阶段 0：项目基线 | 已完成 | 计划、进度、git 基线完成 |
| 阶段 1：硬件验证 | 已完成 | 屏幕/按钮/喇叭/IMU 可用 |
| 阶段 2：TamaLIB 移植 | 已完成到带本地 ROM 可交付状态 | TamaLIB 可编译、可烧录；ROM 本地接入；S3 HAL 已实现；本地 P1 ROM 固件启动成功 |
| 阶段 3：存档和功耗 | 已完成到可交付状态 | 重启可恢复 LittleFS 存档；亮度/音量 NVS 持久化；4 档音量含静音；状态栏档位；空闲降亮；关灯黑房间；竖屏菜单图标；显示睡眠；低电压/睡眠前尽力保存 |
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
| 2026-05-02 | 两键组合 C 采用释放判定的实体键语义 | 短按松开才发送 A/B，长按松开才触发设置；按住一个键期间出现另一个键立即判定为 C，避免时钟设置误加小时 |
| 2026-05-02 | `key1+key2` 保持原版 C/退出，不复用为睡眠入口 | 用户实机操作里 C/退出是高频核心操作；低功耗应走自动显示策略，不能抢组合键语义 |
| 2026-05-02 | P1 关灯时的全亮 LCD 矩阵渲染为黑房间 | TamaLIB 传出的全亮矩阵在当前配色下会变成整块绿色；对用户体验来说，房间关灯应直接显示为黑 |
| 2026-05-02 | 离线/联网后的时间补偿暂不自动实现 | 用户可能离线外出继续玩，回家后再联网；补偿策略需要先定义离线游玩、联网校时和宠物状态推进的优先级 |
| 2026-05-02 | 竖屏优先显示原版 8 菜单图标而不是按键说明 | 玩家需要知道当前选中的照顾功能；按键说明可记忆，菜单语义不应强迫记忆 |
| 2026-05-02 | 设备端菜单标签先用 ASCII 短词 | M5 默认字体对中文不稳定；用图形和短英文避免烧录后乱码 |
| 2026-05-02 | 状态栏显示配置状态而不是瞬时蜂鸣状态 | `sound:on/off` 反映的是 TamaLIB 当前是否正在发声，不是用户关心的音量设置；状态栏应显示亮度/音量档位 |
| 2026-05-02 | Git push 暂时卡在未配置远端 | `git remote -v` 为空，本地提交可继续维护；需要远端仓库 URL 后才能 `git remote add origin ...` 并 push |
| 2026-05-02 | 音量档位加入静音 | 最低非零音量在休眠/夜间仍会叫；用户需要一个真正全关的声音档位 |

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
| 2026-05-02 | 修正 | 根据实机体验重写按钮状态机：短按松开触发 A/B，长按松开触发亮度/音量设置，第二键出现触发 C 并吞掉单键残留 | 本次提交 |
| 2026-05-02 | 验证 | 组合键修正版 `platformio run` 编译通过，Flash 使用约 587949 bytes，RAM 使用约 24312 bytes，并已成功烧录到 `COM4` | 本次提交 |
| 2026-05-02 | 修正 | P1 关灯后的全亮 LCD 矩阵在显示层渲染为黑房间，不再显示为整块绿色 | 本次提交 |
| 2026-05-02 | 开发 | 新增暗屏检测后的夜间亮度和显示睡眠路线；按键唤醒后会吞掉本次唤醒按键，避免误操作宠物 | 本次提交 |
| 2026-05-02 | 修正 | 明确 `key1+key2` 仍然只映射原版 C/退出，不作为睡眠快捷键 | 本次提交 |
| 2026-05-02 | 验证 | 关灯/功耗修正版 `platformio run` 编译通过，Flash 使用约 588717 bytes，RAM 使用约 24320 bytes | 本次提交 |
| 2026-05-02 | 验证 | 关灯/功耗修正版成功烧录到 `COM4`，串口确认 `boot ok: M5StickS3 phase3-storage-power-003` | 本次提交 |
| 2026-05-02 | 修正 | 删除 P1 运行界面的常驻按键说明，改为 4x2 原版菜单图标区：FOOD、LIGHT、GAME、MED、CLEAN、STAT、DISC、CALL | 本次提交 |
| 2026-05-02 | 开发 | 每个菜单格绘制对应 pictogram，并在底部显示当前选中菜单的短提示；CALL/Attention 保留为提示图标 | 本次提交 |
| 2026-05-02 | 验证 | 竖屏菜单 UI 版 `platformio run` 编译通过，Flash 使用约 591733 bytes，RAM 使用约 24320 bytes | 本次提交 |
| 2026-05-02 | 验证 | 竖屏菜单 UI 版成功烧录到 `COM4`，串口确认 `boot ok: M5StickS3 phase3-ui-menu-001` | 本次提交 |
| 2026-05-02 | 文档 | 记录状态栏修正任务、亮度/音量操作方式和 git push 卡点：当前没有 remote | `f1c8c67` |
| 2026-05-02 | 修正 | 顶部状态栏移除瞬时 `sound:on/off`，改为 `BRI`/`VOL` 三格档位显示 | 本次提交 |
| 2026-05-02 | 验证 | 状态栏修正版 `platformio run` 编译通过，Flash 使用约 591925 bytes，RAM 使用约 24320 bytes | 本次提交 |
| 2026-05-02 | 验证 | 状态栏修正版成功烧录到 `COM4`，串口确认 `boot ok: M5StickS3 phase3-ui-status-001` | 本次提交 |
| 2026-05-02 | 文档 | 开始音量静音档修正，目标为 4 档音量且 `0` 档全静音 | 本次提交 |
| 2026-05-02 | 修正 | 音量档位改为 `0 -> 32 -> 96 -> 160`，长按 `key2` 循环；`0` 档会 stop speaker | 本次提交 |
| 2026-05-02 | 修正 | boot 音、按键音和 TamaLIB ROM 蜂鸣均尊重静音档，音量为 `0` 时不再发声 | 本次提交 |
| 2026-05-02 | 修正 | 状态栏音量显示改为 4 档，其中静音档用红色 X 标记 | 本次提交 |
| 2026-05-02 | 验证 | 静音档修正版 `platformio run` 编译通过，Flash 使用约 592145 bytes，RAM 使用约 24320 bytes | 本次提交 |
| 2026-05-02 | 验证 | 静音档修正版成功烧录到 `COM4`，串口确认 `boot ok: M5StickS3 phase3-volume-mute-001` | 本次提交 |
| 2026-05-02 | 文档 | 开始处理休眠时绿色系统 LED 仍亮/闪的问题；目标为显示睡眠时熄灭、唤醒后恢复原状态 | 本次提交 |
| 2026-05-02 | 修正 | 新增 `src/system_led.*`，通过 Stick S3 PI4IO E1 P7 控制 SYS_LEDG，休眠时写 HIGH 熄灭 | 本次提交 |
| 2026-05-02 | 修正 | `power_manager` 在进入显示睡眠时熄灭系统 LED，在按键唤醒时恢复睡眠前 LED 输出状态 | 本次提交 |
| 2026-05-02 | 验证 | 绿灯休眠修正版 `platformio run` 编译通过，Flash 使用约 592601 bytes，RAM 使用约 24320 bytes | 本次提交 |
| 2026-05-02 | 验证 | 绿灯休眠修正版成功烧录到 `COM4`，串口确认 `boot ok: M5StickS3 phase3-led-sleep-001` | 本次提交 |
| 2026-05-02 | 验证 | 75 秒串口监控确认可进入 `power: display idle brightness`；本轮未等待 10 分钟 idle display sleep，绿灯熄灭需实机睡眠肉眼确认 | 本次提交 |
| 2026-05-02 | 文档 | 开始处理 USB 辅助校时；P1 改时钟需要在时钟页按原版 `A+C` 进入 `SET`，现有两键物理映射无法直接表达这个组合 | 本次提交 |
| 2026-05-02 | 开发 | 新增 `src/serial_console.*`，支持 `tap A/B/C/AC`、`dump`、`save`，用于通过 USB 调试口发送原版按键组合和读取 32x16 帧 | 本次提交 |
| 2026-05-02 | 开发 | `buttons` 新增短时注入的 Tama 按键 mask；`tama_app` 新增调试帧打印，便于后续自动化校时和问题复现 | 本次提交 |
| 2026-05-02 | 验证 | USB 辅助校时版 `platformio run` 编译通过，Flash 使用约 594173 bytes，RAM 使用约 24408 bytes | 本次提交 |
| 2026-05-02 | 验证 | USB 辅助校时版成功烧录到 `COM4`，固件版本为 `phase3-serial-clock-001` | 本次提交 |
| 2026-05-02 | 校时 | 通过串口命令进入 P1 时钟 `SET`，在电脑时间 `22:03:00` 确认 ROM 时钟为 `10:03 PM` 并执行 `save` | 本次提交 |
| 2026-05-02 | 分析 | 用户反馈校时后像是黑房间、夜间休眠和 LED 休眠功能回滚；Git 历史确认并未回滚，`7e85c75` 只改串口调试/按键注入/版本号 | 本次提交 |
| 2026-05-02 | 修正 | 根因定位为关灯房间识别过窄：旧逻辑只认几乎全亮帧，打呼动画会产生较多空洞且可能发声，导致黑房间反色、夜间降亮/睡眠和 LED 熄灭链路都不触发 | 本次提交 |
| 2026-05-02 | 修正 | 新增 `src/tama_frame.h` 共享 32x16 帧统计；超过 75% 亮点即判为关灯房间，显示层对关灯房间反色，功耗层同阈值判断夜间状态 | 本次提交 |
| 2026-05-02 | 验证 | 关灯房间修正版 `platformio run` 编译通过，Flash 使用约 594161 bytes，RAM 使用约 24408 bytes；设备当前未连接，尚未烧录实机 | 本次提交 |

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
- `src/power_manager.*`：idle 降亮、暗屏夜间亮度、显示睡眠、按键唤醒、低电压/睡眠前保存入口
- `src/system_led.*`：显示睡眠时关闭 Stick S3 绿色系统 LED，唤醒后恢复睡眠前状态
- `src/serial_console.*`：USB 调试命令，可注入原版 A/B/C/组合键、打印帧、手动保存，用于校时和调试
- `src/tama_frame.h`：共享 Tama 32x16 帧统计和关灯房间识别阈值，避免显示与功耗判断分叉
- `src/display.cpp`：竖屏运行界面显示原版 8 菜单图标、选中项提示、亮度/音量档位状态栏
- `src/main.cpp`：接入设置快捷键和功耗更新
- `src/tama_app.cpp`：接入存档恢复、输入后 dirty 标记和 idle 保存
- `src/audio.cpp`：音量 `0` 时停止 speaker，boot/按键音不发声
- `include/pins.h`：固件版本更新为 `phase3-dark-room-001`

阶段 3 当前操作：

- 短按 `key1`：原版 A，循环菜单/增加小时等
- 短按 `key2`：原版 B，确认/进入时钟等
- 同时按 `key1+key2`：原版 C/退出
- 长按 `key1` 后松开：循环亮度档位 `64 -> 128 -> 200 -> 64`
- 长按 `key2` 后松开：循环音量档位 `0 -> 32 -> 96 -> 160 -> 0`
- USB 串口调试：`tap A/B/C/AC [ms]` 注入原版按键，`dump` 输出 32x16 帧，`save` 手动保存

Git push 卡点：

- 当前 `git remote -v` 为空，仓库没有远端地址
- 需要先提供远端仓库 URL，之后才能执行 `git remote add origin <url>` 和 `git push`

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
- 启动日志出现 `boot ok: M5StickS3 phase3-led-sleep-001`
- 空闲约 30 秒后出现 `power: display idle brightness`
- P1 关灯后的全亮矩阵显示为黑房间，不再是整块绿色
- `key1+key2` 仍然是原版 C/退出
- P1 运行界面底部显示 8 个菜单图标和短标签，不再被按键说明占满
- 顶部状态栏显示亮度/音量档位，不再显示瞬时蜂鸣状态；音量静音档显示红色 X
- 显示睡眠入口已接入绿色系统 LED 熄灭逻辑；正常 idle display sleep 阈值为 10 分钟，需实机放置到睡眠后肉眼确认 LED 是否完全熄灭
- USB 辅助校时已验证：当前 ROM 时钟在 `2026-05-02 22:03:00 -05:00` 对齐到 `10:03 PM`
- Flash/RAM 占用保持安全：约 17.8% Flash、7.4% RAM

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
