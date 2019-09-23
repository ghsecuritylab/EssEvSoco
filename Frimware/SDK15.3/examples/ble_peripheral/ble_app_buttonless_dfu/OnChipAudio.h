#ifndef __ON_CHIP_AUIOD__
#define __ON_CHIP_AUIOD__


#define MENU_SHORT_DI_CNT				4
#define MENU_LONG_DI_CNT				12

struct OnChipPlayInfo
{
    int16_t AudioData;
    uint32_t addr;
    uint32_t DataCnt;
    uint32_t PlayedAudioDataCnt;//�Ѳ��ŵ�����
    uint8_t OneDataCnt;//����Ƭ������Ϊ8k��ʵ�����Ƶ��Ϊ44.1K���øñ�����¼ͬһ�������ͳ�ȥ���Σ�һ�������ܹ����5��
    uint8_t OnePlayCnt;//Ƭ�������������ŵĴ���,���ڿ��������ĳ���
    uint8_t OnePlayCntBak;
    uint16_t IntervalPlayCnt;//Ƭ������������ŵĴ���
    int16_t MaxAudioData;
    uint16_t Multi;
    uint16_t PlayInterval;
};



int32_t GetOnChipAudioData(void);
void PlaySystemStartBeep(unsigned char EngineIndex);
uint32_t OnChipAudioPlay(uint8_t OnePlayCnt, uint16_t IntervalPlayCnt, uint16_t Interval);
uint16_t CheckOnChipAudioIsPlaying(void);
int32_t GetCurrentOnChipAudioData(void);

#endif


