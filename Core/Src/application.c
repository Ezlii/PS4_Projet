/*
 * application.c
 *
 *  Created on: Apr 24, 2025
 *      Author: elias
 */
#include  "application.h"
#include "main.h"
#include "display.h"
#include "vl53l1_api.h"
#include "vl53l1_platform.h"
#include "FSM_Handler.h"
#include "EventsManagement.h"



volatile int32_t lastEncoderValue = 0;
extern COM_InitTypeDef BspCOMInit;
extern SPI_HandleTypeDef hspi1;
extern I2C_HandleTypeDef hi2c3;

VL53L1_Dev_t dev;
VL53L1_DEV Dev = &dev;
VL53L1_DeviceInfo_t deviceInfo;
VL53L1_Error status;

static FSM_States_t myState = eTR_first;
static EventsBuffer_t myEventBuffer;
volatile int32_t encoderCount = 0;



void application(void){
	  Display_Initialize();
	  SH1106_Clear();

	  dev.i2c_slave_address = 0x29;

	  VL53L1_Error status;
	  VL53L1_RangingMeasurementData_t RangingData;

	  status = VL53L1_GetDeviceInfo(Dev, &deviceInfo);

	     if (status == VL53L1_ERROR_NONE) {
	    	 // connection success
	    	 printf("Connection success\n");
	     } else {
	    	// no connection
	    	 printf("No connection\n");
	     }

	     // Sensor initialisieren
	     status = VL53L1_DataInit(Dev);
	     if (status != VL53L1_ERROR_NONE) {
	         printf("DataInit fehlgeschlagen\n");
	     }

	     status = VL53L1_StaticInit(Dev);   // Initialisiert die Register des Sensors
	     if (status != VL53L1_ERROR_NONE) {
	         printf("StaticInit fehlgeschlagen\n");
	     }

	     VL53L1_SetDistanceMode(Dev, 2);  // 1 = Short, 2 = Long
	     VL53L1_SetMeasurementTimingBudgetMicroSeconds(Dev, 50000); // z.B. 50 ms


	     VL53L1_StartMeasurement(Dev);


	     /*
	     while (1)
		 {
			 // Hier gibt es KEIN CheckForDataReady – also direkt Daten abholen
			 status = VL53L1_GetRangingMeasurementData(Dev, &RangingData);

			 if (status == VL53L1_ERROR_NONE && RangingData.RangeStatus == 0) {
				 printf("Distanz: %u mm\n", RangingData.RangeMilliMeter);
			 } else {
				 printf("Ungültige Messung (Status: %u)\n", RangingData.RangeStatus);
			 }

			 // Messung abschließen und neue starten
			 VL53L1_ClearInterruptAndStartMeasurement(Dev);

			 HAL_Delay(50);  // Warten bis nächste Messung fertig ist
		 }


		*/



	  SH1106_WriteString(32, 0, "Epreuve 1", FONT_6x8);
	  SH1106_WriteString(0, 2, "Tension: 8.67 V", FONT_6x8);
	  SH1106_WriteString(0, 4, "Vitesse: 12.5 km/h", FONT_6x8);
	  //SH1106_WriteString(0, 6, "nbr. de levages: 3", FONT_6x8);


	  while(1){


		  // Event Producer
		  eventsManagement_Push(&myEventBuffer, eTimeTickElapsed_10ms);
		  char encoder_string[10];
		  snprintf(encoder_string, sizeof(encoder_string), "%3ld", encoderCount);
		  SH1106_WriteString(0, 6, encoder_string, FONT_6x8);
		  // Event Consumer
		  TrotinettControlTask(&myEventBuffer);
		  HAL_Delay(20);
	  }
}


static void handle_TR_first_EntryFct(void){

}

static void handle_first(FSM_States_t state, EventsTypes_t event){

}

static void handle_TR_second_EntryFct(void){

}

static void handle_second(FSM_States_t state, EventsTypes_t event){

}

static const FSM_State_Handler_t FSM_State_Handler[eNbrOfFSMStates] =
{
{eTR_first, handle_TR_first_EntryFct, handle_first, nullptr},
{eTR_second, handle_TR_second_EntryFct, handle_second, nullptr},
};

static void tran(FSM_States_t newState)
{
    if (nullptr != FSM_State_Handler[myState].pExitHandler)
    {
        FSM_State_Handler[myState].pExitHandler();
    }
    myState = newState;
    if (nullptr != FSM_State_Handler[myState].pEntryHandler)
    {
        FSM_State_Handler[myState].pEntryHandler();
    }
}

void TrotinettControlTask(EventsBuffer_t* pBuff){
	EventsTypes_t event;
	while(eventsManagement_Pop(pBuff, &event)){
		Trotinette_FSM(event);
	}
}


void Trotinette_FSM(EventsTypes_t event)
{
    //assert(myState<eNbrOfFSMStates);
    if (nullptr != FSM_State_Handler[myState].pRunHandler)
    {
        FSM_State_Handler[myState].pRunHandler(myState, event);
    }
}


FSM_States_t MotorControl_getActualState(void)
{
  return myState;
}


//
// Interrupts
//¨

// Callback-Funktion (von HAL automatisch aufgerufen)
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    static uint8_t last_state = 0;

    // Wir reagieren auf beide Pins: CLK oder DT
    if (GPIO_Pin == Rotary_Encoder_SCK_Pin || GPIO_Pin == Rotary_Encoder_DT_Pin)
    {
        // Aktuellen Zustand einlesen
        uint8_t clk = HAL_GPIO_ReadPin(Rotary_Encoder_SCK_GPIO_Port, Rotary_Encoder_SCK_Pin);
        uint8_t dt  = HAL_GPIO_ReadPin(Rotary_Encoder_DT_GPIO_Port,  Rotary_Encoder_DT_Pin);

        uint8_t new_state = (clk << 1) | dt;

        // Tabelle der gültigen Zustandsübergänge (Gray Code)
        int8_t encoder_table[4][4] = {
            {  0, -1,  1,  0 },
            {  1,  0,  0, -1 },
            { -1,  0,  0,  1 },
            {  0,  1, -1,  0 }
        };

        int8_t delta = encoder_table[last_state][new_state];
        encoderCount += delta;
        last_state = new_state;
    }
}
