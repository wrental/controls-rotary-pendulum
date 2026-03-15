/* name: esp_idf_rotary_pendulum.h
 * date: 03-15-2026
 * auth: wren sobolewski, maggie dion
 * desc: header file with pin definitions for esp-idf version of rotary pendulum encoder
 */

#ifndef ESP_IDF_ROTARY_PENDULUM_H
#define ESP_IDF_ROTARY_PENDULUM_H

#define ENC_A       (GPIO_NUM_1)
#define ENC_B       (GPIO_NUM_2)

#define STP_DIR     (GPIO_NUM_3)
#define STP_STEP    (GPIO_NUM_4)
#define STP_SLEEP   (GPIO_NUM_18)
#define STP_RESET   (GPIO_NUM_17)
#define STP_MS_3    (GPIO_NUM_16)
#define STP_MS_2    (GPIO_NUM_15)
#define STP_MS_1    (GPIO_NUM_14)
#define STP_ENABLE  (GPIO_NUM_13)

#endif
