/* name: esp_idf_rotary_pendulum.c
 * date: 03-15-2026
 * auth: wren sobolewski, maggie dion
 * desc: c implementation of esp-idf version of rotary pendulum
 */

#include "esp_idf_rotary_pendulum.h"
#include "encoder.h"
#include "freertos/idf_additions.h"
#include "portmacro.h"
#include "stepper_motor.h"

#include "freertos/FreeRTOS.h"

// main task for rotary pendulum, to be created by xTaskCreate()
void rotary_pendulum_main(void *pvParameters) {

    enc_init();
    stp_init();

    for(;;) {
        for(int i = 1; i < 100; i++) {
            stp_set_speed_hz(i * 50);
            vTaskDelay(1 / portTICK_PERIOD_MS);
        }

        for(int i = 100; i > 1; i--) {
            stp_set_speed_hz(50 * i);
            vTaskDelay(1 / portTICK_PERIOD_MS);
        }

        if(stp_get_dir() == 1) {
            stp_set_dir(0);
        }
        else {
            stp_set_dir(1);
        }

        printf("position (ticks): %i\n", enc_get_pos_ticks());
    }
}

// Start main task loop from rotary_pendulum_main() function
void app_main(void) {
   xTaskCreate(rotary_pendulum_main, "rotary_pendulum_main", 4096, NULL, 1, NULL); 
}
