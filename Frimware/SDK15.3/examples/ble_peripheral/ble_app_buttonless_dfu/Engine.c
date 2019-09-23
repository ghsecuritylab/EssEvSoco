#include "common.h"
#include "engine.h"
#include "MotorSpeed.h"
#include "ActionTick.h"
#include "AgentTest.h"
#include "mixer.h"
#include "SystemError.h"
#include "tea.h"
#include "engine.h"
#include "string.h"

#include "nrf_drv_timer.h"


#define DEFAULT_SLIENT_TIME_VALUE		5000  //Ĭ��5��
#define MIN_SLIENT_TIME_VALUE			500//500ms
#define MAX_SLIENT_TIME_VALUE			60000//60s
#define NO_FLAME_OUT_VALUE				0xFFFFFFFE

//�ٶȸ��¼������������������Сֵ
#define SPEED_UPDATA_INTERVAL_MAX_TIME	1000
//�ٶȸ��¼������������������Сֵ
#define SPEED_UPDATA_INTERVAL_MIN_TIME	4
//�ٶȸ��¼�������������
#define SPEED_UPDATA_INTERVAL_UP			50
#define SPEED_UPDATA_INTERVAL_DOWN			60

//���������Ź���������Ĭ����ʱʱ��
#define ENGINE_OVERLOAD_SOUND_DEFAULT_DELAY_TIME		5000


//���������ص����ֵ����Сֵ
#define MAX_ENGINE_MOMENT_VALUE						3000
#define MIN_ENGINE_MOMENT_VALUE						1
#define DEFAULT_ENGINE_MOMENT_VALUE					500





#define ENGINE_STOP_LAST_TIME		10000 //��λ100ns
#define ENGINE_STOP_MIN_SPEED		300


//#define MAX_AUDIO_FILE_NUM_ONE_DIRECT		20
//#define MAX_AUDIO_FILE_NUM					(MAX_AUDIO_FILE_NUM_ONE_DIRECT * 2)


//const nrf_drv_timer_t MIXER_CHANNEL0_TIMER = NRF_DRV_TIMER_INSTANCE(2);  
//#define MixerChannel0IntHandler nrfx_timer_2_irq_handler
//#define TIMER_MIXER_CHANNEL0          NRF_TIMER2

//const nrf_drv_timer_t MIXER_CHANNEL1_TIMER = NRF_DRV_TIMER_INSTANCE(3);
//#define MixerChannel1IntHandler nrfx_timer_3_irq_handler
//#define TIMER_MIXER_CHANNEL1          NRF_TIMER3






struct EngineInfo engine = 
{
    .SampleRate = DEFAULT_SAMPLE_RATE
};
struct MixerChannelInfo MixerChannel[ENGINE_MIXER_CHANNEL_NUM] = 
{
    {.TimerInterval = (1000000000 / DEFAULT_SAMPLE_RATE)},
    {.TimerInterval = (1000000000 / DEFAULT_SAMPLE_RATE)},
};
struct MixerChannelInfo EngineStartStopVoiceChannel = 
{
    .TimerInterval = (1000000000 / DEFAULT_SAMPLE_RATE)
};

//void MixerChannel0IntHandlerNull(nrf_timer_event_t event_type, void            * p_context) {}
//void MixerChannel1IntHandlerNull(nrf_timer_event_t event_type, void            * p_context) {}


///********************��ʼ��ģ��*************************/
//void EngineMixerChannelInit()
//{
//    uint32_t err_code = NRF_SUCCESS;
//    nrf_drv_timer_config_t timer_cfg;// = NRF_DRV_TIMER_DEFAULT_CONFIG;
//    timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32;
//    timer_cfg.frequency = NRF_TIMER_FREQ_16MHz;
//    timer_cfg.interrupt_priority = IRQ_PROIRITY_TIMER_MIXER0;
//    timer_cfg.mode = NRF_TIMER_MODE_TIMER;
//    timer_cfg.p_context = NULL;
//    err_code = nrf_drv_timer_init(&MIXER_CHANNEL0_TIMER, &timer_cfg, MixerChannel0IntHandlerNull);
//    APP_ERROR_CHECK(err_code);

//    nrf_drv_timer_extended_compare(
//         &MIXER_CHANNEL0_TIMER, NRF_TIMER_CC_CHANNEL0, STANDARD_ARR_VALUE, (nrf_timer_short_mask_t)NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
//    nrf_drv_timer_extended_compare(
//         &MIXER_CHANNEL0_TIMER, NRF_TIMER_CC_CHANNEL1, STANDARD_ARR_VALUE * 2, (nrf_timer_short_mask_t)NRF_TIMER_SHORT_COMPARE1_CLEAR_MASK, true);
//    nrf_drv_timer_extended_compare(
//         &MIXER_CHANNEL0_TIMER, NRF_TIMER_CC_CHANNEL2, STANDARD_ARR_VALUE * 3, (nrf_timer_short_mask_t)NRF_TIMER_SHORT_COMPARE2_CLEAR_MASK, true);

//    nrf_drv_timer_enable(&MIXER_CHANNEL0_TIMER);    


//    err_code = nrf_drv_timer_init(&MIXER_CHANNEL1_TIMER, &timer_cfg, MixerChannel1IntHandlerNull);
//    APP_ERROR_CHECK(err_code);

//    nrf_drv_timer_extended_compare(
//         &MIXER_CHANNEL1_TIMER, NRF_TIMER_CC_CHANNEL0, STANDARD_ARR_VALUE, (nrf_timer_short_mask_t)NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
//    nrf_drv_timer_extended_compare(
//         &MIXER_CHANNEL1_TIMER, NRF_TIMER_CC_CHANNEL1, STANDARD_ARR_VALUE * 2, (nrf_timer_short_mask_t)NRF_TIMER_SHORT_COMPARE1_CLEAR_MASK, true);
//    nrf_drv_timer_extended_compare(
//         &MIXER_CHANNEL1_TIMER, NRF_TIMER_CC_CHANNEL2, STANDARD_ARR_VALUE * 3, (nrf_timer_short_mask_t)NRF_TIMER_SHORT_COMPARE2_CLEAR_MASK, true);

//    nrf_drv_timer_enable(&MIXER_CHANNEL1_TIMER);    

//    
//}



/**************************��������趨ģ��*********************************/
/*
void CalAccelerateArray(int16_t *AccelerationArray, uint16_t factor)
{
    //uint16_t DistFactor;
    int i;
    //���Ǳ�׼�ģ�����ٶȾ��󲻱�
    //if(factor == 10000)
    //    return;

    
    for(i = 0; i < MAX_AUDIO_FILE_NUM_ONE_DIRECT; i++)
    {
        engine.AccelerateArray[i] = (int32_t)AccelerationArray[i] * factor / 10000;
    }

    
}
*/

//���������Ӧ
void CalDecelerartion()
{
    engine.ThrottleDownResponse = ((uint32_t)engine.ThrottleDownResponseOri * 10000) / engine.EngineDecResponse / 2;
}



/*****************************����������Ƶ�Ľ���*********************************/
void EngineMixerChannelVarInit(unsigned char SoundIndex)
{
    int i;


    for(i = 0; i < ENGINE_MIXER_CHANNEL_NUM; i++)
    {
        MixerChannel[i].Fp = OpenFile(ENGINE_INDEX(SoundIndex));
        MixerChannel[i].id = i;
        MixerChannel[i].AmplitudeFactor = 0;
        MixerChannel[i].Buff0ReadyFlag = 0;
        MixerChannel[i].Buff1ReadyFlag = 0;
        MixerChannel[i].Buff2ReadyFlag = 0;
        MixerChannel[i].DataPtr = 0;
        MixerChannel[i].ChannelStatus = CHANNEL_CLOSE;
        MixerChannel[i].NowBuffFlag = 0;
        MixerChannel[i].OriginSpeed = 0;
        MixerChannel[i].ReadFileCnt = 0;
        MixerChannel[i].SoundData = 0;
    }   
}


//ͷ������������һ������ʱ���޹�
//22050��������50ms
//44100��������25ms
int32_t CutAudioFileStartStop(uint32_t *StartAddr, uint32_t * DataCnt)
{
    //50:ms��MP3�ļ�ͷ���̶�����50ms���ݣ�
    //22:Ϊ������22050��1ms�Ĳ����㣬
    // 2:һ��������ռ2Bytes
    *StartAddr = *StartAddr + 50 * (22050 / 1000) * 2;
    //ȥ��ͷȥ��β����ȥ��50ms����Ƶ����
    *DataCnt = *DataCnt - 50 * (22050 / 1000) * 2 * 2;
    return 0;
}

struct PointInfo
{
    int16_t Value;
    uint32_t position;
};


