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

static int pos_ticks = 0;
static uint64_t last_isr_time = 0;
static uint64_t debounce_delay = 50; // microseconds
                                     //
static void IRAM_ATTR enc_isr(void* arg) {
    uint64_t now = esp_timer_get_time();
    if(now - last_isr_time > debounce_delay) {
        if(gpio_get_level(ENC_B)) {
            pos_ticks++;
        }
        else {
            pos_ticks--;
        }
        last_isr_time = esp_timer_get_time();
    }
}

// setup to read values from encoder
void enc_init(void) {
    gpio_reset_pin(ENC_A);
    gpio_set_direction(ENC_A, GPIO_MODE_INPUT);
    gpio_set_pull_mode(ENC_A, GPIO_PULLUP_ONLY);
    gpio_intr_enable(ENC_A);
    gpio_set_intr_type(ENC_A, GPIO_INTR_POSEDGE);
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
