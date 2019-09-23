#include "common.h"
#include "Fs.h"
#include "SystemInfo.h"
#include "DownLoader.h"





//�ļ�������
//0-3072�ֽ�
#define TABLE_BLOCK_USAGE				0
#define FS_VERSION_OFFSET				499
#define FS_ID_OFFSET						500//FS�ı�ʶ��д������豸ID�ţ�12�ֽ�
//�ļ�������������
#define FS_VERSION						0










//ERROR
#define ERROR_READ_OFFSET				-1
#define ERROR_READ_BLOCK_INDEX			-2
#define ERROR_WRITE_OFFSET				-3


struct FsInfo
{
    unsigned int BlockSize;
    unsigned int BlockCnt;
    unsigned int BlockMask;//0xFFFF0000
    unsigned int BlockOffsetMask;//0x0000FFFF
    unsigned char BlockLeftShiftCnt;//BlockSize��Ӧ������λ��,16
    unsigned char BlockUsageTable[16];
};



struct FsInfo Fs;
struct FileInfo File[FS_SUPPORT_FILE];


int DeleteFile(struct FileInfo *Fp);

//ȷ���ļ�ϵͳ��Ϣ
//0:��ȷ
//-1:FS�汾�Ŵ���
//-2:FS�б�ʶλ���豸ID��ƥ��
int CheckFsInformation()
{
    unsigned char Information[13];
    int i;

    DataFlashReadData(SectorAddr(SECTOR_FAT_TABLE) + FS_VERSION_OFFSET, Information, 13);

    if(Information[0] != FS_VERSION)
    {
        return -1;
    }

    for(i = 0; i < 12; i++)
    {
        if(SystemInformation.DeviceId[i] != Information[1 + i])
        {
            return -2;
        }
    }

    return 0;
}

uint16_t FormatFs()
{
    unsigned char Information[13];
    int i;
    int ret;
    BlockErase(BlockAddr(0));

   
    Information[0] = FS_VERSION;
    for(i = 0; i < 12; i++)
    {
        Information[1 + i] = SystemInformation.DeviceId[i];
    }
    ret = DataFlashWriteData(SectorAddr(SECTOR_FAT_TABLE) + FS_VERSION_OFFSET, Information, 13);
    if(ret != 13)
        return 104;

    Information[0] = 0x7F;
    ret = DataFlashWriteData(SectorAddr(SECTOR_FAT_TABLE) + TABLE_BLOCK_USAGE, Information, 1);
    if(ret != 1)
        return 105;

    return 0;
}



//�����ļ��źͶ�Ӧ�Ĺ����ǹ̶���Ӧ�ģ�����OPEN�ɱ�����ִ�У����Ҵ�FSû��CLOSE
void InitFile(struct FileInfo *Fp)
{
    unsigned char EnableRet;
    int i;
    DataFlashReadData(Fp->FileIndexAddr + OFFSET_FILE_ENABLE_FLAG, &EnableRet, 1);
    if(EnableRet == 0)
    {
        DataFlashReadData(Fp->FileIndexAddr + OFFSET_FILE_LENGTH, (unsigned char *)&Fp->FileLength, LENGTH_FILE_LENGTH);
        DataFlashReadData(Fp->FileIndexAddr + OFFSET_BLOCK_CNT, (unsigned char *)&Fp->TotalBlockCnt, LENGTH_BLOCK_CNT);
        if(Fp->FileLength >= FS_ONE_FILE_MAX_SIZE || Fp->TotalBlockCnt >= FS_SUPPORT_MAX_BLOCK_CNT || 
           Fp->FileLength == 0 || Fp->TotalBlockCnt == 0)
        {
            Fp->EnableFlag = DISABLE;
            return;
        }
        Fp->ReadingIndex = 0;
        Fp->WritintIndex = 0;
        DataFlashReadData(Fp->FileIndexAddr + OFFSET_BLOCK_INDEX(Fp->ReadingIndex), (unsigned char *)&Fp->ReadingBlock, 2);
        Fp->WritingBlock = Fp->ReadingBlock;
        
        for(i = 0; i < FS_ONE_FILE_MAX_SIZE / FS_BLOCK_SIZE; i++)
        {
            Fp->BlockTable[i] = 0;
        }
        DataFlashReadData(Fp->FileIndexAddr + OFFSET_BLOCK_INDEX(0), (unsigned char *)Fp->BlockTable, 2 * Fp->TotalBlockCnt);
        Fp->EnableFlag = ENABLE;
    }
    else
    {
        Fp->EnableFlag = DISABLE;
    }
}

struct FileInfo *OpenFile(unsigned char FileIndex)
{
    struct FileInfo *fp;
    if(FileIndex >= FS_SUPPORT_FILE)
        return 0;

    fp = &File[FileIndex];
    return fp;
}


