#ifndef NEO6_H
#define NEO6_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "esp_err.h"
#include "driver/uart.h"


// init params

// Context: This structure will be populated with settings specific to your NEO-6M, 
// which uses 9600 baud, 8 data bits, no parity, and 1 stop bit.


//function prototypes

void uart_init(handle *uarthandle);




#ifdef __cplusplus
}
#endif
#endif /* NEO6_H */