#ifndef ENCODE_H
#define ENCODE_H

#include <stdint.h>

typedef enum {
    ENCODE_LEFT = 0,
    ENCODE_RIGHT = 1
} Encode_Channel_t;

void Encode_Init(void);
void Encode_Clear(Encode_Channel_t channel);
uint16_t Encode_Get_Count(Encode_Channel_t channel);
int16_t Encode_Get_Delta(Encode_Channel_t channel);
int16_t Encode_Calc_Delta(uint16_t now, uint16_t last);

#endif
