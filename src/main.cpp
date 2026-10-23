/*
 * Keyboard36 v0.1 - ESP32-P4 电源自锁(POWER_ON)测试固件
 * Platform: PlatformIO + pioarduino (Arduino framework)
 *
 * 功能:
 *   上电 -> GPIO19 拉高(锁定电源) -> 常态保持, 不再拉低
 *
 * 硬件依据 (Keyboard36_v0.1 PCB 网表核对):
 *   GPIO19/ADC1_CH3 = U6 (ESP32-P4, QFN-104) pad 20 = 网络 /ESP32-P4/POWER_ON
 *   该网络接 Power 子图的电源自锁回路: 拉高=维持整机供电, 拉低=允许断电
 *
 * 预期现象:
 *   - 上电后板子持续供电运行, GPIO19 始终为高(自锁保持)
 *   - 串口每 5 秒打印一次心跳日志
 *
 * 日志/烧录走 H2 调试座 (6P-2.54): 1=3V3 2=U0TX 3=U0RX 4=RESET 5=BOOT 6=GND
 */

#include <Arduino.h>
#include <esp_task_wdt.h>

/* U6 pad20, 网络 /ESP32-P4/POWER_ON */
static constexpr int POWER_ON_GPIO = 19;

static void disable_watchdogs()
{
    /* 任务看门狗(TWDT): 把当前任务(loopTask)移出监视 */
    esp_err_t err = esp_task_wdt_delete(NULL); /* 未被监视时返回 ESP_ERR_NOT_FOUND, 无害 */
    (void)err;
    /* 尝试整体反初始化 TWDT(IDLE 任务仍被监视时会失败, 同样无害;
       失败时 TWDT 继续运行, 但 IDLE 任务在 delay() 期间会正常喂狗) */
    esp_task_wdt_deinit();
}

void setup()
{
    /* 关闭任务看门狗(在拉高电源自锁之前, 防止长延时期间复位) */
    disable_watchdogs();

    /* 尽早拉高 POWER_ON, 防止自锁回路未保持导致掉电 */
    pinMode(POWER_ON_GPIO, OUTPUT);
    digitalWrite(POWER_ON_GPIO, HIGH);

    Serial.begin(115200);
    delay(200);  /* 等串口就绪 */
    Serial.println();
    Serial.println("[power_hold] boot: Keyboard36 v0.1 / ESP32-P4");
    Serial.println("[power_hold] POWER_ON (GPIO19) -> HIGH, power latched");
    Serial.println("[power_hold] task watchdog disabled");

    /* 常态: POWER_ON 保持拉高, 不再释放自锁 */
    Serial.println("[power_hold] POWER_ON stays HIGH forever (latch held)");
}

void loop()
{
    /* 心跳: 确认板子存活且 GPIO19 保持拉高 */
    delay(1000);
    Serial.println("[power_hold] alive, POWER_ON (GPIO19) = HIGH");
}
