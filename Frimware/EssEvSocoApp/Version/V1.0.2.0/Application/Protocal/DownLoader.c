#include <stdint.h>
#include <string.h>
#include "SystemInfo.h"
#include "SpiFlash.h"
#include "DownLoader.h"
#include "AudioFileConvert.h"
#include "ActionTick.h"




//Downloaderռ�õ�FLASH�ռ��С



//��������������
//#define DOWNLOADER_INFO_SECTOR_INDEX			0
//#define DOWNLOADER_CONTENT_SECTOR_INDEX		1


//�������������ݰ�����������
#ifdef PROTOCAL_DD
#define SIZE_ONE_PACKET_CONTENT			14
#define REPORT_CNT_PACKET_LOST				34
#else
#define SIZE_ONE_PACKET_CONTENT			19
#define REPORT_CNT_PACKET_LOST				19
#endif
//һ�����ݿ���������ݰ�����
#define PACKET_CNT_ONE_BLOCK				(BLOCK_SIZE_DATA_TRANSFER / SIZE_ONE_PACKET_CONTENT + 1)
#define DATA_CNT_LAST_PACKET				(BLOCK_SIZE_DATA_TRANSFER % SIZE_ONE_PACKET_CONTENT)

#define MAX_DOWNLOAD_BLOCK_CNT						256 //���ݿ��������


struct DownloaderInfo
{
    FunctionalState IsEnabled;
    uint8_t BlockBuff[BLOCK_SIZE_DATA_TRANSFER];
    uint8_t PacketReadyBuff[PACKET_CNT_ONE_BLOCK];//�����ݴ�������У�����ÿ�����ݰ��Ƿ���յ��ı�־�б�
    uint32_t TotalDataCnt;
    uint32_t TotalBlockCnt;//�ܹ���Ҫ���صĿ������
    uint32_t InfoAddr;
    uint32_t ContentAddr;
    uint32_t BlockIndexNow;
    uint32_t ThisBlockPacketCnt;//��ǰ���ݿ���������ݰ������������һ�����ݿ�ʱ�᲻ͬ
    //uint32_t CheckSum;
    uint32_t LastPacketTime;//�յ������һ���������ݰ���ʱ��
};

struct DownloaderInfo DL;

//for test
uint32_t DlStartTime;
uint32_t DlFinishTime;
uint32_t ConverFinishTime;
uint32_t DlTime;
uint32_t ConverTime;

//�����������ȡ��
//data:������
//divisor:����
uint32_t DivCeil(uint32_t data, uint32_t divisor)
{
    return (data + divisor - 1) / divisor;
}


void DownloaderInit()
{
    //��ʼ����������FLASH��ʼ�ռ�
    DL.InfoAddr = SpiFlash.FlashSize - SIZE_DOWNLOADER_AREA;
    DL.ContentAddr = DL.InfoAddr + SPI_FLASH_SECTOR_SIZE;
}

//��������������������������������
int32_t RestartDownloader()
{
    uint32_t EraseAddr;
    EraseAddr = DL.InfoAddr;
    while(EraseAddr < DL.InfoAddr + SIZE_DOWNLOADER_AREA)
    {
        BlockErase(EraseAddr);
        EraseAddr += WINBOND_FLASH_BLOCK_SIZE;
    }
    return 0;
}



