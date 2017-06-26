//******************************************************************************
/// \Filename: mytasks.h
/// \Author: Hoang Tran
/// \Date: 2017 
/// \Brief: Containing prototype for tasks
//******************************************************************************

#ifndef _MYTASKS_H_
#define _MYTASKS_H_

void vUart_Print(void *pvParameters);
void vRTC_Print(void *pvParameters);
void vCard_Read(void *vParameters);
void vProcess_Door_Succeed(void *vParameters);
void vProcess_Door_Fail(void *vParameters);

#endif /* _MYTASKS_H_*/
