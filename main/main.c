/* main.c - Application main entry point */

/*
 * Copyright (c) 2017 Intel Corporation
 * Additional Copyright (c) 2018 Espressif Systems (Shanghai) PTE LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"

#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_ble_mesh_config_model_api.h"
#include "esp_ble_mesh_local_data_operation_api.h"

#include "board.h"
#include "ble_mesh_example_init.h"
#include "loop.h"


#define TAG "EXAMPLE"

#define MAX_MSG_LENGTH 8
#define PUBLISH_RETRANSMIT_COUNT 0
#define PUBLISH_RETRANSMIT_PERIOD 50 /* not use less 50 otherwise it will turn to negative value in ESP_BLE_MESH_PUBLISH_TRANSMIT macro*/

#define CREATE_GROUP_ADDR(add) ((uint16_t)0xc000 | add)
#define GROUP_SEPTIC CREATE_GROUP_ADDR(10)
#define GROUP_DOORBELL CREATE_GROUP_ADDR(11)
#define GROUP_WATERPUMP CREATE_GROUP_ADDR(12)


#define SEPTIC_MODEL_ID_SERVER    0x0001

static uint8_t dev_uuid[ESP_BLE_MESH_OCTET16_LEN] = { 0x32, 0x10 };

static esp_ble_mesh_cfg_srv_t config_server = {
    .relay = ESP_BLE_MESH_RELAY_ENABLED,
    .beacon = ESP_BLE_MESH_BEACON_ENABLED,
#if defined(CONFIG_BLE_MESH_FRIEND)
    .friend_state = ESP_BLE_MESH_FRIEND_ENABLED,
#else
    .friend_state = ESP_BLE_MESH_FRIEND_NOT_SUPPORTED,
#endif
#if defined(CONFIG_BLE_MESH_GATT_PROXY_SERVER)
    .gatt_proxy = ESP_BLE_MESH_GATT_PROXY_ENABLED,
#else
    .gatt_proxy = ESP_BLE_MESH_GATT_PROXY_NOT_SUPPORTED,
#endif
    .default_ttl = 7,
    /* 3 transmissions with 20ms interval */
    .net_transmit = ESP_BLE_MESH_TRANSMIT(3, 50),
    .relay_retransmit = ESP_BLE_MESH_TRANSMIT(3, 50),
    //vpq: Best result 4 retransmition with 50ms interval. It can be changed from android application 
};

static esp_ble_mesh_model_t root_models[] = {
    ESP_BLE_MESH_MODEL_CFG_SRV(&config_server),
};

static esp_ble_mesh_model_op_t vnd_op[] = {
    ESP_BLE_MESH_MODEL_OP(SEPTIC_OP_SET, 2),
    ESP_BLE_MESH_MODEL_OP(SEPTIC_OP_GET, 1),
    ESP_BLE_MESH_MODEL_OP(SEPTIC_OP_ALARM_STATUS, 1),
    ESP_BLE_MESH_MODEL_OP(SEPTIC_OP_PEND_ALARM_STATUS, 1),
    ESP_BLE_MESH_MODEL_OP_END,
};

ESP_BLE_MESH_MODEL_PUB_DEFINE(vnd_pub, (3 + MAX_MSG_LENGTH), ROLE_NODE);  // MAX_MSG_LENGTH + length(op_code)
static esp_ble_mesh_model_t vnd_models[] = {
    ESP_BLE_MESH_VENDOR_MODEL(CID_ESP, SEPTIC_MODEL_ID_SERVER,
                              vnd_op, &vnd_pub, NULL),
};

// Call this function after provision completed!Even after bind key
void set_publish(void) {
//    esp_err_t err;
    vnd_pub.retransmit = ESP_BLE_MESH_PUBLISH_TRANSMIT(PUBLISH_RETRANSMIT_COUNT, PUBLISH_RETRANSMIT_PERIOD);
    vnd_pub.ttl = 0x07;
    vnd_pub.publish_addr = GROUP_SEPTIC;
    // esp_ble_mesh_model_subscribe_group_addr(vnd_models[0].element->element_addr,
    //                                         vnd_models[0].vnd.company_id,
    //                                         vnd_models[0].vnd.model_id,
    //                                         GROUP_SEPTIC);
    //if you subscribe on your publish address you will receive you own group messages in ESP_BLE_MESH_MODEL_OPERATION_EVT
    //Or filter this messages or don't subscribe
   // Note. If you subscribed one time it will be remembered in storage, so explicitly unsubscribe!
    // esp_ble_mesh_model_unsubscribe_group_addr(vnd_models[0].element->element_addr,
    //                                           vnd_models[0].vnd.company_id,
    //                                           vnd_models[0].vnd.model_id,
    //                                           GROUP_DOORBELL);
//    err=esp_ble_mesh_model_subscribe_group_addr(vnd_models[1].element->element_addr,//!!!! delete. may be receive only by node address
    // err=esp_ble_mesh_model_unsubscribe_group_addr(vnd_models[1].element->element_addr,//!!!! delete. may be receive only by node address
    //                                         vnd_models[1].vnd.company_id,
    //                                         vnd_models[1].vnd.model_id,
    //                                         GROUP_WATERPUMP);
//    ESP_LOGE(TAG,"subscritrion finished with err:%d",err);
}
static esp_ble_mesh_elem_t elements[] = {
    ESP_BLE_MESH_ELEMENT(0, root_models, vnd_models),
};