struct PointInfo GetTheMaxPoint(int16_t *buff, uint32_t cnt)
{
    struct PointInfo Point = {0, 0};
    uint32_t i;
    
    for(i = 0; i < cnt; i++)
    {
        if(buff[i] > Point.Value)
        {
            Point.Value = buff[i];
            Point.position = i;
        }
    }
    return Point;
}


#define MIN_INT16			-32768
#define THROUGH_THESHOLD		(MIN_INT16 / 20)//���ȵ���ֵ���������̵�5%
//��ĩβ��ʼѰ�ң��ҵ���һ��ƥ��Ĳ���
struct PointInfo GetTheLastTroughPoint(int16_t *buff, uint32_t cnt)
{
    bool IsTouchTheTheshold = false;
    struct PointInfo Point = {0, 0};
    Point.position = cnt - 1;

    while(Point.position > 0)
    {
        if(IsTouchTheTheshold == false)
        {
            //�����ҵ����ϲ��ȷ�ֵ�������ĵ�
            if(buff[Point.position] < THROUGH_THESHOLD)
            {
                IsTouchTheTheshold = true;
            }
        }
        else
        {
            //���ǰ��һ�����ݱ�����һ�����ݴ󣬱�ʾ�ҵ��˲���
            //����һ�����ݼ�Ϊ����
            if(buff[Point.position] > buff[Point.position + 1])
            {
                //�ҵ����ȣ�����λ��Ϊ���ȣ�����
                Point.position++;
                break;
            }
        }
        Point.position--;
    }
    Point.Value = buff[Point.position];
    return Point;
}

bool IsCrossZero(int16_t *buff, uint32_t DataCnt)
{
    int32_t i;
    bool Negative = false, Positive = false;
    for(i = 0; i < DataCnt; i++)
    {
        if(buff[i] < 0)
        {
            Negative = true;
        }
        else
        {
            Positive = true;
        }

        if(Negative != false && Positive != false)
        {
            return true;
        }
    }

    return false;
}

//�ҵ��������㣬���ع�������ֵΪ���ĵ�һ���㡣
int32_t GetPositiveCrossZero(int16_t *buff, uint32_t DataCnt)
{
    int32_t i;
    for(i = 0; i < DataCnt - 1; i++)
    {
        if(buff[i] < 0 && buff[i + 1] >= 0)
        {
            return i + 1;
        }
    }

    return -1;
}


//�ҵ��������㣬���ع���ǰ��ֵΪ���ĵ�һ����
int32_t GetNegativeCrossZero(int16_t *buff, uint32_t DataCnt)
{
    int32_t i;
    for(i = 0; i < DataCnt - 1; i++)
    {
        if(buff[i] > 0 && buff[i + 1] <= 0)
        {
            return i;
        }
    }
    return -1;
}

//���������ֵ�Ƿ������0
bool CheckAllDataPositive(int16_t *buff, uint32_t DataCnt)
{
    int32_t i;
    for(i = 0; i < DataCnt; i++)
    {
        if(buff[i] < 0)
            return false;
    }
    return true;
}


//��ȡ��С����ֵ
int32_t GetMinPositive(int16_t *buff, uint32_t DataCnt)
{
    int16_t MiniPositive = 32767;
    int32_t MinPosition = -1;
    int32_t i;
    for(i = 0; i < DataCnt; i++)
    {
        if(buff[i] >= 0 && buff[i] < MiniPositive)
        {
            MiniPositive = buff[i];
            MinPosition = i;
        }
    }
    return MinPosition;
}

int32_t GetMaxNegative(int16_t *buff, uint32_t DataCnt)
{
    int16_t MaxNegative = -30000;
    int32_t MaxPosition = -1;
    int32_t i;
    for(i = 0; i < DataCnt; i++)
    {
        if(buff[i] <= 0 && buff[i] > MaxNegative)
        {
            MaxNegative = buff[i];
            MaxPosition = i;
        }
    }
    return MaxPosition;
}

//�ҵ�MP3�ļ���Ч��Ƶ��β��
//��Ƶ����ʱ����sndת����MP3ǰ�������1֡1152�ֽ�������ݣ�
//�˴�ʶ��������ݶΣ�����������ݶ�ǰ��һ����
int32_t FindTailPoint(int16_t *buff, uint32_t DataCnt)
{
    int32_t ptr;
    bool FindSignature = false;
    int32_t ContinueSignature = 0;
    ptr = DataCnt - 1;

    //�����ҵ���ʶ���ݶ�
    while(ptr > 0)
    {
        if(buff[ptr] > 25000)
        {
            ContinueSignature++;
            if(ContinueSignature >= 20)
            {
                FindSignature = true;
                break;
            }
        }
        else
        {
            ContinueSignature = 0;
        }
        ptr--;
    }

    if(FindSignature == false)
    {
        //û���ҵ���������
        return DataCnt - 1;
    }

    //�ҵ����������ݶ�
    while(ptr > 0)
    {
        //���ԴӺ���ǰ�ҵ���һ���������㣬��Ƶ��ȡ��ʱ����Ҫ���ڲ���
        if(buff[ptr] >= 0 && buff[ptr - 1] < 0)
        {
            break;
        }
        ptr--;
    }

    return ptr;
}



#define CUT_BUFF_SIZE				(2304)//������Ƶ��ȡ�Ļ����С����������Ϊint16_t���ͣ�MP3һ֡1152�ֽ�����
int16_t CutTemp[2304 - 2000];



#define OFFSET_HEAD_START_CUT               (1100)// 22050�������£�Լ50ms�����ݣ�������MP3�ļ�ʱ�������룬��Ҫȥ��    
#define DATA_CNT_HAED_CUT_BUFF              25      
int32_t CutAudioOneSpeed(struct FileInfo *Fp, uint32_t *StartAddr, uint32_t * DataCnt)
{
    int16_t buff[CUT_BUFF_SIZE];
    int32_t position = -1;
    int32_t EndPosition;

    //struct PointInfo MaxPoint;
    //struct PointInfo StartPoint;
    //struct PointInfo LastThroghPoint;
    //struct PointInfo EndPoint;
    
    /*************ͷ������**************/
    ReadFile(Fp, *StartAddr + OFFSET_HEAD_START_CUT * sizeof(int16_t), (uint8_t *)buff, DATA_CNT_HAED_CUT_BUFF * sizeof(int16_t));
    DeconfuseWav_16bit((uint8_t *)buff, DATA_CNT_HAED_CUT_BUFF * sizeof(int16_t));

    if(IsCrossZero(buff, DATA_CNT_HAED_CUT_BUFF) == true)
    {
        //���ڹ���
        position = GetPositiveCrossZero(buff, DATA_CNT_HAED_CUT_BUFF);//����Ѱ���������
        if(position == -1)
        {
            //û��������㣬����Ѱ�ҷ������
            position = GetNegativeCrossZero(buff, DATA_CNT_HAED_CUT_BUFF);//����Ѱ�ҷ������
            if(position == -1)
            {
                //δ�ҵ��������ͷ�����㣬�����ϲ�Ӧ�ó���
            }
        }
    }
    else
    {
        //�����ڹ���
        if(CheckAllDataPositive(buff, DATA_CNT_HAED_CUT_BUFF) == true)
        {
            //�������ݾ�Ϊ��
            position = GetMinPositive(buff, DATA_CNT_HAED_CUT_BUFF);
        }
        else
        {
            //�������ݾ�Ϊ��
            position = GetMaxNegative(buff, DATA_CNT_HAED_CUT_BUFF);
        }
    }

    if(position == -1)
        position = 0;

    /*********β������***********/
    ReadFile(Fp, *StartAddr +  *DataCnt - CUT_BUFF_SIZE * sizeof(int16_t), (uint8_t *)buff, CUT_BUFF_SIZE * sizeof(int16_t));
    DeconfuseWav_16bit((uint8_t *)buff, CUT_BUFF_SIZE * sizeof(int16_t));

    memcpy(CutTemp, &buff[2000], (2304 - 2000) * 2);


    EndPosition = FindTailPoint(buff, CUT_BUFF_SIZE);

/*
    //�ҵ����һ�����ȵ���͵�һ����
    LastThroghPoint = GetTheLastTroughPoint(buff, CUT_BUFF_SIZE);
    //�ҵ����һ�����Ⱥ���Ĺ����
    EndPoint = LastThroghPoint;
    while(EndPoint.position < CUT_BUFF_SIZE)
    {
        if(buff[EndPoint.position] >= 0)
            break;

        EndPoint.position++;
    }
    //LastValue = buff[EndPoint.position];
    //������һ���㣬��һ������ǹ���ǰ��һ����
    EndPoint.position--;
    EndPoint.Value = buff[EndPoint.position];
*/

    //������ʼ��ַ
    *StartAddr = *StartAddr + (OFFSET_HEAD_START_CUT + position) * sizeof(int16_t);
    //�����ȥ����������
    //*DataCnt = *DataCnt -((StartPoint.position + CUT_BUFF_SIZE - EndPoint.position + 1) * sizeof(int16_t));
    *DataCnt = *DataCnt -((OFFSET_HEAD_START_CUT + position + CUT_BUFF_SIZE - EndPosition + 1) * sizeof(int16_t));


    return 0;
    
}






