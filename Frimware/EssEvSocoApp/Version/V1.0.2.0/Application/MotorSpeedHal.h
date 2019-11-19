#ifndef __MOTOR_SPEED_HAL__
#define __MOTOR_SPEED_HAL__


extern uint16_t MaxVehicleCalibrate;//��λkm/h
extern uint16_t HalCalibrate100KmH;
/*
extern uint16_t WheelDiameter;//��λcm
extern uint16_t MotorPoles;//����Լ���
extern uint32_t ReductionRatio;//�ݱ�*100extern uint16_t HalMaxSpeedCalibrate;
*/



void MotorSpeedHalHwInit(void);
void MotorSpeedHalIrqHandler(void);
void HalSpeedTimerIntHandler(void);
int16_t GetDistSpeedHal(void);
void InitThrottleCurve(void);
void CalHalMaxSpeedCalibrate(void);


#endif