int32_t WriteSoundStart(uint8_t *buff, uint32_t length)
{
    uint8_t TmpBuff[LENGTH_DOWNLOADER_FILE_LENGTH + LENGTH_DOWNLOADER_VEHICLE_INDEX + 
                              LENGTH_DOWNLOADER_SOUND_TYPE + LENGHT_DOWNLOADER_GUID + LENGTH_DOWNLOADER_SOUND_NAME_MAX + 1];
    int32_t CmpResultFileLength;
    int32_t CmpResultVehicleIndex;
    int32_t CmpResultOtherInfo;
    uint8_t FileType;
    uint32_t TotalDataCnt;
    uint32_t TotalBlockCnt;
    uint8_t VehicleIndex;
    uint8_t SoundType;
    const uint8_t ClearData = 0xFF;


    //��ȡ��ǰ�������е����� ,���������ļ����ͺ��ļ���Ϣ
    //�����ļ���Ϣ�ĳ��Ⱥͱ�����Ҫ���ص��ļ���Ϣ������ͬ
    DataFlashReadData(DL.InfoAddr + OFFSET_DOWNLOADER_FILE_TYPE, TmpBuff, length + 1);
    memcpy(&TotalDataCnt, buff + OFFSET_DOWNLOADER_FILE_LENGTH - 1, LENGTH_DOWNLOADER_FILE_LENGTH);
    memcpy(&VehicleIndex, buff + OFFSET_DOWNLOADER_VEHICLE_INDEX - 1, LENGTH_DOWNLOADER_VEHICLE_INDEX);
    memcpy(&SoundType, buff + OFFSET_DOWNLOADER_SOUND_TYPE - 1, LENGTH_DOWNLOADER_SOUND_TYPE);
    TotalBlockCnt = DivCeil(TotalDataCnt, BLOCK_SIZE_DATA_TRANSFER);
    if(TotalBlockCnt > MAX_DOWNLOAD_BLOCK_CNT)
    {
        return 1;
    }
    if(VehicleIndex > VEHICLE_CNT)
    {
        return 2;
    }
    if(SoundType > TYPE_ENGINE_SOUND)
    {
        return 3;
    }

    DlStartTime = GetSystemTime();

    //�Ƚϱ���������ļ����������е���Ϣ
    CmpResultFileLength = memcmp(TmpBuff + OFFSET_DOWNLOADER_FILE_LENGTH, buff + OFFSET_DOWNLOADER_FILE_LENGTH - 1, LENGTH_DOWNLOADER_FILE_LENGTH);
    CmpResultVehicleIndex = memcmp(TmpBuff + OFFSET_DOWNLOADER_VEHICLE_INDEX, buff + OFFSET_DOWNLOADER_VEHICLE_INDEX - 1, LENGTH_DOWNLOADER_VEHICLE_INDEX);
    CmpResultOtherInfo = memcmp(TmpBuff + OFFSET_DOWNLOADER_SOUND_TYPE, buff + OFFSET_DOWNLOADER_SOUND_TYPE - 1, length - LENGTH_DOWNLOADER_FILE_LENGTH - LENGTH_DOWNLOADER_VEHICLE_INDEX);
    FileType = TmpBuff[0];
    
    if(FileType != 0 || CmpResultFileLength != 0 || CmpResultOtherInfo != 0)
    {
        //�����������ص���Ϣ���������еĲ�ͬ
        //���������������صĲ��������ļ�
        //�����������������
        RestartDownloader();
        TmpBuff[0] = 0;//��ʶ�����ļ�����Ϊ�����ļ�
        memcpy(TmpBuff + 1, buff, length);
        DataFlashWriteData(DL.InfoAddr + OFFSET_DOWNLOADER_FILE_TYPE, TmpBuff, length + 1);
    }
    else if(CmpResultVehicleIndex != 0)
    {
        uint8_t VehicleIndex;
        VehicleIndex = buff[OFFSET_DOWNLOADER_VEHICLE_INDEX - 1];
        //�����ļ�����Ϣ��ȫ��ͬ
        //����Ҫ���ص�������VehicleIndex��ͬ
        //��Ȼ�ܹ��������أ����ڴ��޸��������е�VehicleIndex
        DataFlashWriteData(DL.InfoAddr + OFFSET_DOWNLOADER_VEHICLE_INDEX, 
                                         &VehicleIndex, LENGTH_DOWNLOADER_VEHICLE_INDEX);

        //���ת����ɵı�־λ
        DataFlashWriteData(DL.InfoAddr + OFFSET_AUDIO_CONVERT_FLAG,
                                          (uint8_t *)&ClearData, LENGTH_AUDIO_CONVERT_FLAG);
    }
    else
    {
        //������Ϣ��ƥ����
        //�ϴ�����ĳ��ԭ�������ж�
        //�����ظ���������
        //���Բ���Ҫ��DOWNLOADER_INFO_SECTOR���κθĶ���
    }

    DL.TotalDataCnt = TotalDataCnt;
    DL.TotalBlockCnt = TotalBlockCnt;
    DL.BlockIndexNow = 0;
    
    DL.IsEnabled = ENABLE;

    //DL.CheckSum = 0;

    DL.LastPacketTime = GetSystemTime();
    
    return 0;
}