int GetEngineInfo(unsigned char SoundIndex)
{
    unsigned char buff[512];
    int i, j;
    uint16_t StartSector;

    
    if(File[ENGINE_INDEX(SoundIndex)].EnableFlag == DISABLE)
    {
        return 4;
    }

    ReadFile(&File[ENGINE_INDEX(SoundIndex)], 0, buff, 512);
    engine.ForwardAudioFileNum = buff[FORWARD_AUDIO_CNT_OFFSET];
    engine.BackwardAudioFileNum = buff[BACKWARD_AUDIO_CNT_OFFSET];
    if(engine.ForwardAudioFileNum == 0)
        return 3;
    else if(engine.ForwardAudioFileNum >= MAX_AUDIO_FILE_NUM_ONE_DIRECT || engine.BackwardAudioFileNum >= MAX_AUDIO_FILE_NUM_ONE_DIRECT)
        return 2;

    //��ȡǰ����������Ϣ
    for(i = 0; i < engine.ForwardAudioFileNum; i++)
    {
        engine.ForwardSpeedArray[i] = buff[SPEED_TABLE_OFFSET + i * 2] * 256;
        engine.ForwardSpeedArray[i] += buff[SPEED_TABLE_OFFSET + i * 2 + 1];
        
        engine.ForwardDataCntArray[i] = buff[AUDIO_LENGTH_OFFSET + i * 4] << 24;
        engine.ForwardDataCntArray[i] += buff[AUDIO_LENGTH_OFFSET + i * 4 + 1] << 16;
        engine.ForwardDataCntArray[i] += buff[AUDIO_LENGTH_OFFSET + i * 4 + 2] << 8;
        engine.ForwardDataCntArray[i] += buff[AUDIO_LENGTH_OFFSET + i * 4 + 3];

        StartSector = buff[AUDIO_START_SECTOR_OFFSET + i * 2] << 8;
        StartSector += buff[AUDIO_START_SECTOR_OFFSET + i * 2 + 1];
        engine.ForwardAudioStartAddrArray[i] = (uint32_t)StartSector * 512;

        if(engine.ForwardSpeedArray[i] == 0 || engine.ForwardDataCntArray[i] == 0 || engine.ForwardDataCntArray[i] >= FS_ONE_FILE_MAX_SIZE ||
            engine.ForwardAudioStartAddrArray[i] == 0 || engine.ForwardAudioStartAddrArray[i] >= FS_SUPPORT_MAX_SIZE)
        {
            return 1;
        }
    }

    //��ȡ������������Ϣ
    for(j = 0; i < engine.ForwardAudioFileNum + engine.BackwardAudioFileNum; i++)
    {
        engine.BackwardSpeedArray[j] = buff[SPEED_TABLE_OFFSET + i * 2] * 256;
        engine.BackwardSpeedArray[j] += buff[SPEED_TABLE_OFFSET + i * 2 + 1];
        engine.BackwardSpeedArray[j] = 0 - engine.BackwardSpeedArray[j];
        
//        engine.BackwardDataCntArray[j] = buff[AUDIO_LENGTH_OFFSET + i * 4] << 24;
//        engine.BackwardDataCntArray[j] += buff[AUDIO_LENGTH_OFFSET + i * 4 + 1] << 16;
//        engine.BackwardDataCntArray[j] += buff[AUDIO_LENGTH_OFFSET + i * 4 + 2] << 8;
//        engine.BackwardDataCntArray[j] += buff[AUDIO_LENGTH_OFFSET + i * 4 + 3];


//        StartSector = buff[AUDIO_START_SECTOR_OFFSET + i * 2] << 8;
//        StartSector += buff[AUDIO_START_SECTOR_OFFSET + i * 2 + 1];
//        engine.BackwardAudioStartAddrArray[i] = (uint32_t)StartSector * 512;

        if(engine.BackwardSpeedArray[j] == 0)
        {
            return 1;
        }
        j++;
    }

    //���й������������ҹ��ع��ܱ��رգ�����Ҫ����������������1�Ӷ����ٶȸ��������ʱ�ܹ���������
    if(engine.OverloadExistFlag == ENABLE && engine.OverloadEnableFlag == DISABLE)
    {
        engine.ForwardAudioFileNum--;
    }
    engine.ForwardLowSpeed = engine.ForwardSpeedArray[0];
    engine.ForwardHighSpeed = engine.ForwardSpeedArray[engine.ForwardAudioFileNum - 1];

    StartSector = buff[START_SOUND_START_SECTOR_OFFSET] << 8;
    StartSector += buff[START_SOUND_START_SECTOR_OFFSET + 1];
    engine.StartAudioStartAddr = StartSector * 512;
    engine.StartAudioDataCnt = buff[START_SOUND_LENGTH_OFFSET] << 24;
    engine.StartAudioDataCnt += buff[START_SOUND_LENGTH_OFFSET + 1] << 16;
    engine.StartAudioDataCnt += buff[START_SOUND_LENGTH_OFFSET + 2] << 8;
    engine.StartAudioDataCnt += buff[START_SOUND_LENGTH_OFFSET + 3];
    if(engine.StartAudioDataCnt == 0 || engine.StartAudioDataCnt >= FS_ONE_FILE_MAX_SIZE ||
            engine.StartAudioStartAddr == 0 || engine.StartAudioStartAddr >= FS_SUPPORT_MAX_SIZE)
    {
        return 5;
    }

    StartSector = buff[STOP_SOUND_START_SECTOR_OFFSET] << 8;
    StartSector += buff[STOP_SOUND_START_SECTOR_OFFSET + 1];
    engine.StopAudioStartAddr = StartSector * 512;
    engine.StopAudioDataCnt = buff[STOP_SOUND_LENGTH_OFFSET] << 24;
    engine.StopAudioDataCnt += buff[STOP_SOUND_LENGTH_OFFSET + 1] << 16;
    engine.StopAudioDataCnt += buff[STOP_SOUND_LENGTH_OFFSET + 2] << 8;
    engine.StopAudioDataCnt += buff[STOP_SOUND_LENGTH_OFFSET + 3];

    //�õ������ļ��Ĳ�����
    memcpy(&engine.SampleRate, buff + AUDIO_SAMPLE_RATE_OFFSET, AUDIO_SAMPLE_RATE_LENGTH);
    if(engine.SampleRate != 44100 && engine.SampleRate != 22050)
    {
        engine.SampleRate = 44100;
    }
    //memcpy(&engine.BitRate, buff + AUDIO_BIT_RATE_OFFSET, AUDIO_BIT_RATE_LENGTH);
    

    engine.ChannelNum = 0;
    engine.SpeedStage = 0;
    engine.LowSpeedChannel = 0;
    engine.HighSpeedChannel = 1;

    engine.BackwardLowSpeed = engine.BackwardSpeedArray[0];
    engine.BackwardHighSpeed = engine.BackwardSpeedArray[engine.BackwardAudioFileNum - 1];
    engine.ForwardLowSpeed = engine.ForwardSpeedArray[0];
    engine.ForwardHighSpeed = engine.ForwardSpeedArray[engine.ForwardAudioFileNum - 1];

    
    MotorSpeed.SlientSpeed = engine.ForwardLowSpeed / 2;


    for(i = 0; i < engine.ForwardAudioFileNum; i++)
    {
        CutAudioOneSpeed(&File[ENGINE_INDEX(SoundIndex)], &engine.ForwardAudioStartAddrArray[i], &engine.ForwardDataCntArray[i]);
    }
    
    CutAudioFileStartStop(&engine.StartAudioStartAddr, &engine.StartAudioDataCnt);
    if(engine.StopAudioDataCnt != 0 && engine.StopAudioStartAddr != 0)
    {
        CutAudioFileStartStop(&engine.StopAudioStartAddr, &engine.StopAudioDataCnt);
    }
    return 0;
}

void EngineStopWorking()
{
    engine.StartAudioStartAddr = 0;
    engine.StartAudioDataCnt = 0;
}


int AnalyzeEngineFile(unsigned char SoundIndex)
{
    int ret;
    engine.status = ENGINE_STOP;
    MotorSpeed.RealSpeed = 0;
    MotorSpeed.DistSpeed = 0;
    EngineMixerChannelVarInit(SoundIndex);
    ret = GetEngineInfo(SoundIndex);
    if(ret != 0)
    {
        SetSystmError(AUDIO_FILE_ERROR);
        EngineStopWorking();
    }
    else
    {
        ClearSystemError(AUDIO_FILE_ERROR);
    }
    GearBoxInit();
    return ret;
}

