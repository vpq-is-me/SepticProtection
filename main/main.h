#ifndef MAIN_H
#define MAIN_H
#include "stdint.h"
#include "esp_ble_mesh_defs.h"

#define CID_ESP     0x02E5

#define SEPTIC_OP_SET               ESP_BLE_MESH_MODEL_OP_3(0x00, CID_ESP)
#define SEPTIC_OP_GET               ESP_BLE_MESH_MODEL_OP_3(0x01, CID_ESP)
#define SEPTIC_OP_ALARM_STATUS      ESP_BLE_MESH_MODEL_OP_3(0x02, CID_ESP)
#define SEPTIC_OP_PEND_ALARM_STATUS ESP_BLE_MESH_MODEL_OP_3(0x03, CID_ESP)

void vendor_publish_message(uint32_t op_code, uint8_t*data, uint16_t length);

typedef struct __attribute__ ((packed)) tSetGetMessage_st{
    uint8_t id;
    int8_t val;
}set_get_message_st;

#endif