#ifndef __MIXER__
#define __MIXER__

#include "SystemHw.h"
#include "fs.h"

typedef enum {CHANNEL_CLOSE = 0, CHANNEL_OPEN = 1} ChannnelStatusFlag;



#define AUDIO_DATA_READ_CNT			180//600//210//159//�ܱ�3����


#define AUDIO_RATE				22050//44100
#define SAMPLE_CNT_1MS			(AUDIO_RATE / 1000)
#define STANDARD_ARR_VALUE		(SYSTEM_CLK / AUDIO_RATE)


#define DAC_OUT_RATE            (22050)
#define DAC_OUT_ARR_VALUE		(SYSTEM_CLK / DAC_OUT_RATE)


#define PERCENT_MIXER_BASE				8192
#define PERCENT_MIXER_BASE_SHIFT		13

//������С����
#define MIN_VOLUME			0
#define MAX_VOLUME			8
#define DEFAULT_SOUND_LEVEL		2
				

/*
//֧�ֿ��л�����������
#define MAX_ENGINE_NUM					2
#define MAX_SOUND_NUM			MAX_ENGINE_NUM
*/

struct MixerChannelInfo
{
    struct FileInfo *Fp;
    s16 SoundData;

    
    s32 AmplitudeFactor;//��ֵΪ0-10000
    u16 OriginSpeed;
    u32 TotalDataCnt;//�����ļ��У����ݵĸ���
    u8 NowBuffFlag;
    u8 Buff0ReadyFlag;
    u8 Buff1ReadyFlag;
    u8 Buff2ReadyFlag;

    u8 buff0[AUDIO_DATA_READ_CNT];
    u8 buff1[AUDIO_DATA_READ_CNT];
    u8 buff2[AUDIO_DATA_READ_CNT];
    u16 Buff0DataCnt;
    u16 Buff1DataCnt;
    u16 Buff2DataCnt;
    u32 ReadFileCnt;
    u32 DataPtr;
    //u16 AudioStartSector;
    u32 AudioStartAddress;
    ChannnelStatusFlag ChannelStatus;
    u8 id;

    u8 PlayCnt;//����йѹ���������ţ�����ʱֵΪ0XFF����ʾһֱ���ţ������ʾ���ŵĴ���

    u32 CycleCnt;
};



struct MixerInfo
{
    FunctionalState MixerEnableFlag;
    unsigned char SoundIndex;//��ֵ��Ҫʱ����flash�е�ϵͳ���ñ��е�ֵ����һֱ
    FunctionalState AmplifierEnable;
    u8 VolumeLevel;
    u8 VolumeSlope;//����б��,��Χ0-10, 0:б��Ϊ0����ԭʼ��Դ����������10���������������PERCENT_STEP_BELOW_MIDDLE���
    u16 VolumeSlopePercent;//����VolumeSlope�͵�ǰ��RealSpeed�������Percent���������������������Ͻ�������
    bool Slinet;
};


extern struct MixerInfo mixer;
extern uint8_t RealVolume;


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





#endif


