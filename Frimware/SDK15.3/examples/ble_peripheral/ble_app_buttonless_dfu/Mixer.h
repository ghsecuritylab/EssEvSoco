#ifndef __MIXER__
#define __MIXER__

#include "SystemHw.h"
#include "fs.h"
#include "Tas5719.h"
#include "AudioPlay.h"

typedef enum {CHANNEL_CLOSE = 0, CHANNEL_OPEN = 1} ChannnelStatusFlag;

#define TIMER_CLK               16000000


#define AUDIO_DATA_READ_CNT			180//600//210//159//�ܱ�3����

#define DEFAULT_SAMPLE_RATE         22050//Ĭ��״̬�µĲ�����
//#define AUDIO_RATE				44100//22050//44100
//#define STANDARD_ARR_VALUE		(TIMER_CLK / AUDIO_RATE)


//#define DAC_OUT_RATE            (22050)
//#define DAC_OUT_ARR_VALUE		(SYSTEM_CLK / DAC_OUT_RATE)


#define PERCENT_MIXER_BASE				8192
#define PERCENT_MIXER_BASE_SHIFT		13

//������С����
#define MIN_VOLUME			0
#define MAX_VOLUME			8
#define DEFAULT_SOUND_LEVEL		2


//���岥�Ż����¼Ӻ�Ķ���

				

/*
//֧�ֿ��л�����������
#define MAX_ENGINE_NUM					2
#define MAX_SOUND_NUM			MAX_ENGINE_NUM
*/

struct MixerChannelInfo
{
    struct FileInfo *Fp;
    int16_t SoundData;

    
    int32_t AmplitudeFactor;//��ֵΪ0-10000
    uint16_t OriginSpeed;
    uint32_t TotalDataCnt;//�����ļ��У����ݵĸ���
    uint8_t NowBuffFlag;
    uint8_t Buff0ReadyFlag;
    uint8_t Buff1ReadyFlag;
    uint8_t Buff2ReadyFlag;

    uint8_t buff0[AUDIO_DATA_READ_CNT];
    uint8_t buff1[AUDIO_DATA_READ_CNT];
    uint8_t buff2[AUDIO_DATA_READ_CNT];
    uint16_t Buff0DataCnt;
    uint16_t Buff1DataCnt;
    uint16_t Buff2DataCnt;
    uint32_t ReadFileCnt;
    uint32_t DataPtr;
    //uint16_t AudioStartSector;
    uint32_t AudioStartAddress;
    ChannnelStatusFlag ChannelStatus;
    uint8_t id;

    uint8_t PlayCnt;//����йѹ���������ţ�����ʱֵΪ0XFF����ʾһֱ���ţ������ʾ���ŵĴ���

    uint32_t CycleCnt;

    uint32_t TimerInterval;//��λns
};



struct MixerInfo
{
    FunctionalState MixerEnableFlag;
    unsigned char SoundIndex;//��ֵ��Ҫʱ����flash�е�ϵͳ���ñ��е�ֵ����һֱ
    FunctionalState AmplifierEnable;
    uint8_t VolumeLevel;
    uint8_t VolumeSlope;//����б��,��Χ0-10, 0:б��Ϊ0����ԭʼ��Դ����������10���������������PERCENT_STEP_BELOW_MIDDLE���
    uint16_t VolumeSlopePercent;//����VolumeSlope�͵�ǰ��RealSpeed�������Percent���������������������Ͻ�������
    bool Slinet;

    
};





extern struct MixerInfo mixer;


int SoundIndexSetBySystemConfig(unsigned char *SoundIndex);
void StopMixer(void);
void StartMixer(void);
void AudioFileReadHandler(struct MixerChannelInfo *ThisChannel);
void AudioOutTimerIntHandler(void);
void AudioChannelGetDataInTimeInterruptWithSecure(struct MixerChannelInfo *ThisChannel);
void AudioOutHwConfig(void);
//int TryChangeSoundIndex(unsigned char DistIndex);
void ChangeSoundIndex(void);

int VolumeSetBySystemConfig(unsigned char *VolumeLevel);
void AddVolume(void);
void MinusVolume(void);
void AmplifierEnable(void);
void AmplifierControlHandler(void);
void HighVoltageMixerHandler(void);
void MixerBleTopLevelHandler(void);
int32_t TryChangeVehileRollBack(unsigned char DistIndex, bool IsStartUp);
int32_t TryChangeVehileIndexLoop(unsigned char DistIndex, bool IsStartUp);
void VolumeHandler(void);
void CalRealVolume(void);
void AudioOutDac(void);
void MixerVarInit(void);






#endif


