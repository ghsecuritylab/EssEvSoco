#include "common.h"
#include <stdint.h>
#include <string.h>
#include "tea.h"
#include "engine.h"
#include "DownLoader.h"
#include "RSA.h"
#include "DecryDesKey.h"
#include "SpiFlash.h"
#include "Fs.h"
#include "Mp3Convert.h"
#include "MixerConfig.h"







#define OFFSET_SECRET_TEA					260
#define OFFSET_SECRET_GUID					350


//���������е�GUID����Դ��д�е�GUID���бȶԣ���һ�����ܹ���д�����򷵻ش���
int ChechAudioGuid(unsigned char *EncyData, uint8_t *GuidDL)
{
    int i;
    unsigned char GuidBuff[LENGHT_DOWNLOADER_GUID];
    unsigned int decryptedLength;
    int status;
    
    for(i = 0; i < 64; i++)
    {
        EncyData[i] = EncyData[i] - 0x4B;
    }

    status = RSAPrivateDecrypt(GuidBuff, &decryptedLength, EncyData, 64, &privateKey);
    if(decryptedLength != 16 || status != 0)
    {
        return 50;
    }

    for(i =0; i < decryptedLength; i++)
    {
        if(GuidBuff[i] != GuidDL[i])
        {
            return 50;
        }
    }
    return 0;
}

int32_t LoadVehicleCfgFromSoundFile(struct FileInfo *fp, uint8_t VehicleIndex)
{
    int16_t AccelerateArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];//��ȡ���Ǵ�˵�����
    uint16_t ThrottleDownResponseOri;
    uint32_t i;
    AnalyzeMixerConfig(VehicleIndex);
    ReadFile(fp, ACCELERATE_TABLE_OFFSET, (uint8_t *)AccelerateArray, MAX_AUDIO_FILE_NUM_ONE_DIRECT * 2);
    ReadFile(fp, OVERLOAD_FLAG_OFFSET, &engine.OverloadExistFlag, sizeof(engine.OverloadExistFlag));
    ReadFile(fp, ENGINE_DECELERATE_INTERVAL_OFFSET, (uint8_t *)&ThrottleDownResponseOri, sizeof(ThrottleDownResponseOri));


    //��Դ�е������Դ�˽��д洢����Ҫ�ĳ�С��
    //engine.ThrottleDownResponseOri = ThrottleDownResponseOri >> 8;
    //engine.ThrottleDownResponseOri |= (uint16_t)(ThrottleDownResponseOri << 8);

    for (i = 0; i < MAX_AUDIO_FILE_NUM_ONE_DIRECT; i++)
    {
        engine.AccelerateArrayOri[i] = (uint16_t)AccelerateArray[i] >> 8;
        engine.AccelerateArrayOri[i] |= (int16_t)((uint16_t)AccelerateArray[i] << 8);
    }

    WriteVehicleParamTable(VehicleIndex);
    
    return 0;
}







