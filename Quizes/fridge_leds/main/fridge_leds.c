#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "FRIDGE";

#define LED_GREEN GPIO_NUM_2 /* fridge ON  */
#define LED_RED   GPIO_NUM_4 /* fridge OFF */
#define LED_ORANGE GPIO_NUM_5 /* pay now    */

#define CHARGE_PER_HALF_SEC 500
#define CHEAP_LIMIT         6000
#define PAY_LIMIT           8000

static int bill = 0;
static bool fridge_on = false;

static SemaphoreHandle_t bill_mutex;
static SemaphoreHandle_t pay_sem;
static TimerHandle_t bill_timer;
static TimerHandle_t fridge_timer;

static void set_fridge_leds(bool on)
{
    gpio_set_level(LED_GREEN, on);
    gpio_set_level(LED_RED, !on);
}

static void bill_timer_cb(TimerHandle_t xTimer)
{
    bool should_pay = false;

    (void)xTimer;

    if (xSemaphoreTake(bill_mutex, 0) == pdTRUE) {
        if (fridge_on) {
            bill += CHARGE_PER_HALF_SEC;
            ESP_LOGI(TAG, "Bill = %d", bill);
            if (bill > PAY_LIMIT) {
                gpio_set_level(LED_ORANGE, 1);
                should_pay = true;
            }
        }
        xSemaphoreGive(bill_mutex);
    }

    if (should_pay) {
        xSemaphoreGive(pay_sem);
    }
}

static void fridge_timer_cb(TimerHandle_t xTimer)
{
    TickType_t next_ticks;

    (void)xTimer;

    if (xSemaphoreTake(bill_mutex, 0) != pdTRUE) {
        xTimerChangePeriod(fridge_timer, pdMS_TO_TICKS(50), 0);
        return;
    }

    fridge_on = !fridge_on;
    set_fridge_leds(fridge_on);

    if (fridge_on) {
        ESP_LOGI(TAG, "Fridge ON");
        next_ticks = (bill < CHEAP_LIMIT) ? pdMS_TO_TICKS(1000) : pdMS_TO_TICKS(500);
    } else {
        ESP_LOGI(TAG, "Fridge OFF");
        next_ticks = (bill < CHEAP_LIMIT) ? pdMS_TO_TICKS(500) : pdMS_TO_TICKS(2000);
    }

    xSemaphoreGive(bill_mutex);
    xTimerChangePeriod(fridge_timer, next_ticks, 0);
}

static void paybill(void *pvParameters)
{
    (void)pvParameters;

    while (1) {
        xSemaphoreTake(pay_sem, portMAX_DELAY);

        xSemaphoreTake(bill_mutex, portMAX_DELAY);
        bill = bill / 4;
        ESP_LOGI(TAG, "paybill: balance = %d", bill);
        if (bill <= PAY_LIMIT) {
            gpio_set_level(LED_ORANGE, 0);
        }
        xSemaphoreGive(bill_mutex);
    }
}

void app_main(void)
{
    gpio_reset_pin(LED_GREEN);
    gpio_reset_pin(LED_RED);
    gpio_reset_pin(LED_ORANGE);
    gpio_set_direction(LED_GREEN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_RED, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_ORANGE, GPIO_MODE_OUTPUT);

    gpio_set_level(LED_GREEN, 0);
    gpio_set_level(LED_RED, 1);
    gpio_set_level(LED_ORANGE, 0);

    bill_mutex = xSemaphoreCreateMutex();
    pay_sem = xSemaphoreCreateBinary();
    if (bill_mutex == NULL || pay_sem == NULL) {
        ESP_LOGE(TAG, "Failed to create mutex/semaphore");
        return;
    }

    xTaskCreate(paybill, "paybill", 2048, NULL, 2, NULL);

    bill_timer = xTimerCreate("bill", pdMS_TO_TICKS(500), pdTRUE, 0, bill_timer_cb);
    fridge_timer = xTimerCreate("fridge", pdMS_TO_TICKS(10), pdFALSE, 0, fridge_timer_cb);
    if (bill_timer == NULL || fridge_timer == NULL) {
        ESP_LOGE(TAG, "Failed to create timers");
        return;
    }

    xTimerStart(bill_timer, 0);
    xTimerStart(fridge_timer, 0);
    ESP_LOGI(TAG, "Fridge system started");
}
