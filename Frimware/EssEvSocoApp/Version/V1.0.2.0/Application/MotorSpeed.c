#include "common.h"
#include "MotorSpeed.h"
#include "Engine.h"
#include "ActionTick.h"


#include "MotorSpeedHal.h"





//������ﵽ����ٲ�����һ��ʱ����ٶȻỺ������
//���ֵ��ʾ����������ռ����ٵİٷֱ�
#define EV_HOLD_SPEED			80  
//���ٶȳ���HOLD�ٶȺ󣬱��ָ�ʱ�䣬Ȼ���ٶȻ����½���HOLD�ٶ�
#define EV_OVER_HOLD_TIME		2000 * TIMER_MULTI

//�ٶȴ�OverHold��HOLD�����У��ٶ�ÿ�½�0.1��ʱ��������λms
#define EV_SPEED_TO_HOLD_RATE    5 * TIMER_MULTI

//�ٶ�һ����ʱ��ǰ������������ٶȵİٷֱ�
#define EV_SPEED1_SPEED_PERCENT					64
#define EV_SPEED2_SPEED_PERCENT					80

//�綯�����ٶȱ��ʣ�HOLD״̬ʱ/���ת��ʱ
//#define EV_ACC_RATIO_NO_HOLD					10




#define EV_ACC_RATIO_SPEED1					10
#define EV_ACC_RATIO_SPEED2					9
#define EV_ACC_RATIO_SPEED3					8







#define EV_HOLD_BUTTON			GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_9)
//#define EV_GEAR_SWITCH1			GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8)
//#define EV_GEAR_SWITCH2			GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9)


                         
//ɲ�����ʱ�䣬�ڸ�ʱ���ڼ�����ű仯
#define BREAK_IN_AREA_TIME_GATE			500  //��λms

//�����λʱ�����ת�ٵİٷֱ�
//���λʱ��һ�㲻�ᵽ����������ת�٣���Ҫ�޶�ESS�ܹ���������ת��
#define MAX_SPEED_PERCENT_IN_MAX_GEAR				80  //��λ%




//�������ŵ�λ�ðٷֱ�
#define UP_GEAR_THROTTLE_PERCENT			95
#define DOWN_GEAR_THROTTLE_PERCENT		50




struct MotorSpeedInfo MotorSpeed = 
{
.EvAccRatio = PERCENT_BASE,
.GearBox.GearBoxEnableFlag = DISABLE,
};


struct TurnInfo Turn;



//#ifndef PROTOCAL_DD
#define ADC1_DR_Address    ((u32)0x4001244C) 
void EvMotorSpeedHwInit()
{
    //�������ƹܽ���������
    GPIO_InitTypeDef GPIO_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    
    DMA_DeInit(DMA1_Channel1);
    DMA_StructInit(&DMA_InitStructure);
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&Turn.TurnAdcValue;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = 1;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);
    DMA_Cmd(DMA1_Channel1, ENABLE);


    ADC_StructInit(&ADC_InitStructure);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
  
    /* ADC1 regular channel14 configuration */ 
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);
  
    /* Enable ADC1 DMA */
    ADC_DMACmd(ADC1, ENABLE);
    
    /* Enable ADC1 */
    ADC_Cmd(ADC1, ENABLE);
  
    /* Enable ADC1 reset calibaration register */   
    ADC_ResetCalibration(ADC1);
    /* Check the end of ADC1 reset calibration register */
    while(ADC_GetResetCalibrationStatus(ADC1));
  
    /* Start ADC1 calibaration */
    ADC_StartCalibration(ADC1);
    /* Check the end of ADC1 calibration */
    while(ADC_GetCalibrationStatus(ADC1));
       
    /* Start ADC1 Software Conversion */ 
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);




    
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

/*
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
*/
}


#define TURN_UP_RESISTANCE		3000//1000
//#define TURN_UP_RESISTANCE		1000

#define TURN_DOWN_RESISTANCE	1000
#define ADC_STANDARD_VOLTAGE			3300
uint16_t GetTurnVoltage()
{
    u32 Temp;
    u32 CurrentVoltage;
    static u32 LastVoltage = 0;
    //�����ѹֵ����ȷ��С�����3λ									  e
    Temp = ((u32)Turn.TurnAdcValue * (TURN_DOWN_RESISTANCE + TURN_UP_RESISTANCE)) / TURN_DOWN_RESISTANCE;
    CurrentVoltage = (Temp * ADC_STANDARD_VOLTAGE) >> 12;



    CurrentVoltage = (CurrentVoltage + LastVoltage * 15) / 16;
    LastVoltage = CurrentVoltage;
    return CurrentVoltage;
}




