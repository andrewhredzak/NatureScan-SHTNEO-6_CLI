#include <stdio.h>
#include <string.h>
#include "sht31.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"


static const char TAG[] = "SHT31.C";




void i2c_master_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = CONFIG_I2C_MASTER_SDA,
        .scl_io_num = CONFIG_I2C_MASTER_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, bus_handle));

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = SHT31_I2C_ADDR_0X44,
        .scl_speed_hz = CONFIG_I2C_MASTER_FREQUENCY,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus_handle, &dev_config, dev_handle));
}


/**
 * @brief starts periodic read to SHT31 sensor 
 */
esp_err_t SHT_START(i2c_master_dev_handle_t dev_handle, uint8_t cmd_msb, uint8_t cmd_lsb)
{
    uint8_t write_cmd[2] = {cmd_msb, cmd_lsb};
    // Print the contents of write_cmd
    //ESP_LOGI(TAG, "write_cmd[0] = 0x%02x, write_cmd[1] = 0x%02x", write_cmd[0], write_cmd[1]);
    //ESP_LOGI(TAG, "sizeof(write_cmd) = %zu", sizeof(write_cmd));
    return i2c_master_transmit(dev_handle, write_cmd, sizeof(write_cmd), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    //return i2c_master_transmit(dev_handle, data, 2, -1);
}


/**
 * @brief reads a temp and humidity measurement 
 */

esp_err_t SHT_READ(i2c_master_dev_handle_t dev_handle,float *temperature, float *humidity)
{
    esp_err_t ret;
    uint8_t raw_values[6] = {0};

    //ESP_LOGI(TAG, "sizeof(raw_values) = %zu", sizeof(raw_values));
    //ESP_LOGI(TAG, "tempurature = %f, humidity = %f");


    ret = i2c_master_receive(dev_handle, raw_values, 6, -1);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read temperature and humidity");
        return ret;
    }


    vTaskDelay(pdMS_TO_TICKS(20)); // Allow time for the measurement to complete
    // print raw values to see
    // we want to send raw values as it will be quicker...
    ESP_LOGI(TAG,"These are the raw_values: %02X %02X %02X %02X %02X %02X %02X", raw_values[0],
         raw_values[1], raw_values[2], raw_values[3], raw_values[4], raw_values[5], raw_values[6]);


    // Convert raw values to temperature and humidity
    uint16_t raw_temp = (raw_values[0] << 8) | raw_values[1];
    uint16_t raw_hum = (raw_values[3] << 8) | raw_values[4];

    *temperature = 175.0f * (float)raw_temp / 65535.0f - 45.0f;
    *humidity = 100.0f * (float)raw_hum / 65535.0f;

    // Print the temperature and humidity values
    ESP_LOGI(TAG, "Temperature: %.2f °C, Humidity: %.2f %%", *temperature, *humidity);
    
    return ESP_OK;



}




void SHT31TAKEDATA_task(void *pvParameters) {
    sht31_task_params_t* params = (sht31_task_params_t*)pvParameters;
    
    // Now you can use:
    //params->dev_handle;
    //params->temperature;
    //params->humidity;
    while (1) {
        //ESP_LOGI("SHT31TAKEDATA_task", "the task is running!"); //this just logs stuff. calle by esp_log
        // sht31 cmd send and read  
        ESP_ERROR_CHECK(SHT_START(params->dev_handle, SHT31_CMD_PERIODIC_MSB_TWO, SHT31_CMD_PERIODIC_LSB_HIGH));
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        ESP_ERROR_CHECK(SHT_READ(params->dev_handle, params->temperature, params->humidity ));
        vTaskDelay(pdMS_TO_TICKS(1000));   // vtaskDelay is from FreeRTOS
    }
    // Don't forget to free the parameters when done
    //free(params);
    //vTaskDelete(NULL);
}