//0:������-1�ļ��쳣
int CheckOneFileWhole(struct FileInfo *Fp)
{
    unsigned char AudioEnableFlag;

    //����ļ�ϵͳ����Ƶ�ļ��Ƿ����
    if(Fp->EnableFlag == DISABLE)
    {
        return 0;
    }

    //��Ƶ�ļ�����
    //�����Ƶ�ļ��Ƿ���ȫд��
    DataFlashReadData(Fp->FileIndexAddr + OFFSET_FILE_WHOLE_FLAG, &AudioEnableFlag, 1);
    if(AudioEnableFlag == 0)
    {
         //��Ƶ�ļ�����ȫ��ȷд��
         return 0;
    }
    else
    {
        //��Ƶ�ļ�δ����ȫ��ȷд��
        return -1;
    }
}

int RecoverOneAudioFile(struct FileInfo *Fp)
{

    DeleteFile(Fp);
    return 0;
}


int TryRecoverOneAudioFile(struct FileInfo *Fp)
{
    int ret;
    ret = CheckOneFileWhole(Fp);
    if(ret == 0)
    {
       //�ļ�����
        return 0;
    }

    //�ļ������������л���
    RecoverOneAudioFile(Fp);
    return 0;
}

void RecoverBadAudio()
{
    int i;
    for(i = 0; i < FS_SUPPORT_FILE; i++)
    {
        TryRecoverOneAudioFile(&File[i]);
    }
}

void GetFsInfo()
{
    int i;
    DataFlashReadData(SectorAddr(SECTOR_FAT_TABLE) + TABLE_BLOCK_USAGE, (unsigned char *)Fs.BlockUsageTable, 16);

    for(i = 0; i < FS_SUPPORT_FILE; i++)
    {
        File[i].FileIndex = i;
        File[i].FileIndexAddr = SectorAddr(SECTOR_FILE(i));
        InitFile(&File[i]);
    }

    RecoverBadAudio();
}

void InitFs()
{
    int i;
    unsigned int BlockSizeTemp;
    int CheckFsRet;
    unsigned char CheckFsFailCnt = 0;
    
    Fs.BlockSize = SpiFlash.FlashBlockSize;
    Fs.BlockCnt = SpiFlash.FlashBlockCnt - SIZE_DOWNLOADER_AREA / Fs.BlockSize;
    //Fs.BlockCnt = SpiFlash.FlashBlockCnt;//for rcplus

    BlockSizeTemp = Fs.BlockSize;
    //i = 0;
    while(BlockSizeTemp != 1)
    {
        BlockSizeTemp = BlockSizeTemp >> 1;
        Fs.BlockLeftShiftCnt++; 
    }
    Fs.BlockMask = 0xFFFFFFFF << Fs.BlockLeftShiftCnt;
    Fs.BlockOffsetMask = 0xFFFFFFFF >> (32 - Fs.BlockLeftShiftCnt);

    for(i = 0; i < 3; i++)
    {
        CheckFsRet = CheckFsInformation();
        if(CheckFsRet != 0)
        {
            CheckFsFailCnt++;
        }
    }

    if(CheckFsFailCnt == 3)
    {
        FormatFs();
    }

    GetFsInfo();
}













int ReadFile(struct FileInfo *Fp, unsigned int offset, unsigned char *buff, unsigned int ReadCnt)
{
    unsigned short ThisReadingIndex;
    unsigned int InBlockOffset;
    unsigned int ReadAddr;
    unsigned int ThisReadCnt;
    //ȷ�϶�ȡ��ƫ�����Ƿ����ļ���
    if(offset >= Fp->FileLength)
    {
        return ERROR_READ_OFFSET;
    }
    
    //ȷ�϶�ȡ�����������Ƿ�����ļ������ڣ��������ȡ�����趨һ�����ʵ�ֵ
    if(offset + ReadCnt > Fp->FileLength)
    {
        ReadCnt = Fp->FileLength - offset;
    }

    while(ReadCnt > 0)
    {
        //��ȡ��ȡ��������block�ڵ�ƫ����
        InBlockOffset = offset & Fs.BlockOffsetMask;
        if(InBlockOffset + ReadCnt > Fs.BlockSize)
        {
            ThisReadCnt = Fs.BlockSize - InBlockOffset;
        }
        else
        {
            ThisReadCnt = ReadCnt;
        }

        //��ȡ��ȡ��blockƫ����
        ThisReadingIndex = offset >> Fs.BlockLeftShiftCnt;
        if(ThisReadingIndex == Fp->ReadingIndex)
        {
            //���ϴζ�ȡ��block��ͬһ��block
            ReadAddr = BlockAddr(Fp->ReadingBlock) + InBlockOffset;
        }
        else
        {
            //����һ�ζ�ȡ�Ĳ���ͬһ��block��ͨ��������ȡblock����block��
            Fp->ReadingIndex = ThisReadingIndex;
            DataFlashReadData(Fp->FileIndexAddr + OFFSET_BLOCK_INDEX(ThisReadingIndex), (unsigned char *)&Fp->ReadingBlock, 2);
            if(Fp->ReadingBlock > Fs.BlockCnt)
            {
                return ERROR_READ_BLOCK_INDEX;
            }
            ReadAddr = BlockAddr(Fp->ReadingBlock) + InBlockOffset;
        }
        DataFlashReadData(ReadAddr, buff, ThisReadCnt);
        ReadCnt -= ThisReadCnt;
        buff += ThisReadCnt;
        offset += ThisReadCnt;
    }


    return ReadCnt;
    
}