//��ȡ��һ���������ݿ������
int32_t GetNextSoundBlockIndex()
{
    int32_t NextBlockIndex = 10000;
    uint8_t BlockUsage[MAX_DOWNLOAD_BLOCK_CNT];
    int32_t i;

    if(DL.IsEnabled == DISABLE)
        return -1;

    //��ȡ��������IndexReady�ı�����ÿ1�ֽڱ�ʾһ�������Ƿ���д�룬0��ʾ�Ѿ�д��
    DataFlashReadData(DL.InfoAddr + OFFSET_DOWNLOADER_INDEX_READY_FLAG, 
                                     BlockUsage, DL.TotalBlockCnt);

    //�ҵ���һ��û��д���Block
    for(i = 0; i < DL.TotalBlockCnt; i++)
    {
        if(BlockUsage[i] != 0)
        {
            NextBlockIndex = i;
            break;
        }
    }

    if(NextBlockIndex != 10000)
    {
        //�п���û�н��յ�BLOCK
        DL.BlockIndexNow = NextBlockIndex;
        //��ս��յ����ݰ����б�׼����ʼ�������ݰ�
        memset(DL.PacketReadyBuff, 0, PACKET_CNT_ONE_BLOCK);
        if(NextBlockIndex == DL.TotalBlockCnt - 1)
        {
            //��ǰ���ݿ������һ�����ݿ�
            if(DL.TotalDataCnt % BLOCK_SIZE_DATA_TRANSFER == 0)
            {
                DL.ThisBlockPacketCnt = DivCeil(BLOCK_SIZE_DATA_TRANSFER, SIZE_ONE_PACKET_CONTENT);
            }
            else
            {
                DL.ThisBlockPacketCnt = DivCeil(DL.TotalDataCnt % BLOCK_SIZE_DATA_TRANSFER, SIZE_ONE_PACKET_CONTENT);
            }
            
        }
        else
        {
            //��ǰ���ݿ鲻�����һ�����ݿ�
            DL.ThisBlockPacketCnt = DivCeil(BLOCK_SIZE_DATA_TRANSFER, SIZE_ONE_PACKET_CONTENT);
        }
    }


    DL.LastPacketTime = GetSystemTime();
    
    return NextBlockIndex;
}


int32_t WriteSoundBlockData(uint8_t *buff, uint32_t length)
{
    uint8_t Index;
    int32_t LastLength;
    if(DL.IsEnabled == DISABLE)
        return -1;

    Index = buff[0];
    //ȷ�����ݰ����ȺϷ�
    if(Index + 1 < DL.ThisBlockPacketCnt)
    {
        //��ǰ���ݰ����ǿ�����һ�����ݰ������ݰ����ȹ̶�
        if(length - 1 != SIZE_ONE_PACKET_CONTENT)
            return -2;
    }
    else if(Index + 1 == DL.ThisBlockPacketCnt)
    {
        //��ǰ�Ǳ�������һ�����ݰ�
        if(DL.BlockIndexNow + 1 < DL.TotalBlockCnt)
        {
            //��ǰ�鲻�����һ��,���ݳ��ȹ̶�
            if(length - 1 != DATA_CNT_LAST_PACKET)
                return -2;
        }
        else
        {
            //��ǰ�������һ��������һ�����ݰ���������Ҫ����
            LastLength = (DL.TotalDataCnt % BLOCK_SIZE_DATA_TRANSFER) % SIZE_ONE_PACKET_CONTENT;
            if(LastLength == 0)
                LastLength = DATA_CNT_LAST_PACKET;
            if(length - 1 != LastLength)
                return -3;
        }
        
    }
    else
    { 
        //index���
        return -4;
    }




    //�����ݿ����������ݿ�Ļ�����
    memcpy(DL.BlockBuff + Index * SIZE_ONE_PACKET_CONTENT, buff + 1, length -1);
    DL.PacketReadyBuff[Index] = 1;

    DL.LastPacketTime = GetSystemTime();
    
    return 0;
}



