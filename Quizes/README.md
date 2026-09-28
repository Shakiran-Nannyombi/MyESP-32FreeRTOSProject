# Quiz

You have connected 2 LEDs to your ESP32 microcontroller: green (Fridge on) and Red (Fridge off). For each half-second the fridge is on, it accumulates a bill of UGX 500. When the total bill is less than UGX 6,000, the fridge is switched on for 1 second and off for half a second. Otherwise, the fridge will be on for only half a second and off for 2 seconds. Once the bill exceeds UGX 8,000, an orange LED lights, and a task called paybill clears ¾ of the bill and prints the total balance. Implement the above system on Wokwi and FreeRTOS-based C code using ESP-IDF and an ESPRESSIF board of your choice . Use software timers

![Microcontroller Image](./picture1.png)