//д��Ŀռ���64Kblock�ڣ�����д������쳣
int WriteFile(struct FileInfo *Fp, unsigned int offset, unsigned char *buff, unsigned int WriteCnt)
{
    unsigned short ThisWritingIndex;
    unsigned int InBlockOffset;
    unsigned int WriteAddr;
    int ret;
    //ȷ�϶�ȡ��ƫ�����Ƿ����ļ���
    if(offset >= Fp->FileLength)
    {
        return ERROR_WRITE_OFFSET;
    }
    //ȷ�϶�ȡ�����������Ƿ�����ļ������ڣ��������ȡ�����趨һ�����ʵ�ֵ
    if(offset + WriteCnt > Fp->FileLength)
    {
        WriteCnt = Fp->FileLength - offset;
    }
    //��ȡ��ȡ��������block�ڵ�ƫ����
    InBlockOffset = offset & Fs.BlockOffsetMask;
    //��ȡ��ȡ��blockƫ����
    ThisWritingIndex = offset >> Fs.BlockLeftShiftCnt;
    if(ThisWritingIndex == Fp->WritintIndex)
    {
        //���ϴζ�ȡ��block��ͬһ��block
        WriteAddr = BlockAddr(Fp->WritingBlock) + InBlockOffset;
    }
    else
    {
        Fp->WritintIndex = ThisWritingIndex;
        //����һ�ζ�ȡ�Ĳ���ͬһ��block��ͨ��������ȡblock����block��
        DataFlashReadData(Fp->FileIndexAddr + OFFSET_BLOCK_INDEX(ThisWritingIndex), (unsigned char *)&Fp->WritingBlock, 2);
        if(Fp->WritingBlock > Fs.BlockCnt)
        {
            return ERROR_READ_BLOCK_INDEX;
        }
        WriteAddr = BlockAddr(Fp->WritingBlock) + InBlockOffset;
    }
    ret = DataFlashWriteData(WriteAddr, buff, WriteCnt);

    return ret;
}

