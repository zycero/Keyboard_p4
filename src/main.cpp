/*
 * Keyboard36 v0.1 - ESP32-P4 电源自锁(POWER_ON)测试固件
 * Platform: PlatformIO + pioarduino (Arduino framework)
 *
 * 功能:
 *   上电 -> GPIO25 立即输出高电平
 *        -> 延时 6min -> GPIO19 拉高(锁定电源) -> 常态保持, 不再拉低
 *
 * 硬件依据 (Keyboard36_v0.1 PCB 网表核对):
 *   GPIO19/ADC1_CH3 = U6 (ESP32-P4, QFN-104) pad 20 = 网络 /ESP32-P4/POWER_ON
 *   该网络接 Power 子图的电源自锁回路: 拉高=维持整机供电, 拉低=允许断电
 *   GPIO25 = 额外控制输出(启动即拉高)
 *
 * 预期现象:
 *   - 上电后 6 分钟内需按住 P_SW 维持供电(倒计时日志)
 *   - 6 分钟后 GPIO19 拉高自锁, 此后松开 P_SW 板子不掉电
 *   - GPIO25 全程输出高电平
 *   - 串口每 1 秒打印一次心跳日志(含引脚实际电平回读)
 *
 * 日志/烧录走 H2 调试座 (6P-2.54): 1=3V3 2=U0TX 3=U0RX 4=RESET 5=BOOT 6=GND
 */

#include <Arduino.h>
#include <esp_task_wdt.h>

/* U6 pad20, 网络 /ESP32-P4/POWER_ON */
static constexpr int POWER_ON_GPIO   = 19;
/* 额外控制输出引脚 */
static constexpr int ENABLE_GPIO25   = 25;
/* 上电到拉高自锁的延时(毫秒): 6 分钟 */
static constexpr uint32_t LATCH_DELAY_MS = 360000UL;

static bool s_latched = false;
static uint32_t s_bootMs = 0;

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
    /* 关闭任务看门狗(在自锁延时期间防止复位) */
    disable_watchdogs();

    /* POWER_ON 先保持低电平(未自锁, 需按住 P_SW 供电) */
    pinMode(POWER_ON_GPIO, OUTPUT);
    digitalWrite(POWER_ON_GPIO, LOW);

    /* GPIO25 启动即输出高电平 */
    pinMode(ENABLE_GPIO25, OUTPUT);
    digitalWrite(ENABLE_GPIO25, HIGH);

    Serial.begin(115200);
    delay(200);  /* 等串口就绪 */
    Serial.println();
    Serial.println("[power_hold] boot: Keyboard36 v0.1 / ESP32-P4");
    Serial.printf("[power_hold] GPIO25 -> HIGH (immediate), readback=%d\r\n",
                  digitalRead(ENABLE_GPIO25));
    Serial.printf("[power_hold] GPIO19 = LOW now, will latch HIGH after %lu ms\r\n",
                  (unsigned long)LATCH_DELAY_MS);
    Serial.println("[power_hold] HOLD power key P_SW during countdown!");

    s_bootMs = millis();
}

void loop()
{
    /* 非阻塞判断 6min 到时, 到时拉高 GPIO19 自锁(只执行一次) */
    if (!s_latched && (millis() - s_bootMs) >= LATCH_DELAY_MS) {
        digitalWrite(POWER_ON_GPIO, HIGH);
        s_latched = true;
        Serial.printf("[power_hold] *** GPIO19 -> HIGH, power latched! (t=%lu ms) ***\r\n",
                      (unsigned long)(millis() - s_bootMs));
        Serial.printf("[power_hold] readback: GPIO19=%d GPIO25=%d\r\n",
                      digitalRead(POWER_ON_GPIO), digitalRead(ENABLE_GPIO25));
    }

    delay(1000);  /* 1 秒心跳 */

    uint32_t elapsed = millis() - s_bootMs;
    if (!s_latched) {
        /* 倒计时期间: 每秒打印剩余秒数 */
        Serial.printf("[power_hold] alive t=%lus, GPIO19=LOW, remaining=%lus, GPIO25=%d\r\n",
                      (unsigned long)(elapsed / 1000),
                      (unsigned long)((LATCH_DELAY_MS - elapsed) / 1000),
                      digitalRead(ENABLE_GPIO25));
    } else {
        /* 自锁后: 报告两引脚实际电平 */
        Serial.printf("[power_hold] alive t=%lus, GPIO19=%d(latched), GPIO25=%d\r\n",
                      (unsigned long)(elapsed / 1000),
                      digitalRead(POWER_ON_GPIO), digitalRead(ENABLE_GPIO25));
    }
}
