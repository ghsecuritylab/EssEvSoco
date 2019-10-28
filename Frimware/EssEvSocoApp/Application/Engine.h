#ifndef __ENGINE__
#define __ENGINE__

#include <stdint.h>


typedef enum {ENGINE_STOP = 0, ENGINE_RUNNING = 1, 
                         ENGINE_STARTING = 2, ENGINE_STOPING = 3} EngineStatusFlag;

#define MIXER_CHANNEL0_TIMER			TIM6
#define MIXER_CHANNEL1_TIMER			TIM7

                         
#define MAX_AUDIO_FILE_NUM_ONE_DIRECT				20
#define BACKWARD_AUDIO_CNT							5

//����ͨ����
#define ENGINE_MIXER_CHANNEL_NUM		2

//һ������ͷ����ռ����
#define ONE_AUDIO_HEAD_SECTOR_CNT		4
//ÿ��������ͷ����Ϣƫ�������ܴ�С2048�ֽ�
#define FORWARD_AUDIO_CNT_OFFSET		20
#define BACKWARD_AUDIO_CNT_OFFSET	21
#define SPEED_TABLE_OFFSET		22
#define AUDIO_LENGTH_OFFSET	102
#define AUDIO_START_SECTOR_OFFSET		262
#define ACCELERATE_TABLE_OFFSET		354
#define OVERLOAD_FLAG_OFFSET			394
//#define OVERLOAD_DELAY_TIME_OFFSET	395
//#define AUDIO_DATA_SECRET_OFFSET		397
#define START_SOUND_LENGTH_OFFSET			342
#define START_SOUND_START_SECTOR_OFFSET		346
#define STOP_SOUND_LENGTH_OFFSET				348
#define STOP_SOUND_START_SECTOR_OFFSET		352
#define AUDIO_INFO_GEARBOX_OFFSET			396
#define ENGINE_DECELERATE_INTERVAL_OFFSET			428
//#define BREAK_SOUND_LENGTH_OFFSET			430
//#define BREAK_SOUND_START_SECTOR				434


struct EngineInfo
{
    EngineStatusFlag status;

    FunctionalState FlameOutSwitch;
    uint16_t FlameOutTime;//���ٶȴ��ڵ������䣬����MaxSlientTime��û�б仯����Ϩ��
    u32 InFlameOutTime;
    u32 StopTime;//����ر��ѽ��е�ʱ��

    
    u16 ThrottleUpResponse;//������Ӧ��ֵԽ����ӦԽ��,��λ100ns
    u16 ThrottleDownResponse;
    uint16_t ThrottleDownResponseOri;//����ֵ����EngineDecResponseһ�����õ�ThrottleDownResponse

    

    s16 SpeedStage;//��ʶ��ǰ�ٶ����ڵ��ٶȽ׶Σ��е���ʱ����ߵ��Ƕ��ٶ�����
    s16 UnMixSpeed;//�Ӹ�ֵ����׼�ٶ�֮������䣬����Ҫ���������ٶȽ׶�Ϊ0ʱ����ֵ��Ч
    u8 ChannelNum;//ͨ�������������������ж��Ƿ���Ҫ����
    u8 LowSpeedChannel;
    u8 HighSpeedChannel;

    FunctionalState OverloadExistFlag;
    FunctionalState OverloadEnableFlag;
    u16 OverloadDelayTime;

    //s16 AccelerateArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];
    s16 AccelerateArrayOri[MAX_AUDIO_FILE_NUM_ONE_DIRECT];
    u16 EngineAccResponseP;
    uint16_t EngineAccResponseD;
    uint16_t EngineDecResponse;


    s16 ForwardLowSpeed;//ǰ����Դ�ĵ���
    s16 ForwardHighSpeed;//ǰ����Դ�ĸ���
    s16 BackwardLowSpeed;//ǰ����Դ�ĵ���
    s16 BackwardHighSpeed;//ǰ����Դ�ĸ���

    s16 ForwardSpeedArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];
    uint32_t ForwardDataCntArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];//�������������ĸ���
    s16 BackwardSpeedArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];
    //u32 BackwardDataCntArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];//�������������ĸ���
    uint32_t ForwardAudioStartAddrArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];
    //u32 BackwardAudioStartAddrArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];
    //s16 GearBoxAccelerateArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];

    u8 ForwardAudioFileNum;
    u8 BackwardAudioFileNum;

    uint32_t StartAudioDataCnt;
    uint32_t StartAudioStartAddr;
    uint32_t StopAudioDataCnt;
    uint32_t StopAudioStartAddr;

    uint32_t MaxSpeedLimit;//���ݵ綯����λ��ͬ������ٶ�Ҳ�᲻ͬ
};

extern struct EngineInfo engine;
extern struct MixerChannelInfo MixerChannel[ENGINE_MIXER_CHANNEL_NUM];
extern struct MixerChannelInfo EngineStartStopVoiceChannel;


int EngineSetByMixerConfig(u32 *FlameOutTime, u16 *ThrottleDownResponse, u16 *ThrottleUpResponse, FunctionalState *OverloadExistFlag,
                                                            FunctionalState *OverloadEnableFlag, u16 *OverloadDelayTime, s16 *AccelerateArray, FunctionalState *EngineResponseEnable,
                                                            u16 *EngineResponse);
void MixerChannel0IntHandler(void);
void MixerChannel1IntHandler(void);
s32 GetEngineAudioData(void);
void EngineHandler(void);
void EngineMixerChannelInit(void);
int AnalyzeEngineFile(unsigned char EngineIndex);
void CalDecelerartion(void);
void CalAccelerateArray(s16 *AccelerationArray, u16 factor);
u32 EngineStartHandler(void);


#endif


