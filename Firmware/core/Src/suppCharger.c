#include "suppCharger.h"

StackType_t Task_SuppCharger_Stack_Array[ TASK_SUPP_CHARGING_STACK_SIZE ];
StaticTask_t Task_SuppCharger_Buffer;

// User's BQ Handle
BQ_HandleTypeDef bq_handle;

// I2C Handle
I2C_HandleTypeDef hi2c;

uint16_t supp_vbat_mv = 0;

static bool supp_vbat_valid = false;

#define MAX_BQ25756E_DELAY_TICKS pdMS_TO_TICKS(100)

#define BQ25756E_PRINT_DELAY_TICKS pdMS_TO_TICKS(2000)

static void packSuppChargerStatus(supp_charger_status_t suppChargerMsg, uint8_t msgArr[8]){

    // Supp Charger Status is bits 0-3 of byte 0
    msgArr[0] = (suppChargerMsg.Supplemental_Charger_Status & 0x0F);

    // BQ25756E_Error is bits 4-6 of byte 0
    msgArr[0] |= ((suppChargerMsg.BQ25756E_Error << 4) & 0x70);

    // BQ25756E_Watchdog is bit 7 of byte 0
    msgArr[0] |= ((suppChargerMsg.BQ25756E_Watchdog << 7) & 0x80);

    // // Supplemental_Charge_Current is bits 8-23
    // msgArr[1] = (suppChargerMsg.Supplemental_Charge_Current >> 8) & 0xFF; // high byte
    // msgArr[2] = suppChargerMsg.Supplemental_Charge_Current & 0xFF; // low byte

    // // Supp_Charge_Current_Limit is bits 24-39
    // msgArr[3] = (suppChargerMsg.Supp_Charge_Current_Limit >> 8) & 0xFF; // high byte
    // msgArr[4] = suppChargerMsg.Supp_Charge_Current_Limit & 0xFF; // low byte

    msgArr[1] = suppChargerMsg.Supplemental_Charge_Current & 0xFF;         // low byte first
    msgArr[2] = (suppChargerMsg.Supplemental_Charge_Current >> 8) & 0xFF;  // high byte second

    msgArr[3] = suppChargerMsg.Supp_Charge_Current_Limit & 0xFF;         // low byte first
    msgArr[4] = (suppChargerMsg.Supp_Charge_Current_Limit >> 8) & 0xFF;  // high byte second

    // FrameID_Supp_Charger is bits 40-47
    msgArr[5] = suppChargerMsg.FrameID_Supp_Charger & 0xFF;
}

static void setSuppChargingStatus(supp_charger_status_t* suppChargerMsg, bq25756e_charge_status_t charge_state){
    if(suppChargerMsg == NULL){
        return;
    }
    supp_charger_status_supplemental_charger_status_e chargeStatus = SUPP_CHARGER_STATUS_SUPPLEMENTAL_CHARGER_STATUS_CHARGE_DISABLED;
    switch (charge_state){
        case BQ25756E_NOT_STARTED:
            chargeStatus = SUPP_CHARGER_STATUS_SUPPLEMENTAL_CHARGER_STATUS_CHARGE_DISABLED;
            break;
        case BQ25756E_TRICKLE:
            chargeStatus = SUPP_CHARGER_STATUS_SUPPLEMENTAL_CHARGER_STATUS_TRICKLE_CHARGING;
            break;
        case BQ25756E_PRE:
            chargeStatus = SUPP_CHARGER_STATUS_SUPPLEMENTAL_CHARGER_STATUS_PRECHARGE;
            break;
        case BQ25756E_FAST:
            chargeStatus = SUPP_CHARGER_STATUS_SUPPLEMENTAL_CHARGER_STATUS_FAST_CHARGING;
            break;
        case BQ25756E_TAPER:
            chargeStatus = SUPP_CHARGER_STATUS_SUPPLEMENTAL_CHARGER_STATUS_TAPER;
            break;
        case BQ25756E_TOP_OFF:
            chargeStatus = SUPP_CHARGER_STATUS_SUPPLEMENTAL_CHARGER_STATUS_TOPPING_OFF;
            break;
        case BQ25756E_DONE_CHRG:
            chargeStatus = SUPP_CHARGER_STATUS_SUPPLEMENTAL_CHARGER_STATUS_DONE;
            break;
        default:
            chargeStatus = SUPP_CHARGER_STATUS_SUPPLEMENTAL_CHARGER_STATUS_ERROR;
            break;  
    }
    suppChargerMsg->Supplemental_Charger_Status = chargeStatus;
}