#define MAX_VOLUME_SLOPE            20
#define PERCENT_STEP_BELOW_MIDDLE   500
#define PERCENT_STEP_ABOVE_MIDDLE   500
uint16_t CalVolumeSlopePercent(int16_t RealSpeed)
{
    uint32_t VolumeSlopePercent;

    if(engine.status == ENGINE_RUNNING || engine.status == ENGINE_STARTING)
    {
        if(mixer.VolumeSlope > MAX_VOLUME_SLOPE)
        {
            mixer.VolumeSlope = 0;
        }

    
        //б�ʷ�Ϊ10����ÿһ�����ж�Ӧ������ٵ���������������ٵ���������
        //����ٵı�����ϵ��0��1��1��0.95��2��0.9����������������9��0.55��10��0.5
        //����ٵı�����ϵ��0��1��2��1.1��2��1.2.............9)1.9;10)2.0
        if(RealSpeed < 0)
        {
            RealSpeed = -RealSpeed;
        }

        if(RealSpeed < engine.ForwardLowSpeed)
        {
            return 10000 - PERCENT_STEP_BELOW_MIDDLE * mixer.VolumeSlope;
        }
        else if(RealSpeed > engine.ForwardHighSpeed)
        {
            return 10000 + PERCENT_STEP_ABOVE_MIDDLE * mixer.VolumeSlope;
        }
        else
        {
            int16_t MiddleSpeed;
            MiddleSpeed = (engine.ForwardHighSpeed + engine.ForwardLowSpeed) >> 1;
            if(RealSpeed <= MiddleSpeed)
            {
                uint32_t LowSpeedPercent = 10000 - PERCENT_STEP_BELOW_MIDDLE * mixer.VolumeSlope;
                uint32_t SpeedPercent = (RealSpeed - engine.ForwardLowSpeed) * 10000 / (MiddleSpeed - engine.ForwardLowSpeed);
                VolumeSlopePercent = LowSpeedPercent + SpeedPercent * (10000 - LowSpeedPercent) / 10000;
            }
            else
            {
                uint32_t HighSpeedPercent = 10000 + PERCENT_STEP_ABOVE_MIDDLE * mixer.VolumeSlope;
                uint32_t SpeedPercent = (RealSpeed - MiddleSpeed) * 10000 / (engine.ForwardHighSpeed - MiddleSpeed);
                VolumeSlopePercent = 10000 + SpeedPercent * (HighSpeedPercent - 10000) / 10000;
            }
        }

        
    }
    else
    {
        VolumeSlopePercent = 10000;
    }
    return VolumeSlopePercent;
}


/*********************�����ٶȼ���ģ��*************************/
//���а����˶���RC�źż��㣬ɲ��״̬�жϣ������Ĵ���
//������Ӧ�������յó�һ��ʵ���ٶ�
void CalSpeedAndBrakeAndGearBoxHandler()
{
    static uint32_t MixTimeInterval = 0;

    //�Ӽ��ٵĦ���ͬ�����Ը��������γɲ�ͬ�ļӼ�������
    if(((MotorSpeed.VehicleStatu == GO_FORWARD) && (MotorSpeed.DistSpeed >= MotorSpeed.RealSpeed) && (GetSystemTime() - MixTimeInterval >= (engine.ThrottleUpResponse >> REAL_SPEED_CAL_MULT_SHIFT)))
        || ((MotorSpeed.VehicleStatu == GO_FORWARD) && (MotorSpeed.DistSpeed < MotorSpeed.RealSpeed) && (GetSystemTime() - MixTimeInterval >= engine.ThrottleDownResponse))
        || ((MotorSpeed.VehicleStatu == GO_BACKWARD) && (MotorSpeed.DistSpeed <= MotorSpeed.RealSpeed) && (GetSystemTime() - MixTimeInterval >= (engine.ThrottleUpResponse >> REAL_SPEED_CAL_MULT_SHIFT)))
        || ((MotorSpeed.VehicleStatu == GO_BACKWARD) && (MotorSpeed.DistSpeed > MotorSpeed.RealSpeed) && (GetSystemTime() - MixTimeInterval >= engine.ThrottleDownResponse))
        || ((MotorSpeed.VehicleStatu == BREAK) && (GetSystemTime() - MixTimeInterval >= engine.ThrottleDownResponse)))
    {
        MixTimeInterval = GetSystemTime();
        
        
        MotorSpeed.DistSpeed = CalDistSpeed();


        
        if(engine.status == ENGINE_RUNNING)
        {
            /*if(MotorSpeed.GearBox.GearBoxEnableFlag == DISABLE)
            {
                 //����Ŀ���ٶȼ������ǰʵ���ٶ�
                MotorSpeed.RealSpeed = GetMotorRealSpeedWithoutGearbox(MotorSpeed.DistSpeed, MotorSpeed.RealSpeed);
            }
            else
            {
                MotorSpeed.RealSpeed = GetMotorRealSpeedWithGearBox(MotorSpeed.DistSpeed, MotorSpeed.RealSpeed);
            }*/
           
            ////����������������ʱ����л���
            //SetMixerChannel(Speed);
            MotorSpeed.RealSpeed = GetMotorRealSpeedWithoutGearbox(MotorSpeed.DistSpeed, MotorSpeed.RealSpeed);
        }

        mixer.VolumeSlopePercent = CalVolumeSlopePercent(MotorSpeed.RealSpeed);
    }
}



/********************************����ʵ���ٶȼ���mixer����**************************************/
#define STAGE_MIX_PERCENT                       6 //��λ��10%����һ���ٶ������ڣ�����������Ƶ����������ٷֱ�
void SetMixerStatus(int16_t speed)
{
    int i;
    uint8_t NowSpeedStage = 0;
    if(MotorSpeed.VehicleStatu == GO_FORWARD || 
          (MotorSpeed.BreakDirect == FORWARD_BREAK && MotorSpeed.VehicleStatu == BREAK))
    //if(speed >= 0)
    {
        //��ȡ��ǰ�ٶ����ڵ������Ρ�2�����غϣ���ȡ�ߵ��Ƕ�
        //��һΪ�˲�������Խ�磬�����һ���Ѿ������жϣ��ض������һ����
        for(i = 0; i < engine.ForwardAudioFileNum - 1; i++)
        {
            if(speed > engine.ForwardSpeedArray[i])//�ٶȵ��ڵ�ʱ����ϵ��ٵĽ׶�
            {
                NowSpeedStage++;//�˴���֤������ʱ���ٶȺܸ��Ƿ�͵������Ρ�
            }
            else
            {
                break;
            }
        }
        engine.SpeedStage = NowSpeedStage;
    
        
        if(NowSpeedStage > 0)
        {  
            engine.UnMixSpeed = (engine.ForwardSpeedArray[NowSpeedStage] - engine.ForwardSpeedArray[NowSpeedStage - 1]) * STAGE_MIX_PERCENT / 10 +
                                                       engine.ForwardSpeedArray[NowSpeedStage - 1];
            //��Ҫ���ٶ��Ƿ����ڻ�����������ж�
            if(speed >= engine.UnMixSpeed)
            {//���ٶ����ڲ���Ҫ�������ӵ�����
                engine.ChannelNum = 1;
            }
            else
            {//�ٶ�������Ҫ�������ӵ�����
                engine.ChannelNum = 2;
            }
        }
        else
        {
            //���ٶ����ڵ�0�׶Σ���ض�ֻ��һ��ͨ��
            engine.ChannelNum = 1;
        }
    }
    else
    {
        speed = 0 - speed;
        //��ȡ��ǰ�ٶ����ڵ������Ρ�2�����غϣ���ȡ�ߵ��Ƕ�
        //��һΪ�˲�������Խ�磬�����һ���Ѿ������жϣ��ض������һ����
        for(i = 0; i < engine.BackwardAudioFileNum - 1; i++)
        {
            if(speed > engine.BackwardSpeedArray[i])//�ٶȵ��ڵ�ʱ����ϵ��ٵĽ׶�
            {
                NowSpeedStage++;//�˴���֤������ʱ���ٶȺܸ��Ƿ�͵������Ρ�
            }
            else
            {
                break;
            }
        }
        engine.SpeedStage = NowSpeedStage;
    
        
        if(NowSpeedStage > 0)
        {  
            engine.UnMixSpeed = (engine.BackwardSpeedArray[NowSpeedStage] - engine.BackwardSpeedArray[NowSpeedStage - 1]) * STAGE_MIX_PERCENT / 10 +
                                                       engine.BackwardSpeedArray[NowSpeedStage - 1];
            //��Ҫ���ٶ��Ƿ����ڻ�����������ж�
            if(speed >= engine.UnMixSpeed)
            {//���ٶ����ڲ���Ҫ�������ӵ�����
                engine.ChannelNum = 1;
            }
            else
            {//�ٶ�������Ҫ�������ӵ�����
                engine.ChannelNum = 2;
            }
        }
        else
        {
            //���ٶ����ڵ�0�׶Σ���ض�ֻ��һ��ͨ��
            engine.ChannelNum = 1;
        }
    }
}


