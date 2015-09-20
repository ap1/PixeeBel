#ifndef __VISUALIZE_H
#define __VISUALIZE_H

#include <stdint.h>


#define PORT         (3000)

void visualize_init();

void visualize_send( uint16_t id, uint16_t channel, uint16_t mag );

#endif // __VISUALIZE_H

