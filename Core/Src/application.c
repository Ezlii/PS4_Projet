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


volatile int32_t lastEncoderValue = 0;
extern COM_InitTypeDef BspCOMInit;
extern SPI_HandleTypeDef hspi1;
extern TIM_HandleTypeDef htim2;
extern I2C_HandleTypeDef hi2c3;

VL53L1_Dev_t dev;
VL53L1_DEV Dev = &dev;
VL53L1_DeviceInfo_t deviceInfo;
VL53L1_Error status;


void application(void){
	  Display_Initialize();
	  SH1106_Clear();

	  dev.i2c_slave_address = 0x29;

	  VL53L1_Error status;
	  VL53L1_RangingMeasurementData_t RangingData;

	  status = VL53L1_GetDeviceInfo(Dev, &deviceInfo);

	     if (status == VL53L1_ERROR_NONE) {
	    	 // connection success
	     } else {
	    	// no connection
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


	     uint8_t dataReady = 0;
	     uint16_t distance_mm;

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



	  // Starten des Encoders (alle Kanäle)
	  if (HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL) != HAL_OK)
	  {
		  // Fehlerbehandlung, falls Start fehlschlägt
		  Error_Handler();
	  }

	  // Ersten Zählerstand sichern
	  lastEncoderValue = __HAL_TIM_GET_COUNTER(&htim2);



	  SH1106_WriteString(32, 0, "Epreuve 1", FONT_6x8);
	  SH1106_WriteString(0, 2, "Tension: 8.67 V", FONT_6x8);
	  SH1106_WriteString(0, 4, "Vitesse: 12.5 km/h", FONT_6x8);
	  SH1106_WriteString(0, 6, "nbr. de levages: 3", FONT_6x8);


	  while(1){
		  // Aktuellen Zählerstand abrufen
		  int32_t currentEncoderValue = __HAL_TIM_GET_COUNTER(&htim2);

		  // Prüfen, ob sich der Zählerstand geändert hat
		 if (currentEncoderValue != lastEncoderValue)
		 {
			 if (currentEncoderValue > lastEncoderValue)
			 {
				 // Encoder wurde nach rechts gedreht
				 SH1106_WriteString_AllAtOnce(0, 0, "rechts", FONT_6x8);
			 }
			 else
			 {
				 // Encoder wurde nach links gedreht
				 SH1106_WriteString_AllAtOnce(0, 0, "liks", FONT_6x8);
			 }
			 // Aktualisieren des letzten Zählerstandes
			 lastEncoderValue = currentEncoderValue;
		 }

		  if (GPIO_PIN_RESET == HAL_GPIO_ReadPin(SW_Rotary_Encoder_GPIO_Port, SW_Rotary_Encoder_Pin))
		  {
			  SH1106_WriteString_AllAtOnce(0, 0, "pushed", FONT_6x8);
		  }
		  HAL_Delay(10);
	  }
}