void OpenAudioFile(struct MixerChannelInfo *Channel, uint16_t Stage)
{
    //uint32_t ModNum;
    //uint32_t MinusCnt;
    Channel->ChannelStatus = CHANNEL_OPEN;
    Channel->TotalDataCnt = engine.ForwardDataCntArray[Stage];
    Channel->OriginSpeed = engine.ForwardSpeedArray[Stage];
    Channel->AudioStartAddress = engine.ForwardAudioStartAddrArray[Stage];

    Channel->Buff0ReadyFlag = 0;
    Channel->Buff1ReadyFlag = 0;
    Channel->Buff2ReadyFlag = 0;
    Channel->NowBuffFlag = 0;
    Channel->ReadFileCnt = 0;

    
    if(Stage == engine.ForwardAudioFileNum - 1 && engine.OverloadExistFlag == ENABLE && engine.OverloadEnableFlag == ENABLE)
    {
        //����ǹ��ص����������ͷ����ʼ����
        Channel->ReadFileCnt = 0;
    }
    else
    {
        //����ת�ٵ������������λ�ÿ�ʼ����
//        ModNum = GetSystemTime() % Channel->TotalDataCnt;
//        MinusCnt = ModNum % AUDIO_DATA_READ_CNT;
//        Channel->ReadFileCnt = ModNum - MinusCnt;

        Channel->ReadFileCnt = 0;
    }



}



void TryOpenAudioFile(struct MixerChannelInfo *Channel, uint16_t Stage)
{
    if(Channel->ChannelStatus == CHANNEL_CLOSE)
    {
        OpenAudioFile(Channel, Stage);
        
    }
}

void TryCloseAudioFile(struct MixerChannelInfo *Channel)
{
    if(Channel->ChannelStatus == CHANNEL_OPEN)
    {
        Channel->Buff0ReadyFlag = 0;
        Channel->Buff1ReadyFlag = 0;
        Channel->Buff2ReadyFlag = 0;
        Channel->DataPtr = 0;
        Channel->ReadFileCnt = 0;
        Channel->SoundData = 0;
        Channel->ChannelStatus = CHANNEL_CLOSE;

    }
}

void MixAudio(int16_t MotorSpeed)
{
    uint32_t factor_temp;

    if(MotorSpeed < 0)
    {
        MotorSpeed = 0 - MotorSpeed;
    }

    //�������ȵ�������С���򲥷ŵ�������
    if(MotorSpeed < engine.ForwardLowSpeed)
    {
        MotorSpeed = engine.ForwardLowSpeed;
    }

    
    //�޻���
    if(engine.ChannelNum == 1)
    {
         if(MixerChannel[engine.LowSpeedChannel].OriginSpeed == engine.ForwardSpeedArray[engine.SpeedStage])
         {
              //ͨ����״̬����
              MixerChannel[engine.LowSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE;
              MixerChannel[engine.HighSpeedChannel].AmplitudeFactor = 0;

              if(engine.LowSpeedChannel == 0)
              {
                  MixerChannel[0].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.LowSpeedChannel].OriginSpeed / MotorSpeed;
              }   
              else
                  MixerChannel[1].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.LowSpeedChannel].OriginSpeed / MotorSpeed;

              TryCloseAudioFile(&MixerChannel[engine.HighSpeedChannel]);

         }
         else if(MixerChannel[engine.HighSpeedChannel].OriginSpeed == engine.ForwardSpeedArray[engine.SpeedStage])
         {
              //ͨ��һ״̬����
              MixerChannel[engine.LowSpeedChannel].AmplitudeFactor = 0;
              MixerChannel[engine.HighSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE;

              if(engine.HighSpeedChannel == 1)
                  MixerChannel[1].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.HighSpeedChannel].OriginSpeed / MotorSpeed;
              else
                  MixerChannel[0].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.HighSpeedChannel].OriginSpeed / MotorSpeed;

              TryCloseAudioFile(&MixerChannel[engine.LowSpeedChannel]);
         }
         else
         {
             TryCloseAudioFile(&MixerChannel[engine.LowSpeedChannel]);
             TryCloseAudioFile(&MixerChannel[engine.HighSpeedChannel]);

             engine.LowSpeedChannel = 0;
             engine.HighSpeedChannel = 1;

             OpenAudioFile(&MixerChannel[engine.LowSpeedChannel], engine.SpeedStage);
             
             //����ͨ���������ϣ���ѡ��ͨ��0
             MixerChannel[engine.LowSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE;
             MixerChannel[engine.HighSpeedChannel].AmplitudeFactor = 0;

             
             MixerChannel[0].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.LowSpeedChannel].OriginSpeed / MotorSpeed;
         }
    }
    else if(engine.ChannelNum == 2)//��Ҫ����
    {
        //����ͨ��״̬������
        if(MixerChannel[engine.LowSpeedChannel].OriginSpeed == engine.ForwardSpeedArray[engine.SpeedStage - 1] &&
            MixerChannel[engine.HighSpeedChannel].OriginSpeed == engine.ForwardSpeedArray[engine.SpeedStage])
        {
            //���ļ����Դ򿪣����Ѿ����������
            //�����������ļ�
            //���뵽����ٶ����䣬Ȼ������
            //���뵽����ٶ����䣬Ȼ����
            TryOpenAudioFile(&MixerChannel[engine.LowSpeedChannel], engine.SpeedStage - 1);
            TryOpenAudioFile(&MixerChannel[engine.HighSpeedChannel], engine.SpeedStage);

            
            factor_temp = (MotorSpeed - MixerChannel[engine.LowSpeedChannel].OriginSpeed) * PERCENT_MIXER_BASE / 
                                  (engine.UnMixSpeed - MixerChannel[engine.LowSpeedChannel].OriginSpeed);   
            MixerChannel[engine.LowSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE - ((uint32_t)((uint32_t)factor_temp * (uint32_t)factor_temp) >> PERCENT_MIXER_BASE_SHIFT);
            MixerChannel[engine.HighSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE - (((uint32_t)(PERCENT_MIXER_BASE - factor_temp) * (uint32_t)(PERCENT_MIXER_BASE - factor_temp)) >> PERCENT_MIXER_BASE_SHIFT);  
            //MixerChannel[engine.LowSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE - factor_temp;
            //MixerChannel[engine.HighSpeedChannel].AmplitudeFactor = factor_temp;
            
            if(engine.LowSpeedChannel == 0)
            {
                MixerChannel[0].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.LowSpeedChannel].OriginSpeed / MotorSpeed;
                MixerChannel[1].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.HighSpeedChannel].OriginSpeed / MotorSpeed;
            }
            else
            {
                MixerChannel[1].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.LowSpeedChannel].OriginSpeed / MotorSpeed;
                MixerChannel[0].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.HighSpeedChannel].OriginSpeed / MotorSpeed;
            }
        }
        //���ٶȱ���ʱ�������һ��ʱ
        else if(MixerChannel[engine.LowSpeedChannel].OriginSpeed == engine.ForwardSpeedArray[engine.SpeedStage])
        {
            TryCloseAudioFile(&MixerChannel[engine.HighSpeedChannel]);
            OpenAudioFile(&MixerChannel[engine.HighSpeedChannel], engine.SpeedStage - 1);

            if(engine.LowSpeedChannel == 0)
            {
                engine.LowSpeedChannel = 1;
                engine.HighSpeedChannel = 0;
            }
            else
            {
                engine.LowSpeedChannel = 0;
                engine.HighSpeedChannel = 1;
            }
            
           

            factor_temp = (MotorSpeed - MixerChannel[engine.LowSpeedChannel].OriginSpeed) * PERCENT_MIXER_BASE / 
                                  (engine.UnMixSpeed - MixerChannel[engine.LowSpeedChannel].OriginSpeed);
            MixerChannel[engine.LowSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE - ((uint32_t)((uint32_t)factor_temp * (uint32_t)factor_temp) >> PERCENT_MIXER_BASE_SHIFT);
            MixerChannel[engine.HighSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE - (((uint32_t)(PERCENT_MIXER_BASE - factor_temp) * (uint32_t)(PERCENT_MIXER_BASE - factor_temp)) >> PERCENT_MIXER_BASE_SHIFT);  
            //MixerChannel[engine.LowSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE - factor_temp;
            //MixerChannel[engine.HighSpeedChannel].AmplitudeFactor = factor_temp;
            
            if(engine.LowSpeedChannel == 0)
            {
                MixerChannel[0].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.LowSpeedChannel].OriginSpeed / MotorSpeed;
                MixerChannel[1].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.HighSpeedChannel].OriginSpeed / MotorSpeed;     
            }
            else
            {
                MixerChannel[1].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.LowSpeedChannel].OriginSpeed / MotorSpeed;
                MixerChannel[0].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.HighSpeedChannel].OriginSpeed / MotorSpeed;
            }
        }
        //���ٶȱ�죬�����һ��ʱ
        else if(MixerChannel[engine.HighSpeedChannel].OriginSpeed == engine.ForwardSpeedArray[engine.SpeedStage - 1])
        {
            TryCloseAudioFile(&MixerChannel[engine.LowSpeedChannel]);
            OpenAudioFile(&MixerChannel[engine.LowSpeedChannel], engine.SpeedStage);
            if(engine.LowSpeedChannel == 0)
            {
                engine.LowSpeedChannel = 1;
                engine.HighSpeedChannel = 0;
            }
            else
            {
                engine.LowSpeedChannel = 0;
                engine.HighSpeedChannel = 1;
            }

            
            factor_temp = (MotorSpeed - MixerChannel[engine.LowSpeedChannel].OriginSpeed) * PERCENT_MIXER_BASE / 
                                  (engine.UnMixSpeed - MixerChannel[engine.LowSpeedChannel].OriginSpeed);
            MixerChannel[engine.LowSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE - ((uint32_t)((uint32_t)factor_temp * (uint32_t)factor_temp) >> PERCENT_MIXER_BASE_SHIFT);
            MixerChannel[engine.HighSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE - (((uint32_t)(PERCENT_MIXER_BASE - factor_temp) * (uint32_t)(PERCENT_MIXER_BASE - factor_temp)) >> PERCENT_MIXER_BASE_SHIFT);  
            //MixerChannel[engine.LowSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE - factor_temp;
            //MixerChannel[engine.HighSpeedChannel].AmplitudeFactor = factor_temp;
            
            if(engine.LowSpeedChannel == 0)
            {
                MixerChannel[0].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.LowSpeedChannel].OriginSpeed / MotorSpeed;
                MixerChannel[1].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.HighSpeedChannel].OriginSpeed / MotorSpeed; 
            }
            else
            {
                MixerChannel[1].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.LowSpeedChannel].OriginSpeed / MotorSpeed;
                MixerChannel[0].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.HighSpeedChannel].OriginSpeed / MotorSpeed; 
            }
        }
        //���ٶ�ͼƬʱ?Ӧ�ò��ᷢ��
        //����else���ᷢ��
        else
        {
            TryCloseAudioFile(&MixerChannel[engine.LowSpeedChannel]);
            TryCloseAudioFile(&MixerChannel[engine.HighSpeedChannel]);
            engine.LowSpeedChannel = 0;
            engine.HighSpeedChannel = 1;
            OpenAudioFile(&MixerChannel[engine.LowSpeedChannel], engine.SpeedStage - 1);
            OpenAudioFile(&MixerChannel[engine.HighSpeedChannel], engine.SpeedStage);


            factor_temp = (MotorSpeed - MixerChannel[engine.LowSpeedChannel].OriginSpeed) * PERCENT_MIXER_BASE / 
                                  (engine.UnMixSpeed - MixerChannel[engine.LowSpeedChannel].OriginSpeed);
            MixerChannel[engine.LowSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE - ((uint32_t)((uint32_t)factor_temp * (uint32_t)factor_temp) >> PERCENT_MIXER_BASE_SHIFT);
            MixerChannel[engine.HighSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE - (((uint32_t)(PERCENT_MIXER_BASE - factor_temp) * (uint32_t)(PERCENT_MIXER_BASE - factor_temp)) >> PERCENT_MIXER_BASE_SHIFT);  
            //MixerChannel[engine.LowSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE - factor_temp;
            //MixerChannel[engine.HighSpeedChannel].AmplitudeFactor = factor_temp;
            
            if(engine.LowSpeedChannel == 0)
            {
                MixerChannel[0].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.LowSpeedChannel].OriginSpeed / MotorSpeed;
                MixerChannel[1].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.HighSpeedChannel].OriginSpeed / MotorSpeed; 
            }
            else
            {
                MixerChannel[1].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.LowSpeedChannel].OriginSpeed / MotorSpeed;
                MixerChannel[0].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.HighSpeedChannel].OriginSpeed / MotorSpeed; 
            }
        }
    }
}