//EvSpeedLevelInfo EvSpeedLevel;
//EvSpeedLevelInfo New;
uint8_t P_Level;
void EvMotorSpeedHandler()
{
    static uint32_t LastHandleTime;
    //static 
    EvSpeedLevelInfo ThisEvSpeedLevel;

    if(GetSystemTime() < LastHandleTime + 500 * TIMER_MULTI)
        return;

    LastHandleTime = GetSystemTime();

    P_Level = EV_HOLD_BUTTON;

    if(EV_HOLD_BUTTON == Bit_SET)
    {
        //�Ƴ��˵�λ����
        ThisEvSpeedLevel = EV_SPEED_HOLD;
    }
    else
    {
           
        ThisEvSpeedLevel = EV_SPEED3;
    }
/*
    else if(EV_GEAR_SWITCH1 == Bit_SET && EV_GEAR_SWITCH2 == Bit_SET)
    {
        ThisEvSpeedLevel = EV_SPEED2;
    }
    else if(EV_GEAR_SWITCH2 == Bit_RESET)
    {
        ThisEvSpeedLevel = EV_SPEED3;
    }
    else
    {
        ThisEvSpeedLevel = EV_SPEED1;
    }
*/

    //if(ThisEvSpeedLevel == MotorSpeed.EvSpeedLevel)
    //    return;
    MotorSpeed.EvSpeedLevel = ThisEvSpeedLevel;

//   if(New == EvSpeedLevel)
//        return;
//   EvSpeedLevel = New;
//   ThisEvSpeedLevel = New;

   
    MotorSpeed.EvMaxSpeed = engine.ForwardHighSpeed;

    if(ThisEvSpeedLevel == EV_SPEED_HOLD)
    {
        MotorSpeed.EvAccRatio = (PERCENT_BASE * engine.EngineAccResponseP * 4) / 10 / 10000;
    }
    else
    {
        //HALʵʱת��ģʽ�£��������Ħ�г����أ���Ӧ̫��ᵼ������̨��״̬�����ʵ�������Ӧ
        MotorSpeed.EvAccRatio = (PERCENT_BASE * engine.EngineAccResponseP * 4) / 10 / 3 / 10000;
    }
    
    
    //�����ٶȵ�λ���趨��ǰ��λ������ٶȼ���Ӧ����
    /*if(ThisEvSpeedLevel == EV_SPEED_HOLD)
    {
        MotorSpeed.EvMaxSpeed = engine.ForwardHighSpeed;
        MotorSpeed.EvAccRatio = (PERCENT_BASE * engine.EngineAccResponseP * 4) / 10 / 10000;
    }
    else if(ThisEvSpeedLevel == EV_SPEED1)
    {
        MotorSpeed.EvMaxSpeed = (s32)engine.ForwardHighSpeed * EV_SPEED1_SPEED_PERCENT / 100;
        if(MotorSpeed.SpeedMode == SPEED_MODE_HAL)
        {
            MotorSpeed.EvAccRatio = (PERCENT_BASE * engine.EngineAccResponseD * 4) / 10 / 10000;
        }
        else
        {
            MotorSpeed.EvAccRatio = (PERCENT_BASE * (uint32_t)engine.EngineAccResponseD  * 4) / 10 / EV_ACC_RATIO_SPEED1 / 10000;
        }
    }
    else if(ThisEvSpeedLevel == EV_SPEED2)
    {
        MotorSpeed.EvMaxSpeed = (s32)engine.ForwardHighSpeed * EV_SPEED2_SPEED_PERCENT / 100;
        if(MotorSpeed.SpeedMode == SPEED_MODE_HAL)
        {
            MotorSpeed.EvAccRatio = (PERCENT_BASE * engine.EngineAccResponseD * 4) / 10 / 10000;
        }
        else
        {
            MotorSpeed.EvAccRatio = (PERCENT_BASE * (uint32_t)engine.EngineAccResponseD * 4) / 10 / EV_ACC_RATIO_SPEED2 / 10000;
        }
    }
    else
    {
        MotorSpeed.EvMaxSpeed = engine.ForwardHighSpeed;
        if(MotorSpeed.SpeedMode == SPEED_MODE_HAL)
        {
            MotorSpeed.EvAccRatio = (PERCENT_BASE * engine.EngineAccResponseD * 4) / 10 / 10000;
        }
        else
        {
            MotorSpeed.EvAccRatio = (PERCENT_BASE * (uint32_t)engine.EngineAccResponseD * 4) / 10 / EV_ACC_RATIO_SPEED3 / 10000;
        }
    }*/
    //��������ٶȽ���HOLD�ٶȵļ���
    MotorSpeed.EvHoldSpeed = engine.ForwardLowSpeed + ((s32)((s32)MotorSpeed.EvMaxSpeed - engine.ForwardLowSpeed) * EV_HOLD_SPEED) / 100; 
}

//#endif




//#ifndef PROTOCAL_DD
/******************����Ŀ���ٶȼ���ģ��*************************/
s16 GestEvDistSpeed(s16 DistSpeedRaw)
{
    static bool SpeedOverHoldFlag = FALSE;//֮ǰ�ٶ��Ƿ񳬹�AFTER_MAX_SPEED_ENGINE_DOWN_PERCENT�ı�־
    static uint32_t OverHoldTime = 0;
    static uint32_t LastHoldSpeedFlushTime;//��HOLD�ٶȸ�Ԥ�����У��ٶȸ��µ�ʱ��
    static s16 LastHoldSpeed;//�ٶ���HOLD�����ڳ���ָ��ʱ��󣬶��ٶȽ��и�Ԥʱ�ĵ�ǰ�ٶ�
    
    s16 DistSpeed;

    if(SpeedOverHoldFlag == FALSE)
    {
        //֮ǰ�ٶ���HOLD֮��
        if(DistSpeedRaw >= MotorSpeed.EvHoldSpeed && MotorSpeed.RealSpeed >= DistSpeedRaw)
        {
            //�ٶȳ���HOLD����¼����HOLD�����ʱ��
            OverHoldTime = GetSystemTime();
            SpeedOverHoldFlag = TRUE;
        }
        DistSpeed = DistSpeedRaw;
    }
    else
    {
        if(DistSpeedRaw < MotorSpeed.EvHoldSpeed)
        {
            //�ٶȻ��䵽HOLD���£��������ٶȸ�Ԥ
            DistSpeed = DistSpeedRaw;
            SpeedOverHoldFlag = FALSE;
        }
        else
        {
            //�ٶ���HOLD������
            if(GetSystemTime() < OverHoldTime + EV_OVER_HOLD_TIME)
            {
                //�ոս���HOLD���򣬶����ٶȲ������������
                DistSpeed = DistSpeedRaw;
                //��¼��ǰ���ٶȣ����ں����ٶȸ�Ԥ�ļ���
                LastHoldSpeed = DistSpeedRaw;
                LastHoldSpeedFlushTime = GetSystemTime();
            }
            else
            {
                //����HOLD��ʱ�䳬��EV_OVER_HOLD_TIME�������ٶȸ�Ԥ
                if(GetSystemTime() < LastHoldSpeedFlushTime + EV_SPEED_TO_HOLD_RATE)
                {
                    //��δ����һ�����ݸ���ʱ��
                    DistSpeed = LastHoldSpeed;
                }
                else
                {
                    //�������ݸ�Ԥ��ʱ�䣬����һ���ٶȸ�Ԥ
                    DistSpeed = LastHoldSpeed - ((MotorSpeed.EvMaxSpeed - MotorSpeed.EvHoldSpeed) / (100 - EV_HOLD_SPEED)) / 10;
                    //����Ѿ��½�����HOLD�ٶȣ���������HOLD�ٶ�
                    if(DistSpeed < MotorSpeed.EvHoldSpeed)
                        DistSpeed = MotorSpeed.EvHoldSpeed;
                    LastHoldSpeed = DistSpeed;
                    LastHoldSpeedFlushTime = GetSystemTime();
                }
            }
        }        
    }
    return DistSpeed;
}
//#endif


