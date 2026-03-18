/* name: stepper_motor.c
 * date: 03-16-2026
 * auth: wren sobolewski, maggie dion
 * desc: c implementation of stepper motor component of rotary pendulum, using mcpwm
 */

#include "stepper_motor.h"
#include "esp_idf_rotary_pendulum.h"

#include "driver/gpio.h"
#include "hal/gpio_types.h"
#include "freertos/FreeRTOS.h"
#include "driver/mcpwm_prelude.h"
#include "driver/mcpwm_gen.h"
#include "hal/mcpwm_types.h"

static uint8_t stp_dir = 0;


// MCPWM Controls

// set up timer for a default 50Hz PWM signal
mcpwm_timer_handle_t timer_0;
mcpwm_timer_config_t timer_0_config = {
    .group_id = 0,
    .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT, // default 80MHz
    .resolution_hz = 1 * 1000 * 1000, // 1 MHz
    .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
    .period_ticks = 20000, // ticks per period - at 20k should create 50Hz PWM
    .intr_priority = 0,
    .flags.update_period_on_empty = 1,
};

// set up operator
mcpwm_oper_handle_t oper_0;
mcpwm_operator_config_t oper_0_config = {
    .group_id = 0,
    .intr_priority = 0,
};

// removed comparator component - (hopefully) not needed

// set up PWM generator
mcpwm_gen_handle_t generator_0;
mcpwm_generator_config_t generator_0_config = {
    .gen_gpio_num = STP_STEP,
};

// manual gpio init done to avoid misinputs for different configs
static void stp_gpio_init(void) {

    gpio_reset_pin(STP_DIR);
    gpio_reset_pin(STP_STEP);
    gpio_reset_pin(STP_SLEEP);
    gpio_reset_pin(STP_RESET);
    gpio_reset_pin(STP_ENABLE);

    gpio_set_direction(STP_DIR,     GPIO_MODE_OUTPUT);
    gpio_set_direction(STP_STEP,    GPIO_MODE_OUTPUT);
    gpio_set_direction(STP_SLEEP,   GPIO_MODE_OUTPUT);
    gpio_set_direction(STP_RESET,   GPIO_MODE_OUTPUT);
    gpio_set_direction(STP_ENABLE,  GPIO_MODE_OUTPUT);

    gpio_set_pull_mode(STP_DIR,     GPIO_PULLDOWN_ONLY);
    gpio_set_pull_mode(STP_STEP,    GPIO_PULLDOWN_ONLY);
    gpio_set_pull_mode(STP_SLEEP,   GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(STP_RESET,   GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(STP_ENABLE,  GPIO_PULLDOWN_ONLY);

    gpio_set_level(STP_DIR,         0);
    stp_dir = 0;
    gpio_set_level(STP_STEP,        0);
    gpio_set_level(STP_SLEEP,       1);
    gpio_set_level(STP_RESET,       1);
    gpio_set_level(STP_ENABLE,      0);

}

static void stp_mcpwm_init(void) {
    mcpwm_new_timer(&timer_0_config, &timer_0);
    mcpwm_new_operator(&oper_0_config, &oper_0);
    mcpwm_operator_connect_timer(oper_0, timer_0);
    mcpwm_new_generator(oper_0, &generator_0_config, &generator_0);

    mcpwm_generator_set_action_on_timer_event(generator_0, 
            MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_FULL, MCPWM_GEN_ACTION_TOGGLE));

    mcpwm_timer_enable(timer_0);
    mcpwm_timer_start_stop(timer_0, MCPWM_TIMER_START_NO_STOP);
}


void stp_init(void) {
    stp_gpio_init();

    gpio_set_level(STP_RESET, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(STP_RESET, 1);

    stp_mcpwm_init();
}

void stp_set_dir(uint8_t direction) {
    gpio_set_level(STP_DIR, direction);
    stp_dir = direction;
}

uint8_t stp_get_dir(void) {
    return stp_dir;
}

/**
 * @brief set step frequency of motor in hz (steps per second)
 * @param frequency steps per second (hz)
 */
void stp_set_speed_hz(uint32_t frequency) {
    uint32_t period_ticks = timer_0_config.resolution_hz / frequency;
    mcpwm_timer_set_period(timer_0, period_ticks);
}

void stp_move(uint8_t direction, uint32_t speed) {
    stp_dir = direction;

}