void SetMixerChannel(int16_t speed)
{
    //static int16_t LastSpeed = 0;
    //if(LastSpeed == speed)
    //{
    //    return;
    //}

    //LastSpeed = speed;
    SetMixerStatus(speed);
    MixAudio(speed);
}


/******************************������������***********************************/
uint32_t EngineStartHandler()
{

    int i;

    if(engine.StartAudioStartAddr == 0 && engine.StartAudioDataCnt == 0)
    {
        return 0;
    }

    //��ͼ�رջ���ͨ��
    for(i = 0;  i < ENGINE_MIXER_CHANNEL_NUM; i++)
    {
        //���ڳ�ʼ�ٶȽ������㣬�´ν������ʱ�������¿�ʼ�������ļ���
        //������������ļ��ظ��򿪵�����
        MixerChannel[i].OriginSpeed = 0;
        //��ͼ�رջ���ͨ������Ƶ�ļ������ԭ��Ӧ���Ѿ������˹ر�
        TryCloseAudioFile(&MixerChannel[i]);
    }

    EngineStartStopVoiceChannel.Fp = OpenFile(ENGINE_INDEX(mixer.SoundIndex));

    //�Ի���ͨ�������ݽ��ж�ȡ������������������ɺ��ܹ����̲���,��ȡ��ɺ��ͨ���������㣬���򲥷�����ǰ��������
    //���湤��������
    //SetMixerChannel(GetMotorSpeed());
    //��ȡ����ٵ�����
    //ʹ�������+1���ٶȣ���Ϊ���ܹ���һʱ���ȡ�����ͨ�������ݣ�����ӵ������ϼ�ʱ������һ��
    SetMixerChannel(engine.ForwardSpeedArray[0] + 1);
    if(MixerChannel[0].AmplitudeFactor != 0)
    {
        //����ͨ���ı��ض�����Ϊ0��������
        MixerChannel[0].AmplitudeFactor = 0;
        AudioFileReadHandler(&MixerChannel[0]);
        AudioFileReadHandler(&MixerChannel[0]);
        AudioFileReadHandler(&MixerChannel[0]);
    }
    if(MixerChannel[1].AmplitudeFactor != 0)
    {
        //����ͨ���ı��ض�����Ϊ0��������
        MixerChannel[1].AmplitudeFactor = 0;
        AudioFileReadHandler(&MixerChannel[1]);
        AudioFileReadHandler(&MixerChannel[1]);
        AudioFileReadHandler(&MixerChannel[1]);
    }
    //������ͨ���Ŀ��Ʊ������г�ʼ��

    EngineStartStopVoiceChannel.AudioStartAddress = engine.StartAudioStartAddr;
    EngineStartStopVoiceChannel.TotalDataCnt = engine.StartAudioDataCnt;
    

    EngineStartStopVoiceChannel.NowBuffFlag = 0;
    EngineStartStopVoiceChannel.ReadFileCnt = 0;
    EngineStartStopVoiceChannel.NowBuffFlag = 0;
    EngineStartStopVoiceChannel.Buff0ReadyFlag = 0;
    EngineStartStopVoiceChannel.Buff1ReadyFlag = 0;
    EngineStartStopVoiceChannel.Buff2ReadyFlag = 0;
    EngineStartStopVoiceChannel.DataPtr = 0;
    EngineStartStopVoiceChannel.ReadFileCnt = 0;
    EngineStartStopVoiceChannel.SoundData = 0;
    EngineStartStopVoiceChannel.AmplitudeFactor = PERCENT_MIXER_BASE;

    //���ò���Ƶ��Ϊ44100
    //����ʱ����ֵ�ݴ棬��ֵ����������ʱ��ͨ��һ�Ĳ�������
    //������������������ɺ�ָ���
    //Engine.Tim7ArrBackup = MIXER_CHANNEL1_TIMER->ARR;
    //����ͨ���ı��ض�����Ϊ0��������
    //Engine.AmplitudeFactorCh0Backup = MixerChannel[0].AmplitudeFactor;
    //Engine.AmplitudeFactorCh1Backup = MixerChannel[1].AmplitudeFactor;
    //MIXER_CHANNEL1_TIMER.p_reg->CC[0] = (uint32_t)STANDARD_ARR_VALUE;
    EngineStartStopVoiceChannel.TimerInterval = (1000000000 / engine.SampleRate);
    MixerChannel[0].SoundData = 0;
    MixerChannel[1].SoundData = 0;
    

    engine.status = ENGINE_STARTING;

    AudioFileReadHandler(&EngineStartStopVoiceChannel);
    AudioFileReadHandler(&EngineStartStopVoiceChannel);
    AudioFileReadHandler(&EngineStartStopVoiceChannel);

    //��ʼ������ʵ���ٶ�ֵΪ�����ٶȵ���ֵ
    //MotorSpeed.RealSpeed = engine.ForwardSpeedArray[0] * 9 / 10;
    //MotorSpeed.RealSpeed = engine.ForwardSpeedArray[0];

    
    return 0;
}


