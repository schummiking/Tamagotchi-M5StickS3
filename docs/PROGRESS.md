# 开发进度

更新时间：2026-05-02

## 当前状态

项目已完成阶段 2 的带本地 P1 ROM 可交付状态：用户本地 `data/tama.zip` 中的 `tama.bin` 已匹配 MAME `tama` / P1 World，已生成 ignored 的 `data/rom.h`，带 ROM 固件已编译并烧录到 M5StickS3。串口启动日志确认 `tamalib: initialized with local ROM`。

已完成：

- 用 Kiro CLI + `claude-opus-4.6` 讨论总体开发方案
- 明确路线：TamaLIB 原版核心优先，AI 作为 overlay/输入外挂层
- 明确 AI 顺序：先 Gemini 3 Flash 文字交互，后小智/语音
- 明确存档策略：TamaLIB 状态用 LittleFS，配置用 NVS
- 完成阶段 1：屏幕、按钮、喇叭、IMU 实机验证
- 完成阶段 2：TamaLIB submodule、HAL、ROM 本地生成入口、无 ROM 启动页

下一步：

- 进入阶段 3：LittleFS 存档、基础音量/亮度、功耗和可玩性打磨
- 用户侧可继续观察屏幕和物理操作：`key1`=A、`key2`=B、`key1+key2`=C

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
| 任务 | 阶段 2：带 P1 ROM 实机验证 |
| 状态 | 已完成到可交付状态 |
| 验收标准 | `data/tama.zip` 不提交；从 zip 内 `tama.bin` 生成 ignored 的 `data/rom.h`；带 ROM 固件编译并烧录；串口显示 TamaLIB 已初始化；屏幕进入 P1 界面；A/B/C 输入路径保持 `key1`/`key2`/组合键映射 |

## 里程碑进度

| 阶段 | 状态 | 验收标准 |
| --- | --- | --- |
| 阶段 0：项目基线 | 已完成 | 计划、进度、git 基线完成 |
| 阶段 1：硬件验证 | 已完成 | 屏幕/按钮/喇叭/IMU 可用 |
| 阶段 2：TamaLIB 移植 | 已完成到带本地 ROM 可交付状态 | TamaLIB 可编译、可烧录；ROM 本地接入；S3 HAL 已实现；本地 P1 ROM 固件启动成功 |
| 阶段 3：存档和功耗 | 未开始 | 重启不丢档，基础省电可用 |
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

## 阶段 2 交付物

- `.gitmodules` / `lib/tamalib`：TamaLIB submodule
- `lib/hal_types.h`：TamaLIB 目标类型定义
- `src/tama_app.*`：TamaLIB 应用桥接和 HAL
- `src/display.*`：无 ROM setup 页面和 32x16 到 128x64 渲染
- `src/buttons.*`：`key1`/`key2`/组合键到 A/B/C 的实时 mask
- `data/README.md`：本地 ROM 放置说明
- `tools/rom_to_header.py`：本地 ROM dump 转 `data/rom.h`

## 下一步详情

### 本地 ROM 验证

当前本地已存在并已使用：

- `data/tama.zip`：用户本地提供，ignored，不提交
- `data/tama.bin`：从 zip 提取，ignored，不提交
- `data/rom.h`：生成文件，ignored，不提交

预计操作：

```powershell
python tools\rom_to_header.py path\to\local_rom.txt data\rom.h --format text
.\.venv\Scripts\platformio.exe run
.\.venv\Scripts\platformio.exe run --target upload
```

注意：

- P1/P2 的 MAME 文件常见大小是 12288 bytes，应使用 `--format words-be16`。
- converter 已支持 6144 source words 自动补零到 8192 words。
- 不要用 `packed12-be` 手动处理 MAME P1/P2 文件；12288 bytes 虽然数学上能被 packed12 解成 8192 words，但内容会错。

最小验收：

- 屏幕从 ROM 准备页切换到 P1 LCD 画面
- 可看到初始画面或蛋
- `key1`、`key2`、`key1+key2` 能完成 A/B/C 操作
- 原版蜂鸣器声音可播放

### 阶段 3 预备

预计文件：

- `src/tama_storage.*`
- `src/settings.*`
- `src/power.*`

最小验收：

- 重启后可恢复 TamaLIB 状态
- 音量/亮度可保存
- 空闲时降低刷新/亮度，不影响手动操作
