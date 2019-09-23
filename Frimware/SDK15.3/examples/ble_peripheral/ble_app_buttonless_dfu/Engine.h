#ifndef __ENGINE__
#define __ENGINE__

#include <stdint.h>


typedef enum {ENGINE_STOP = 0, ENGINE_RUNNING = 1, 
                         ENGINE_STARTING = 2, ENGINE_STOPING = 3} EngineStatusFlag;

//#define MIXER_CHANNEL0_TIMER			TIM6
//#define MIXER_CHANNEL1_TIMER			TIM7

                         
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

#define AUDIO_SAMPLE_RATE_OFFSET            450
#define AUDIO_SAMPLE_RATE_LENGTH            4
#define AUDIO_BIT_RATE_OFFSET               (AUDIO_SAMPLE_RATE_OFFSET + AUDIO_SAMPLE_RATE_LENGTH)
#define AUDIO_BIT_RATE_LENGTH               2


struct EngineInfo
{
    uint32_t SampleRate;//Mp3������
    //uint16_t BitRate;//Mp3����,ʵ�ʲ�û������
    EngineStatusFlag status;

    

    FunctionalState FlameOutSwitch;
    uint16_t FlameOutTime;//���ٶȴ��ڵ������䣬����MaxSlientTime��û�б仯����Ϩ��
    uint32_t InFlameOutTime;
    uint32_t StopTime;//����ر��ѽ��е�ʱ��

    
    uint16_t ThrottleUpResponse;//������Ӧ��ֵԽ����ӦԽ��,��λ100ns
    uint16_t ThrottleDownResponse;
    uint16_t ThrottleDownResponseOri;//����ֵ����EngineDecResponseһ�����õ�ThrottleDownResponse

    

    int16_t SpeedStage;//��ʶ��ǰ�ٶ����ڵ��ٶȽ׶Σ��е���ʱ����ߵ��Ƕ��ٶ�����
    int16_t UnMixSpeed;//�Ӹ�ֵ����׼�ٶ�֮������䣬����Ҫ���������ٶȽ׶�Ϊ0ʱ����ֵ��Ч
    uint8_t ChannelNum;//ͨ�������������������ж��Ƿ���Ҫ����
    uint8_t LowSpeedChannel;
    uint8_t HighSpeedChannel;

    FunctionalState OverloadExistFlag;
    FunctionalState OverloadEnableFlag;
    uint16_t OverloadDelayTime;

    //int16_t AccelerateArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];
    int16_t AccelerateArrayOri[MAX_AUDIO_FILE_NUM_ONE_DIRECT];
    uint16_t EngineAccResponseP;
    uint16_t EngineAccResponseD;
    uint16_t EngineDecResponse;


    int16_t ForwardLowSpeed;//ǰ����Դ�ĵ���
    int16_t ForwardHighSpeed;//ǰ����Դ�ĸ���
    int16_t BackwardLowSpeed;//ǰ����Դ�ĵ���
    int16_t BackwardHighSpeed;//ǰ����Դ�ĸ���

    int16_t ForwardSpeedArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];
    uint32_t ForwardDataCntArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];//�������������ĸ���
    int16_t BackwardSpeedArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];
    //uint32_t BackwardDataCntArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];//�������������ĸ���
    uint32_t ForwardAudioStartAddrArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];
    //uint32_t BackwardAudioStartAddrArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];
    //int16_t GearBoxAccelerateArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];

    uint8_t ForwardAudioFileNum;
    uint8_t BackwardAudioFileNum;

    uint32_t StartAudioDataCnt;
    uint32_t StartAudioStartAddr;
    uint32_t StopAudioDataCnt;
    uint32_t StopAudioStartAddr;

    uint32_t MaxSpeedLimit;//���ݵ綯����λ��ͬ������ٶ�Ҳ�᲻ͬ
};

extern struct EngineInfo engine;
extern struct MixerChannelInfo MixerChannel[ENGINE_MIXER_CHANNEL_NUM];
extern struct MixerChannelInfo EngineStartStopVoiceChannel;


int EngineSetByMixerConfig(uint32_t *FlameOutTime, uint16_t *ThrottleDownResponse, uint16_t *ThrottleUpResponse, FunctionalState *OverloadExistFlag,
                                                            FunctionalState *OverloadEnableFlag, uint16_t *OverloadDelayTime, int16_t *AccelerateArray, FunctionalState *EngineResponseEnable,
                                                            uint16_t *EngineResponse);
//void MixerChannel0IntHandler(void);
//void MixerChannel1IntHandler(void);
int32_t GetEngineAudioData(void);
void EngineHandler(void);
void EngineMixerChannelInit(void);
int AnalyzeEngineFile(unsigned char EngineIndex);
void CalDecelerartion(void);
void CalAccelerateArray(int16_t *AccelerationArray, uint16_t factor);
uint32_t EngineStartHandler(void);
void EnableMixerChannelTimer(void);
void DisableMixerChannelTimer(void);
void TryCloseAudioFile(struct MixerChannelInfo *Channel);





#endif