int16_t GetDistSpeedWithoutGearBox(uint16_t TurnVoltage)
{
    int32_t speed;
    int32_t percent;
    if(TurnVoltage < VOLTAGE_TURN_IDLE)
    {
        speed = engine.ForwardLowSpeed;
    }
    else
    {
        percent = TurnVoltage - VOLTAGE_TURN_IDLE;
        percent = (percent * 10000) / (VOLTAGE_TURN_MAX - VOLTAGE_TURN_IDLE);
        if(percent > 10000)
            percent  = 10000;
        speed = engine.ForwardLowSpeed + (MotorSpeed.EvMaxSpeed - engine.ForwardLowSpeed) * percent / 10000;
    }

/*
    //��P���£��������ٻ���
    if(MotorSpeed.EvSpeedLevel != EV_SPEED_HOLD)
    {
        //��D������������
        speed = GestEvDistSpeed(speed);
    }
*/
    
    return (int16_t)speed;
}
	

//Ŀ��:�õ�Ŀ���ٶȺ���������λ��
//��������������������
int16_t GetDistSpeedWithGearBox(uint16_t TurnVoltage)
{
    int32_t speed;
    int32_t percent;
    if(TurnVoltage < MotorSpeed.GearBox.DownGearThrottlePosition)
    {
        //���Ŵ��ڼ�������
        if(TurnVoltage < VOLTAGE_TURN_IDLE)
        {
            speed = engine.ForwardLowSpeed;
        }
        else
        {
            percent = TurnVoltage - VOLTAGE_TURN_IDLE;
            percent = (percent * 10000) / (MotorSpeed.GearBox.UpGearThrottlePosition - VOLTAGE_TURN_IDLE);
            if(percent > 10000)
                percent  = 10000;
            speed = engine.ForwardLowSpeed + (MotorSpeed.GearBox.ChangeGearSpeed - engine.ForwardLowSpeed) * percent / 10000;
        }
        MotorSpeed.GearBox.ThrottlePosition = DOWN_GEAR_THROTTLE_POSITION;
    }
    else if(TurnVoltage < MotorSpeed.GearBox.UpGearThrottlePosition)
    {
        //�м����򣬲�����Ҳ������
        percent = TurnVoltage - VOLTAGE_TURN_IDLE;
        percent = (percent * 10000) / (MotorSpeed.GearBox.UpGearThrottlePosition - VOLTAGE_TURN_IDLE);
        if(percent > 10000)
            percent  = 10000;
        speed = engine.ForwardLowSpeed + (MotorSpeed.GearBox.ChangeGearSpeed - engine.ForwardLowSpeed) * percent / 10000;
        MotorSpeed.GearBox.ThrottlePosition = HOLD_GEAR_THROTTLE_POSITION;
    }
    else
    {
        //��������������
        if(MotorSpeed.GearBox.GearBoxStatus == GEAR_UPING)
        {
            //�����������ٹ����У����ٶ��趨Ϊ�����,Ϊ�����������ٹ��̵�ƽ��
            speed = engine.ForwardLowSpeed;	
        }
        else
        {
            if(MotorSpeed.GearBox.GeraLevel == MotorSpeed.GearBox.MaxGearLevel)
            {
                /*//����������ߵ�λ��������ָʾʵ��Ŀ���ٶ�
                //Ŀ���ٶȵ���������������ֵͨ���ݱ����
                PwmDifference = (s32)((s32)MotorSpeed.HighCycle - MotorSpeed.GearBox.ChangeGearThrottlePosition);
                PwmDifference = PwmDifference * 10000 / (SystemConfig.ForwardHighPwm - MotorSpeed.GearBox.ChangeGearThrottlePosition);
                //�������ٶ�Ϊ��������ٶȵ������
                Speed = MotorSpeed.GearBox.AfterUpGearSpeed + (SystemConfig.ForwardHighSpeed - MotorSpeed.GearBox.AfterUpGearSpeed) * PwmDifference / 10000;*/
                percent = TurnVoltage - VOLTAGE_TURN_IDLE;
                percent = (percent * 10000) / (VOLTAGE_TURN_MAX - VOLTAGE_TURN_IDLE);
                if(percent > 10000)
                    percent  = 10000;
                speed = engine.ForwardLowSpeed + (engine.ForwardHighSpeed * MAX_SPEED_PERCENT_IN_MAX_GEAR / 100 - engine.ForwardLowSpeed) * percent / 10000;
                
            }
            else
            {
                //δ������ߵ���������ָʾĿ���ٶ�Ϊ����ٶ�
                //���ٶ��趨Ϊ����٣���������
                speed = engine.ForwardHighSpeed;
            }
            
        }
        MotorSpeed.GearBox.ThrottlePosition = UP_GEAR_THROTTLE_POSITION;
    }
    return (int16_t)speed;
}