//������Ƶ�ļ���ɾ��
//����ʹ�õ�block������Ƶ�ļ��������������в���
//�Ի��յ�block�����в���
int DeleteFile(struct FileInfo *Fp)
{
    unsigned char BlockUsageTable[FS_SUPPORT_MAX_BLOCK_CNT / 8];
    int i;
    unsigned short int BlockIndex;
    unsigned short int TableOffset;
    unsigned char ShiftCnt;
    //unsigned int FileBlockCnt;
    int WriteRet;
    
    Fp->EnableFlag = DISABLE;
//    DataFlashReadData(Fp->FileIndexAddr + OFFSET_BLOCK_CNT, (unsigned char *)&FileBlockCnt, LENGTH_BLOCK_CNT);
//    if(FileBlockCnt >= Fs.BlockCnt)
//    {
//        FileBlockCnt = 0;
//    }

//    //��ȡBlockʹ������б�
//    if(FileBlockCnt != 0)
//    {
//        //�����ļ�ռ�õ�block
//        DataFlashReadData(SectorAddr(SECTOR_FAT_TABLE) + TABLE_BLOCK_USAGE, BlockUsageTable, FS_SUPPORT_MAX_BLOCK_CNT / 8);
//        //ԭ�е��ļ�ռ����һ����block�ռ䣬������Щblock���Ƴ�,�ռ��ͷ�
//        for(i = 0; i < FileBlockCnt; i++)
//        {
//            //��ȡ�ļ����ݵ�����������BLOCKʹ��������е���Ӧλ�������
//            DataFlashReadData(Fp->FileIndexAddr + OFFSET_BLOCK_INDEX(i), (unsigned char *)&BlockIndex, 2);
//            if(BlockIndex < Fs.BlockCnt)
//            {
//                TableOffset = BlockIndex >> 3;
//                ShiftCnt = BlockIndex & 7;
//                ShiftCnt = 7 - ShiftCnt;
//                BlockUsageTable[TableOffset] = BlockUsageTable[TableOffset] | (1 << ShiftCnt);
//                Fs.BlockUsageTable[TableOffset] = BlockUsageTable[TableOffset] | (1 << ShiftCnt);
//            }
//        }
//        WriteRet = DataFlashWriteData(SectorAddr(SECTOR_FAT_TABLE) + TABLE_BLOCK_USAGE, BlockUsageTable, FS_SUPPORT_MAX_BLOCK_CNT / 8);
//    }

    //����block�Ķ�̬���뼰���䣬���Կ���Block����ռ�ã��������ļ���block�������ֶ���Ϣ����δ��д��
    //�����ڻ����ļ�ʱ����Ҫ���ļ�������BLOCKʹ�ñ���б���
    DataFlashReadData(SectorAddr(SECTOR_FAT_TABLE) + TABLE_BLOCK_USAGE, BlockUsageTable, FS_SUPPORT_MAX_BLOCK_CNT / 8);
    //ԭ�е��ļ�ռ����һ����block�ռ䣬������Щblock���Ƴ�,�ռ��ͷ�
    i = 0;
    do
    {
        //��ȡ�ļ����ݵ�����������BLOCKʹ��������е���Ӧλ�������
        DataFlashReadData(Fp->FileIndexAddr + OFFSET_BLOCK_INDEX(i), (unsigned char *)&BlockIndex, 2);
        if(BlockIndex < Fs.BlockCnt)
        {
            TableOffset = BlockIndex >> 3;
            ShiftCnt = BlockIndex & 7;
            ShiftCnt = 7 - ShiftCnt;
            BlockUsageTable[TableOffset] = BlockUsageTable[TableOffset] | (1 << ShiftCnt);
            Fs.BlockUsageTable[TableOffset] = BlockUsageTable[TableOffset] | (1 << ShiftCnt);
        }
        i++;
    }while(BlockIndex != 0xFFFF && i < MAX_BLOCK_INDEX_CNT);
    WriteRet = DataFlashWriteData(SectorAddr(SECTOR_FAT_TABLE) + TABLE_BLOCK_USAGE, BlockUsageTable, FS_SUPPORT_MAX_BLOCK_CNT / 8);

    SectorErase(Fp->FileIndexAddr);
    return WriteRet;
}












int GetFsFreeBlockCnt()
{
    int i;
    int BitCnt;
    int EmptyBlock = 0;

    unsigned char BlockUsageTable[FS_SUPPORT_MAX_BLOCK_CNT / 8];

    DataFlashReadData(SectorAddr(SECTOR_FAT_TABLE) + TABLE_BLOCK_USAGE, BlockUsageTable, FS_SUPPORT_MAX_BLOCK_CNT / 8);

    for(i = 0; i < Fs.BlockCnt / 8; i++)
    {
        Fs.BlockUsageTable[i] = BlockUsageTable[i];
        for(BitCnt = 0; BitCnt < 8; BitCnt++)
        {
            if(((BlockUsageTable[i] >> BitCnt) & 0x01) != 0)
            {
                EmptyBlock++;
            }
        }
    }

    return EmptyBlock;
}

