#include "common.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "BleError.h"
#include "Password.h"
#include "ActionTick.h"
#include "rsa.h"
#include "SystemConfig.h"
#include "Tea.h"
#include "SystemInfo.h"


#define TEA_ENC_CYCLE           17



uint8_t Password[LENGTH_PASSWORD];//֧��ASCII��0x00-0x7F
bool RandomValid = FALSE;//ÿ�������������ΪTRUE��ÿ��������ز�������FALSE
static uint8_t PasswordRandom[LENGTH_PASSWORD_RANDOM];
bool PassFlag = FALSE;//����ͨ����ʶ������û��ͨ������ʱ��������Ϣ�Ĵ��󷵻�

int32_t CheckPasswordValid(uint8_t *Password)
{
    int32_t i;
    for(i = 0; i < LENGTH_PASSWORD; i++)
    {
        if(Password[i] < PASSWORD_CHAR_MIN || Password[i] > PASSWORD_CHAR_MAX)
            return ERROR_SET_PASSWORD_CHAR_ILLEGAL;
    }
    return 0;
}

uint32_t RandomStart;
/** 
 * [R_RandomCreateMy description]�������ڲ���RSA����Ե������
 * @Author   tin39
 * @DateTime 2019��3��13��T9:07:42+0800
 * @param    random                   [description]
 */
/*
void R_RandomCreateMy(R_RANDOM_STRUCT *random)
{
    int i;
    random->bytesNeeded = 0;
    random->outputAvailable = 0;

    RandomStart = GetSystemTime();
    srand(RandomStart);
    for(i = 0; i < 16; i++)
    {
        random->state[i] = (uint8_t)rand();
    }
	
    for(i = 0; i < 16; i++)
    {
        random->output[i] = 204;
    }
}
*/


/** 
 * [MakeRamdomRsaKey description]��������������
 * @Author   tin39
 * @DateTime 2019��3��13��T9:09:02+0800
 * @param                             [description]
 * @return                            [description]
 */
/*
int32_t MakeRamdomRsaKey()
{
    R_RANDOM_STRUCT randomStruct;
    R_RSA_PROTO_KEY protoKey;
    
    int status;

    Start = GetSystemTime();

    R_RandomCreateMy(&randomStruct);

    protoKey.bits = 512;
    protoKey.useFermat4 = 1;

    IWDG_ReloadCounter();
    status = R_GeneratePEMKeys(&PasswordPublicKey, &PasswordPrivateKey, &protoKey, &randomStruct);
    if (status)
    {
        return status;
    }
    IWDG_ReloadCounter();

    Stop = GetSystemTime();
    Interval = Stop - Start;
    
    return 0;
}
*/

uint32_t Start, Stop, Interval;

/** 
 * [AuthorizePassword description]��������У��
 * @Author   tin39
 * @DateTime 2019��3��13��T16:32:32+0800
 * @param    SecurePassword           [description]
 * @return                            [description]
 */
int32_t AuthorizePassword(uint8_t *SecurePassword)
{
    int32_t ret;
    uint32_t TeaKey[4];
    __align(4) uint8_t PasswordApp[LENGTH_PASSWORD];
    uint8_t Temp[LENGTH_PASSWORD_RANDOM + LENGTH_DEVICE_ID];
    MD5_CTX Md5Content;
    int32_t i;


    //���������Ƿ����������
    if(RandomValid == FALSE)
    {
        return ERROR_RANDOM_NOT_REQUEST;
    }
    RandomValid = FALSE;

    memcpy(Temp, SystemInformation.DeviceId, LENGTH_DEVICE_ID);
    memcpy(Temp + LENGTH_DEVICE_ID, PasswordRandom, LENGTH_PASSWORD_RANDOM);

    Start = GetSystemTime();

    MD5Init(&Md5Content);
    MD5Update(&Md5Content, Temp, LENGTH_PASSWORD_RANDOM + LENGTH_DEVICE_ID);
    MD5Final((uint8_t *)TeaKey, &Md5Content);

    memcpy(PasswordApp, SecurePassword, LENGTH_PASSWORD);

    for(i = 0; i < TEA_ENC_CYCLE; i++)
    {
        decrypt((uint32_t *)PasswordApp, TeaKey);
        decrypt((uint32_t *)(PasswordApp + 8), TeaKey);
    }
    


    Stop = GetSystemTime();
    Interval = Stop - Start;
    

    ret = memcmp(PasswordApp, Password, LENGTH_PASSWORD);
    if(ret != 0)
    {
        PassFlag = FALSE;
        return ERROR_AUTHORIZATION_PASSWORD;
    }

    PassFlag = TRUE;

    return 0;
}


/** 
 * [SetPassword description]�����趨�����棬�趨������ͨ��RSA���ܽ��д���
 * @Author   tin39
 * @DateTime 2019��3��13��T16:37:50+0800
 * @param    SecurePassword           [description]
 * @return                            [description]
 */
