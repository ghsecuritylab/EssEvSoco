#include <stdint.h>
#include <string.h>
#include "SystemInfo.h"
#include "SpiFlash.h"
#include "DownLoader.h"
#include "FirmwareConvert.h"
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
    uint32_t CheckSum;
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


//DowmloadMode:0-�ϵ�������1-��������
int32_t WriteFirmwareStart(uint32_t FirmwareSize, uint32_t CheckSum, DownloadModeInfo DownloadMode)
{
    uint8_t TmpBuff[LENGTH_DOWNLOADER_FILE_TYPE + LENGTH_DOWNLOADER_FILE_LENGTH + LENGTH_DOWNLOADER_CHECK_SUM];
    int32_t CmpResultFileLength;
    int32_t CmpResultCheckSum;
    uint8_t FileType;
    uint32_t TotalBlockCnt;



    //��ȡ��ǰ�������е����� ,���������ļ����ͺ��ļ���Ϣ
    //�����ļ���Ϣ�ĳ��Ⱥͱ�����Ҫ���ص��ļ���Ϣ������ͬ
    DataFlashReadData(DL.InfoAddr + OFFSET_DOWNLOADER_FILE_TYPE, TmpBuff, 
                        LENGTH_DOWNLOADER_FILE_TYPE + LENGTH_DOWNLOADER_FILE_LENGTH + LENGTH_DOWNLOADER_CHECK_SUM);
    TotalBlockCnt = DivCeil(FirmwareSize, BLOCK_SIZE_DATA_TRANSFER);
    if(TotalBlockCnt > MAX_DOWNLOAD_BLOCK_CNT)
    {
        return 1;
    }

    DlStartTime = GetSystemTime();

    //�Ƚϱ���������ļ����������е���Ϣ
    CmpResultFileLength = memcmp(TmpBuff + OFFSET_DOWNLOADER_FILE_LENGTH, &FirmwareSize, LENGTH_DOWNLOADER_FILE_LENGTH);
    CmpResultCheckSum = memcmp(TmpBuff + OFFSET_DOWNLOADER_CHECK_SUM, &CheckSum, LENGTH_DOWNLOADER_CHECK_SUM);
    FileType = TmpBuff[0];

    if(DownloadMode == DOWNLOAD_MODE_REDOWNLOAD || FileType != 1 || CmpResultFileLength != 0 || CmpResultCheckSum != 0)
    {
        //������ģʽ
        //���߱����������ص��ļ����������еĲ�ͬ
        //���߱�����������ص��ļ�����Ϣ���ϴ����صĲ�ͬ
        RestartDownloader();
        TmpBuff[0] = 1;//��ʶ�����ļ�����Ϊ�̼��ļ�
        memcpy(TmpBuff + OFFSET_DOWNLOADER_FILE_LENGTH, &FirmwareSize, LENGTH_DOWNLOADER_FILE_LENGTH);
        memcpy(TmpBuff + OFFSET_DOWNLOADER_CHECK_SUM, &CheckSum, LENGTH_DOWNLOADER_CHECK_SUM);
        DataFlashWriteData(DL.InfoAddr + OFFSET_DOWNLOADER_FILE_TYPE, TmpBuff, 
                              LENGTH_DOWNLOADER_FILE_TYPE + LENGTH_DOWNLOADER_FILE_LENGTH + LENGTH_DOWNLOADER_CHECK_SUM);
    }
    else
    {
        //������Ϣƥ���ϣ����Ҷϵ�����
        //��ʼ�ϵ�����
        //�ϴ�����ĳ��ԭ�������ж�
        //�����ظ���������
        //���Բ���Ҫ��DOWNLOADER_INFO_SECTOR���κθĶ���
    }

    DL.TotalDataCnt = FirmwareSize;
    DL.TotalBlockCnt = TotalBlockCnt;
    DL.BlockIndexNow = 0;
    
    DL.IsEnabled = ENABLE;

    DL.CheckSum = 0;

    DL.LastPacketTime = GetSystemTime();
    
    return 0;
}

//��ȡ��һ���������ݿ������
int32_t GetNextFirmwareBlockIndex()
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


int32_t WriteFirmwareBlockData(uint8_t *buff, uint32_t length)
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

int32_t CheckFirmwareBlockData(uint8_t BlockIndex, uint8_t *LostPacketIndex)
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
        }
        DataFlashWriteData(DL.ContentAddr + DL.BlockIndexNow * BLOCK_SIZE_DATA_TRANSFER, 
                                          DL.BlockBuff, WriteCnt);
        
        //�Ե�ǰBlock��Flag����д��
        DataFlashWriteData(DL.InfoAddr + OFFSET_DOWNLOADER_INDEX_READY_FLAG + DL.BlockIndexNow,
                                          (uint8_t *)&ZeroData, 1);

        uint32_t CheckCnt = WriteCnt /4;
        uint32_t CheckData;
        for(i = 0; i < CheckCnt; i++)
        {
            memcpy(&CheckData, DL.BlockBuff + i * 4, 4);
            DL.CheckSum ^= CheckData;
        }
        
    }


    DL.LastPacketTime = GetSystemTime();
        
    return LostPacketCnt;
}

int32_t WriteFirmwareFinish()
{
    int32_t NextBlockIndex;
    //uint8_t DownFinishFlag;
    //uint8_t ConvertFinishFlag;
    //const uint8_t ZeroData = 0;
    int32_t ret = 0;

    NextBlockIndex = GetNextFirmwareBlockIndex();
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


    //Todo
    ret = FirmwareFileConvert(DL.InfoAddr, DL.TotalDataCnt);
    if(ret != 0)
    {
        goto end;
    }

    ConverFinishTime = GetSystemTime();
    ConverTime = (ConverFinishTime - DlFinishTime) / 10;

    DL.LastPacketTime = GetSystemTime();
end:
    
    DL.IsEnabled = DISABLE;
    return ret;
}



FunctionalState IsFirmwareDownloading()
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




