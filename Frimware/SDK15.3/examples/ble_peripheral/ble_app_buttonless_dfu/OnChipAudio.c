#include "common.h"
#include "OnChipAudio.h"
#include "mixer.h"
#include "ActionTick.h"
#include "SystemInfo.h"

struct OnChipPlayInfo OnChipPlayer;


//Ƭ����������ʼ��ַ����48.5K����ʼ
#define OnChipAudioStartAddr				(OnChipAudioMenuAddr + 0x200)   //

#define AUDIO_DI			0

#define ON_CHIP_ONE_PLAY_CNT            5


//0δ�ڲ���
uint16_t CheckOnChipAudioIsPlaying()
{
    return OnChipPlayer.IntervalPlayCnt;
}

//Interva��λms
uint32_t OnChipAudioPlay(uint8_t OnePlayCnt, uint16_t IntervalPlayCnt, uint16_t Interval)
{
    if(CheckOnChipAudioIsPlaying() != 0)
        return 4;

    OnChipPlayer.addr = OnChipAudioMenuAddr + 512;//*(uint32_t *)(OnChipAudioMenuAddr);
    OnChipPlayer.DataCnt = *(uint32_t *)(OnChipAudioMenuAddr + 4);
    

    //���������ַ�Ƿ�С����С��ַ
    if(OnChipPlayer.addr < OnChipAudioStartAddr)
        return 1;
    //���������ֹ��ַ�Ƿ񳬳�Ƭ��������ַ��Χ
    if((OnChipPlayer.addr + OnChipPlayer.DataCnt) >= SYSTEM_INFO_ADDR)
        return 2;


    //�������ļ���С���м�飬����0xffff���޷�ʹ��DMA���в���
    if(OnChipPlayer.DataCnt > 0xffff)
        return 3;

    OnChipPlayer.OnePlayCnt = OnePlayCnt;
    OnChipPlayer.OnePlayCntBak = OnePlayCnt;
    OnChipPlayer.IntervalPlayCnt = IntervalPlayCnt;
    OnChipPlayer.OneDataCnt = ON_CHIP_ONE_PLAY_CNT;
    OnChipPlayer.PlayedAudioDataCnt = 0;
    OnChipPlayer.AudioData = *(uint16_t *)OnChipPlayer.addr + 0x8000;
    OnChipPlayer.PlayInterval = Interval * 10;

    return 0;
}

int32_t GetOnChipAudioData()
{
    static uint32_t NextPlayTime = 0;
    //��Ƭ���������ŵ����
    if(CheckOnChipAudioIsPlaying() == 0)
    {
        OnChipPlayer.AudioData = 0;
        return OnChipPlayer.AudioData;
    }

    if(GetSystemTime() < NextPlayTime)
    {
        //��δ������ʱ�䣬�򷵻أ������м���Ĳ��ŵε���
        OnChipPlayer.AudioData = 0;
        return OnChipPlayer.AudioData;
    }

    if(OnChipPlayer.OneDataCnt > 0)
    {
        OnChipPlayer.OneDataCnt--;
        return OnChipPlayer.AudioData;
    }
    else
    {
        OnChipPlayer.PlayedAudioDataCnt += 2;
        if(OnChipPlayer.PlayedAudioDataCnt >= OnChipPlayer.DataCnt)
        {
            OnChipPlayer.PlayedAudioDataCnt = 0;
            OnChipPlayer.OnePlayCnt--;
            if(OnChipPlayer.OnePlayCnt == 0)
            {
                //һ�β������
                OnChipPlayer.OnePlayCnt = OnChipPlayer.OnePlayCntBak;
                NextPlayTime = OnChipPlayer.PlayInterval + GetSystemTime();
                OnChipPlayer.IntervalPlayCnt--;
            }
            
        }
        
        OnChipPlayer.AudioData = *(uint16_t *)(OnChipPlayer.addr + OnChipPlayer.PlayedAudioDataCnt)
                                                  + 0x8000;
        OnChipPlayer.OneDataCnt = ON_CHIP_ONE_PLAY_CNT;    
        return OnChipPlayer.AudioData;
    }
}

int32_t GetCurrentOnChipAudioData()
{
    return OnChipPlayer.AudioData;
}


void PlaySystemStartBeep(unsigned char EngineIndex)
{
    OnChipAudioPlay(2, EngineIndex + 1, 100);
}
