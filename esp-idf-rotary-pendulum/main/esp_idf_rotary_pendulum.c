/* name: esp_idf_rotary_pendulum.c
 * date: 03-15-2026
 * auth: wren sobolewski, maggie dion
 * desc: c implementation of esp-idf version of rotary pendulum
 */

#include "esp_idf_rotary_pendulum.h"
#include "encoder.h"
#include "stepper_motor.h"

#include "freertos/FreeRTOS.h"

// main task for rotary pendulum, to be created by xTaskCreate()
void rotary_pendulum_main(void *pvParameters) {

    enc_init();
    stp_init();
    
    // TODO setup interrupt routine instead of continuous polling loop
    for(;;) {
        printf("position (ticks): %i\n", enc_get_pos_ticks());
   
    }
}

// Start main task loop from rotary_pendulum_main() function
void app_main(void) {
   xTaskCreate(rotary_pendulum_main, "rotary_pendulum_main", 4096, NULL, 2, NULL); 
}
