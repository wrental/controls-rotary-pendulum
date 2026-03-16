
#include "esp_idf_rotary_pendulum.h"
#include "stepper_motor.h"

#include "driver/gpio.h"
#include "driver/mcpwm_prelude.h"


// MCPWM Controls
mcpwm_timer_handle_t timer_0;
mcpwm_timer_config_t timer_0_config = {
    .group_id = 0,
    .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
    .resolution_hz = 1 * 1000 * 1000, // 1 MHz
    .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
    .period_ticks = 1500, 
    .intr_priority = 0,
};

mcpwm_oper_handle_t oper_0;
mcpwm_operator_config_t oper_0_config = {
    .group_id = 0,
    .intr_priority = 0,
};

mcpwm_cmpr_handle_t comparator_0;
mcpwm_comparator_config_t comparator_0_config = {
    .flags.update_cmp_on_tez = true
};

mcpwm_gen_handle_t generator_0;
mcpwm_generator_config_t generator_0_config = {
    .gen_gpio_num = STP_STEP,
};

void stp_init(void) {
    mcpwm_new_timer(&timer_0_config, &timer_0);
    mcpwm_new_operator(&oper_0_config, &oper_0);
    mcpwm_operator_connect_timer(oper_0, timer_0);
    mcpwm_new_comparator(oper_0, &comparator_0_config, &comparator_0);
    mcpwm_new_generator(oper_0, &generator_0_config, &generator_0);
    mcpwm_generator_set_action_on_timer_event(generator_0, MCPWM_GEN_TIMER_EVENT_ACTION(
                MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH));
    mcpwm_generator_set_action_on_compare_event(generator_0, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,
                comparator_0, MCPWM_GEN_ACTION_LOW));
    mcpwm_timer_enable(timer_0);
    mcpwm_timer_start_stop(timer_0, MCPWM_TIMER_START_NO_STOP);
    mcpwm_comparator_set_compare_value(comparator_0, 750);
}



