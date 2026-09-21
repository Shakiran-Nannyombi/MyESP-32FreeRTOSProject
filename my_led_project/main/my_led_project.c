#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "LED";

#define LED1 GPIO_NUM_2
#define LED2 GPIO_NUM_4

void blink_task_1(void *pvParameters)
{
    gpio_reset_pin(LED1);
    gpio_set_direction(LED1, GPIO_MODE_OUTPUT);

    while (1) {
        gpio_set_level(LED1, 1);
        ESP_LOGI(TAG, "LED1 ON");
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_set_level(LED1, 0);
        ESP_LOGI(TAG, "LED1 OFF");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void blink_task_2(void *pvParameters)
{
    gpio_reset_pin(LED2);
    gpio_set_direction(LED2, GPIO_MODE_OUTPUT);

    while (1) {
        gpio_set_level(LED2, 1);
        ESP_LOGI(TAG, "LED2 ON");
        vTaskDelay(pdMS_TO_TICKS(200));
        gpio_set_level(LED2, 0);
        ESP_LOGI(TAG, "LED2 OFF");
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting FreeRTOS tasks...");
    xTaskCreate(blink_task_1, "Blink1", 2048, NULL, 1, NULL);
    xTaskCreate(blink_task_2, "Blink2", 2048, NULL, 1, NULL);
}
