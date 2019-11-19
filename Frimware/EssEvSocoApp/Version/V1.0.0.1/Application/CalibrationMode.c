#include "common.h"

#include "stdint.h"

#include "stdbool.h"
#include "key.h"
#include "ActionTick.h"
#include "OnChipAudio.h"
#include "mixer.h"
#include "MotorSpeedHal.h"
#include "SystemConfig.h"
#include "mixer.h"


bool CalibrationCheck()
{
    uint32_t KeyDownTime = 0;
    for(int i = 0; i < 10; i++)
    {
        if(GET_KEY_MINUS == 0)
        {
            KeyDownTime++;
        }
        mDelay(2);
    }

    if(KeyDownTime == 10)
        return true;
    else
        return false;
}


extern uint32_t HalCntPerPeriod;
void CalibrationMode()
{
    uint8_t VolumeLevelBk;
    VolumeLevelBk = mixer.VolumeLevel;
    RealVolume = mixer.VolumeLevel * 8;
    AmplifierEnable();

    
    //���Ž���궨ģʽ������
    OnChipAudioPlay(10, 1, 1000);
    mDelay(1000);

    //�ȴ������ɿ�
    while(1)
    {
        uint32_t KeyDownTime = 0;
        for(int i = 0; i < 10; i++)
        {
            if(GET_KEY_MINUS == 0)
            {
                KeyDownTime++;
            }
            mDelay(2);
        }
        if(KeyDownTime == 0)
            break;
    }

    //�ȴ���������1s
    uint32_t LongPressTime = 0;
    while(1)
    {
        uint32_t KeyDownTime = 0;
        for(int i = 0; i < 10; i++)
        {
            if(GET_KEY_MINUS == 0)
            {
                KeyDownTime++;
            }
            mDelay(2);
        }
        if(KeyDownTime > 8)
        {
            LongPressTime += 20;
        }
        else
        {
            LongPressTime = 0;
        }

        if(LongPressTime >= 1000)
            break;
    }

    if(HalCntPerPeriod >= 10)
    {
        HalCalibrate100KmH = HalCntPerPeriod;
        CalHalMaxSpeedCalibrate();
        WriteSystemParamTable();
        OnChipAudioPlay(10, 1, 1000);
        mDelay(1000);
    }
    else
    {
        OnChipAudioPlay(1, 3, 50);
        mDelay(2000);
    }

    

    

    mixer.VolumeLevel = VolumeLevelBk;
    RealVolume = mixer.VolumeLevel * 8;

    //�ȴ������ɿ�
    while(1)
    {
        uint32_t KeyDownTime = 0;
        for(int i = 0; i < 10; i++)
        {
            if(GET_KEY_MINUS == 0)
            {
                KeyDownTime++;
            }
            mDelay(2);
        }
        if(KeyDownTime == 0)
            break;
    }
}