/*************************ʵ���ٶȼ���*******************************/
//�����������������λ��ֵ
//#define GEAR_RATIO_BASE_SHIFT			13
//#define GEAR_RATIO_BASE					(1 << GEAR_RATIO_BASE_SHIFT)
//pֵ�ĵ�������������������
#define KP_FACTOR					1000		
#define THORTTLE_MIN_FACTOR			100
#define THORTTLE_MAX_FACTOR		800
#define KP_SPEED_DOWN_PERCENT		1
#define KP_UPGEAR_DOWN_PERCENT	2
uint16_t Acc;
s16 DoAccelerateWithMotorCurve(s16 NowSpeed, s16 Error, u32 GearRatio)
{
    s16 DistError;
    int i;
    s32 Accelerate;
    //u16 AccelerateLow;
    //u16 AccelerateHigh;
    s32 percent;
    s32 temp;
    s32 ErrorFactor;

    

    if(NowSpeed >= 0)
    {
    
        if(Error > 0)
        {
            //�ó��ٶ����ĸ����䷶Χ��
            for(i = 0; i < engine.ForwardAudioFileNum; i++)
            {
                if(NowSpeed <= engine.ForwardSpeedArray[i])
                {
                    break;
                }
            }
    
            //�����ٶȼ��㵱�µļ�������
            if(i == 0)
            {
                //���������ٶ�С����С�ٶȣ���ֱ�Ӳ��õ���ʱ������
                Accelerate = (s32)engine.AccelerateArrayOri[0] * GearRatio;
                Accelerate = Accelerate >> PERCENT_BASE_SHIFT;
            }
            else if(i >= engine.ForwardAudioFileNum)
            {
                //���������ٶȴ��ڷ���������ٶȣ���ֱ�Ӳ��÷���������ٶ�ʱ������
                Accelerate = (s32)engine.AccelerateArrayOri[engine.ForwardAudioFileNum - 1] * GearRatio;
                Accelerate = Accelerate >> PERCENT_BASE_SHIFT;
            }
            else
            {
                //�м�׶Σ�ͨ���ٷֱ����������ص�ֵ
                if(engine.AccelerateArrayOri[i] == engine.AccelerateArrayOri[i - 1])
                {
                    //�����ڵ��������ٶ�������ͬ�����������
                    Accelerate = (s32)engine.AccelerateArrayOri[i] * GearRatio;
                    Accelerate = Accelerate >> PERCENT_BASE_SHIFT;
                }
                else
                {
                    //���ݰٷֱȼ��㵱�µļ��ٶ�����
                    percent = (s32)((s32)(NowSpeed - engine.ForwardSpeedArray[i - 1]) * PERCENT_BASE);
                    percent = percent / (engine.ForwardSpeedArray[i] - engine.ForwardSpeedArray[i - 1]);
                    if(engine.AccelerateArrayOri[i] > engine.AccelerateArrayOri[i - 1])
                    {
                        Accelerate = (s32)((s32)(engine.AccelerateArrayOri[i] - engine.AccelerateArrayOri[i - 1]) * percent);
                        Accelerate = (Accelerate >> PERCENT_BASE_SHIFT) + engine.AccelerateArrayOri[i - 1];
                    }
                    else
                    {
                        Accelerate = (s32)((s32)(engine.AccelerateArrayOri[i - 1] - engine.AccelerateArrayOri[i]) * (PERCENT_BASE - percent)); 
                        Accelerate = (Accelerate >> PERCENT_BASE_SHIFT) + engine.AccelerateArrayOri[i];                       
                    }
                    Accelerate = Accelerate * GearRatio;
                    Accelerate = Accelerate >> PERCENT_BASE_SHIFT;
                }
    
            }
    
            //���ݼ��ٶȵ����غ��ٶȲ�ֵ(���Ŵ�С)���������ӵ��ٶ�
            percent = (s32)(Error << PERCENT_BASE_SHIFT) / engine.ForwardSpeedArray[engine.ForwardAudioFileNum - 1];
            ErrorFactor = THORTTLE_MIN_FACTOR + (((THORTTLE_MAX_FACTOR - THORTTLE_MIN_FACTOR) * percent) >> PERCENT_BASE_SHIFT);
            temp = ErrorFactor *  Accelerate;
            DistError = temp / KP_FACTOR;

            Acc = Accelerate;
        }
        else
        {
            if(Error < -3000)
                Error = -3000;
            if(MotorSpeed.GearBox.GearBoxStatus == NORMAL_DRIVE || MotorSpeed.GearBox.GearBoxStatus == GEAR_DOWN)
            {
                DistError = (s16)((Error * KP_SPEED_DOWN_PERCENT / 100));// >> REAL_SPEED_CAL_MULT_SHIFT);
            }
            else
            {
                DistError = (s16)((Error * KP_UPGEAR_DOWN_PERCENT / 100));// >> REAL_SPEED_CAL_MULT_SHIFT);
            }
        }

        
    }
    else
    {
        if(Error < 0)
        {
            NowSpeed = 0 - NowSpeed;
           //�ó��ٶ����ĸ����䷶Χ��
            for(i = 0; i < engine.BackwardAudioFileNum; i++)
            {
                if(NowSpeed <= engine.BackwardSpeedArray[i])
                {
                    break;
                }
            }
    
            //�����ٶȼ��㵱�µļ�������
            if(i == 0)
            {
                //���������ٶ�С����С�ٶȣ���ֱ�Ӳ��õ���ʱ������
                Accelerate = engine.AccelerateArrayOri[0] * GearRatio;
                Accelerate = Accelerate >> PERCENT_BASE_SHIFT;
            }
            else if(i >= engine.BackwardAudioFileNum)
            {
                //���������ٶȴ��ڷ���������ٶȣ���ֱ�Ӳ��÷���������ٶ�ʱ������
                Accelerate = engine.AccelerateArrayOri[engine.BackwardAudioFileNum - 1] * GearRatio;
                Accelerate = Accelerate >> PERCENT_BASE_SHIFT;
            }
            else
            {
                //�м�׶Σ�ͨ���ٷֱ����������ص�ֵ
                if(engine.AccelerateArrayOri[i] == engine.AccelerateArrayOri[i - 1])
                {
                    //�����ڵ��������ٶ�������ͬ�����������
                    Accelerate = engine.AccelerateArrayOri[i] * GearRatio;
                    Accelerate = Accelerate >> PERCENT_BASE_SHIFT;
                }
                else
                {
                    //���ݰٷֱȼ��㵱�µļ��ٶ�����
                    percent = (s32)((s32)(engine.BackwardSpeedArray[i - 1] - NowSpeed) * PERCENT_BASE);
                    percent = percent / (engine.BackwardSpeedArray[i - 1] - engine.BackwardSpeedArray[i]);
                    if(engine.AccelerateArrayOri[i] > engine.AccelerateArrayOri[i - 1])
                    {
                        Accelerate = (s32)((s32)(engine.AccelerateArrayOri[i] - engine.AccelerateArrayOri[i - 1]) * percent);
                        Accelerate = (Accelerate >> PERCENT_BASE_SHIFT) + engine.AccelerateArrayOri[i - 1];
                    }
                    else
                    {
                        Accelerate = (s32)((s32)(engine.AccelerateArrayOri[i - 1] - engine.AccelerateArrayOri[i]) * (PERCENT_BASE - percent)); 
                        Accelerate = (Accelerate >> PERCENT_BASE_SHIFT) + engine.AccelerateArrayOri[i];
                    }
                    Accelerate = Accelerate * GearRatio;
                    Accelerate = Accelerate >> PERCENT_BASE_SHIFT;
                }
    
            }
    
            //���ݼ��ٶȵ����غ��ٶȲ�ֵ(���Ŵ�С)���������ӵ��ٶ�
            percent = (s32)(Error << PERCENT_BASE_SHIFT) / engine.BackwardSpeedArray[engine.BackwardAudioFileNum - 1];
            ErrorFactor = (0 - THORTTLE_MIN_FACTOR) + (((THORTTLE_MAX_FACTOR - THORTTLE_MIN_FACTOR) * percent) >> PERCENT_BASE_SHIFT);
            temp = ErrorFactor *  Accelerate;
            DistError = temp / KP_FACTOR;
        }
        else
        {
            if(Error < -3000)
                Error = -3000;
            if(MotorSpeed.GearBox.GearBoxStatus == NORMAL_DRIVE || MotorSpeed.GearBox.GearBoxStatus == GEAR_DOWN)
            {
                DistError = (s16)((Error * KP_SPEED_DOWN_PERCENT / 100));// >> REAL_SPEED_CAL_MULT_SHIFT);
            }
            else
            {
                DistError = (s16)((Error * KP_UPGEAR_DOWN_PERCENT / 100));// >> REAL_SPEED_CAL_MULT_SHIFT);
            }
            
        }
    }
    return DistError;
}

