#ifndef __DOWNLOADER__
#define __DOWNLOADER__
#include "ActionTick.h"
#include "fs.h"

//��������Ϣ�����ռ����
#define OFFSET_DOWNLOADER_FILE_TYPE					0
#define LENGTH_DOWNLOADER_FILE_TYPE					1

#define OFFSET_DOWNLOADER_FILE_LENGTH					(OFFSET_DOWNLOADER_FILE_TYPE + LENGTH_DOWNLOADER_FILE_TYPE)
#define LENGTH_DOWNLOADER_FILE_LENGTH					LENGTH_FILE_LENGTH//  4

#define OFFSET_DOWNLOADER_VEHICLE_INDEX				(OFFSET_DOWNLOADER_FILE_LENGTH + LENGTH_DOWNLOADER_FILE_LENGTH)
#define LENGTH_DOWNLOADER_VEHICLE_INDEX				1

#define OFFSET_DOWNLOADER_SOUND_TYPE					(OFFSET_DOWNLOADER_VEHICLE_INDEX + LENGTH_DOWNLOADER_VEHICLE_INDEX)
#define LENGTH_DOWNLOADER_SOUND_TYPE					1

#define OFFSET_DOWNLOADER_GUID						(OFFSET_DOWNLOADER_SOUND_TYPE + LENGTH_DOWNLOADER_SOUND_TYPE)
#define LENGHT_DOWNLOADER_GUID						16

#define OFFSET_DOWNLOADER_SOUND_NAME				(OFFSET_DOWNLOADER_GUID + LENGHT_DOWNLOADER_GUID)
#define LENGTH_DOWNLOADER_SOUND_NAME_MAX			SIZE_SOUND_NAME//40

#define OFFSET_DOWNLOADER_INDEX_READY_FLAG			512

#define OFFSET_DOWNLOAD_FINISH_FLAG					3072
#define LENGTH_DOWNLOAD_FINISH_FLAG					1

#define OFFSET_AUDIO_CONVERT_FLAG						(OFFSET_DOWNLOAD_FINISH_FLAG + LENGTH_DOWNLOAD_FINISH_FLAG)
#define LENGTH_AUDIO_CONVERT_FLAG						1


#define SIZE_DOWNLOADER_AREA				512 * 1024


#define DOWNLOAD_TIME_OUT_TIME			3000 * TIMER_MULTI

int32_t WriteSoundStart(uint8_t *buff, uint32_t length);
void DownloaderInit(void);
int32_t GetNextSoundBlockIndex(void);
int32_t WriteSoundBlockData(uint8_t *buff, uint32_t length);
int32_t CheckSoundBlockData(uint8_t BlockIndex, uint8_t *LostPacketIndex, uint32_t CheckSum, bool CheckSumValid);
int32_t WriteSoundFinish(void);
FunctionalState IsAudioDownloading(void);
uint32_t GetLastDownloadMessageTime(void);
void StopDonwload(void);
void SetDlPacketDataCnt(uint16_t DataLength);


#endif

