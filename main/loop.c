#include "loop.h"
#include "board.h"
#include "esp_err.h"
#include "esp_random.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

#define TAG "LOOP"
#define ABSi(x) (x<0 ? -x : x)

 
typedef struct {
    uint8_t alarm_pend_fgs;   //bitwise alarm flags. Flags, that still present in system. This flags cleared automatically, but not cleared by "reset" remotely
    uint8_t alarm_nack_fgs;   //bitwise alarm flags. New flags that are still not acknowledged.Will set when appropriate bit in 'alarm_pend_fgs' changed from 0 to 1, and cleared remotely
} tSeptic_st;

tSeptic_st septic_data = {
    .alarm_pend_fgs=0, 
    .alarm_nack_fgs=0,
    };
void GetCurrentBoardState(void);
void loop_task(void* arg) {
    uint8_t msg_rrobin = 0;//round robin counter for publishing info
    uint32_t msg_opcode;
    uint8_t* msg_p;
    DigInInit();
    while (1) {  
        vTaskDelay(pdMS_TO_TICKS(3300));
        ESP_LOGI(TAG, "nACK: 0x%02x pend: 0x%02x",(int)septic_data.alarm_nack_fgs,(int)septic_data.alarm_pend_fgs);
        GetCurrentBoardState();
        //*******periodical send message block*************************
        switch (msg_rrobin) {
            case 0:
                msg_opcode = SEPTIC_OP_ALARM_STATUS;
                msg_p = (uint8_t*)&(septic_data.alarm_nack_fgs);
                break;
            default: //case 1
                msg_opcode = SEPTIC_OP_PEND_ALARM_STATUS;
                msg_p = (uint8_t*)&(septic_data.alarm_pend_fgs);
                break;
        }
        if(++msg_rrobin>1)msg_rrobin=0;
        vendor_publish_message(msg_opcode,msg_p,1);
    }  // END while(1)
}
void GetCurrentBoardState(void){
    uint32_t tmp=dig_in_snap;
    uint8_t snap=tmp&0xff;
    if(IsButtonPressed()) septic_data.alarm_nack_fgs=0;
    septic_data.alarm_nack_fgs|=(septic_data.alarm_pend_fgs^snap)&snap;//set to 1 new arised alarms
    septic_data.alarm_pend_fgs=snap;    
}

int8_t SepticGetParameter(Septic_param_id id){
    int8_t res;
    switch(id){
        case SEPTIC_PARAM_ID_ALARMS_PEND:
            res=septic_data.alarm_pend_fgs;
            break;
        case SEPTIC_PARAM_ID_ALARMS_NACK:
            res=septic_data.alarm_nack_fgs;
            break;
        default:
            res=-1;
            break;
    }
    return res;
}
int8_t SepticSetParameter(Septic_param_id id, int8_t val){
    int8_t res;
    res=val;
    switch(id){
        case SEPTIC_PARAM_ID_ALARMS_NACK:
            septic_data.alarm_nack_fgs=*(uint8_t*)&val;
            break;
        default:
            res=-1;
            break;
    }
    return res;    
}