static esp_ble_mesh_comp_t composition = {
    .cid = CID_ESP,
    .elements = elements,
    .element_count = ARRAY_SIZE(elements),
};

static esp_ble_mesh_prov_t provision = {
    .uuid = dev_uuid,
};

static void prov_complete(uint16_t net_idx, uint16_t addr, uint8_t flags, uint32_t iv_index)
{
    ESP_LOGI(TAG, "net_idx 0x%03x, addr 0x%04x", net_idx, addr);
    ESP_LOGI(TAG, "flags 0x%02x, iv_index 0x%08x", flags, iv_index);
    LED_toggle();
}

static void example_ble_mesh_provisioning_cb(esp_ble_mesh_prov_cb_event_t event,
                                             esp_ble_mesh_prov_cb_param_t *param)
{
    switch (event) {
    case ESP_BLE_MESH_PROV_REGISTER_COMP_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_PROV_REGISTER_COMP_EVT, err_code %d", param->prov_register_comp.err_code);
        break;
    case ESP_BLE_MESH_NODE_PROV_ENABLE_COMP_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_PROV_ENABLE_COMP_EVT, err_code %d", param->node_prov_enable_comp.err_code);
        break;
    case ESP_BLE_MESH_NODE_PROV_LINK_OPEN_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_PROV_LINK_OPEN_EVT, bearer %s",
            param->node_prov_link_open.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT");
        break;
    case ESP_BLE_MESH_NODE_PROV_LINK_CLOSE_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_PROV_LINK_CLOSE_EVT, bearer %s",
            param->node_prov_link_close.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT");
        break;
    case ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT");
        prov_complete(param->node_prov_complete.net_idx, param->node_prov_complete.addr,
            param->node_prov_complete.flags, param->node_prov_complete.iv_index);
        break;
    case ESP_BLE_MESH_NODE_PROV_RESET_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_PROV_RESET_EVT");
        break;
    case ESP_BLE_MESH_NODE_SET_UNPROV_DEV_NAME_COMP_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_SET_UNPROV_DEV_NAME_COMP_EVT, err_code %d", param->node_set_unprov_dev_name_comp.err_code);
        break;
    default:
        break;
    }
}

static void example_ble_mesh_config_server_cb(esp_ble_mesh_cfg_server_cb_event_t event,
                                              esp_ble_mesh_cfg_server_cb_param_t *param)
{
    if (event == ESP_BLE_MESH_CFG_SERVER_STATE_CHANGE_EVT) {
        switch (param->ctx.recv_op) {
        case ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD:
            ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD");
            ESP_LOGI(TAG, "net_idx 0x%04x, app_idx 0x%04x",
                param->value.state_change.appkey_add.net_idx,
                param->value.state_change.appkey_add.app_idx);
            ESP_LOG_BUFFER_HEX("AppKey", param->value.state_change.appkey_add.app_key, 16);
            break;
        case ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND:
            ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND");
            ESP_LOGI(TAG, "elem_addr 0x%04x, app_idx 0x%04x, cid 0x%04x, mod_id 0x%04x",
                param->value.state_change.mod_app_bind.element_addr,
                param->value.state_change.mod_app_bind.app_idx,
                param->value.state_change.mod_app_bind.company_id,
                param->value.state_change.mod_app_bind.model_id);
            set_publish();
            break;
        default:
            break;
        }
    }
}

