/*
 * FSM_Handler.h
 *
 *  Created on: May 9, 2025
 *      Author: elias
 */

#ifndef INC_FSM_HANDLER_H_
#define INC_FSM_HANDLER_H_

#include "EventsTypes.h"

typedef enum{
	eTR_first,
	eTR_second,
	eNbrOfFSMStates
}FSM_States_t;

typedef void (*EntryFct)(void);
typedef void (*RunFct)(FSM_States_t state, EventsTypes_t event);
typedef void (*ExitFct)(void);

typedef struct
{
	FSM_States_t state;
    EntryFct pEntryHandler;
    RunFct pRunHandler;
    ExitFct pExitHandler;
} FSM_State_Handler_t;


void FSM_Init(void);

#endif /* INC_FSM_HANDLER_H_ */