//ͨ��ǿ�ƿ���RealSpeed�������Ƿ񲥷Ź�������
s16 EngineOverloadHandler(s16 RealSpeed)
{
    static u32 InOverloadSpeedTime = 0;

    if(RealSpeed > engine.ForwardSpeedArray[engine.ForwardAudioFileNum - 2])
    {
        //��������ж�
        if(InOverloadSpeedTime == 0)
        {
            //�ٶȿ�ʼ�����������ٶȣ���¼��ʱ���
            InOverloadSpeedTime = GetSystemTime();
            RealSpeed = engine.ForwardSpeedArray[engine.ForwardAudioFileNum - 2];
        }
        else
        {
            //���ų����ڹ�������
            if(GetSystemTime() - InOverloadSpeedTime >= engine.OverloadDelayTime * TIMER_MULTI)
            {
                //���Բ��Ź��ص��������ٶȽ��뵽��������
            }
            else
            {
                RealSpeed = engine.ForwardSpeedArray[engine.ForwardAudioFileNum - 2];
            }
        }
    }
    else
    {
        //�˳����ؼ�¼���ж�
        InOverloadSpeedTime = 0;
        //OverloadPlayFlag = DISABLE;
    }

    return RealSpeed;
    
}

s16 RealSpeedOtherFilter(s16 RealSpeed)
{
    //���з�����������������ش���
    if(engine.OverloadExistFlag == ENABLE && engine.OverloadEnableFlag == ENABLE)
    {
        RealSpeed = EngineOverloadHandler(RealSpeed);
    }

    //����������ǵ���ģ���ʵ���ٶ�С��0ʱ���������ٶ�0
    if(RealSpeed < 0)
    {
        RealSpeed = 0;
    }


    //����ɲ������ش���
    if(MotorSpeed.VehicleStatu != BREAK)
    {
        if(RealSpeed >= 0)
        {
            //�������򣬾���Ϊǰ��
            MotorSpeed.VehicleStatu = GO_FORWARD;
        }
        else
        {
            MotorSpeed.VehicleStatu = GO_BACKWARD;
        }
    }
    else
    {
    }

    return RealSpeed;
}



