# keyboard36-p4 — 电源自锁(GPIO19)测试固件

Keyboard36 v0.1 主控 **ESP32-P4** 的最小验证固件, 基于 **PlatformIO + pioarduino**。

## 功能

```
上电 → GPIO19 拉高(电源自锁) → 保持 30 秒 → 拉低(释放自锁) → 结束
```

| 供电方式 | 预期现象 |
|---|---|
| 电池供电 | 启动后倒计时 30 秒, 到点**整机自动关机** |
| USB/VBUS 供电 | 倒计时结束后仅打印释放日志, 板子不会关机(属正常) |

## 硬件依据

- `GPIO19/ADC1_CH3` = U6 (ESP32-P4, QFN-104) **pad 20** = 网络 `/ESP32-P4/POWER_ON`
- 该网络接 Power 子图的电源自锁回路(按键 P_SW 开机 → GPIO19 保持)
- 已通过 `Keyboard36_v0.1.kicad_pcb` 网表 + `ESP32-P4.kicad_sch` 引脚名双重核对

## 为什么用 pioarduino(而不是官方 espressif32 平台)?

PlatformIO **官方** `espressif32` 平台长期停留在 Arduino core 2.x / IDF 4.4,
**不支持 ESP32-P4**。pioarduino 是社区维护的分支(Arduino core 3.3.x / ESP-IDF
v5.5.x), 已完整支持 P4/S31 等新芯片, `platformio.ini` 里直接用 URL 引用即可。

## 板卡 ID 说明(本项目为什么有 boards/keyboard36_p4.json)

`platformio.ini` 的 `board = xxx` 必须对应平台包里 `boards/xxx.json` 的板卡定义
(芯片、Flash 大小、分区表、编译宏等)。pioarduino 自带的 `esp32-p4-evboard` 是
Espressif 官方 EV 板定义, 默认开启 `-DBOARD_HAS_PSRAM` 并按 16MB Flash 分区,
对本自研板不完全适用。因此本项目在 `boards/` 下自定义了 `keyboard36_p4.json`:

- 去掉 `BOARD_HAS_PSRAM`(板上 U1 SOP-8 是否为 PSRAM 待确认, 加回即一个字段)
- Flash 按 **4MB 保守配置**(default.csv 分区, 4/8/16MB 芯片均可安全启动)
- 确认 U5 实际容量后可按 `platformio.ini` 内注释切回 16MB

## 烧录 / 调试口(H2, 6P-2.54mm)

| Pin | 信号 | 接 USB-TTL 转接板 |
|---|---|---|
| 1 | 3V3 | (可不接) |
| 2 | U0TX | → RX |
| 3 | U0RX | → TX |
| 4 | RESET(EN) | 手动进下载模式用 |
| 5 | BOOT | 手动进下载模式用 |
| 6 | GND | → GND |

日志输出: UART0, 115200-8N1。

## 构建与烧录

```bash
pio run                    # 编译(首次会自动下载 pioarduino 平台和工具链, 较慢)
pio run -t upload          # 烧录(接 H2, 先手动进下载模式, 见下)
pio device monitor         # 串口监视
```

**手动进入下载模式**(板子无自动复位电路时):

1. 按住 BOOT(H2-Pin5 接地)
2. 点按一下 RESET(H2-Pin4 短接接地后断开)
3. 松开 BOOT → 芯片进入 ROM 下载模式
4. 执行 `pio run -t upload --upload-port COM<x>`

> P4 的 USB 口(接的是 USB-OTG 外设)也支持 ROM 下载模式, 可尝试直接用 USB
> 线烧录: 按住 BOOT 复位后, 设备会以 USB 形式出现, esptool 可识别。

## 注意事项

- 首次构建需要联网下载平台包和 riscv32 工具链(约几百 MB), 请耐心等待。
- 若 Flash 相关报错: 确认 U5 型号后在 `platformio.ini` 调整 `flash_size`。
- 断电瞬间 GPIO19 已被固件主动拉低; 若需重新开机, 按电源键 P_SW。
