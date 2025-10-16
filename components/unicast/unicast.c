#include "unicast.h"
#include "esp_log.h"
#include "esp_crc.h"
#include "string.h"
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "freertos/queue.h"
#include "nvs_flash.h"
#include "esp_random.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "unicast.h"


const char *TAG = "unicast source file";
uint8_t s_example_broadcast_mac[ESP_NOW_ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
uint8_t s_ADAM_peer_mac[ESP_NOW_ETH_ALEN] = { 0x94, 0x54, 0xC5, 0xB1, 0x02, 0x60 }; // 94:54:c5:b1:02:60
// Hardcoded PMK (16 bytes, same on both devices)
static const uint8_t NS_pmk[16] = {
    0x01, 0xD2, 0xE3, 0xA4, 0xD5, 0x06, 0x07, 0x99,
    0x09, 0x0A, 0x0B, 0xAC, 0x0D, 0x0E, 0x0F, 0x10
};
// Hardcoded LMK for ADAM peer (16 bytes, same on both devices for this pair)
static const uint8_t NS_lmk[16] = {
    0xAA, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
    0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0xAA
};
uint16_t s_example_espnow_seq[EXAMPLE_ESPNOW_DATA_MAX] = { 0, 0 };
QueueHandle_t s_example_espnow_queue;

// ---- SHT31 -> ESPNOW bridge ----
static QueueHandle_t s_sht_queue = NULL; //declare SHT31 queue

// ---- GPS -> ESPNOW bridge ----
static QueueHandle_t s_gprmc_queue = NULL;  //declare the GPS queue

// ---- Combined Payload ----
static uint8_t s_combined_payload[sizeof(sht31_raw_sample_t) + sizeof(gprmc_data_t)] = {0};

static void sht_queue_task(void *arg) {
    sht31_raw_sample_t s;
    while (1) {
        if (xQueueReceive(s_sht_queue, &s, portMAX_DELAY) == pdTRUE) {
            // Copy SHT31 data into the first part of the combined payload
            memcpy(s_combined_payload, &s, sizeof(sht31_raw_sample_t));
        }
    }
}

esp_err_t unicast_sensor_queue_init(size_t depth) {
    if (s_sht_queue) return ESP_OK;
    s_sht_queue = xQueueCreate(depth, sizeof(sht31_raw_sample_t));
    if (!s_sht_queue) {
        ESP_LOGE(TAG, "Failed to create SHT31 sensor queue");
        return ESP_FAIL;
    }
    if (xTaskCreate(sht_queue_task, "sht_queue_task", 2048, NULL, 4, NULL) != pdPASS) {
        vQueueDelete(s_sht_queue);
        s_sht_queue = NULL;
        ESP_LOGE(TAG, "Failed to create sht_queue_task");
        return ESP_FAIL;
    }
    return ESP_OK;
}

bool unicast_sensor_queue_push(sht31_raw_sample_t sample) {
    if (!s_sht_queue) return false;
    return xQueueSend(s_sht_queue, &sample, 0) == pdTRUE;
}

static void gprmc_queue_task(void *arg) {
    gprmc_data_t gps_data;
    while (1) {
        if (xQueueReceive(s_gprmc_queue, &gps_data, portMAX_DELAY) == pdTRUE) {
            // Copy GPS data into the second part of the combined payload, after SHT31 data
            ESP_LOGI(TAG, "gpsprint unloading s_gprmc_queue: Lat %.6f, Lon %.6f, Speed %.2f, Course %.2f",
                 gps_data.latitude, gps_data.longitude, gps_data.speed, gps_data.course);
            memcpy(s_combined_payload + sizeof(sht31_raw_sample_t), &gps_data, sizeof(gprmc_data_t));
            ESP_LOG_BUFFER_HEX("Combined Payload:", s_combined_payload, sizeof(s_combined_payload));
        }
    }
}

esp_err_t unicast_gprmc_queue_init(size_t depth) {
    if (s_gprmc_queue) return ESP_OK;
    s_gprmc_queue = xQueueCreate(depth, sizeof(gprmc_data_t));
    if (!s_gprmc_queue) {
        ESP_LOGE(TAG, "Failed to create GPRMC queue");
        return ESP_FAIL;
    }
    if (xTaskCreate(gprmc_queue_task, "gprmc_queue_task", 2048, NULL, 4, NULL) != pdPASS) {
        vQueueDelete(s_gprmc_queue);
        s_gprmc_queue = NULL;
        ESP_LOGE(TAG, "Failed to create gprmc_queue_task");
        return ESP_FAIL;
    }
    return ESP_OK;
}

bool unicast_gprmc_queue_push(gprmc_data_t sample) {
    if (!s_gprmc_queue) return false;
    return xQueueSend(s_gprmc_queue, &sample, 0) == pdTRUE;
}



void unicast_print_test() {
    printf("START:  %s! \n", TAG);

}


/*static void example_espnow_deinit(example_espnow_send_param_t *send_param);  */

/* WiFi should start before using ESPNOW */
void example_wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(ESPNOW_WIFI_MODE) );
    ESP_ERROR_CHECK( esp_wifi_start());
    ESP_ERROR_CHECK( esp_wifi_set_channel(CONFIG_ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE));

