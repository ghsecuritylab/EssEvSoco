#include "common.h"
#include "rsa.h"
#include "SystemInfo.h"
#include "DecryDesKey.h"
#include "Authorize.h"

struct AuthorizeInfo Authorize;

R_RSA_PUBLIC_KEY AuthorizeRsaPublicKey;


void GetAuthorizeRSAPulbicKey()
{
    unsigned char *DataPtr;
    int i;
    unsigned char RadomDataBuff[1024];

    
    AuthorizeRsaPublicKey.bits = 512;
    DataPtr = (unsigned char *)(SECURE_START_ADDR);
    for(i = 0; i < 1024; i++)
    {
        RadomDataBuff[i] = *DataPtr;
        DataPtr++;
    }

    GetRsaKeyContent(AuthorizeRsaPublicKey.exponent, 64, OFFSET_PUBLIC_EXPONENT, RadomDataBuff);
    GetRsaKeyContent(AuthorizeRsaPublicKey.modulus, 64, 888, RadomDataBuff);
}


#define AUTHORIZE_DATA_LENGTH_OFFSET			0
#define AUTHORIZE_ID_OFFSET						2
#define AUTHORIZE_TYPE_OFFSET						18
#define AUTHORIZE_RESOURCE_GUID_OFFSET			19
#define AUTHORIZE_DEVICE_GUID_OFFSET				35
#define AUTHORIZE_DEVICE_GUID_MASK_OFFSET		51
//#define AUTHORIZE_USERID_OFFSET					51
//#define SIGNATUREDATA_OFFSET						71
int AuthorizeAudioDownloadInfo(unsigned char *buff)
{
    unsigned short int DataLength;
    unsigned char SignatureEncyData[64];
    int i;
    MD5_CTX Md5Data;
    unsigned char Md5Result[16];
    unsigned char DeviceId[16];
    unsigned char DeviceIdMask[16];
    unsigned char SignatureData[16];
    unsigned int decryptedLength;
    int status;
    
    //���ݳ��Ƚ��������ݣ������������ֶα���
    DataLength = (uint16_t)(((uint16_t)buff[AUTHORIZE_DATA_LENGTH_OFFSET] << 8) | buff[AUTHORIZE_DATA_LENGTH_OFFSET + 1]);
    DataLength -= 64;

    //��ȡǩ������
    for(i = 0; i < 64; i++)
    {
        //����2�ֽ�Ϊ���ݳ��ȵ��ֶγ���
        SignatureEncyData[i] = buff[DataLength + AUTHORIZE_ID_OFFSET + i];
    }

    IWDG_ReloadCounter();
   
    status = RSAPublicDecrypt(SignatureData, &decryptedLength, SignatureEncyData, 64, &AuthorizeRsaPublicKey);
    if(decryptedLength != 16 || status != 0)
    {
        return 48;
    }

    IWDG_ReloadCounter();

    //�����Ȩ���ݵ�MD5ֵ
    MD5Init(&Md5Data);
    MD5Update(&Md5Data, buff + AUTHORIZE_ID_OFFSET, DataLength);
    MD5Final(Md5Result, &Md5Data);

    //�Ƚϼ����MD5ֵ��ǩ������
    for(i = 0; i < decryptedLength; i++)
    {
        if(Md5Result[i] != SignatureData[i])
        {
            return 48;
        }
    }

    for(i = 0; i < 16; i++)
    {
        DeviceId[i] = buff[AUTHORIZE_DEVICE_GUID_OFFSET + i];
        DeviceIdMask[i] = buff[AUTHORIZE_DEVICE_GUID_MASK_OFFSET + i];
        Authorize.WriteAudioGUID[i] = buff[AUTHORIZE_RESOURCE_GUID_OFFSET + i];
    }

    for(i = 0; i < 16; i++)
    {
        //�����豸ID�ļ�Ȩ�Ƚϣ����������������������Ȩ�ļ�ͨ���ԣ�����ĳЩ��ѵ���Դ��ֻ��Ҫһ����Ȩ�ļ�
        if((DeviceId[i] & DeviceIdMask[i]) != (SystemInformation.DeviceId[i] & DeviceIdMask[i]))
        {
            return 49;
        }
    }

    IWDG_ReloadCounter();

    //��Ȩͨ�����������Ȩ��
    Authorize.AuthorizeType = (AuthorizeTypeFlag)buff[AUTHORIZE_TYPE_OFFSET];

    return 0;
}

//����Ȩ�����е�GUID����Դ��д�е�GUID���бȶԣ���һ�����ܹ���д�����򷵻ش���
int AuthorizeAudioGuid(unsigned char *EncyData)
{
    int i;
    unsigned char GuidBuff[256];
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
        if(GuidBuff[i] != Authorize.WriteAudioGUID[i])
        {
            return 50;
        }
    }



    return 0;
}

