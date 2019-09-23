#ifndef __AUDIO_PLAY__
#define __AUDIO_PLAY__

#define AUDIO_OUT_BUFF_SIZE              28   //IS2 DMA��������� (1000000 / 44100) * AUDIO_OUT_BUFF_SIZEΪһ�����岥�ŵ�ʱ��616us

#define AUDIO_OUT_FREQUENCY                44100
#define AUDIO_OUT_INTERVAL                  (1000000000 / 44100)//ÿ�������Ƶ����֮���ʱ��������λns

/** 
 * [AudioNode description]������Ƶ������Ľڵ㣬������Ƶ���ݺ�ʱ�����Ϣ��ʱ���ĵ�λΪns
 * @Author   tin39
 * @DateTime 2019��7��23��T9:30:14+0800
 * @return                            [description]
 */
struct AudioNodeInfo
{
    int32_t data;
    uint64_t time;
};

struct AudioOutInfo
{
    //�������ͨ�����ݲ����ڵ�ͽڵ�����
    struct AudioNodeInfo AudioNode[2][AUDIO_OUT_BUFF_SIZE * 3];//ÿ�����������Ƶ�ڵ㣬������Ƶ���ݺ�ʱ���
    int16_t AudioNodeCnt[2];

    int32_t I2sBuff[2][AUDIO_OUT_BUFF_SIZE];
    uint64_t timeStamp[AUDIO_OUT_BUFF_SIZE];
    int32_t *I2sSendingBuffPtr;
    int32_t *I2sFillingBuffPtr;
};

extern struct AudioOutInfo AudioOut;

void SetI2sData(uint16_t AudioData);
void AudioOutVarInit(void);
void I2sInit(void);
void I2sIntRestart(void);
void I2sIntStop(void);
void ClearAudioPlayBuff(void);



#endif