s16 GetMotorRealSpeedWithoutGearbox(s16 DistSpeed, s16 RealSpeed)
{
    s16 Error;
    s16 Adjust;
    
    //static s16 RealSpeed = 0;
    /*if(MotorSpeed.RealSpeedChangeFlag == 1)
    {
        MotorSpeed.RealSpeedChangeFlag = 0;
        RealSpeed = MotorSpeed.RealSpeed;
    }*/
    
    //����õ�������ʵ���ٶ�
    Error = DistSpeed - RealSpeed;
    //��ƫ����е���
    if(Error)
    {
        //Adjust = DoAccelerateWithMotorCurve(RealSpeed, Error, PERCENT_BASE);
        Adjust = DoAccelerateWithMotorCurve(RealSpeed, Error, (u32)(MotorSpeed.EvAccRatio >> REAL_SPEED_CAL_MULT_SHIFT));
        if(Adjust)
        {
            RealSpeed = Adjust + RealSpeed;
        }
        else
        {
            if(Error > 0 )
                RealSpeed = RealSpeed + 1;
            else
                RealSpeed = RealSpeed - 1;
        }
    }

    
    RealSpeed = RealSpeedOtherFilter(RealSpeed);
    
    return RealSpeed;
    
}


s16 GetMotorRealSpeedWithGearBox(s16 DistSpeed, s16 RealSpeed)
{
    s16 Error;
    s16 Adjust;
    uint16_t EngineAcc;
    
    //static s16 RealSpeed = 0;

    /*if(MotorSpeed.RealSpeedChangeFlag == 1)
    {
        //�������йأ����������������������øñ�ʶ�������±������е�RealSpeed�ı���
        MotorSpeed.RealSpeedChangeFlag = 0;
        RealSpeed = MotorSpeed.RealSpeed;
    }*/

    Error = DistSpeed - RealSpeed;
    //��ƫ����е���
    if(Error)
    {
        EngineAcc = ((uint32_t)MotorSpeed.GearBox.Ratio[MotorSpeed.GearBox.GeraLevel] * engine.EngineAccResponseP) / 10000;
        Adjust = DoAccelerateWithMotorCurve(RealSpeed, Error, EngineAcc);
        if(Adjust)
        {
            RealSpeed = Adjust + RealSpeed;
        }
        else
        {
            if(Error > 0 )
                RealSpeed = RealSpeed + 1;
            else
                RealSpeed = RealSpeed - 1;
        }
    }

    if(MotorSpeed.GearBox.GearBoxStatus == NORMAL_DRIVE)
    {
        //����ģʽ�£������������ŵı仯���仯
        if(MotorSpeed.GearBox.ThrottlePosition == UP_GEAR_THROTTLE_POSITION)
        {
            //����ӵ�
            MotorSpeed.GearBox.GearBoxStatus = GEAR_UP;
        }
        else if(MotorSpeed.GearBox.ThrottlePosition == DOWN_GEAR_THROTTLE_POSITION)
        {
            if(MotorSpeed.GearBox.GeraLevel > 1)
            {
                //һ�����ϣ����н���
                MotorSpeed.GearBox.GearBoxStatus = GEAR_DOWN;
                MotorSpeed.GearBox.InGearDownTime = GetSystemTime();
            }
        }
        else if(MotorSpeed.GearBox.ThrottlePosition == REVERSE_THROTTLE_POSITION)
        {
            //���Ž��뷴�����䣬��ֱ�Ӽ���1��
            MotorSpeed.GearBox.GeraLevel = 1;
        }
    }
    else if(MotorSpeed.GearBox.GearBoxStatus == GEAR_DOWN)
    {
        //�ڽ���״̬��
        if(MotorSpeed.GearBox.ThrottlePosition == REVERSE_THROTTLE_POSITION)
        {
            //���Ž��뷴�����䣬ֱ�Ӽ���1����������������ģʽ
            MotorSpeed.GearBox.GeraLevel = 1;
            MotorSpeed.GearBox.GearBoxStatus = NORMAL_DRIVE;
            
        }
        else if(MotorSpeed.GearBox.ThrottlePosition == UP_GEAR_THROTTLE_POSITION)
        {
            //��������
            MotorSpeed.GearBox.GearBoxStatus = GEAR_UP;
        }
        else if(MotorSpeed.GearBox.ThrottlePosition == HOLD_GEAR_THROTTLE_POSITION)
        {
            //��������ģʽ
            MotorSpeed.GearBox.GearBoxStatus = NORMAL_DRIVE;
        }
        else
        {
            if(GetSystemTime() - MotorSpeed.GearBox.InGearDownTime >= MotorSpeed.GearBox.DownGearDelayTime * TIMER_MULTI)
            {
                //������ʱ�ﵽ�����н�������������1�����������ģʽ
                MotorSpeed.GearBox.GeraLevel--;
                MotorSpeed.GearBox.InGearDownTime = GetSystemTime();
                if(MotorSpeed.GearBox.GeraLevel == 1)
                {
                    //��������1��������н���
                    MotorSpeed.GearBox.GearBoxStatus = NORMAL_DRIVE;
                }
            }
        }
    }
    else if(MotorSpeed.GearBox.GearBoxStatus == GEAR_UP)
    {
        //����״̬��
        if(MotorSpeed.GearBox.ThrottlePosition == REVERSE_THROTTLE_POSITION)
        {
            //���ŷ���ֱ�ӽ�Ϊ1��
            MotorSpeed.GearBox.GeraLevel = 1;
            MotorSpeed.GearBox.GearBoxStatus = NORMAL_DRIVE;
            
        }
        else if(MotorSpeed.GearBox.ThrottlePosition == DOWN_GEAR_THROTTLE_POSITION)
        {
            //��������͵��򽵵��������������ģʽ
            if(MotorSpeed.GearBox.GeraLevel > 1)
            {
                //һ�����ϣ����н���
                MotorSpeed.GearBox.GearBoxStatus = GEAR_DOWN;
                MotorSpeed.GearBox.InGearDownTime = GetSystemTime();
            }
            else
            {
                MotorSpeed.GearBox.GearBoxStatus = NORMAL_DRIVE;
            }
        }
        else if(MotorSpeed.GearBox.ThrottlePosition == HOLD_GEAR_THROTTLE_POSITION)
        {
            MotorSpeed.GearBox.GearBoxStatus = NORMAL_DRIVE;
        }
        else
        {
            if(RealSpeed >= MotorSpeed.GearBox.ChangeGearSpeed  && MotorSpeed.GearBox.GeraLevel < MotorSpeed.GearBox.MaxGearLevel)
            {
                //�ٶȳ��������ٶȣ���������ǰ�ĵȴ�
                
                MotorSpeed.GearBox.GearBoxStatus = GEAR_UP_WAITING;
                MotorSpeed.GearBox.InGearUpTime = GetSystemTime();
            }
        }
    }
    else if(MotorSpeed.GearBox.GearBoxStatus == GEAR_UPING)
    {
        if(MotorSpeed.GearBox.ThrottlePosition == REVERSE_THROTTLE_POSITION)
        {
            MotorSpeed.GearBox.GeraLevel = 1;
            MotorSpeed.GearBox.GearBoxStatus = NORMAL_DRIVE;
            
        }
        else if(MotorSpeed.GearBox.ThrottlePosition == DOWN_GEAR_THROTTLE_POSITION)
        {
            if(MotorSpeed.GearBox.GeraLevel > 1)
            {
                //һ�����ϣ����н���
                MotorSpeed.GearBox.GearBoxStatus = GEAR_DOWN;
                MotorSpeed.GearBox.InGearDownTime = GetSystemTime();
            }
            else
            {
                MotorSpeed.GearBox.GearBoxStatus = NORMAL_DRIVE;
            }
        }
        else if(MotorSpeed.GearBox.ThrottlePosition == HOLD_GEAR_THROTTLE_POSITION)
        {
            MotorSpeed.GearBox.GearBoxStatus = NORMAL_DRIVE;
        }
        else 
        {
            if(RealSpeed <= MotorSpeed.GearBox.AfterUpGearSpeed)
            {
                //���ű��֣��ٶȽ�����Ӧ�ݱ��µ�ת��

                    //δ����ߵ�����������
                    MotorSpeed.GearBox.GearBoxStatus =  GEAR_UP;
                    MotorSpeed.GearBox.GeraLevel++;
            }
        }
    }
    else if(MotorSpeed.GearBox.GearBoxStatus == GEAR_UP_WAITING)
    {
        if(MotorSpeed.GearBox.ThrottlePosition == REVERSE_THROTTLE_POSITION)
        {
            MotorSpeed.GearBox.GeraLevel = 1;
            MotorSpeed.GearBox.GearBoxStatus = NORMAL_DRIVE;
            
        }
        else if(MotorSpeed.GearBox.ThrottlePosition == DOWN_GEAR_THROTTLE_POSITION)
        {
            if(MotorSpeed.GearBox.GeraLevel > 1)
            {
                //һ�����ϣ����н���
                MotorSpeed.GearBox.GearBoxStatus = GEAR_DOWN;
                MotorSpeed.GearBox.InGearDownTime = GetSystemTime();
            }
            else
            {
                MotorSpeed.GearBox.GearBoxStatus = NORMAL_DRIVE;
            }
        }
        else if(MotorSpeed.GearBox.ThrottlePosition == HOLD_GEAR_THROTTLE_POSITION)
        {
            MotorSpeed.GearBox.GearBoxStatus = NORMAL_DRIVE;
        }
        else 
        {
            if(GetSystemTime() > MotorSpeed.GearBox.InGearUpTime + MotorSpeed.GearBox.UpGearDelayTime * TIMER_MULTI)
            {
                //����ǰ�ȴ�ʱ�䵽����������
                MotorSpeed.GearBox.GearBoxStatus =  GEAR_UPING;
            }
        }
    }
    
    //�����������У����ٶȴ��������ٶȣ���ǿ���������ٶȣ���ֹ���ֹ�������
    if(MotorSpeed.GearBox.GeraLevel != MotorSpeed.GearBox.MaxGearLevel)
    {
        if(RealSpeed > MotorSpeed.GearBox.ChangeGearSpeed)
        {
            RealSpeed = MotorSpeed.GearBox.ChangeGearSpeed;
        }
    }


    RealSpeed = RealSpeedOtherFilter(RealSpeed);

    return RealSpeed;
}


