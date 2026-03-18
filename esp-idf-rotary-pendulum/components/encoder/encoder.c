/* name: encoder.c
 * date: 03-16-2026
 * auth: wren sobolewski, maggie dion
 * desc: c implementation of encoder logic
 */

#include "encoder.h"
#include "esp_idf_rotary_pendulum.h"

#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "hal/gpio_types.h"

static int pos_ticks = 0;

static void IRAM_ATTR enc_isr(void* arg) {
    if(gpio_get_level(ENC_A) == gpio_get_level(ENC_B)) {
        pos_ticks++;
    }
    else {
        pos_ticks--;
    }
}

// setup to read values from encoder
void enc_init(void) {
    gpio_install_isr_service(0);
    gpio_reset_pin(ENC_A);
    gpio_set_direction(ENC_A, GPIO_MODE_INPUT);
    gpio_set_pull_mode(ENC_A, GPIO_PULLUP_ONLY);
    gpio_intr_enable(ENC_A);
    gpio_set_intr_type(ENC_A, GPIO_INTR_ANYEDGE);
    gpio_isr_handler_add(ENC_A, enc_isr, NULL);

    gpio_reset_pin(ENC_B);
    gpio_set_direction(ENC_B, GPIO_MODE_INPUT);
    gpio_set_pull_mode(ENC_B, GPIO_PULLUP_ONLY);
}

int enc_get_pos_ticks(void) {
    return (int) pos_ticks;
}

double enc_get_pos_deg(void) {
    return (double) pos_ticks * 0.6;
}