#define MAX_CNT_CONVERT_FILE			MAX_AUDIO_FILE_NUM_ONE_DIRECT + BACKWARD_AUDIO_CNT + 2
//InfoAddrΪ��������Info�ĵ�ַ
int32_t AudioFileConvert(uint32_t InfoAddr, uint32_t DlTotalSize)
{
    uint32_t i;
    uint32_t SecretAddr = InfoAddr + SPI_FLASH_SECTOR_SIZE;//�������ݵĵ�ַ
    uint32_t HeadAddr = SecretAddr + 512;//��Ƶ�ļ�ͷ�ĵ�ַ
    //uint32_t ContentAddr = HeadAddr + 2048;//��Ƶ�ļ����ݵĵ�ַ
    uint8_t TmpBuff[512];
    int32_t ret = 0;
    uint8_t ForwardAudioFileNum, BackwardAudioFileNum;
    uint16_t StartSector, StopSector;
    uint32_t ForwardDataCntArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];//, BackwardDataCntArray[BACKWARD_AUDIO_CNT];
    uint32_t ForwardAudioStartAddrArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];//, BackwardAudioStartAddrArray[BACKWARD_AUDIO_CNT];
    uint32_t StartDataCnt, StopDataCnt;
    uint32_t StartAudioStartAddr, StopAudioStartAddr;
    bool StopAudioExistFlag = (bool)FALSE;
    struct FileInfo *fp;
    uint8_t VehicleIndex;
    uint8_t ZeroData = 0;
    
    
    
    

    /*************�����������ݴ���***************/
    DataFlashReadData(SecretAddr, TmpBuff, 512);
    //��ȡ��Դ��TEA��Կ
    ret = DecryTeaKey((unsigned char *)(TmpBuff + OFFSET_SECRET_TEA), TeaKeyTable);
    if(ret != 0)
        return ret;
    

    //����Դ�е�GUID����У�飬ȷ�Ϻ��������е��Ƿ�һ��
    uint8_t GuidDL[LENGHT_DOWNLOADER_GUID];//�������е�GUID
    DataFlashReadData(InfoAddr + OFFSET_DOWNLOADER_GUID, GuidDL, LENGHT_DOWNLOADER_GUID);
    ret = ChechAudioGuid(TmpBuff + OFFSET_SECRET_GUID, GuidDL);
    if(ret != 0)
        return ret;

    
    /***************��Ƶ�ļ�ͷ���ݴ���****************/
    DataFlashReadData(HeadAddr, TmpBuff, 512);
    DecryptContent(TmpBuff, 512);

    //��ȡǰ����Ƶ���� 
    ForwardAudioFileNum = TmpBuff[FORWARD_AUDIO_CNT_OFFSET];
    BackwardAudioFileNum = TmpBuff[BACKWARD_AUDIO_CNT_OFFSET];
    if(ForwardAudioFileNum == 0)
    {
        return -10;
    }
    if(ForwardAudioFileNum > MAX_AUDIO_FILE_NUM_ONE_DIRECT || BackwardAudioFileNum > BACKWARD_AUDIO_CNT)
    {
        return -11;
    }

    //��ȡǰ�������Ĵ�С�͵�ַ
    for(i = 0; i < ForwardAudioFileNum; i++)
    {
        ForwardDataCntArray[i] = TmpBuff[AUDIO_LENGTH_OFFSET + i * 4] << 24;
        ForwardDataCntArray[i] += TmpBuff[AUDIO_LENGTH_OFFSET + i * 4 + 1] << 16;
        ForwardDataCntArray[i] += TmpBuff[AUDIO_LENGTH_OFFSET + i * 4 + 2] << 8;
        ForwardDataCntArray[i] += TmpBuff[AUDIO_LENGTH_OFFSET + i * 4 + 3];

        StartSector = TmpBuff[AUDIO_START_SECTOR_OFFSET + i * 2] << 8;
        StartSector += TmpBuff[AUDIO_START_SECTOR_OFFSET + i * 2 + 1];
        ForwardAudioStartAddrArray[i] = (uint32_t)StartSector * 512; //�˵�ַ��Ϊ�ļ��ڵĵ�ַ

        if(ForwardDataCntArray[i] == 0 || ForwardDataCntArray[i] >= FS_ONE_FILE_MAX_SIZE ||
            ForwardAudioStartAddrArray[i] == 0 || ForwardAudioStartAddrArray[i] >= FS_SUPPORT_MAX_SIZE)
        {
            return -12;
        }

    }

    //�����Ⱥ���ʼ��ַ��Ϣ���
    memset(TmpBuff + AUDIO_LENGTH_OFFSET, 0xFF, 4 * (ForwardAudioFileNum + BackwardAudioFileNum));
    memset(TmpBuff + AUDIO_START_SECTOR_OFFSET, 0xFF, 2 * (ForwardAudioFileNum + BackwardAudioFileNum));
    

    //����Ҫ�Ժ����������д���ֱ��������������������

    //������
    StartDataCnt = TmpBuff[START_SOUND_LENGTH_OFFSET] << 24;
    StartDataCnt += TmpBuff[START_SOUND_LENGTH_OFFSET + 1] << 16;
    StartDataCnt += TmpBuff[START_SOUND_LENGTH_OFFSET + 2] << 8;
    StartDataCnt += TmpBuff[START_SOUND_LENGTH_OFFSET + 3];
    StartSector = TmpBuff[START_SOUND_START_SECTOR_OFFSET] << 8;
    StartSector += TmpBuff[START_SOUND_START_SECTOR_OFFSET + 1];
    StartAudioStartAddr = StartSector * 512;
    if(StartDataCnt == 0 || StartDataCnt >= FS_ONE_FILE_MAX_SIZE ||
            StartAudioStartAddr == 0 || StartAudioStartAddr >= FS_SUPPORT_MAX_SIZE)
    {
        return -13;
    }
    memset(TmpBuff + START_SOUND_LENGTH_OFFSET, 0xFF, 4);
    memset(TmpBuff + START_SOUND_START_SECTOR_OFFSET, 0xFF, 2);

    //Ϩ���������Բ�����
    StopDataCnt = TmpBuff[STOP_SOUND_LENGTH_OFFSET] << 24;
    StopDataCnt += TmpBuff[STOP_SOUND_LENGTH_OFFSET + 1] << 16;
    StopDataCnt += TmpBuff[STOP_SOUND_LENGTH_OFFSET + 2] << 8;
    StopDataCnt += TmpBuff[STOP_SOUND_LENGTH_OFFSET + 3];
    StopSector = TmpBuff[STOP_SOUND_START_SECTOR_OFFSET] << 8;
    StopSector += TmpBuff[STOP_SOUND_START_SECTOR_OFFSET + 1];
    StopAudioStartAddr = StopSector * 512;
    if(StopDataCnt == 0 || StopDataCnt >= FS_ONE_FILE_MAX_SIZE ||
            StopAudioStartAddr == 0 || StopAudioStartAddr >= FS_SUPPORT_MAX_SIZE)
    {
        StopAudioExistFlag = (bool)FALSE;
    }
    else
    {
        StopAudioExistFlag = (bool)TRUE;
        memset(TmpBuff + STOP_SOUND_LENGTH_OFFSET, 0xFF, 4);
        memset(TmpBuff + STOP_SOUND_START_SECTOR_OFFSET, 0xFF, 2);
    }

    //���ݵ�һ��MP3��ͷ��3�ֽ���Ϣ��������Ƶ�ļ��Ĳ����ʺ�����
    struct Mp3FormatInfo Mp3Format;
    GetAudioMp3Format(HeadAddr + ForwardAudioStartAddrArray[0], ForwardDataCntArray[0], &Mp3Format);
    //�������ʺ�������Ϣд�뵽��Ƶ�ļ�ͷ��
    memcpy(TmpBuff + AUDIO_SAMPLE_RATE_OFFSET, &Mp3Format.SampleRate, AUDIO_SAMPLE_RATE_LENGTH);
    memcpy(TmpBuff + AUDIO_BIT_RATE_OFFSET, &Mp3Format.BitRate, AUDIO_BIT_RATE_LENGTH);
    
    



    //����һ���յ��ļ�
    RecoverBadAudio();
    //DataFlashWriteData(InfoAddr + OFFSET_DOWNLOADER_VEHICLE_INDEX, 
    //                                     &VehicleIndex, LENGTH_DOWNLOADER_VEHICLE_INDEX);
    DataFlashReadData(InfoAddr + OFFSET_DOWNLOADER_VEHICLE_INDEX, 
                                         &VehicleIndex, LENGTH_DOWNLOADER_VEHICLE_INDEX);
    fp = &File[ENGINE_INDEX(VehicleIndex)];                                     
    ret = CreateEmptyFile(fp);
    if(ret != 0)
    {
        goto Out;
    }

    //д��GUID��SoundName���ļ���Ϣ��
    ret = DataFlashWriteData(fp->FileIndexAddr + OFFSET_FILE_GUID, GuidDL, SIZE_GUID);
    if(ret != SIZE_GUID)
    {
        ret = -17;
        goto Out;
    }
    uint8_t NameDl[LENGTH_DOWNLOADER_SOUND_NAME_MAX];
    DataFlashReadData(InfoAddr + OFFSET_DOWNLOADER_SOUND_NAME, NameDl, LENGTH_DOWNLOADER_SOUND_NAME_MAX);
    ret = DataFlashWriteData(fp->FileIndexAddr + OFFSET_FILE_NAME, NameDl, SIZE_SOUND_NAME);
    if(ret != SIZE_SOUND_NAME)
    {
        ret = -35;
        goto Out;
    }
    
    //���ļ����ȼ���ʼ��ַ��Ϣ����Ϊ�գ�Ȼ��д���ļ�ͷ��
    //�ļ����Ⱥ���ʼ��ַ���ڶ�ȡ��Ϣ�����
    WriteFileWithAlloc(fp, TmpBuff, 512);
    //ʣ��ͷ��1.5K����Ϣд��
    for(i = 0; i < 3; i++)
    {
        DataFlashReadData(HeadAddr + 512 * (i + 1), TmpBuff, 512);
        DecryptContent(TmpBuff, 512);
        WriteFileWithAlloc(fp, TmpBuff, 512);
    }

    /************************��Ƶ���ݵĽ��ܡ����롢������д��*****************************/
    uint32_t StartAddrTmp;
    //ǰ����������
    for(i = 0; i < ForwardAudioFileNum; i++)
    {
        //��WAV�ĵ�ǰ���ȱ���
        //WAV�ĵ�ǰ����Ϊ��ǰMP3ת����WAV�����ʼ��ַ
        StartAddrTmp = fp->FileLength;
        ret = DecryptionConvertMp3(fp, HeadAddr + ForwardAudioStartAddrArray[i], ForwardDataCntArray[i]);
        if(ret < 0)
        {
            goto Out;
        }
            
        //���WAV�ļ�û�а���512�ֽڶ���
        if(fp->FileLength % 512 != 0)
        {
            ret = -18;
            goto Out;
        }

        //�����ݳ��Ⱥ���ʼ��ַ��Ϣ��MP3��Ϣ����ΪWAV�ļ�
        ForwardDataCntArray[i] = ret;
        ForwardAudioStartAddrArray[i] =StartAddrTmp;
    }

    //������������
    //��WAV�ĵ�ǰ���ȱ���
    //WAV�ĵ�ǰ����Ϊ��ǰMP3ת����WAV�����ʼ��ַ
    StartAddrTmp = fp->FileLength;
    ret = DecryptionConvertMp3(fp, HeadAddr + StartAudioStartAddr, StartDataCnt);
    if(ret < 0)
    {
        goto Out;
    }

        
    //���WAV�ļ�û�а���512�ֽڶ���
    if(fp->FileLength % 512 != 0)
    {
        ret = -27;
        goto Out;
    }

    //�����ݳ��Ⱥ���ʼ��ַ��Ϣ��MP3��Ϣ����ΪWAV�ļ�
    StartAudioStartAddr = StartAddrTmp;
    StartDataCnt = ret;

    //Ϩ����������
    if(StopAudioExistFlag == true)
    {
        //��WAV�ĵ�ǰ���ȱ���
        //WAV�ĵ�ǰ����Ϊ��ǰMP3ת����WAV�����ʼ��ַ
        StartAddrTmp = fp->FileLength;
        ret = DecryptionConvertMp3(fp, HeadAddr + StopAudioStartAddr, StopDataCnt);
        if(ret < 0)
        {
            goto Out;
        }
            
        //���WAV�ļ�û�а���512�ֽڶ���
        if(fp->FileLength % 512 != 0)
        {
            ret = -28;
            goto Out;
        }

        //�����ݳ��Ⱥ���ʼ��ַ��Ϣ��MP3��Ϣ����ΪWAV�ļ�
        StopAudioStartAddr = StartAddrTmp;
        StopDataCnt = ret;
    }


    /******************�����������ȼ���ʼλ��д��********************/
    for(i = 0; i < ForwardAudioFileNum; i++)
    {
        TmpBuff[i * 4] = ForwardDataCntArray[i] >> 24;
        TmpBuff[i * 4 + 1] = ForwardDataCntArray[i] >> 16;
        TmpBuff[i * 4 + 2] = ForwardDataCntArray[i] >> 8;
        TmpBuff[i * 4 + 3] = ForwardDataCntArray[i];
    }
    ret = WriteFile(fp, AUDIO_LENGTH_OFFSET, TmpBuff, ForwardAudioFileNum * 4);
    if(ret != ForwardAudioFileNum * 4)
    {
        ret = -29;
        goto Out;
    }

    for(i = 0; i < ForwardAudioFileNum; i++)
    {
        StartSector = ForwardAudioStartAddrArray[i] / 512;
        TmpBuff[i * 2] = StartSector >> 8;
        TmpBuff[i * 2 + 1] = StartSector;
    }
    ret = WriteFile(fp, AUDIO_START_SECTOR_OFFSET, TmpBuff, ForwardAudioFileNum * 2);
    if(ret != ForwardAudioFileNum * 2)
    {
        ret = -30;
        goto Out;
    }

    //������������Ϩ���������ֶ��������ģ����Կ���һ����д��
    uint32_t len;
    TmpBuff[0] = StartDataCnt >> 24;
    TmpBuff[1] = StartDataCnt >> 16;
    TmpBuff[2] = StartDataCnt >> 8;
    TmpBuff[3] = StartDataCnt;
    StartSector = StartAudioStartAddr / 512;
    TmpBuff[4] = StartSector >> 8;
    TmpBuff[5] = StartSector;
    if(StopAudioExistFlag == TRUE)
    {
        TmpBuff[6] = StopDataCnt >> 24;
        TmpBuff[7] = StopDataCnt >> 16;
        TmpBuff[8] = StopDataCnt >> 8;
        TmpBuff[9] = StopDataCnt;
        StartSector = StopAudioStartAddr / 512;
        TmpBuff[10] = StartSector >> 8;
        TmpBuff[11] = StartSector;
        len = 12;
    }
    else
    {
        len = 6;
    }
    ret = WriteFile(fp, START_SOUND_LENGTH_OFFSET, TmpBuff, len);
    if(ret != len)
    {
        ret = -31;
        goto Out;
    }

    //д���ļ�����,������������ʹ�ܱ��д�뵽�ļ�����
    //DataFlashReadData(InfoAddr + OFFSET_DOWNLOADER_FILE_LENGTH, TmpBuff, LENGTH_DOWNLOADER_FILE_LENGTH);
    ret = DataFlashWriteData(fp->FileIndexAddr + OFFSET_FILE_LENGTH, (uint8_t *)&fp->FileLength, LENGTH_FILE_LENGTH);
    if(ret != LENGTH_FILE_LENGTH)
    {
        ret = -32;
        goto Out;
    }


    ret = DataFlashWriteData(fp->FileIndexAddr + OFFSET_BLOCK_CNT, (uint8_t *)&fp->TotalBlockCnt, LENGTH_BLOCK_CNT);
    if(ret != LENGTH_BLOCK_CNT)
    {
        ret = -33;
        goto Out;
    }

    //���ļ���ȫд��ǰ������Դ�е�����д�뵽��������
    LoadVehicleCfgFromSoundFile(fp, VehicleIndex);
    
    
    ret = DataFlashWriteData(fp->FileIndexAddr + OFFSET_FILE_WHOLE_FLAG, &ZeroData, SIZE_ENABLE_FLAG);
    if(ret != SIZE_ENABLE_FLAG)
    {
        ret = -34;
        goto Out;
    }

    ret = 0;

    
    //InitFile(fp);
Out:
    //��Ƶ�ļ������˱仯
    //����Ƶ�ļ����г�ʼ��
    InitFile(fp);
    return ret;
}
   

