#ifndef CS_H
#define CS_H

#define NUM_CS_PINS 2

typedef struct {
  uint8_t pin; 
} CS_Map;

static const CS_Map cs_pin_map[NUM_CS_PINS] = {
    [0] = { .pin=12 }, // number is defined in ../lib/pin_num.h
    [1] = { .pin=13 }
};

extern void CurrSour(void);

#endif // CS_H 