void EngineStartingHandler()
{
    uint32_t distance;
    AudioFileReadHandler(&EngineStartStopVoiceChannel);
    AudioFileReadHandler(&MixerChannel[0]);
    //����һ����ɺ󣬶�ȡָ��ָ���ļ�0ƫ�ƴ������ʾ�������
    if(EngineStartStopVoiceChannel.ReadFileCnt == 0)
    {
        //��ԭ����ͨ��1��Ƶ��
        //MIXER_CHANNEL1_TIMER->ARR = Engine.Tim7ArrBackup;
        //��ԭԭ�Ȼ���ͨ���ı���
        //MixerChannel[0].AmplitudeFactor = Engine.AmplitudeFactorCh0Backup;
        //MixerChannel[1].AmplitudeFactor = Engine.AmplitudeFactorCh1Backup;
        //�趨����ͨ�����ٶȣ���ֹ�ڼ������ʱ�ٶȲ�ƥ�䵼����������
        //MixerChannel[0].OriginSpeed = 0;
        //MixerChannel[1].OriginSpeed = MixerStatus.ForwardSpeedArray[0];
        engine.status = ENGINE_RUNNING;

        //������ͨ���Ŀ��Ʊ������г�ʼ��
        EngineStartStopVoiceChannel.NowBuffFlag = 0;
        EngineStartStopVoiceChannel.ReadFileCnt = 0;
        EngineStartStopVoiceChannel.NowBuffFlag = 0;
        EngineStartStopVoiceChannel.Buff0ReadyFlag = 0;
        EngineStartStopVoiceChannel.Buff1ReadyFlag = 0;
        EngineStartStopVoiceChannel.Buff2ReadyFlag = 0;
        EngineStartStopVoiceChannel.DataPtr = 0;
        EngineStartStopVoiceChannel.ReadFileCnt = 0;
        EngineStartStopVoiceChannel.SoundData = 0;

        //���õ�ǰ�ٶ�Ϊ����
        //���������������ɵ������У����ݷ��ż��ٵ���Ӧ�ٶ�
        //MotorSpeed.RealSpeed = 0;//SystemConfig.LowSpeed;
        MotorSpeed.RealSpeed = engine.ForwardLowSpeed;
        engine.InFlameOutTime = GetSystemTime();
        //MotorSpeed.RealSpeedChangeFlag = 1;

        //���¾���ʱ�䣬��ֹ�����������ڵ�������ʱ��ֱ�ӽ���stop
        engine.InFlameOutTime = GetSystemTime();
    }
    else if(EngineStartStopVoiceChannel.ReadFileCnt >= ((EngineStartStopVoiceChannel.TotalDataCnt >> 3) * 6))
    {
        //����������3/4����ʼ���뵡�ٵ�����
        distance = EngineStartStopVoiceChannel.ReadFileCnt - ((EngineStartStopVoiceChannel.TotalDataCnt >> 3) * 6);
        //�˴�distance*10000��������������ʽ��Ŵ�1000��
        MixerChannel[engine.LowSpeedChannel].AmplitudeFactor = (distance * 512 / 
                                                                                    (EngineStartStopVoiceChannel.TotalDataCnt - ((EngineStartStopVoiceChannel.TotalDataCnt >> 3) * 6))) * 16;                        
                                                                                    
        EngineStartStopVoiceChannel.AmplitudeFactor = (PERCENT_MIXER_BASE - MixerChannel[engine.LowSpeedChannel].AmplitudeFactor);
    }
}


/**********************************����Ϩ����******************************************/
uint32_t EngineStopHandlerWithoutStopFile()
{
    MixerChannel[engine.LowSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE;
    MixerChannel[engine.HighSpeedChannel].AmplitudeFactor = 0;

    MixerChannel[engine.HighSpeedChannel].SoundData = 0;

    engine.status = ENGINE_STOPING;
    engine.StopTime = 0;
    return 0;
}

void EngineStopingHandlerWithoutStopFile()
{
    static uint32_t StartStopTime = 0;
    uint32_t SpeedTemp;
    AudioFileReadHandler(&MixerChannel[engine.LowSpeedChannel]);
    if(engine.StopTime == 0)
    {
        //���Źر�����ʱ���ν���
        StartStopTime = GetSystemTime() - 1;
        engine.StopTime  = 1;
    }
    else
    {
        engine.StopTime = GetSystemTime() - StartStopTime;
        if(engine.StopTime < ENGINE_STOP_LAST_TIME)
        {
            //����ֹͣ��������������
            //�����𽥼���
            //percent = 
            MixerChannel[engine.LowSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE - engine.StopTime * PERCENT_MIXER_BASE / ENGINE_STOP_LAST_TIME;
            SpeedTemp = engine.StopTime * ((uint32_t)engine.ForwardSpeedArray[0] * 4 / 5 - ENGINE_STOP_MIN_SPEED);
            SpeedTemp = SpeedTemp / ENGINE_STOP_LAST_TIME;
            SpeedTemp = engine.ForwardSpeedArray[0] * 4 / 5 - SpeedTemp;
            if(engine.LowSpeedChannel == 0)
            {
                MixerChannel[0].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.LowSpeedChannel].OriginSpeed / SpeedTemp;
            }
            else
            {
                MixerChannel[1].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.LowSpeedChannel].OriginSpeed / SpeedTemp;
            }
        }
        else
        {
            //����ֹͣ�����������
            engine.status = ENGINE_STOP;
            TryCloseAudioFile(&MixerChannel[0]);
            TryCloseAudioFile(&MixerChannel[1]);
            MixerChannel[0].OriginSpeed = 0;
            MixerChannel[1].OriginSpeed = 0;
            StopMixer();   
        }
    }
    
    
}


uint32_t EngineStopHandlerWithStopFile()
{
    EngineStartStopVoiceChannel.Fp = OpenFile(ENGINE_INDEX(mixer.SoundIndex));
    EngineStartStopVoiceChannel.TotalDataCnt = engine.StopAudioDataCnt;
    EngineStartStopVoiceChannel.AudioStartAddress= engine.StopAudioStartAddr;
    EngineStartStopVoiceChannel.NowBuffFlag = 0;
    EngineStartStopVoiceChannel.ReadFileCnt = 0;
    EngineStartStopVoiceChannel.NowBuffFlag = 0;
    EngineStartStopVoiceChannel.Buff0ReadyFlag = 0;
    EngineStartStopVoiceChannel.Buff1ReadyFlag = 0;
    EngineStartStopVoiceChannel.Buff2ReadyFlag = 0;
    EngineStartStopVoiceChannel.DataPtr = 0;
    EngineStartStopVoiceChannel.ReadFileCnt = 0;
    EngineStartStopVoiceChannel.SoundData = 0;
    EngineStartStopVoiceChannel.AmplitudeFactor = PERCENT_MIXER_BASE;

    //��ȡ����رյ��������ݣ���������2��ͨ����3�����壬����
    //ʱ���㹻�˴���ȡ��
    AudioFileReadHandler(&EngineStartStopVoiceChannel);
    AudioFileReadHandler(&EngineStartStopVoiceChannel);
    AudioFileReadHandler(&EngineStartStopVoiceChannel);

    engine.status = ENGINE_STOPING;
    MixerChannel[0].AmplitudeFactor = 0;
    MixerChannel[1].AmplitudeFactor = 0;
    //MIXER_CHANNEL1_TIMER.p_reg->CC[0] = (uint32_t)STANDARD_ARR_VALUE;
    EngineStartStopVoiceChannel.TimerInterval = 1000000000 / engine.SampleRate;


    EngineStartStopVoiceChannel.AmplitudeFactor = PERCENT_MIXER_BASE;
    
    MixerChannel[engine.LowSpeedChannel].SoundData = 0;
    MixerChannel[engine.HighSpeedChannel].SoundData = 0;

    engine.status = ENGINE_STOPING;
    engine.StopTime = 0;
    return 0;
}


