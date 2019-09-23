#include "common.h"
#include "nrf_drv_i2s.h"
#include "AudioPlay.h"
#include "mixer.h"
#include "engine.h"
#include "TestModel.h"
#include "OnChipAudio.h"


struct AudioOutInfo AudioOut;


void I2sInit(void);

void AudioOutVarInit()
{
    AudioOut.I2sSendingBuffPtr = AudioOut.I2sBuff[1];
    AudioOut.I2sFillingBuffPtr = AudioOut.I2sBuff[0];
    //��ʼ����0���ڵ�һ�μ����ʱ�򣬻�ƥ�䵽��0�����ݣ�Ȼ��ʼ����������
    AudioOut.AudioNodeCnt[0] = AudioOut.AudioNodeCnt[1] = 1;
}

static void i2s_data_handler(nrf_drv_i2s_buffers_t const * p_released,
                         uint32_t                      status)
{
    //������д�����е�����ʱ���
    uint64_t NowTimeStamp = AudioOut.timeStamp[AUDIO_OUT_BUFF_SIZE - 1];
    uint64_t *TimeStampPtr = AudioOut.timeStamp;
    for(uint32_t i = 0; i < AUDIO_OUT_BUFF_SIZE; i++)
    {
        NowTimeStamp += AUDIO_OUT_INTERVAL;
        *TimeStampPtr++ = NowTimeStamp;
    }
    uint64_t EndTimeStamp = NowTimeStamp;


    /********************����CH0������׼��***********************/
    int16_t NodePosition = AudioOut.AudioNodeCnt[0] - 1;//�ϴ����һ�����ݵ�λ��
    struct AudioNodeInfo *NodePtr = &AudioOut.AudioNode[0][NodePosition];//�ϴ����һ���ڵ�
    //���ݵ�ǰ��timeStamp�е�һ��ʱ��㣬Ѱ���ϴ�CH0�����һ����ǰ�ڸ�ʱ���Ľڵ�
    while(NodePosition >= 0)
    {
        if(NodePtr->time <= AudioOut.timeStamp[0])
        {
            //�ҵ��ڵ�
            break;
        }
        //û���ҵ�����ǰ��һ���ڵ�
        NodePosition--;
        NodePtr--;
    }
    //���ҵ�������ڵ��𣬽�֮��Ľڵ㸴�Ƶ�����CH0��ͷ��
    for(uint32_t i = 0; i < AudioOut.AudioNodeCnt[0] - NodePosition; i++)
    {
        AudioOut.AudioNode[0][i] = *NodePtr++;
    }
    AudioOut.AudioNodeCnt[0] = AudioOut.AudioNodeCnt[0] - NodePosition;//��¼���ƹ����Ľڵ�����
    //����CH0�µĽڵ��ʱ����㼰���ݶ�ȡ
    NowTimeStamp = AudioOut.AudioNode[0][AudioOut.AudioNodeCnt[0] - 1].time;
    NodePtr = &AudioOut.AudioNode[0][AudioOut.AudioNodeCnt[0]];
    do
    {
        //����һ�����ݵĶ�ȡ
        AudioChannelGetDataInTimeInterruptWithSecure(&MixerChannel[0]);
        NodePtr->data = MixerChannel[0].SoundData;
        //��¼���ݵ�ʱ���
        NowTimeStamp += MixerChannel[0].TimerInterval;
        NodePtr->time = NowTimeStamp;

        //��¼+1
        AudioOut.AudioNodeCnt[0]++;
        NodePtr++;
    }while(NowTimeStamp < EndTimeStamp);//ֱ��ʱ���timeStamp���һ���ڵ��ʱ�����

    /*******************����CH1������׼��************************/
    NodePosition = AudioOut.AudioNodeCnt[1] - 1;//�ϴ����һ�����ݵ�λ��
    NodePtr = &AudioOut.AudioNode[1][NodePosition];//�ϴ����һ���ڵ�
    //���ݵ�ǰ��timeStamp�е�һ��ʱ��㣬Ѱ���ϴ�CH0�����һ����ǰ�ڸ�ʱ���Ľڵ�
    while(NodePosition >= 0)
    {
        if(NodePtr->time <= AudioOut.timeStamp[0])
        {
            //�ҵ��ڵ�
            break;
        }
        //û���ҵ�����ǰ��һ���ڵ�
        NodePosition--;
        NodePtr--;
    }
    //���ҵ�������ڵ��𣬽�֮��Ľڵ㸴�Ƶ�����CH0��ͷ��
    for(uint32_t i = 0; i < AudioOut.AudioNodeCnt[1] - NodePosition; i++)
    {
        AudioOut.AudioNode[1][i] = *NodePtr++;
    }
    AudioOut.AudioNodeCnt[1] = AudioOut.AudioNodeCnt[1] - NodePosition;//��¼���ƹ����Ľڵ�����
    //����CH1�µĽڵ��ʱ����㼰���ݶ�ȡ
    NowTimeStamp = AudioOut.AudioNode[1][AudioOut.AudioNodeCnt[1] - 1].time;
    NodePtr = &AudioOut.AudioNode[1][AudioOut.AudioNodeCnt[1]];
    do
    {
        //����һ�����ݵĶ�ȡ
        uint32_t interval;
        if(engine.status == ENGINE_RUNNING || (engine.status == ENGINE_STOPING && engine.StopAudioDataCnt == 0))
        {
            interval = MixerChannel[1].TimerInterval;
            //MIXER_CHANNEL1_TIMER.p_reg->CC[0] = MIXER_CHANNEL1_TIMER.p_reg->CC[0] + MixerChannel[1].TimerInterval;
            //��������ʱ�����߲���Ϩ��������Ϩ�������ɵ��ٺϳ�
            AudioChannelGetDataInTimeInterruptWithSecure(&MixerChannel[1]);
            NodePtr->data = MixerChannel[1].SoundData;
        }
        else if(engine.status == ENGINE_STARTING || (engine.status == ENGINE_STOPING && engine.StopAudioDataCnt != 0))
        {
            interval = EngineStartStopVoiceChannel.TimerInterval;
            //MIXER_CHANNEL1_TIMER.p_reg->CC[0] = MIXER_CHANNEL1_TIMER.p_reg->CC[0] + EngineStartStopVoiceChannel.TimerInterval;
            //��������ʱ�����߲���Ϩ��������Ϩ�������ж�������
            AudioChannelGetDataInTimeInterruptWithSecure(&EngineStartStopVoiceChannel);
            NodePtr->data = EngineStartStopVoiceChannel.SoundData;
        }
        else
        {
            interval = (1000000000 / engine.SampleRate);
            //MIXER_CHANNEL1_TIMER.p_reg->CC[0] = MIXER_CHANNEL1_TIMER.p_reg->CC[0] + STANDARD_ARR_VALUE;
            MixerChannel[1].SoundData = 0;
            EngineStartStopVoiceChannel.SoundData = 0;
            NodePtr->data = 0;
        }
        
        //��¼���ݵ�ʱ���
        NowTimeStamp += interval;
        NodePtr->time = NowTimeStamp;

        //��¼+1
        AudioOut.AudioNodeCnt[1]++;
        NodePtr++;
    }while(NowTimeStamp < EndTimeStamp);//ֱ��ʱ���timeStamp���һ���ڵ��ʱ�����

    //���б��λ������ݵ�����
    struct AudioNodeInfo *NodePtrCh0, *NodePtrCh1, *NextNodePtrCh0, *NextNodePtrCh1;
    NodePtrCh0 = &AudioOut.AudioNode[0][0];
    NodePtrCh1 = &AudioOut.AudioNode[1][0];
    NextNodePtrCh0 = &AudioOut.AudioNode[0][1];
    NextNodePtrCh1 = &AudioOut.AudioNode[1][1];
    for(uint32_t i = 0; i < AUDIO_OUT_BUFF_SIZE; i++)
    {
        uint64_t ThisTime = AudioOut.timeStamp[i];
        //Ѱ��CH0���ʵ�node���ǰ�������Ƶʱ�������2��node֮��
        while(1)
        {
            if(NodePtrCh0->time <= ThisTime && NextNodePtrCh0->time >= ThisTime)
            {
                break;
            }
            NodePtrCh0++;
            NextNodePtrCh0++;
        }
        //Ѱ��CH1���ʵ�node���ǰ�������Ƶʱ�������2��node֮��
        while(1)
        {
            if(NodePtrCh1->time <= ThisTime && NextNodePtrCh1->time >= ThisTime)
            {
                break;
            }
            NodePtrCh1++;
            NextNodePtrCh1++;
        }
        //����CH0��CH1�ڵ�ǰ�����Ƶʱ������Ƶ����
        int32_t percent;
        int32_t data0, data1;
        int32_t DataOnchip;
        uint32_t TimeDistanceNodeToThisTime, TimeDisatanceTwoNode;
        TimeDistanceNodeToThisTime = ThisTime - NodePtrCh0->time;
        TimeDisatanceTwoNode = NextNodePtrCh0->time - NodePtrCh0->time;
        percent = (TimeDistanceNodeToThisTime) * 10000 / (TimeDisatanceTwoNode);
        data0 = NodePtrCh0->data + (NextNodePtrCh0->data - NodePtrCh0->data) * percent / 10000;
        TimeDistanceNodeToThisTime = ThisTime - NodePtrCh1->time;
        TimeDisatanceTwoNode = NextNodePtrCh1->time - NodePtrCh1->time;
        percent = (TimeDistanceNodeToThisTime) * 10000 / (TimeDisatanceTwoNode);
        data1 = NodePtrCh1->data + (NextNodePtrCh1->data - NodePtrCh1->data) * percent / 10000;
        DataOnchip = GetOnChipAudioData();
        int32_t data;
        data = data0 + data1 + DataOnchip;
        data = data * (int16_t)mixer.VolumeSlopePercent / 10000;
        if(data > 32767)
        {
            data = 32767;
        }
        else if(data < -32768)
        {
            data = -32768;
        }
        AudioOut.I2sFillingBuffPtr[i] = data << 16 | data;
        
    }


    //�����趨���ɺ󣬸������ͻ����������д�����ָ�룬������I2S�ķ������ݵ�ַ
    int32_t *PtrTmp;
    PtrTmp = AudioOut.I2sFillingBuffPtr;
    AudioOut.I2sFillingBuffPtr = AudioOut.I2sSendingBuffPtr;
    AudioOut.I2sSendingBuffPtr = PtrTmp;
    NRF_I2S->TXD.PTR = (uint32_t)AudioOut.I2sSendingBuffPtr;

}                         


