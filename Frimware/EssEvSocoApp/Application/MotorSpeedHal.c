#include "common.h"
#include <stdint.h>
#include "engine.h"
#include "SystemHw.h"
#include "MotorSpeed.h"

#define HAL_SPEED_TIMER			TIM3


uint32_t TotalHalCnt;
uint32_t HalCntPerPeriod;
EXTITrigger_TypeDef  TriggerState;

uint16_t HalCalibrate100KmH;//����100km/h�£�ÿ200ms��HAL������
uint16_t HalMaxSpeedCalibrate;
uint16_t MaxVehicleCalibrate;//��λkm/h
/*
uint16_t WheelDiameter;//��λcm
uint16_t MotorPoles;//����Լ���
uint32_t ReductionRatio;//���ٱ�*100
*/


void MotorSpeedHalHwInit()
{
    GPIO_InitTypeDef        GPIO_InitStructure;
    NVIC_InitTypeDef 	NVIC_InitStructure;
    EXTI_InitTypeDef        EXTI_InitStructure;

    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQChannel;//TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    EXTI_InitStructure.EXTI_Line = EXTI_Line1;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    TriggerState = EXTI_Trigger_Rising;
    

    EXTI_ClearITPendingBit(EXTI_Line1);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
 
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQChannel;//TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
 
    TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
    TIM_TimeBaseStructure.TIM_Period = 20000 - 1;   //44100       
    TIM_TimeBaseStructure.TIM_Prescaler = SYSTEM_CLK / 100000 - 1;       
    TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;    
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
    TIM_TimeBaseInit(HAL_SPEED_TIMER, &TIM_TimeBaseStructure);

    
    TIM_ClearITPendingBit(HAL_SPEED_TIMER, TIM_IT_Update);
    TIM_ITConfig(HAL_SPEED_TIMER, TIM_IT_Update, ENABLE);

    TIM_Cmd(HAL_SPEED_TIMER, ENABLE);

    
}


void MotorSpeedHalIrqHandler()
{
 //   uint32_t i;
//    EXTI_InitTypeDef        EXTI_InitStructure;
    /*if(TriggerState == EXTI_Trigger_Rising)
    {
        for(i = 0; i < 3; i++)
        {
            if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == Bit_RESET)
                goto exit;
        }

        EXTI_InitStructure.EXTI_Line = EXTI_Line8;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        EXTI_Init(&EXTI_InitStructure);
        TriggerState = EXTI_Trigger_Falling;
    }
    else
    {
        for(i = 0; i < 3; i++)
        {
            if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == Bit_SET)
                goto exit;
        }

        EXTI_InitStructure.EXTI_Line = EXTI_Line8;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        EXTI_Init(&EXTI_InitStructure);
        TriggerState = EXTI_Trigger_Rising;
        TotalHalCnt++;
    }*/

    TotalHalCnt++;
    
//exit:
    EXTI_ClearITPendingBit(EXTI_Line1);
}


void HalSpeedTimerIntHandler()
{
    static uint32_t LastHalCnt;
    uint32_t HalCntPerPeriodTmp;
    TIM_ClearITPendingBit(HAL_SPEED_TIMER, TIM_IT_Update);
    HalCntPerPeriodTmp = TotalHalCnt - LastHalCnt;
    LastHalCnt = TotalHalCnt;

    //HalCntPerPeriod = (HalCntPerPeriod * 3 + HalCntPerPeriodTmp) >> 2;
    HalCntPerPeriod = HalCntPerPeriodTmp;
    
    
}



//uint32_t CurveHallSpeed[2] = {40, 60};//��������б�ʱ仯����Ӧ�Ļ����ٶ�
//uint32_t DistSpeedPercent[2];//��������б�ʱ仯�����ٶȱ���

/*
void InitThrottleCurve()
{
    uint32_t percent1, percent2;
    uint32_t PercentInterval;

    percent1 = CurveHallSpeed[0] * 10000 / HalMaxSpeedCalibrate;
    percent2 = CurveHallSpeed[1] * 10000 / HalMaxSpeedCalibrate;
    PercentInterval = percent2 - percent1;

    DistSpeedPercent[0] = percent1 + 1000 * MotorSpeed.ThrottleCurveLevel;//(10000 - percent1) * MotorSpeed.ThrottleCurveLevel / 10;
    if(DistSpeedPercent[0] > 10000)
        DistSpeedPercent[0] = 10000;
        
    DistSpeedPercent[1] = DistSpeedPercent[0] + PercentInterval;
    if(DistSpeedPercent[1] > 10000)
        DistSpeedPercent[1] = 10000;
}
*/