/************************������״̬��ʼ��**********************/
void GearBoxSpeedReset()
{
    //����ٶ���Ϊ�����ٶ�
    if(engine.OverloadEnableFlag == ENABLE && engine.OverloadExistFlag == ENABLE)
    {
        MotorSpeed.GearBox.ChangeGearSpeed = (s32)engine.ForwardSpeedArray[engine.ForwardAudioFileNum - 2] * MotorSpeed.GearBox.UpGearSpeedPercent / 100;
    }
    else
    {
        MotorSpeed.GearBox.ChangeGearSpeed = (s32)engine.ForwardSpeedArray[engine.ForwardAudioFileNum - 1] * MotorSpeed.GearBox.UpGearSpeedPercent / 100;
    }

    //�趨������ɺ󷢶���Ӧ�õ��ٶ�
    MotorSpeed.GearBox.AfterUpGearSpeed = (s32)MotorSpeed.GearBox.ChangeGearSpeed * MotorSpeed.GearBox.AfterUpGearSpeedPercent / 100;
    //����趨̫�ͣ��ȵ��ٵͣ����»����ﲻ�����������Ӷ��������޷�����
    if(MotorSpeed.GearBox.AfterUpGearSpeed < engine.ForwardLowSpeed)
    {
        MotorSpeed.GearBox.AfterUpGearSpeed = engine.ForwardLowSpeed;
    }
}