//#define I2S_DATA_BLOCK_CNT          1
uint32_t I2sData;

void SetI2sData(uint16_t AudioData)
{
    I2sData = AudioData;
}


void I2sInit()
{
    nrf_drv_i2s_config_t config;
    uint32_t err_code = NRF_SUCCESS;

    
    
    // In Master mode the MCK frequency and the MCK/LRCK ratio should be
    // set properly in order to achieve desired audio sample rate (which
    // is equivalent to the LRCK frequency).
    // For the following settings we'll get the LRCK frequency equal to
    // 15873 Hz (the closest one to 16 kHz that is possible to achieve).
    config.sdin_pin  = I2S_CONFIG_SDIN_PIN;
    config.sdout_pin = I2S_CONFIG_SDOUT_PIN;
    config.mck_pin   = I2S_CONFIG_MCK_PIN;
    config.lrck_pin  = I2S_CONFIG_LRCK_PIN;
    config.sck_pin   = I2S_CONFIG_SCK_PIN;
    config.mck_setup = NRF_I2S_MCK_32MDIV11;
    config.ratio     = NRF_I2S_RATIO_64X;//32000000/11/64 = 45454Hz������
	
    config.channels  = NRF_I2S_CHANNELS_STEREO;
    config.mode      = NRF_I2S_MODE_MASTER;
    config.format    = NRF_I2S_FORMAT_I2S;
    config.irq_priority = IRQ_PROIRITY_I2S;
    config.sample_width = NRF_I2S_SWIDTH_16BIT;
    config.alignment = NRF_I2S_ALIGN_LEFT;
    
    
    err_code = nrf_drv_i2s_init(&config, i2s_data_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_i2s_buffers_t const initial_buffers = {
        .p_tx_buffer = (uint32_t const *)AudioOut.I2sSendingBuffPtr,
        .p_rx_buffer = NULL,
    };

    err_code = nrf_drv_i2s_start(&initial_buffers, AUDIO_OUT_BUFF_SIZE, 0);
    APP_ERROR_CHECK(err_code);
}

void I2sIntStop()
{
    nrf_i2s_int_disable(NRF_I2S, NRF_I2S_INT_STOPPED_MASK |
                                 NRF_I2S_INT_TXPTRUPD_MASK);
}

void I2sIntRestart()
{
    nrf_i2s_int_enable(NRF_I2S, NRF_I2S_INT_STOPPED_MASK |
                                 NRF_I2S_INT_TXPTRUPD_MASK);
}

void ClearAudioPlayBuff()
{
    int32_t i;
    for(i = 0; i < AUDIO_OUT_BUFF_SIZE; i++)
    {
        AudioOut.I2sBuff[0][i] = 0;
        AudioOut.I2sBuff[1][i] = 0;
    }
}