int32_t SetPassword(uint8_t *SecurePassword)
{
    int32_t ret;
    uint32_t TeaKey[4];
    __align(4) uint8_t PasswordNew[LENGTH_PASSWORD];
    uint8_t Temp[LENGTH_PASSWORD_RANDOM + LENGTH_DEVICE_ID];
    MD5_CTX Md5Content;
    uint32_t i;

    //���������Ƿ����������
    if(RandomValid == FALSE)
    {
        return ERROR_RANDOM_NOT_REQUEST;
    }
    RandomValid = FALSE;

    memcpy(Temp, SystemInformation.DeviceId, LENGTH_DEVICE_ID);
    memcpy(Temp + LENGTH_DEVICE_ID, PasswordRandom, LENGTH_PASSWORD_RANDOM);

    MD5Init(&Md5Content);
    MD5Update(&Md5Content, Temp, LENGTH_PASSWORD_RANDOM + LENGTH_DEVICE_ID);
    MD5Final((uint8_t *)TeaKey, &Md5Content);

    memcpy(PasswordNew, SecurePassword, LENGTH_PASSWORD);

    for(i = 0; i < TEA_ENC_CYCLE; i++)
    {
        decrypt((uint32_t *)PasswordNew, TeaKey);
        decrypt((uint32_t *)(PasswordNew + 8), TeaKey);
    }
    

   

    //���PASSWORD���ݵĺϷ��ԣ�����ΪASCII
    ret = CheckPasswordValid(PasswordNew);
    if(ret != 0)
    {
        return ret;
    }

    memcpy(Password, PasswordNew, LENGTH_PASSWORD);

    WriteSystemParamTable();
    return 0;
}



/** 
 * [ResetPassword description]��������Ϊ0
 * @Author   tin39
 * @DateTime 2019��3��14��T8:58:16+0800
 * @param    SecurePassword           [description]
 * @return                            [description]
 */
int32_t ResetPassword(uint8_t *SecurePassword)
{
    uint32_t TeaKey[4];
    __align(4) uint8_t PasswordReset[LENGTH_PASSWORD];
    uint8_t OldPassword[LENGTH_PASSWORD];
    uint8_t Temp[LENGTH_PASSWORD_RANDOM + LENGTH_DEVICE_ID];
    MD5_CTX Md5Content;
    uint32_t i;

    

    //���������Ƿ����������
    if(RandomValid == FALSE)
    {
        return ERROR_RANDOM_NOT_REQUEST;
    }
    RandomValid = FALSE;

    memcpy(Temp, SystemInformation.DeviceId, LENGTH_DEVICE_ID);
    memcpy(Temp + LENGTH_DEVICE_ID, PasswordRandom, LENGTH_PASSWORD_RANDOM);

    MD5Init(&Md5Content); 
    MD5Update(&Md5Content, Temp, LENGTH_PASSWORD_RANDOM + LENGTH_DEVICE_ID);
    MD5Final((uint8_t *)TeaKey, &Md5Content);

    memcpy(PasswordReset, SecurePassword, LENGTH_PASSWORD);

    for(i = 0; i < TEA_ENC_CYCLE; i++)
    {
        decrypt((uint32_t *)PasswordReset, TeaKey);
        decrypt((uint32_t *)(PasswordReset + 8), TeaKey);
    }
    



    //ȷ��������������Ϊȫ0
    for(i = 0; i < LENGTH_PASSWORD; i++)
    {
        if(PasswordReset[i] != 0x30)
        {
            return ERROR_RESET_PASSWORD_DISMATCH;
        }
    }

    //����ԭ����
    memcpy(OldPassword, Password, LENGTH_PASSWORD);
    //�������벢����
    memset(Password, 0x30, LENGTH_PASSWORD);
    WriteSystemParamTable();

    //����8���飬8����û�жϵ磬�ָ�ԭ����
    mDelay(8000);


    //���ó�ʱ�����û�ԭ����
    memcpy(Password, OldPassword, LENGTH_PASSWORD);
    WriteSystemParamTable();

    //��������û����ȷ�ķ��أ�
    return ERRPR_RESET_PASSWORD_TIMEOUT;
}


/** 
 * [IsPasswordPassed description]����������֤ͨ���Ĳ�ѯ
 * @Author   tin39
 * @DateTime 2019��3��12��T16:15:55+0800
 * @param                             [description]
 * @return                            [description]
 */
bool IsPasswordPassed()
{
    return PassFlag;
}



/** 
 * [SetPasswordInvalid description]��BLE�Ͽ�ʱ���趨BLE
 * @Author   tin39
 * @DateTime 2019��3��27��T8:58:49+0800
 * @param                             [description]
 */
void SetPasswordInvalid()
{
    PassFlag = FALSE;
}




/** 
 * [GetPasswordRandom description]����8���ֽڵ����������
 * @Author   tin39
 * @DateTime 2019��3��13��T11:08:25+0800
 * @param                             [description]
 * @return                            [description]
 */
uint8_t * GetPasswordRandom()
{
    uint32_t i;
    srand(GetSystemTime());
    for(i = 0; i < LENGTH_PASSWORD_RANDOM; i++)
    {
        PasswordRandom[i] = (uint8_t)rand();
    }
    //ʹ�ܱ�־λ
    RandomValid = TRUE;

    
    return PasswordRandom;
}

/*
uint8_t TestBuff[18];
uint8_t Md5Result[16];
void Md5Test()
{
    int i;
    MD5_CTX Md5Content;
    for(i = 0; i < 18; i++)
    {
        TestBuff[i] = i;
    }

    Start = GetSystemTime();

    

    MD5Init(&Md5Content);
    MD5Update(&Md5Content, TestBuff, 18);
    MD5Final(Md5Result, &Md5Content);

    Stop = GetSystemTime();
    Interval = Stop - Start;
}
*/


