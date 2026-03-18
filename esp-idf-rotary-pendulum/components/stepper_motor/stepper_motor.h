/* name: stepper_motor.h
 * date: 03-16-2026
 * auth: wren sobolewski, maggie dion
 * desc: function calls, definitions for stepper motor component
 */

#ifndef STEPPER_MOTOR_H
#define STEPPER_MOTOR_H

#include "stdint.h"

void stp_init(void);
void stp_set_dir(uint8_t direction);
uint8_t stp_get_dir(void);
void stp_set_speed_hz(uint32_t speed);

/**
 * move stepper motor
 * @param direction 0 or 1
 * @param speed uint32_t for speed
 */
void stp_move(uint8_t direction, uint32_t speed);

#endif