#if CONFIG_ESPNOW_ENABLE_LONG_RANGE
    ESP_ERROR_CHECK( esp_wifi_set_protocol(ESPNOW_WIFI_IF, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR) );
#endif
}

/* ESPNOW sending or receiving callback function is called in WiFi task.
 * Users should not do lengthy operations from this task. Instead, post
 * necessary data to a queue and handle it from a lower priority task. */
static void example_espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    example_espnow_event_t evt;
    example_espnow_event_send_cb_t *send_cb = &evt.info.send_cb;

    if (mac_addr == NULL) {
        ESP_LOGE(TAG, "Send cb arg error");
        return;
    }

    evt.id = EXAMPLE_ESPNOW_SEND_CB;
    memcpy(send_cb->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
    send_cb->status = status;
    if (xQueueSend(s_example_espnow_queue, &evt, ESPNOW_MAXDELAY) != pdTRUE) {
        ESP_LOGW(TAG, "Send send queue fail");
    }
}

static void example_espnow_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len)
{
    example_espnow_event_t evt;
    example_espnow_event_recv_cb_t *recv_cb = &evt.info.recv_cb;
    uint8_t * mac_addr = recv_info->src_addr;
    uint8_t * des_addr = recv_info->des_addr;

    if (mac_addr == NULL || data == NULL || len <= 0) {
        ESP_LOGE(TAG, "Receive cb arg error");
        return;
    }

    if (IS_BROADCAST_ADDR(des_addr)) {
        /* If added a peer with encryption before, the receive packets may be
         * encrypted as peer-to-peer message or unencrypted over the broadcast channel.
         * Users can check the destination address to distinguish it.
         */
        ESP_LOGD(TAG, "Receive broadcast ESPNOW data");
    } else {
        ESP_LOGD(TAG, "Receive unicast ESPNOW data");
    }

    evt.id = EXAMPLE_ESPNOW_RECV_CB;
    memcpy(recv_cb->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
    recv_cb->data = malloc(len);
    if (recv_cb->data == NULL) {
        ESP_LOGE(TAG, "Malloc receive data fail");
        return;
    }
    memcpy(recv_cb->data, data, len);
    recv_cb->data_len = len;
    if (xQueueSend(s_example_espnow_queue, &evt, ESPNOW_MAXDELAY) != pdTRUE) {
        ESP_LOGW(TAG, "Send receive queue fail");
        free(recv_cb->data);
    }
}

/* Parse received ESPNOW data. */
int example_espnow_data_parse(uint8_t *data, uint16_t data_len, uint8_t *state, uint16_t *seq, uint32_t *magic)
{
    example_espnow_data_t *buf = (example_espnow_data_t *)data;
    uint16_t crc, crc_cal = 0;

    if (data_len < sizeof(example_espnow_data_t)) {
        ESP_LOGE(TAG, "Receive ESPNOW data too short, len:%d", data_len);
        return -1;
    }

    *state = buf->state;
    *seq = buf->seq_num;
    *magic = buf->magic;
    crc = buf->crc;
    buf->crc = 0;
    crc_cal = esp_crc16_le(UINT16_MAX, (uint8_t const *)buf, data_len);

    if (crc_cal == crc) {
        return buf->type;
    }

    return -1;
}

/* Prepare ESPNOW data to be sent. */
void example_espnow_data_prepare(example_espnow_send_param_t *send_param)
{
    example_espnow_data_t *buf = (example_espnow_data_t *)send_param->buffer;

    assert(send_param->len >= sizeof(example_espnow_data_t));

    buf->type = IS_BROADCAST_ADDR(send_param->dest_mac) ? EXAMPLE_ESPNOW_DATA_BROADCAST : EXAMPLE_ESPNOW_DATA_UNICAST;
    buf->state = send_param->state;
    buf->seq_num = s_example_espnow_seq[buf->type]++;
    buf->crc = 0;
    buf->magic = send_param->magic;
    int payload_space = send_param->len - sizeof(example_espnow_data_t);
    if (send_param->payload && send_param->payload_len > 0) {
        int copy_len = (send_param->payload_len < payload_space) ? send_param->payload_len : payload_space;
        memcpy(buf->payload, send_param->payload, copy_len);
        if (copy_len < payload_space) {
            memset(buf->payload + copy_len, 0, payload_space - copy_len); // zero pad
        }
    } else {
        esp_fill_random(buf->payload, payload_space);
    }
    buf->crc = esp_crc16_le(UINT16_MAX, (uint8_t const *)buf, send_param->len);
}

static void example_espnow_task(void *pvParameter)
{
    example_espnow_event_t evt;
    uint8_t recv_state = 0;
    uint16_t recv_seq = 0;
    uint32_t recv_magic = 0;
    bool is_broadcast = false;
    int ret;

    vTaskDelay(5000 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "Start sending unicast data");

    /* Start sending unicast ESPNOW data. */
    example_espnow_send_param_t *send_param = (example_espnow_send_param_t *)pvParameter;
    ESP_LOG_BUFFER_HEX("line 213 before esp_now_send unicast:", send_param->buffer, send_param->len);
    if (esp_now_send(send_param->dest_mac, send_param->buffer, send_param->len) != ESP_OK) {
        ESP_LOGE(TAG, "Unicast Send error");
        example_espnow_deinit(send_param);
        vTaskDelete(NULL);
    }


    while (xQueueReceive(s_example_espnow_queue, &evt, portMAX_DELAY) == pdTRUE) {
        switch (evt.id) {
            case EXAMPLE_ESPNOW_SEND_CB:
            {
                example_espnow_event_send_cb_t *send_cb = &evt.info.send_cb;
                is_broadcast = IS_BROADCAST_ADDR(send_cb->mac_addr);

                ESP_LOGD(TAG, "Send data to "MACSTR", status1: %d", MAC2STR(send_cb->mac_addr), send_cb->status);

                if (!is_broadcast) {
                    send_param->count--;
                    if (send_param->count == 0) {
                        ESP_LOGI(TAG, "Send done");
                        example_espnow_deinit(send_param);
                        vTaskDelete(NULL);
                    }
                }

                /* Delay a while before sending the next data. */
                if (send_param->delay > 0) {
                    vTaskDelay(send_param->delay/portTICK_PERIOD_MS);
                }

                wifi_mode_t mode;
                esp_wifi_get_mode(&mode);
                const char *mode_str = "UNKNOWN";
                if (mode == WIFI_MODE_STA) mode_str = "STA";
                else if (mode == WIFI_MODE_AP) mode_str = "AP";
                else if (mode == WIFI_MODE_APSTA) mode_str = "AP+STA";

                

                ESP_LOGI("CLI test", "WiFi mode: %s, sending data to "MACSTR"", mode_str, MAC2STR(send_cb->mac_addr));

                memcpy(send_param->dest_mac, send_cb->mac_addr, ESP_NOW_ETH_ALEN);
                example_espnow_data_prepare(send_param);

                // Print the payload before transmission
                if (send_param->payload && send_param->payload_len > 0) {
                    //ESP_LOG_BUFFER_HEX("temp+humid payload:", send_param->payload, send_param->payload_len);
                    //printf("payload: %.*s\n", send_param->payload_len, (char *)send_param->payload);
                }

                /* Send the next data after the previous data is sent. */
                ESP_LOG_BUFFER_HEX("line 269 temp+humid payload:", send_param->payload, send_param->payload_len);
                ESP_LOG_BUFFER_HEX("line 270 send_param->buffer:", send_param->buffer, send_param->len);
                if (esp_now_send(send_param->dest_mac, send_param->buffer, send_param->len) != ESP_OK) {
                    ESP_LOGE(TAG, "Unicast Send error 2");
                    example_espnow_deinit(send_param);
                    vTaskDelete(NULL);
                }
                break;
            }
            case EXAMPLE_ESPNOW_RECV_CB:
            {
                example_espnow_event_recv_cb_t *recv_cb = &evt.info.recv_cb;

                ret = example_espnow_data_parse(recv_cb->data, recv_cb->data_len, &recv_state, &recv_seq, &recv_magic);
                free(recv_cb->data);
                if (ret == EXAMPLE_ESPNOW_DATA_BROADCAST) {
                    ESP_LOGI(TAG, "Receive %dth broadcast data from: "MACSTR", len: %d", recv_seq, MAC2STR(recv_cb->mac_addr), recv_cb->data_len);

                    /* If MAC address does not exist in peer list, add it to peer list. */
                    if (esp_now_is_peer_exist(recv_cb->mac_addr) == false) {
                        esp_now_peer_info_t *peer = malloc(sizeof(esp_now_peer_info_t));
                        if (peer == NULL) {
                            ESP_LOGE(TAG, "Malloc peer information fail");
                            example_espnow_deinit(send_param);
                            vTaskDelete(NULL);
                        }
                        memset(peer, 0, sizeof(esp_now_peer_info_t));
                        peer->channel = CONFIG_ESPNOW_CHANNEL;
                        peer->ifidx = ESPNOW_WIFI_IF;
                        peer->encrypt = false;
                        memcpy(peer->lmk, NS_lmk, ESP_NOW_KEY_LEN);
                        memcpy(peer->peer_addr, recv_cb->mac_addr, ESP_NOW_ETH_ALEN);
                        ESP_ERROR_CHECK( esp_now_add_peer(peer) );
                        free(peer);
                    }

                    /* Indicates that the device has received broadcast ESPNOW data. */
                    if (send_param->state == 0) {
                        send_param->state = 1;
                    }

                    /* If receive broadcast ESPNOW data which indicates that the other device has received
                     * broadcast ESPNOW data and the local magic number is bigger than that in the received
                     * broadcast ESPNOW data, stop sending broadcast ESPNOW data and start sending unicast
                     * ESPNOW data.
                     */
                    if (recv_state == 1) {
                        /* The device which has the bigger magic number sends ESPNOW data, the other one
                         * receives ESPNOW data.
                         */
                        if (send_param->unicast == false && send_param->magic >= recv_magic) {
                    	    ESP_LOGI(TAG, "Start sending unicast data");
                    	    ESP_LOGI(TAG, "send data to "MACSTR"", MAC2STR(recv_cb->mac_addr));

                    	    /* Start sending unicast ESPNOW data. */
                            memcpy(send_param->dest_mac, recv_cb->mac_addr, ESP_NOW_ETH_ALEN);
                            example_espnow_data_prepare(send_param);
                            ESP_LOG_BUFFER_CHAR("line 324 before esp_now_send unicast?:", send_param->buffer, send_param->len);
                            if (esp_now_send(send_param->dest_mac, send_param->buffer, send_param->len) != ESP_OK) {
                                ESP_LOGE(TAG, "Unicast Send error 1");
                                example_espnow_deinit(send_param);
                                vTaskDelete(NULL);
                            }
                            else {
                                send_param->broadcast = false;
                                send_param->unicast = true;
                            }
                        }
                    }
                }
                else if (ret == EXAMPLE_ESPNOW_DATA_UNICAST) {
                    ESP_LOGI(TAG, "Receive %dth unicast data from: "MACSTR", len: %d", recv_seq, MAC2STR(recv_cb->mac_addr), recv_cb->data_len);

                    /* If receive unicast ESPNOW data, also stop sending broadcast ESPNOW data. */
                    send_param->broadcast = false;
                }
                else {
                    ESP_LOGI(TAG, "Receive error data from: "MACSTR"", MAC2STR(recv_cb->mac_addr));
                }
                break;
            }
            default:
                ESP_LOGE(TAG, "Callback type error: %d", evt.id);
                break;
        }
    }
}

esp_err_t example_espnow_init(void)
{
    example_espnow_send_param_t *send_param;

    s_example_espnow_queue = xQueueCreate(ESPNOW_QUEUE_SIZE, sizeof(example_espnow_event_t));
    if (s_example_espnow_queue == NULL) {
        ESP_LOGE(TAG, "Create mutex fail");
        return ESP_FAIL;
    }

    /* Initialize ESPNOW and register sending and receiving callback function. */
    ESP_ERROR_CHECK( esp_now_init() );
    ESP_ERROR_CHECK( esp_now_register_send_cb(example_espnow_send_cb) );
    ESP_ERROR_CHECK( esp_now_register_recv_cb(example_espnow_recv_cb) );
#if CONFIG_ESPNOW_ENABLE_POWER_SAVE
    ESP_ERROR_CHECK( esp_now_set_wake_window(CONFIG_ESPNOW_WAKE_WINDOW) );
    ESP_ERROR_CHECK( esp_wifi_connectionless_module_set_wake_interval(CONFIG_ESPNOW_WAKE_INTERVAL) );
#endif
    /* Set primary master key. */
    ESP_ERROR_CHECK( esp_now_set_pmk((uint8_t *)NS_pmk) );

    /* Initialize sending parameters. */
    send_param = malloc(sizeof(example_espnow_send_param_t));
    if (send_param == NULL) {
        ESP_LOGE(TAG, "Malloc send parameter fail");
        vSemaphoreDelete(s_example_espnow_queue);
        esp_now_deinit();
        return ESP_FAIL;
    }
    memset(send_param, 0, sizeof(example_espnow_send_param_t));
    send_param->unicast = true;  // Start in unicast mode
    send_param->broadcast = false;
    send_param->state = 0;
    send_param->magic = esp_random();
    send_param->count = CONFIG_ESPNOW_SEND_COUNT;
    send_param->delay = CONFIG_ESPNOW_SEND_DELAY;

    // Use the combined payload buffer
    send_param->payload = s_combined_payload;
    send_param->payload_len = sizeof(s_combined_payload);

    // Compute total frame length = header + payload
    size_t header_len = sizeof(example_espnow_data_t);
    send_param->len = header_len + send_param->payload_len;

    send_param->buffer = malloc(send_param->len);
    if (send_param->buffer == NULL) {
        ESP_LOGE(TAG, "Malloc send buffer fail");
        free(send_param);
        vSemaphoreDelete(s_example_espnow_queue);
        esp_now_deinit();
        return ESP_FAIL;
    }

    // Set destination MAC to known peer (replace with actual peer MAC)
    memcpy(send_param->dest_mac, s_ADAM_peer_mac, ESP_NOW_ETH_ALEN);

    // Add unicast peer to peer list
    esp_now_peer_info_t *unicast_peer = malloc(sizeof(esp_now_peer_info_t));
    if (unicast_peer == NULL) {
        ESP_LOGE(TAG, "Malloc unicast peer information fail");
        free(send_param->buffer);
        free(send_param);
        vSemaphoreDelete(s_example_espnow_queue);
        esp_now_deinit();
        return ESP_FAIL;
    }
    memset(unicast_peer, 0, sizeof(esp_now_peer_info_t));
    ESP_LOGI(TAG, "unicast channel: %d", CONFIG_ESPNOW_CHANNEL);
    unicast_peer->channel = CONFIG_ESPNOW_CHANNEL;
    unicast_peer->ifidx = ESPNOW_WIFI_IF;
    unicast_peer->encrypt = false;
    memcpy(unicast_peer->lmk, NS_lmk, ESP_NOW_KEY_LEN); //unicast local master key
    memcpy(unicast_peer->peer_addr, s_ADAM_peer_mac, ESP_NOW_ETH_ALEN);
    ESP_ERROR_CHECK( esp_now_add_peer(unicast_peer) );
    free(unicast_peer);

    example_espnow_data_prepare(send_param);

    xTaskCreate(example_espnow_task, "example_espnow_task", 2048, send_param, 4, NULL);

    return ESP_OK;
}

static void example_espnow_deinit(example_espnow_send_param_t *send_param)
{
    free(send_param->buffer);
    free(send_param);
    vSemaphoreDelete(s_example_espnow_queue);
    esp_now_deinit();
}
