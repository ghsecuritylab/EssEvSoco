#ifndef __MP3_CONVERT__
#define __MP3_CONVERT__


struct Mp3FormatInfo
{
    uint32_t SampleRate;//�����ʡ���λ��HZ
    uint16_t BitRate;//MP3��������λ��kbps
};


int32_t DecryptionConvertMp3(struct FileInfo *DistFp, uint32_t StartAddr, uint32_t FileLength);
int32_t GetAudioMp3Format(uint32_t StartAddr, uint32_t FileLength, struct Mp3FormatInfo *Mp3Format);


#endif