/*
#define HAL_CNT_IDLE				1
//#define HAL_CNT_MAX				60
int16_t GetDistSpeedHal()
{
    int32_t speed;
    int32_t percent;
    int32_t SpeedPercent;
    if(HalCntPerPeriod < HAL_CNT_IDLE)
    {
        speed = engine.ForwardLowSpeed;
    }
    else
    {
        if(HalCntPerPeriod <= CurveHallSpeed[0])
        {
            percent = HalCntPerPeriod * 10000 / CurveHallSpeed[0];
            SpeedPercent = DistSpeedPercent[0] * percent / 10000;
            //speed = engine.ForwardLowSpeed + (engine.ForwardHighSpeed - engine.ForwardLowSpeed) * SpeedPercent / 10000;
        }
        else if(HalCntPerPeriod <= CurveHallSpeed[1])
        {
            percent = (HalCntPerPeriod - CurveHallSpeed[0]) * 10000 / (CurveHallSpeed[1] - CurveHallSpeed[0]);
            SpeedPercent = DistSpeedPercent[0] + (DistSpeedPercent[1] - DistSpeedPercent[0]) * percent / 10000;
            if(SpeedPercent > 10000)
                SpeedPercent = 10000;
            
        }
        else
        {
            if(HalMaxSpeedCalibrate == CurveHallSpeed[1])
            {
                SpeedPercent = 10000;
                
            }
            else
            {
                percent = (HalCntPerPeriod - CurveHallSpeed[1]) * 10000 / (HalMaxSpeedCalibrate - CurveHallSpeed[1]);
                SpeedPercent = DistSpeedPercent[1] + (10000 - DistSpeedPercent[1]) * percent / 10000;
                if(SpeedPercent > 10000)
                    SpeedPercent = 10000;
            }
            
        }
        speed = engine.ForwardLowSpeed + (engine.ForwardHighSpeed - engine.ForwardLowSpeed) * SpeedPercent / 10000;
    }
    
    //speed = GestEvDistSpeed(speed);
    return (int16_t)speed;
}
*/

#define CHAN_MAX_VALUE 10000

#define KMAX 100


int expou(int x, unsigned short int k)
{
	// k*x*x*x + (1-k)*x
	// 0 <= k <= 100

	unsigned int val = (x * x / CHAN_MAX_VALUE * x / CHAN_MAX_VALUE * k
		+ (KMAX - k) * x + KMAX / 2) / KMAX;

	return val;
}

int expo(int value, int k_Ratio)
{
	int  y;
	int  k;

	unsigned neg = value < 0;

	k = neg ? k_Ratio : k_Ratio;


	if (k == 0)
		return value;

	if (neg)
		value = -value; //absval

	if (k < 0)
	{
		y = CHAN_MAX_VALUE - expou(CHAN_MAX_VALUE - value, -k);
	}
	else
	{
		y = expou(value, k);
	}

	return neg ? -y : y;
}

void CalHalMaxSpeedCalibrate()
{
/*
    uint64_t temp;
    uint64_t VehicleSpeed;
    uint64_t perimeter;//�ܳ�
    temp = MotorPoles * 2;//�ż��Գ�2���ż���������ʾ�����еתת��һȦ����һ�������ź����ϴ������ź����ڸ���
    temp = temp * ReductionRatio;//���㳵�ֻ�е��תһȦ���źŸ���������ݱȺ�Ŵ�100����ToothRatioΪ�ݱ�* 100����һ��100���ڶԳ�ToothRatio�Ŵ��100�����ڶ���100�����ڷŴ���߾��ȡ�
    VehicleSpeed = MaxVehicleCalibrate * 100 * 10 / 36 ;//��KM/H��M/S���㣬����Ŵ�100����Ҳ�����ΪCM/S
    perimeter = (WheelDiameter * 314);//�����ܳ�����λm * 10000
    HalMaxSpeedCalibrate = temp * VehicleSpeed / perimeter / 5; //����5����Ϊ200ms��ȡһ�����ݣ�temp�Ŵ�100����VehicleSpeed�Ŵ�100����perimeter�Ŵ�10000�������õ���
*/

    //���ݰٹ��ﳵ����HAL�ı궨ֵ�ͱ궨������ٽ��м��㣬�ó���������ٶȶ�Ӧ��HAL�궨ֵ
    HalMaxSpeedCalibrate = (uint32_t)HalCalibrate100KmH * MaxVehicleCalibrate / 100;
}


int16_t GetDistSpeedHal()
{
    int32_t speed;
    int32_t ThPercent;
    int32_t SpeedPercent;
    ThPercent = (HalCntPerPeriod * 10000) / HalMaxSpeedCalibrate;
    if(ThPercent > 10000)
        ThPercent = 10000;
    SpeedPercent = expo(ThPercent, -MotorSpeed.ThrottleCurveLevel * 10);
    speed = engine.ForwardLowSpeed + (engine.ForwardHighSpeed - engine.ForwardLowSpeed) * SpeedPercent / 10000;
    return (int16_t)speed;
}

/*
int16_t GetDistSpeedHal()
{
    int32_t speed;
    int32_t percent;
    if(HalCntPerPeriod < HAL_CNT_IDLE)
    {
        speed = engine.ForwardLowSpeed;
    }
    else
    {
        percent = HalCntPerPeriod - HAL_CNT_IDLE;
        percent = (percent * 10000) / (HalMaxSpeedCalibrate - HAL_CNT_IDLE);
        if(percent > 10000)
            percent  = 10000;
        speed = engine.ForwardLowSpeed + (engine.ForwardHighSpeed - engine.ForwardLowSpeed) * percent / 10000;
    }
    
    //speed = GestEvDistSpeed(speed);
    return (int16_t)speed;
}
*/