int32_t CheckSoundBlockData(uint8_t BlockIndex, uint8_t *LostPacketIndex, uint32_t CheckSum, bool CheckSumValid)
{
    int32_t LostPacketCnt;
    int32_t i;
    const uint8_t ZeroData = 0;

    //���Ϸ���
    if(DL.IsEnabled == DISABLE)
        return -1;

    if(BlockIndex != DL.BlockIndexNow)
        return -2;

    LostPacketCnt = 0;
    for(i = 0; i < DL.ThisBlockPacketCnt; i++)
    {
        if(DL.PacketReadyBuff[i] == 0)
        {
            //��¼��ʧ�����ݰ�
            LostPacketIndex[LostPacketCnt] = i;
            LostPacketCnt++;
            //һ������ϱ�19����ʧ�����ݰ�
            if(LostPacketCnt >= REPORT_CNT_PACKET_LOST)
                break;
        }
    }

    if(LostPacketCnt == 0)
    {
        uint32_t WriteCnt;
        uint32_t CheckSumCal;

        //���ݰ�ȫ�����յ��������ݽ���д��
        if(BlockIndex + 1 != DL.TotalBlockCnt)
        {
            //��ǰ�鲻�����һ�����ݿ飬ȫ����д��
            WriteCnt = BLOCK_SIZE_DATA_TRANSFER;
        }
        else
        {
            //��ǰ�������һ�����ݿ飬д��������Ҫ����
            WriteCnt = DL.TotalDataCnt % BLOCK_SIZE_DATA_TRANSFER;
            if(WriteCnt == 0)
                WriteCnt = BLOCK_SIZE_DATA_TRANSFER;

            //��������һ�����ݿ飬���ݲ���1K��ʣ�ಿ��������0������䣬���ں���CheckSum�����
            for(int k = WriteCnt; k < BLOCK_SIZE_DATA_TRANSFER; k++)
            {
                DL.BlockBuff[k] = 0;
            }
        }

        CheckSumCal = 0;
        for(i = 0; i < BLOCK_SIZE_DATA_TRANSFER / 4; i++)
        {
            uint32_t temp;
            memcpy(&temp, &DL.BlockBuff[i * 4], 4);
            CheckSumCal += temp;
        }

        if(CheckSumValid == FALSE || CheckSumCal == CheckSum)
        {
            //�������Ҫ���飬����У��ͨ������������ݵ�д�롣


            
            DataFlashWriteData(DL.ContentAddr + DL.BlockIndexNow * BLOCK_SIZE_DATA_TRANSFER, 
                                          DL.BlockBuff, WriteCnt);
        
            //�Ե�ǰBlock��Flag����д��
            DataFlashWriteData(DL.InfoAddr + OFFSET_DOWNLOADER_INDEX_READY_FLAG + DL.BlockIndexNow,
                                              (uint8_t *)&ZeroData, 1);
        }

        
        

/*
        uint32_t CheckCnt = WriteCnt / 4;
        uint32_t CheckData;
        for(i = 0; i < CheckCnt; i++)
        {
            memcpy(&CheckData, DL.BlockBuff + i * 4, 4);
            DL.CheckSum ^= CheckData;
        }
*/
        
    }


    DL.LastPacketTime = GetSystemTime();
        
    return LostPacketCnt;
}

int32_t WriteSoundFinish()
{
    int32_t NextBlockIndex;
    uint8_t DownFinishFlag;
    uint8_t ConvertFinishFlag;
    const uint8_t ZeroData = 0;
    int32_t ret = 0;

    NextBlockIndex = GetNextSoundBlockIndex();
    if(NextBlockIndex < 0)
    {
        //����
        ret = NextBlockIndex;
        goto end;
    }
    else if(NextBlockIndex != 10000)
    {
        //�������ݿ�û��д��
        ret = -2;
        goto end;
    }
    else
    {
        //������ȫ��д��
    }


    DlFinishTime = GetSystemTime();
    DlTime = (DlFinishTime - DlStartTime) / 10;

    DataFlashReadData(DL.InfoAddr + OFFSET_DOWNLOAD_FINISH_FLAG, 
                                     &DownFinishFlag, LENGTH_DOWNLOAD_FINISH_FLAG);
    DataFlashReadData(DL.InfoAddr + OFFSET_AUDIO_CONVERT_FLAG, 
                                     &ConvertFinishFlag, LENGTH_AUDIO_CONVERT_FLAG);

    if(ConvertFinishFlag == 0)
    {
        //����Դ�Ѿ������ת����ֱ�ӷ��سɹ�
        goto end;
    }

    if(DownFinishFlag == 0xFF)
    {
        //д����Դ������ɵı�ʾ
        DataFlashWriteData(DL.InfoAddr + OFFSET_DOWNLOAD_FINISH_FLAG,
                                          (uint8_t *)&ZeroData, LENGTH_DOWNLOAD_FINISH_FLAG);
    }

    


    //Todo
    ret = AudioFileConvert(DL.InfoAddr, DL.TotalDataCnt);
    if(ret != 0)
    {
        goto end;
    }

    DataFlashWriteData(DL.InfoAddr + OFFSET_AUDIO_CONVERT_FLAG,
                                          (uint8_t *)&ZeroData, LENGTH_AUDIO_CONVERT_FLAG);

    ConverFinishTime = GetSystemTime();
    ConverTime = (ConverFinishTime - DlFinishTime) / 10;

    DL.LastPacketTime = GetSystemTime();
end:
    
    DL.IsEnabled = DISABLE;
    return ret;
}

uint8_t ConvertFlag;
void ConvertTest()
{
    DataFlashReadData(DL.InfoAddr + OFFSET_DOWNLOADER_FILE_LENGTH , (uint8_t *)&DL.TotalDataCnt, 4);
    if(ConvertFlag != 0)
        AudioFileConvert(DL.InfoAddr, DL.TotalDataCnt);
}


FunctionalState IsAudioDownloading()
{
    return DL.IsEnabled;
}

uint32_t GetLastDownloadMessageTime()
{
    return DL.LastPacketTime;
}

void StopDonwload()
{
    DL.IsEnabled = DISABLE;
}