//0:busy, 1:free, -1:error
int CheckBlockFree(unsigned char *BlockUsageTable, unsigned short int BlockIndex)
{
    unsigned short int TableOffset;
    unsigned char ShiftCnt;
    if(BlockIndex >= Fs.BlockCnt)
        return -1;

    TableOffset = BlockIndex >> 3;
    ShiftCnt = BlockIndex & 7;
    ShiftCnt = 7 - ShiftCnt;
    if((BlockUsageTable[TableOffset] & (1 << ShiftCnt)) == 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

//0:δ�ҵ����е�block, 
//����:���е�block��
unsigned short SearchFreeBlock(unsigned char *BlockUsageTable, unsigned short int StartBlockIndex)
{
    int ret;

    while(1)
    {
        ret = CheckBlockFree(BlockUsageTable, StartBlockIndex);
        if(ret == -1)
        {
            return 0;
        }
        else if(ret == 0)
        {
            StartBlockIndex++;
        }
        else if(ret == 1)
        {
            return StartBlockIndex;
        }
        else
        {
            return 0;
        }
    }
}

//���ж���FAT����blockʹ��������ļ�������block�������ڱ������н��в���
uint16_t AllocBlockToFile(struct FileInfo *Fp, unsigned int NeedBlockCnt, unsigned int FileLength)
{
    unsigned char BlockUsageTable[FS_SUPPORT_MAX_BLOCK_CNT / 8];
    int i;
    unsigned short int BlockIndex;
    unsigned short int TableOffset;
    unsigned char ShiftCnt;
    unsigned int FileBlockCnt;

    unsigned short int ThisBlockIndex;
    //int ret;
    unsigned char ZeroData;
    int WriteRet;
    uint16_t ErrorCnt = 0;

    Fp->EnableFlag = DISABLE;
    //��ȡblockʹ�ñ�
    DataFlashReadData(SectorAddr(SECTOR_FAT_TABLE) + TABLE_BLOCK_USAGE, BlockUsageTable, FS_SUPPORT_MAX_BLOCK_CNT / 8);
    //��ȡ�ļ�ԭ��ռ�õ�block����
    DataFlashReadData(Fp->FileIndexAddr + OFFSET_BLOCK_CNT, (unsigned char *)&FileBlockCnt, LENGTH_BLOCK_CNT);
    if(FileBlockCnt >= Fs.BlockCnt)
    {
        FileBlockCnt = 0;
    }

    //���ļ�ԭ��ռ����block������л���
    if(FileBlockCnt != 0)
    {
        //�����ļ�ռ�õ�block
        //ԭ�е��ļ�ռ����һ����block�ռ䣬������Щblock���Ƴ�,�ռ��ͷ�
        for(i = 0; i < FileBlockCnt; i++)
        {
            //��ȡ�ļ����ݵ�����������BLOCKʹ��������е���Ӧλ�������
            DataFlashReadData(Fp->FileIndexAddr + OFFSET_BLOCK_INDEX(i), (unsigned char *)&BlockIndex, 2);
            if(BlockIndex < Fs.BlockCnt)
            {
                TableOffset = BlockIndex >> 3;
                ShiftCnt = BlockIndex & 7;
                ShiftCnt = 7 - ShiftCnt;
                BlockUsageTable[TableOffset] = BlockUsageTable[TableOffset] | (1 << ShiftCnt);
                Fs.BlockUsageTable[TableOffset] = BlockUsageTable[TableOffset] | (1 << ShiftCnt);
            }
        }
    }

    SectorErase(Fp->FileIndexAddr);

    ThisBlockIndex = 0;
    //���п���block�ļ��������估����
    for(i = 0; i < NeedBlockCnt; i++)
    {
        IWDG_ReloadCounter();
        ThisBlockIndex = SearchFreeBlock(BlockUsageTable, ThisBlockIndex);
        if(ThisBlockIndex == 0)//δ�ҵ����еĿ�
            break;

        //д��������
        WriteRet = DataFlashWriteData(Fp->FileIndexAddr + OFFSET_BLOCK_INDEX(i), (unsigned char *)&ThisBlockIndex, 2);
        if(WriteRet != 2)
        {
            ErrorCnt++;
        }
        Fp->BlockTable[i] = ThisBlockIndex;
        //�������������block
        BlockErase(BlockAddr(ThisBlockIndex));
        //��block���н���block���Ϊ��ʹ��
        TableOffset = ThisBlockIndex >> 3;
        ShiftCnt = ThisBlockIndex & 7;
        ShiftCnt = 7 - ShiftCnt;
        BlockUsageTable[TableOffset] = BlockUsageTable[TableOffset] & ~(1 << ShiftCnt);
        Fs.BlockUsageTable[TableOffset] = BlockUsageTable[TableOffset] & ~(1 << ShiftCnt);
    }

    //д���ļ������������Ϣ
    WriteRet = DataFlashWriteData(Fp->FileIndexAddr + OFFSET_FILE_LENGTH, (unsigned char *)&FileLength, LENGTH_FILE_LENGTH);
    if(WriteRet != LENGTH_FILE_LENGTH)
    {
        ErrorCnt++;
    }
    WriteRet = DataFlashWriteData(Fp->FileIndexAddr + OFFSET_BLOCK_CNT, (unsigned char *)&i, LENGTH_BLOCK_CNT);
    if(WriteRet != LENGTH_BLOCK_CNT)
    {
        ErrorCnt++;
    }
    ZeroData = 0;
    WriteRet = DataFlashWriteData(Fp->FileIndexAddr + OFFSET_FILE_ENABLE_FLAG, &ZeroData, 1);
    if(WriteRet != 1)
    {
        ErrorCnt++;
    }

    //д��block��
    WriteRet = DataFlashWriteData(SectorAddr(SECTOR_FAT_TABLE) + TABLE_BLOCK_USAGE, BlockUsageTable, FS_SUPPORT_MAX_BLOCK_CNT / 8);
    if(WriteRet != FS_SUPPORT_MAX_BLOCK_CNT / 8)
    {
        ErrorCnt++;
    }


    InitFile(Fp);
    if(ErrorCnt == 0)
    {
        return 0;
    }
    else
    {
        return 200 + ErrorCnt;
    }
}


int CreateFile(struct FileInfo *Fp, unsigned int FileLength)
{
    unsigned int TotalFreeBlock = 0;
    unsigned int NeedBlockCnt;
    unsigned int FileBlockCnt;
    //unsigned char IsFileHaveBlock = 1;//������ʶ�����ļ��Ƿ���ռ��FAT���е�block
    /*if(Fp->EnableFlag == DISABLE)
    {
        TotalFreeBlock = GetFsFreeBlockCnt();
    }
    else
    {
        TotalFreeBlock = GetFsFreeBlockCnt() + Fp->TotalBlockCnt;
    }*/
    //�����ļ��Ƿ�enable��ȥflash��ȡռ�õ�block����
    //��ֹ���������������жϿ��������ļ�DISABLE����BLOCK��ռ�õ����
    DataFlashReadData(Fp->FileIndexAddr + OFFSET_BLOCK_CNT, (unsigned char *)&FileBlockCnt, LENGTH_BLOCK_CNT);
    if(FileBlockCnt >= Fs.BlockCnt)
    {
        FileBlockCnt = 0;
    }
    TotalFreeBlock = GetFsFreeBlockCnt() + FileBlockCnt;

    NeedBlockCnt = (FileLength >> Fs.BlockLeftShiftCnt);// + 1;
    if((FileLength & (0xFFFFFFFF >> Fs.BlockLeftShiftCnt)) != 0)
    {
        NeedBlockCnt++;
    }
    if(NeedBlockCnt > TotalFreeBlock)
    {
        return -1;
    }

    if(AllocBlockToFile(Fp, NeedBlockCnt, FileLength) != 0)
        return -1;
        
    return 0;
}

//����ԭ���ļ���Block
//�������д��OFFSET_FILE_ENABLE_FLAG��Ϣ
int32_t CreateEmptyFile(struct FileInfo *Fp)
{
    unsigned char BlockUsageTable[FS_SUPPORT_MAX_BLOCK_CNT / 8];
    int i;
    unsigned short int BlockIndex;
    unsigned short int TableOffset;
    unsigned char ShiftCnt;
    //unsigned int FileBlockCnt;

    //unsigned short int ThisBlockIndex;
    //int ret;
    unsigned char ZeroData;
    int WriteRet;
    //uint16_t ErrorCnt = 0;

    Fp->EnableFlag = DISABLE;
    
//    //��ȡ�ļ�ԭ��ռ�õ�block����
//    DataFlashReadData(Fp->FileIndexAddr + OFFSET_BLOCK_CNT, (unsigned char *)&FileBlockCnt, LENGTH_BLOCK_CNT);
//    if(FileBlockCnt >= Fs.BlockCnt)
//    {
//        FileBlockCnt = 0;
//    }

//    //���ļ�ԭ��ռ����block������л���
//    if(FileBlockCnt != 0)
//    {
//        //��ȡblockʹ�ñ�
//        DataFlashReadData(SectorAddr(SECTOR_FAT_TABLE) + TABLE_BLOCK_USAGE, BlockUsageTable, FS_SUPPORT_MAX_BLOCK_CNT / 8);
//    
//        //�����ļ�ռ�õ�block
//        //ԭ�е��ļ�ռ����һ����block�ռ䣬������Щblock���Ƴ�,�ռ��ͷ�
//        for(i = 0; i < FileBlockCnt; i++)
//        {
//            //��ȡ�ļ����ݵ�����������BLOCKʹ��������е���Ӧλ�������
//            DataFlashReadData(Fp->FileIndexAddr + OFFSET_BLOCK_INDEX(i), (unsigned char *)&BlockIndex, 2);
//            if(BlockIndex < Fs.BlockCnt)
//            {
//                TableOffset = BlockIndex >> 3;
//                ShiftCnt = BlockIndex & 7;
//                ShiftCnt = 7 - ShiftCnt;
//                BlockUsageTable[TableOffset] = BlockUsageTable[TableOffset] | (1 << ShiftCnt);
//                Fs.BlockUsageTable[TableOffset] = BlockUsageTable[TableOffset] | (1 << ShiftCnt);
//            }
//        }

//        //д��block��
//        WriteRet = DataFlashWriteData(SectorAddr(SECTOR_FAT_TABLE) + TABLE_BLOCK_USAGE, BlockUsageTable, FS_SUPPORT_MAX_BLOCK_CNT / 8);
//        if(WriteRet != FS_SUPPORT_MAX_BLOCK_CNT / 8)
//        {
//            return -15;
//        }
//    }

    //����block�Ķ�̬���뼰���䣬���Կ���Block����ռ�ã��������ļ���block�������ֶ���Ϣ����δ��д��
    //�����ڻ����ļ�ʱ����Ҫ���ļ�������BLOCKʹ�ñ���б���
    DataFlashReadData(SectorAddr(SECTOR_FAT_TABLE) + TABLE_BLOCK_USAGE, BlockUsageTable, FS_SUPPORT_MAX_BLOCK_CNT / 8);
    //ԭ�е��ļ�ռ����һ����block�ռ䣬������Щblock���Ƴ�,�ռ��ͷ�
    i = 0;
    do
    {
        //��ȡ�ļ����ݵ�����������BLOCKʹ��������е���Ӧλ�������
        DataFlashReadData(Fp->FileIndexAddr + OFFSET_BLOCK_INDEX(i), (unsigned char *)&BlockIndex, 2);
        if(BlockIndex < Fs.BlockCnt)
        {
            TableOffset = BlockIndex >> 3;
            ShiftCnt = BlockIndex & 7;
            ShiftCnt = 7 - ShiftCnt;
            BlockUsageTable[TableOffset] = BlockUsageTable[TableOffset] | (1 << ShiftCnt);
            Fs.BlockUsageTable[TableOffset] = BlockUsageTable[TableOffset] | (1 << ShiftCnt);
        }
        i++;
    }while(BlockIndex != 0xFFFF && i < MAX_BLOCK_INDEX_CNT);
    WriteRet = DataFlashWriteData(SectorAddr(SECTOR_FAT_TABLE) + TABLE_BLOCK_USAGE, BlockUsageTable, FS_SUPPORT_MAX_BLOCK_CNT / 8);
    if(WriteRet != FS_SUPPORT_MAX_BLOCK_CNT / 8)
    {
        return -15;
    }
    

    //ɾ���ļ���������Ϣ
    SectorErase(Fp->FileIndexAddr);

   

    //д���ļ������������Ϣ
    ZeroData = 0;
    WriteRet = DataFlashWriteData(Fp->FileIndexAddr + OFFSET_FILE_ENABLE_FLAG, &ZeroData, 1);
    if(WriteRet != 1)
    {
        return -14;
    }

    Fp->FileLength = 0;
    Fp->WritintIndex = 0;
    Fp->TotalBlockCnt = 0;
    Fp->FreeDataCntInBlock = 0;
    

    

    return 0;
}









unsigned int  GetFileLength(struct FileInfo *Fp)
{
    unsigned int length;
    DataFlashReadData(Fp->FileIndexAddr + OFFSET_FILE_LENGTH, (unsigned char *)&length, LENGTH_FILE_LENGTH);
    if(length >= SpiFlash.FlashSize)
    {
        length = 0;
    }
    return length;
}


unsigned int GetFsSize()
{
    return ((Fs.BlockCnt - 1) * Fs.BlockSize);
}


unsigned int GetFsEmptySize()
{
    return (GetFsFreeBlockCnt() * Fs.BlockSize);
}


/*void ReadSoundConfig(unsigned char SoundIndex, unsigned int offset, unsigned char *buff, unsigned int length)
{
    DataFlashReadData(SectorAddr(SECTOR_SOUND_CONFIG(SoundIndex)) + offset, buff, length);
}

#define MIXER_CONFIG_SIZE			512
int WriteSoundConfig(unsigned char SoundIndex, unsigned int offset, unsigned char *buff, unsigned int length)
{
    int WriteRet;
    if(offset + length > MIXER_CONFIG_SIZE)
        return -1;

        
    WriteRet = DataFlashWriteData(SectorAddr(SECTOR_SOUND_CONFIG(SoundIndex)) + offset, buff, length);
    return WriteRet;
}



void ReadSystemConfig(unsigned int offset, unsigned char *buff, unsigned int length)
{
    DataFlashReadData(SectorAddr(SECTOR_SYSTEM_CONFIG) + offset, buff, length);
}

#define SYSTEM_CONFIG_SIZE				512
int WriteSystemConfig(unsigned int offset, unsigned char *buff, unsigned int length)
{
    int WriteRet;
    if(offset + length > SYSTEM_CONFIG_SIZE)
        return -1;
    WriteRet = DataFlashWriteData(SectorAddr(SECTOR_SYSTEM_CONFIG) + offset, buff, length);
    return WriteRet;
}*/

int32_t WriteParamSector(uint8_t *ParamTable, uint32_t ParamAddr)
{
    if(ParamAddr % SPI_FLASH_SECTOR_SIZE != 0)
        return -1;

    if(ParamAddr > SectorAddr((1 + VEHICLE_CNT)))
        return -2;

    SectorErase(ParamAddr);
    DataFlashDirectWriteData(ParamAddr, ParamTable, SPI_FLASH_SECTOR_SIZE);
    //DataFlashWriteData(ParamAddr, ParamTable, SPI_FLASH_SECTOR_SIZE);

    return 0;
}

int32_t AllocOneFreeBlock(struct FileInfo *Fp)
{
    unsigned short int TableOffset;
    unsigned char ShiftCnt;
    //unsigned int FileBlockCnt;

    unsigned short int ThisBlockIndex;//Block���ļ�ϵͳ�е�������
    int WriteRet;

    
    //���п���block�ļ��������估����
    IWDG_ReloadCounter();
    ThisBlockIndex = 0;
    ThisBlockIndex = SearchFreeBlock(Fs.BlockUsageTable, ThisBlockIndex);
    if(ThisBlockIndex == 0)//δ�ҵ����еĿ�
        return -20;

    //д��������
    WriteRet = DataFlashWriteData(Fp->FileIndexAddr + OFFSET_BLOCK_INDEX(Fp->TotalBlockCnt), (unsigned char *)&ThisBlockIndex, 2);
    if(WriteRet != 2)
    {
        return -21;
    }
    Fp->BlockTable[Fp->TotalBlockCnt] = ThisBlockIndex;
    //�������������block
    BlockErase(BlockAddr(ThisBlockIndex));
    //��block���н���block���Ϊ��ʹ��
    TableOffset = ThisBlockIndex >> 3;
    ShiftCnt = ThisBlockIndex & 7;
    ShiftCnt = 7 - ShiftCnt;
    //BlockUsageTable[TableOffset] = BlockUsageTable[TableOffset] & ~(1 << ShiftCnt);
    Fs.BlockUsageTable[TableOffset] = Fs.BlockUsageTable[TableOffset] & ~(1 << ShiftCnt);

    //д��block��
    WriteRet = DataFlashWriteData(SectorAddr(SECTOR_FAT_TABLE) + TABLE_BLOCK_USAGE, Fs.BlockUsageTable, FS_SUPPORT_MAX_BLOCK_CNT / 8);
    if(WriteRet != FS_SUPPORT_MAX_BLOCK_CNT / 8)
    {
        return -22;
    }

    //����ɹ�TotalBlockCnt����1
    Fp->TotalBlockCnt++;
    Fp->WritintIndex = Fp->TotalBlockCnt - 1;//++;
    return ThisBlockIndex;
}

//д���������֧��ָ��offset
//offset�������ݵ�д���Զ�����
//д������У�Fp->FileLength��ʾoffset
int WriteFileWithAlloc(struct FileInfo *Fp, unsigned char *buff, unsigned int WriteCnt)
{
    uint32_t ThisWriteCnt;
    int32_t ret;
    unsigned int InBlockOffset;
    unsigned short ThisWritingIndex;
    uint32_t WrittenDataCnt = 0;
    uint32_t WriteAddr;

    //���Կ��д�룬������2��д��
    while(WriteCnt > 0)
    {
        //������ݿ���ʣ��ռ�Ϊ0�����п���Block������
        if(Fp->FreeDataCntInBlock == 0)
        {
            //���ڿ������ݿգ�����һ���յ����ݿ�
            ret = AllocOneFreeBlock(Fp);
            if(ret < 0)
                return ret;

            ThisWritingIndex = ret;
            //�����뵽�����ݿ鸳ֵ����ǰ����д�����ݿ�
            Fp->WritingBlock = ThisWritingIndex;
            Fp->FreeDataCntInBlock = WINBOND_FLASH_BLOCK_SIZE;
        }

        if(WriteCnt > Fp->FreeDataCntInBlock)
        {
            //�����ݿ���಻����������д�룬��Ҫ����������е����ݿ�
            ThisWriteCnt = Fp->FreeDataCntInBlock;
        }
        else
        {
            //���п��пռ��㹻��ֱ��д��
            ThisWriteCnt = WriteCnt;
        }

        //��ȡ��ȡ��������block�ڵ�ƫ����
        InBlockOffset = Fp->FileLength & Fs.BlockOffsetMask;
        //��ȡд��ľ��������ַ
        WriteAddr = BlockAddr(Fp->WritingBlock) + InBlockOffset;
        ret = DataFlashWriteData(WriteAddr, buff, ThisWriteCnt);
        if(ret != ThisWriteCnt)
        {
            return ret;
        }

        WriteCnt -= ThisWriteCnt;
        buff += ThisWriteCnt;
        Fp->FileLength += ThisWriteCnt;
        Fp->FreeDataCntInBlock -= ThisWriteCnt;
        WrittenDataCnt += ThisWriteCnt;
    }

    return WrittenDataCnt;
}

//int32_t f_read (
//	struct FileInfo *Fp, 		/* Pointer to the file object */
//	void *buff,		/* Pointer to data buffer */
//	UINT btr,		/* Number of bytes to read */
//	UINT *br		/* Pointer to number of bytes read */
//)
//{
//    uint32_t ReadCnt;
//    ReadCnt = ReadFile(Fp, Fp->ReadSeek, buff, btr);
//    Fp->ReadSeek += ReadCnt;
//    *br = ReadCnt;

//    return 0;
//}

//int32_t f_seek(
//	struct FileInfo *Fp, 	/* Pointer to the file object */
//	uint32_t ofs		/* File pointer from top of file */
//)
//{
//    if(ofs >= Fp->FileLength)
//        return -1;

//    Fp->ReadSeek = ofs;
//    return 0;
//}


