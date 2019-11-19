#include <stdint.h>
#include <string.h>


//�Բ������м�飬������쳣���õ������������и���
//�ڲ�����ȡʱ���Ǳ�ϵͳĬ�ϲ�������
//�ڲ���д��ʱ���Ǳ�ϵͳ�е�ǰ��������
//��ȷ����0
/*
ParamAddr:ָ����������ָ�� 
ParamLength:�����ĳ���
CheckLength:��������ȡ�����ݳ���
DefaultValuePtr:ָ��Ĭ��ֵ��ָ��
Min:��Сֵ
Max:���ֵ
*/
int32_t CheckParamIndex(unsigned char *ParamAddr, int ParamLength, int CheckLength, unsigned int DefaultValue, unsigned int Min, unsigned int Max)
{
    unsigned int Param;
    unsigned int *DefaultValuePtr;

    Param = 0;
    //��char��unsigned short int��unsigned int���͵Ĳ�������У��
    memcpy((unsigned char *)&Param, ParamAddr, ParamLength);
    if(Param > Max || Param < Min || ParamLength != CheckLength)
    {
        //���Ȳ���ȷ������ֵ������Ч��Χ�ڵĽ�Ĭ��ֵд�ظ�����
        DefaultValuePtr = &DefaultValue;
         memcpy(ParamAddr, DefaultValuePtr, ParamLength);
    }
    return ParamLength;
}


int32_t CheckParamBuff(unsigned char *ParamAddr, int ParamLength, uint8_t DefaultValue, uint8_t Min, uint8_t Max)
{
    uint32_t i;
    uint8_t Param;
    uint8_t *DefaultValuePtr;
    for(i = 0; i < ParamLength; i++)
    {
        memcpy((uint8_t *)&Param, ParamAddr + i, sizeof(uint8_t));
        if(Param > Max || Param < Min)
        {
            //���Ȳ���ȷ������ֵ������Ч��Χ�ڵĽ�Ĭ��ֵд�ظ�����
            DefaultValuePtr = &DefaultValue;
            memcpy(ParamAddr + i, DefaultValuePtr, sizeof(uint8_t));
        }
    }
    return ParamLength;
}

unsigned int CalSum(unsigned int *StartAddr, unsigned int IntLength)
{
    int i;
    unsigned int sum = 0;
    for(i = 0; i < IntLength; i++)
    {
        sum += *StartAddr;
        StartAddr++;
    }

    return sum;
}

//���ڴ��б�����ֵʵʱд�뵽���ñ���Ķ�Ӧλ����


void WriteOneParamToTable(unsigned char *SourceAddr, int offset, int Length, unsigned char *DistTable)
{
    unsigned char *TablePtr;
    TablePtr = DistTable + offset;
    memcpy(TablePtr, SourceAddr, Length);
}

int32_t InitOneParam(uint8_t *ConfigBuff, uint8_t *Param, int32_t offset, int32_t length, uint32_t DefaultValue, uint32_t Min, uint32_t Max)
{
    memcpy(Param, ConfigBuff + offset, length);
    CheckParamIndex(Param, length, length, DefaultValue, Min, Max);
    return 0;
}

int32_t InitBuffParam(uint8_t *ConfigBuff, uint8_t *Param, int32_t offset, int32_t length, uint8_t DefaultValue, uint8_t Min, uint8_t Max)
{
    memcpy(Param, ConfigBuff + offset, length);
    CheckParamBuff(Param, length, DefaultValue, Min, Max);
    return 0;
}

