/* name: encoder.h
 * date: 03-16-2026
 * auth: wren sobolewski, maggie dion
 * desc: header file to define functions relating to encoder
 */

#ifndef ENCODER_H
#define ENCODER_H

void enc_init(void);
int enc_get_pos_ticks(void);
double enc_get_pos_deg(void);

#endif
