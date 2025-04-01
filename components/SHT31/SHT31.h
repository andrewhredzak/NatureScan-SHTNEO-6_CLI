#ifndef SHT31_H
#define SHT31_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"




// Configuration macros 
#define I2C_MASTER_SCL_IO           CONFIG_I2C_MASTER_SCL        /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           CONFIG_I2C_MASTER_SDA              /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              I2C_NUM_0                   /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ          CONFIG_I2C_MASTER_FREQUENCY        /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000     // timeout 3 seconds



#define SHT31_I2C_ADDR_0X44 0x44 // device address given from senserion 
#define SHT31_I2C_ADDR_0X45 0x45 
#define SHT31_I2C_WRITE_ADDR 0x00        /*!< Register addresses of the "who am I" register */





/*
 * Periodic Measurement Mode Commands:
 *   - 'MPS' => measurements per second
 *   - 'Repeatability' => High (H), Medium (M), Low (L)
 */

#define SHT31_CMD_PERIODIC_HALF_H 0x2032 
#define SHT31_CMD_PERIODIC_HALF_M 0x2024
#define SHT31_CMD_PERIODIC_HALF   0x20
#define SHT31_CMD_PERIODIC_MSB_TWO  0x22  // 2 mps
#define SHT31_CMD_PERIODIC_LSB_HIGH 0x36  // high repeatability 

#define SHT31_CMD_FETCHDATA_MSB 0xE0
#define SHT31_CMD_FETCHDATA_LSB 0x00



typedef struct {
    i2c_master_dev_handle_t *dev_handle;
    float *temperature;
    float *humidity;
} sht31_task_params_t;





// Function prototypes
void i2c_master_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle);
esp_err_t SHT_START(i2c_master_dev_handle_t dev_handle, uint8_t cmd_msb, uint8_t cmd_lsb);
esp_err_t SHT_READ(i2c_master_dev_handle_t dev_handle,float *temperature, float *humidity);
void SHT31TAKEDATA_task(void *pvParameters);






#ifdef __cplusplus
}
#endif
#endif /* SHT31_H */