void GearBoxThrottlePositionReset()
{
/*
    if(RcCapture.ForwardHighPwm > RcCapture.MiddleThrottlePwm)
    {
       MotorSpeed.GearBox.UpGearThrottlePosition = (u32)(RcCapture.ForwardHighPwm - RcCapture.MiddleThrottlePwm) 
                                                                                     * UP_GEAR_THROTTLE_PERCENT / 100 + RcCapture.MiddleThrottlePwm;
       MotorSpeed.GearBox.DownGearThrottlePosition = (u32)(RcCapture.ForwardHighPwm - RcCapture.MiddleThrottlePwm) 
                                                                                     * DOWN_GEAR_THROTTLE_PERCENT / 100 + RcCapture.MiddleThrottlePwm;                                                                              
    }
    else
    {
        MotorSpeed.GearBox.UpGearThrottlePosition = (u32)(RcCapture.MiddleThrottlePwm - RcCapture.ForwardHighPwm) 
                                                                                     * (100 - UP_GEAR_THROTTLE_PERCENT) / 100 + RcCapture.ForwardHighPwm;
        MotorSpeed.GearBox.DownGearThrottlePosition = (u32)(RcCapture.MiddleThrottlePwm - RcCapture.ForwardHighPwm) 
                                                                                     * (100 - DOWN_GEAR_THROTTLE_PERCENT) / 100 + RcCapture.ForwardHighPwm;         
    }
*/
    MotorSpeed.GearBox.UpGearThrottlePosition = ((u32)(VOLTAGE_TURN_MAX - VOLTAGE_TURN_IDLE) * UP_GEAR_THROTTLE_PERCENT) / 100 + VOLTAGE_TURN_IDLE;
    MotorSpeed.GearBox.DownGearThrottlePosition = ((u32)(VOLTAGE_TURN_MAX - VOLTAGE_TURN_IDLE) * DOWN_GEAR_THROTTLE_PERCENT) / 100 + VOLTAGE_TURN_IDLE; 
}

/*
void GetGearBoxAccelerateArray()
{
    int i;
    int AccelerateArrayFactor = 40;
    for(i = 0; i < MAX_AUDIO_FILE_NUM_ONE_DIRECT; i++)
    {
           
        engine.GearBoxAccelerateArray[i] = engine.AccelerateArray[i] * AccelerateArrayFactor / 100;
    }
}
*/

void GearBoxInit()
{

    GearBoxSpeedReset();
    //��ȡ��λ����
    //MotorSpeed.GearBox.MaxGearLevel = DEFAULT_GEAR_LEVEL;
    MotorSpeed.GearBox.GeraLevel = 1;//��ʼʱ��λ��1��
    //�趨�ڶ�����Ϊ�����ٶ�
    
    

    //��ʼ��������ݱ�
    //
    MotorSpeed.GearBox.Ratio[0] = ((PERCENT_BASE * 200 * 4) / 10 / 100);
    MotorSpeed.GearBox.Ratio[1] = ((PERCENT_BASE * 150 * 4) / 10 / 100);
    MotorSpeed.GearBox.Ratio[2] = ((PERCENT_BASE * 50 * 4) / 10 / 100);
    MotorSpeed.GearBox.Ratio[3] = ((PERCENT_BASE * 25 * 4) / 10 / 100);
    MotorSpeed.GearBox.Ratio[4] = ((PERCENT_BASE * 12 * 4) / 10 / 100);
    MotorSpeed.GearBox.Ratio[5] = ((PERCENT_BASE * 8 * 4) / 10 / 100);
    MotorSpeed.GearBox.Ratio[6] = ((PERCENT_BASE * 6 * 4) / 10 / 100);

    //��������ڻ���������PWMֵ
    GearBoxThrottlePositionReset();

    MotorSpeed.GearBox.GearBoxStatus = NORMAL_DRIVE;
    //MotorSpeed.GearBox.UpGearDelayTime = 4000;
    //MotorSpeed.GearBox.DownGearDelayTime = 1000;
    MotorSpeed.GearBox.ThrottlePosition = DOWN_GEAR_THROTTLE_POSITION;

    //MotorSpeed.GearBox.UpGearKpDown = 2;

}


uint16_t CalDistSpeed()
{
    uint16_t DistSpeed;
    Turn.TurnVoltage = GetTurnVoltage();
    
    /*if(MotorSpeed.GearBox.GearBoxEnableFlag == DISABLE)
    {
         if(MotorSpeed.SpeedMode == SPEED_MODE_HAL)
         {

             if(MotorSpeed.EvSpeedLevel == EV_SPEED_HOLD)
             {
                 MotorSpeed.DistSpeedRaw = GetDistSpeedWithoutGearBox(Turn.TurnVoltage);
             }
             else
             {
                 MotorSpeed.DistSpeedRaw = GetDistSpeedHal();
             }

             //MotorSpeed.DistSpeedRaw = GetDistSpeedHal();
         }
         else
         {
             //SPEED_MODE_TURN
             MotorSpeed.DistSpeedRaw = GetDistSpeedWithoutGearBox(Turn.TurnVoltage);
         }
    }
    else
    {
       MotorSpeed.DistSpeedRaw = GetDistSpeedWithGearBox(Turn.TurnVoltage);
    }*/
    if(MotorSpeed.EvSpeedLevel == EV_SPEED_HOLD)
    {
        MotorSpeed.DistSpeedRaw = GetDistSpeedWithoutGearBox(Turn.TurnVoltage);
    }
    else
    {
        MotorSpeed.DistSpeedRaw = GetDistSpeedHal();
    }
    //MotorSpeed.DistSpeedRaw = GetDistSpeedHal();
    //ɲ�������Ŀ���ٶ�,����ɲ��״̬����ǿ��Ŀ���ٶ�Ϊ0
    DistSpeed = MotorSpeed.DistSpeedRaw;
    return DistSpeed;
}

bool IsTurnOverStart()
{
    return Turn.TurnVoltage > VOLTAGE_TURN_START? TRUE : FALSE;
}

bool IsTurnBelowStop()
{
    return Turn.TurnVoltage < VOLTAGE_TURN_STOP? TRUE : FALSE;
}