void EngineStopingHandlerWithStopFile()
{
    AudioFileReadHandler(&EngineStartStopVoiceChannel);
    if(EngineStartStopVoiceChannel.ReadFileCnt == 0)
    {
        engine.status = ENGINE_STOP;
        TryCloseAudioFile(&MixerChannel[0]);
        TryCloseAudioFile(&MixerChannel[1]);
        MixerChannel[0].OriginSpeed = 0;
        MixerChannel[1].OriginSpeed = 0;
        EngineStartStopVoiceChannel.SoundData = 0;
        StopMixer();    

    }
    
}


/******************************����״̬��״̬�ж�***********************************/
void EngineStatusHandler()
{
     if(engine.status == ENGINE_STOP)
    {
        //���浱ǰ״̬Ϊ�ر�
        //�����ٶȴ��ھ�Ĭ�ٶ�һ��ֵʱ����������
        //if(IsTurnOverStart() ==   true && mixer.MixerEnableFlag == ENABLE)
        //�ϵ������
        //if(mixer.MixerEnableFlag == ENABLE)
        //if(IsTurnOverStart() ==   true && mixer.MixerEnableFlag == ENABLE)
        if((MotorSpeed.DistSpeed > engine.ForwardLowSpeed || MotorSpeed.EvSpeedLevel != EV_SPEED_HOLD)
            && mixer.MixerEnableFlag != DISABLE)
        {
            mixer.VolumeSlopePercent = CalVolumeSlopePercent(engine.ForwardLowSpeed);
            EngineStartHandler();
        }
    }
    else if(engine.status == ENGINE_STARTING)
    {
        //������������
        
        EngineStartingHandler();
        
    }
    else if(engine.status == ENGINE_STOPING)
    {
        //����Ϩ�����
        if(engine.StopAudioDataCnt == 0)
        {
            EngineStopingHandlerWithoutStopFile();
        }
        else
        {
            EngineStopingHandlerWithStopFile();
        }
    }
    else
    {
        SetMixerChannel(MotorSpeed.RealSpeed);
        
        //���浱ǰ״̬Ϊ����״̬

           //�ڵ������䣬һ��ʱ���ٶ�û�䣬��Ϩ��

           
           //����Ϩ��

            if(engine.FlameOutSwitch != DISABLE //���趨�˲�Ϩ����ֱ����ת��else��
              && MotorSpeed.VehicleStatu != BREAK
              //&& IsTurnBelowStop() == true
              && MotorSpeed.RealSpeed <= engine.ForwardLowSpeed
              && MotorSpeed.EvSpeedLevel == EV_SPEED_HOLD)//ֻ����HOLDʱ�Ż�Ϩ��
            {
                if(GetSystemTime() - engine.InFlameOutTime >= engine.FlameOutTime * TIMER_MULTI)
                {
                    //������һ��ʱ���ڶ���Ϩ���������
                    if(engine.StopAudioDataCnt == 0)
                    { 
                        EngineStopHandlerWithoutStopFile();
                    }
                    else
                    {
                        EngineStopHandlerWithStopFile();
                    }
                }
            }
            else
            {
                engine.InFlameOutTime = GetSystemTime();
                //MotorSpeed.SlientSpeed = MotorSpeed.RealSpeed;
            }


            //����������������ȡ����ͨ������������
            if(MixerChannel[0].AmplitudeFactor != 0)
                AudioFileReadHandler(&MixerChannel[0]);
            if(MixerChannel[1].AmplitudeFactor != 0)
                AudioFileReadHandler(&MixerChannel[1]);
    }
}


void EngineHandler()
{
    CalSpeedAndBrakeAndGearBoxHandler();
    EngineStatusHandler();
}




//uint32_t TimerCnt[2];
//uint32_t CCDistance[4];

///**************************�������ͨ�����жϴ���***********************************/
//void MixerChannel0IntHandler()
//{
//    
//    TIMER_MIXER_CHANNEL0->TASKS_CAPTURE[3] = 1;
//    CCDistance[0] = TIMER_MIXER_CHANNEL0->CC[3];
//    //CCDistance[0] = TIMER_MIXER_CHANNEL0->CC[1] - TIMER_MIXER_CHANNEL0->CC[0];
//    //MIXER_CHANNEL0_TIMER.p_reg->CC[0] = MIXER_CHANNEL0_TIMER.p_reg->CC[0] + MixerChannel[0].TimerInterval;
//    TIMER_MIXER_CHANNEL0->EVENTS_COMPARE[0] = 0;
//    TIMER_MIXER_CHANNEL0->EVENTS_COMPARE[1] = 0;
//    TIMER_MIXER_CHANNEL0->EVENTS_COMPARE[2] = 0;
//    
//    TIMER_MIXER_CHANNEL0->CC[0] = MixerChannel[0].TimerInterval;
//    //nrf_timer_event_clear(TIMER_MIXER_CHANNEL0, NRF_TIMER_EVENT_COMPARE0);
//    
//    AudioChannelGetDataInTimeInterruptWithSecure(&MixerChannel[0]);

//    AudioOutDac();
//    TimerCnt[0]++;
//    
//}


//void MixerChannel1IntHandler()
//{
//    TIMER_MIXER_CHANNEL1->TASKS_CAPTURE[3] = 1;
//    CCDistance[1] = TIMER_MIXER_CHANNEL1->CC[3];
//    TIMER_MIXER_CHANNEL1->EVENTS_COMPARE[0] = 0;
//    TIMER_MIXER_CHANNEL1->EVENTS_COMPARE[1] = 0;
//    TIMER_MIXER_CHANNEL1->EVENTS_COMPARE[2] = 0;
//    //nrf_timer_event_clear(TIMER_MIXER_CHANNEL1, NRF_TIMER_EVENT_COMPARE0);
//    if(engine.status == ENGINE_RUNNING || (engine.status == ENGINE_STOPING && engine.StopAudioDataCnt == 0))
//    {
//        TIMER_MIXER_CHANNEL1->CC[0] = MixerChannel[1].TimerInterval;
//        //MIXER_CHANNEL1_TIMER.p_reg->CC[0] = MIXER_CHANNEL1_TIMER.p_reg->CC[0] + MixerChannel[1].TimerInterval;
//        //��������ʱ�����߲���Ϩ��������Ϩ�������ɵ��ٺϳ�
//        AudioChannelGetDataInTimeInterruptWithSecure(&MixerChannel[1]);
//    }
//    else if(engine.status == ENGINE_STARTING || (engine.status == ENGINE_STOPING && engine.StopAudioDataCnt != 0))
//    {
//        TIMER_MIXER_CHANNEL1->CC[0] = EngineStartStopVoiceChannel.TimerInterval;
//        //MIXER_CHANNEL1_TIMER.p_reg->CC[0] = MIXER_CHANNEL1_TIMER.p_reg->CC[0] + EngineStartStopVoiceChannel.TimerInterval;
//        //��������ʱ�����߲���Ϩ��������Ϩ�������ж�������
//        AudioChannelGetDataInTimeInterruptWithSecure(&EngineStartStopVoiceChannel);
//    }
//    else
//    {
//        TIMER_MIXER_CHANNEL1->CC[0] = STANDARD_ARR_VALUE;
//        //MIXER_CHANNEL1_TIMER.p_reg->CC[0] = MIXER_CHANNEL1_TIMER.p_reg->CC[0] + STANDARD_ARR_VALUE;
//        MixerChannel[1].SoundData = 0;
//        EngineStartStopVoiceChannel.SoundData = 0;
//    }

//    AudioOutDac();
//    TimerCnt[1]++;

//    //TimerCC[TimerPtr] = MIXER_CHANNEL1_TIMER.p_reg->CC[0];
//    //TimerPtr++;
//}


//int32_t GetEngineAudioData()
//{
//    int32_t data;
//    if(engine.status == ENGINE_RUNNING || (engine.status == ENGINE_STOPING && engine.StopAudioDataCnt == 0))
//    {
//        //������������������������Ϩ����������û��Ϩ��������Ϩ������ͨ�����ٺϳ�
//        data = (int32_t)MixerChannel[0].SoundData + (int32_t)MixerChannel[1].SoundData;
//    }
//    else if(engine.status == ENGINE_STOP)
//    {
//        data = 0;
//    }
//    else
//    {
//        data = (int32_t)EngineStartStopVoiceChannel.SoundData + (int32_t)MixerChannel[0].SoundData;
//    }
//    data = data * mixer.VolumeSlopePercent / 10000;
//    return data;
//}

//void EnableMixerChannelTimer()
//{
//    nrf_drv_timer_enable(&MIXER_CHANNEL0_TIMER);    
//    nrf_drv_timer_enable(&MIXER_CHANNEL1_TIMER);    
//}

//void DisableMixerChannelTimer()
//{
//    nrf_drv_timer_disable(&MIXER_CHANNEL0_TIMER);    
//    nrf_drv_timer_disable(&MIXER_CHANNEL1_TIMER);    
//}