static void example_ble_mesh_custom_model_cb(esp_ble_mesh_model_cb_event_t event,
                                             esp_ble_mesh_model_cb_param_t *param)
{
    switch (event) {
        case ESP_BLE_MESH_MODEL_OPERATION_EVT:
            ESP_LOGI(TAG, "receive message from peer with rmt addr 0x%04x, dst_addr 0x%04x",
                            param->model_operation.ctx->addr,param->model_operation.ctx->recv_dst);
            if(param->model_operation.opcode==SEPTIC_OP_SET) {
                uint8_t id;
                int8_t val;
                set_get_message_st *cmd=(set_get_message_st*)param->model_operation.msg;
                set_get_message_st answer;
                id=cmd->id;
                val=cmd->val;
                answer.val=SepticSetParameter(id,val);
                answer.id=id;
                ESP_LOGI(TAG, "SET message-> param ID=%d, answer=0x%04x",*param->model_operation.msg,answer.val);
                esp_err_t err = esp_ble_mesh_server_model_send_msg(&vnd_models[0],
                                        param->model_operation.ctx, SEPTIC_OP_ALARM_STATUS,
                                        sizeof(answer), (uint8_t *)&answer);
                if (err) ESP_LOGE(TAG, "Failed to send message 0x%02x", SEPTIC_OP_ALARM_STATUS);
            }else{//only SEPTIC_OP_GET posible
                uint8_t id;
                set_get_message_st answer;
                id=*param->model_operation.msg;
                answer.val=SepticGetParameter(id);
                answer.id=id;
                ESP_LOGI(TAG, "GET message-> param ID=%d, answer=0x%02x",*param->model_operation.msg,answer.val);
                esp_err_t err = esp_ble_mesh_server_model_send_msg(&vnd_models[0],
                                        param->model_operation.ctx, SEPTIC_OP_ALARM_STATUS,
                                        sizeof(answer), (uint8_t *)&answer);
                if (err) ESP_LOGE(TAG, "Failed to send message 0x%02x", SEPTIC_OP_ALARM_STATUS);
            }
            break;
        case ESP_BLE_MESH_MODEL_SEND_COMP_EVT:
            if (param->model_send_comp.err_code) {
                ESP_LOGE(TAG, "Failed to send message 0x%06x", param->model_send_comp.opcode);
                break;
            }
            ESP_LOGI(TAG, "Send 0x%06x", param->model_send_comp.opcode);
            break;
        case ESP_BLE_MESH_MODEL_PUBLISH_COMP_EVT:
            ESP_LOGI(TAG, "Complete publish message with result err=%d", param->model_publish_comp.err_code);
            break;
        case ESP_BLE_MESH_CLIENT_MODEL_RECV_PUBLISH_MSG_EVT:
            ESP_LOGI(TAG,"We receive publish message length=%d",param->client_recv_publish_msg.length);
            ESP_LOGI(TAG,"opcode: 0x%06x; src_addr: 0x%04x, dst_addr: 0x%04x",param->client_recv_publish_msg.opcode,
                                                                              param->client_recv_publish_msg.ctx->addr,
                                                                              param->client_recv_publish_msg.ctx->recv_dst);
            break;
        default:
            ESP_LOGI(TAG, "Unproccessed event in model CB event: %d", event);
            break;
    }
}
void vendor_publish_message(uint32_t op_code, uint8_t*data, uint16_t length) {
    // uint8_t temp[5]={1,2,3,4,5};
    // if(op_code==SEPTIC_OP_PEND_ALARM_STATUS){
    //     esp_ble_mesh_model_publish(&vnd_models[0], op_code, 5, temp, ROLE_NODE);
    // }else
    esp_ble_mesh_model_publish(&vnd_models[0], op_code, length, data, ROLE_NODE);
}

static esp_err_t ble_mesh_init(void)
{
    esp_err_t err;

    esp_ble_mesh_register_prov_callback(example_ble_mesh_provisioning_cb);
    esp_ble_mesh_register_config_server_callback(example_ble_mesh_config_server_cb);
    esp_ble_mesh_register_custom_model_callback(example_ble_mesh_custom_model_cb);

    err = esp_ble_mesh_init(&provision, &composition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize mesh stack");
        return err;
    }


    err = esp_ble_mesh_node_prov_enable(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable mesh node");
        return err;
    }

    bt_mesh_set_device_name("Home/Septic");
    ESP_LOGI(TAG, "BLE Mesh Node initialized");
    set_publish();
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT,ESP_PWR_LVL_P9);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV,ESP_PWR_LVL_P9);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN,ESP_PWR_LVL_P9);
    return ESP_OK;
}

void app_main(void)
{
    esp_err_t err;

    ESP_LOGI(TAG, "Initializing...");

    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    board_init();

    err = bluetooth_init();
    if (err) {
        ESP_LOGE(TAG, "esp32_bluetooth_init failed (err %d)", err);
        return;
    }

    ble_mesh_get_dev_uuid(dev_uuid);

    /* Initialize the Bluetooth Mesh Subsystem */
    err = ble_mesh_init();
    if (err) {
        ESP_LOGE(TAG, "Bluetooth mesh init failed (err %d)", err);
    }
    xTaskCreate(loop_task,"main_loop_task",LOOP_STACK_SIZE,NULL,10,NULL);
}