uint8_t get_supp_vbat( bool *valid){
    if(valid != NULL){
        *valid = supp_vbat_valid;
    }

    return supp_vbat_mv;
}
void update_supp_vbat_valid(bool valid){
    portENTER_CRITICAL();
    supp_vbat_valid = valid;
    portEXIT_CRITICAL();
}

void suppCharger(){

    bq25756e_init(&bq_handle, &hi2c);

    bq25756e_charge_status_t charge_state = BQ25756E_NOT_STARTED;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    TickType_t xLastPrintTime = xTaskGetTickCount();

     // give the bq25756e chip time to startup
    vTaskDelay(pdMS_TO_TICKS(1000));

    int16_t charge_current;

    supp_charger_status_t suppChargerMsg = {0};
    uint8_t suppChargerMsgData[8] = {0};

    uint32_t chargeCurrentLimit_Ma = 0;

    uint8_t frameID = 0;



    while(1){


        // check if we're able to charge
        EventBits_t bitsSet = bq25756e_preReqBit_wait(BQ25756E_NUM_PREREQS, 0);

        if(bitsSet == BQ25756E_ALL_PREREQ_BITS){

            // supp vicor quiescent current is about 400mA
            chargeCurrentLimit_Ma = 600;
            bq25756e_charge(MAX_BQ25756E_DELAY_TICKS, chargeCurrentLimit_Ma);

        }
        else{

            chargeCurrentLimit_Ma = 0;
            bq25756e_charge_disable(MAX_BQ25756E_DELAY_TICKS);
        }

        // set the charge current of the supp charger
        suppChargerMsg.Supp_Charge_Current_Limit = chargeCurrentLimit_Ma;
        bq25756e_charge(MAX_BQ25756E_DELAY_TICKS, chargeCurrentLimit_Ma);

        // only print the status every BQ25756E_PRINT_DELAY ticks
        bq25756e_serial_config_t printEnabled = BQ25756E_SERIAL_DISABLE;
        if(xLastPrintTime + BQ25756E_PRINT_DELAY_TICKS <= xTaskGetTickCount()){
            xLastPrintTime = xTaskGetTickCount();
            printEnabled = BQ25756E_SERIAL_ENABLE;

            printf("Supp Charge Current Setpoint: %d mA\n\r", chargeCurrentLimit_Ma);
        }

        // read the charge status and pack it into the CAN message struct
        bq25756e_dump_status(&charge_state, printEnabled, MAX_BQ25756E_DELAY_TICKS); 
        setSuppChargingStatus(&suppChargerMsg, charge_state);

        // read the charge current
        bq25756e_dump_charge_current(&charge_current, printEnabled, MAX_BQ25756E_DELAY_TICKS); 
        suppChargerMsg.Supplemental_Charge_Current = charge_current;


        // read if there was a watchdog trip
        uint8_t watchdogOk = 0;
        bq25756e_dump_wdg(&watchdogOk, printEnabled, MAX_BQ25756E_DELAY_TICKS);
        suppChargerMsg.BQ25756E_Watchdog = watchdogOk == 1 ? SUPP_CHARGER_STATUS_BQ25756E_WATCHDOG_OK : SUPP_CHARGER_STATUS_BQ25756E_WATCHDOG_NOT_OK;

        // pet the bq25756e watchdog
        bq25756e_pet_wdg(MAX_BQ25756E_DELAY_TICKS);

        frameID = (frameID + 1) % 255;
        suppChargerMsg.FrameID_Supp_Charger = frameID;

        // pack the supp charger status CAN messaage into a byte array
        packSuppChargerStatus(suppChargerMsg, suppChargerMsgData);

        // send Supp Charger Status CAN message
        canbus_send(CAN_ID_SUPP_CHARGER_STATUS, CAN_DLC_SUPP_CHARGER_STATUS, suppChargerMsgData, MAX_BQ25756E_DELAY_TICKS);

        /*
            Due to hardware bugs in the voltage sense for supp batt, we're using the VBAT_ADC register of suppCharger to read the voltage
            SuppBatt is also powered by the HVDCDC output, so this method only works while HVDCDC is enabled.
            If the bq25756e is not able to read the VBAT_ADC register, we will fall back to the original voltage sense method of reading the voltage from the supp batt sense resistor.
        */
        if(bq25756e_dump_batt_voltage(&supp_vbat_mv, printEnabled, MAX_BQ25756E_DELAY_TICKS) == BQ25756E_OK){
            supp_vbat_valid = true;
        }
        else{
            supp_vbat_valid = false;
        }


        vTaskDelayUntil(&xLastWakeTime, SUPPLEMENTAL_CHARGER_THREAD_DELAY_TICKS);
    }
}