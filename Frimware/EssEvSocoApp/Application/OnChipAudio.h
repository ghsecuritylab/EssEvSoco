#ifndef __ON_CHIP_AUIOD__
#define __ON_CHIP_AUIOD__


#define MENU_SHORT_DI_CNT				4
#define MENU_LONG_DI_CNT				12

struct OnChipPlayInfo
{
    s16 AudioData;
    u32 addr;
    u32 DataCnt;
    u32 PlayedAudioDataCnt;//�Ѳ��ŵ�����
    u8 OneDataCnt;//����Ƭ������Ϊ8k��ʵ�����Ƶ��Ϊ44.1K���øñ�����¼ͬһ�������ͳ�ȥ���Σ�һ�������ܹ����5��
    u8 OnePlayCnt;//Ƭ�������������ŵĴ���,���ڿ��������ĳ���
    u8 OnePlayCntBak;
    u16 IntervalPlayCnt;//Ƭ������������ŵĴ���
    s16 MaxAudioData;
    u16 Multi;
    u16 PlayInterval;
};



s32 GetOnChipAudioData(void);
void PlaySystemStartBeep(unsigned char EngineIndex);
u32 OnChipAudioPlay(u8 OnePlayCnt, u16 IntervalPlayCnt, u16 Interval);
u16 CheckOnChipAudioIsPlaying(void);
s32 GetCurrentOnChipAudioData(void);

#endif


