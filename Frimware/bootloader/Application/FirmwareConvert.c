#include "main.h"
#include <string.h>
#include "Downloader.h"


int32_t FirmwareFileConvert(uint32_t InfoAddr, uint32_t DlTotalSize)
{
    uint32_t i;
    uint32_t SecretAddr = InfoAddr + SPI_FLASH_SECTOR_SIZE;//�������ݵ�ַ
    uint32_t FirmwareContentAddr = SecretAddr + 512;//��Ƶ�ļ�ͷ�ĵ�ַ

    uint8_t TmpBuff[512];
    uint8_t FileType;
    uint32_t FileLength;
    uint32_t CheckSum;
    uint32_t CheckSumCal = 0;
    int32_t ret;
    uint32_t FirmwarePageNum;
    uint32_t ContentLength;
    uint32_t DistAddr;
    uint32_t SourceAddr;
    int32_t RemainCnt;
    FLASH_Status FLASHStatus = FLASH_COMPLETE;
    uint8_t chainCipherBlock[16];
    uint8_t FirmwareContent[16];
    uint32_t FirmwareOneWord;

    DataFlashReadData(InfoAddr + OFFSET_DOWNLOADER_FILE_TYPE, &FileType, LENGTH_DOWNLOADER_FILE_TYPE);
    DataFlashReadData(InfoAddr + OFFSET_DOWNLOADER_FILE_LENGTH, (uint8_t *)&FileLength, LENGTH_DOWNLOADER_FILE_LENGTH);
    if(FileType != 1)
        return -1;

    if(FileLength != DlTotalSize)
        return -2;

    if(FileLength < 10 * 1024 || FileLength > 200 * 1024)//�̼���С�϶���10K-200K֮��
        return -3;

    //�ļ���Ȼ��16�ֽڶ����
    if(FileLength % 16 != 0)
        return -5;

    DataFlashReadData(SecretAddr, TmpBuff, 512);
    ret = DecryDesKey(TmpBuff + AES_OFFSET, AESKeyTable);
    if(ret != 0)
    {
        return -4;
    }

    

    //�ļ��������У��
    RemainCnt = FileLength;
    SourceAddr = SecretAddr;
    while(RemainCnt > 0)
    {
        DataFlashReadData(SourceAddr, FirmwareContent, 16);
        for(i = 0; i < (16 / sizeof(uint32_t)); i++)
        {
            memcpy(&FirmwareOneWord, FirmwareContent + i * sizeof(uint32_t), sizeof(uint32_t));
            CheckSumCal ^= FirmwareOneWord;
        }
        SourceAddr += 16;
        RemainCnt -= 16;
    }
    DataFlashReadData(InfoAddr + OFFSET_DOWNLOADER_CHECK_SUM, (uint8_t *)&CheckSum, LENGTH_DOWNLOADER_CHECK_SUM);
    if(CheckSumCal != CheckSum)
        return -6;
    
    FLASH_Unlock();
    //�Թ̼�������в���
    ContentLength = FileLength - 512;//512�ֽ�Ϊ���ܲ���
    FirmwarePageNum = DivCeil(ContentLength, STM32_PAGE_SIZE);
    for(i = 0; i < FirmwarePageNum && (FLASHStatus == FLASH_COMPLETE); i++)
    {
        FLASHStatus = FLASH_ErasePage(APPLICATION_ADDRESS + (STM32_PAGE_SIZE * i));
    }
    //����ϵͳ��Ϣҳ�Ĳ�����
    FLASH_ErasePage(SYSTEM_INFO_ADDR);



    //��ʼ���н��ܺ͹̼�д��
    SourceAddr = FirmwareContentAddr;
    DistAddr = APPLICATION_ADDRESS;
    RemainCnt = ContentLength;
    while(RemainCnt > 0)
    {
        memset(chainCipherBlock, 0, 16);
        aesDecInit();//��ִ�н��ܳ�ʼ��֮ǰ����ΪAES_Key_Table��ֵ��Ч����������
        DataFlashReadData(SourceAddr, FirmwareContent, 16);
        aesDecrypt(FirmwareContent, chainCipherBlock);//AES���ܣ��������ݴ����dat���棬�����ܾ��ܵõ�֮ǰ�����ġ�
        for(i = 0; i < (16 / sizeof(uint32_t)); i++)
        {
            memcpy(&FirmwareOneWord, FirmwareContent + i * sizeof(uint32_t), sizeof(uint32_t));
            FLASH_ProgramWord(DistAddr + i * sizeof(uint32_t), FirmwareOneWord);
        }
        SourceAddr += 16;
        DistAddr += 16;
        RemainCnt -= 16;
    }
    

    FLASH_ProgramWord(SYSTEM_INFO_ADDR + FIRMWARE_COMPLETE_FLAG_OFFSET, 0);
    FLASH_Lock();
    return 0;
}





