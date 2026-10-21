/*
 * Keyboard36 v0.1 - ESP32-P4 电源自锁(POWER_ON)测试固件
 * Platform: PlatformIO + pioarduino (Arduino framework)
 *
 * 功能:
 *   上电 -> GPIO19 拉高(锁定电源) -> 保持 30 秒 -> 拉低(释放自锁)
 *
 * 硬件依据 (Keyboard36_v0.1 PCB 网表核对):
 *   GPIO19/ADC1_CH3 = U6 (ESP32-P4, QFN-104) pad 20 = 网络 /ESP32-P4/POWER_ON
 *   该网络接 Power 子图的电源自锁回路: 拉高=维持整机供电, 拉低=允许断电
 *
 * 预期现象:
 *   - 电池供电: 上电启动, 30 秒后板子自动关机
 *   - USB 供电: VBUS 直接供电, 30 秒后仅日志提示, 板子不会关机
 *
 * 日志/烧录走 H2 调试座 (6P-2.54): 1=3V3 2=U0TX 3=U0RX 4=RESET 5=BOOT 6=GND
 */

#include <Arduino.h>

/* U6 pad20, 网络 /ESP32-P4/POWER_ON */
static constexpr int      POWER_ON_GPIO      = 19;
/* 保持供电时长(秒) */
static constexpr uint32_t POWER_HOLD_SECONDS = 30;

void setup()
{
    /* 尽早拉高 POWER_ON, 防止自锁回路未保持导致掉电 */
    pinMode(POWER_ON_GPIO, OUTPUT);
    digitalWrite(POWER_ON_GPIO, HIGH);

    Serial.begin(115200);
    delay(200);  /* 等串口就绪 */
    Serial.println();
    Serial.println("[power_hold] boot: Keyboard36 v0.1 / ESP32-P4");
    Serial.println("[power_hold] POWER_ON (GPIO19) -> HIGH, power latched");

    /* 倒计时 30 秒 */
    for (uint32_t s = POWER_HOLD_SECONDS; s > 0; s--) {
        Serial.printf("[power_hold] power hold: %2lu s remaining\r\n",
                      (unsigned long)s);
        delay(1000);
    }

    /* 释放自锁 */
    digitalWrite(POWER_ON_GPIO, LOW);
    Serial.println("[power_hold] POWER_ON (GPIO19) -> LOW, latch released");
}

void loop()
{
    /* 电池供电: 板子此刻应已断电关机, 走不到这里
       USB  供电: 板子仍存活, 空转提示 */
    delay(5000);
    Serial.println("[power_hold] board still alive => powered from VBUS/USB");
}
