#ifndef __SYSTEM_ERROR__
#define __SYSTEM_ERROR__

//ÿ8λ��ʾһյ��
//����0��ʾ������1��ʾ��˸1��,0xFF��ʾ����
typedef enum {NO_ERROR = 0, CONFIG_VERSION_ERROR = 0x01FF03FF, 
                        BATTERY_NONE_ERROR = 0x01FF01FF, AUDIO_FILE_ERROR = 0x02FF01FF, BATTERY_LOW_ERROR = 0x02FF02FF,
                        BATTERY_HIGH_ERROR = 0x02FF03FF, START_BATTERY_HIGH_ERROR = 0x04FF04FF} SystemErrorFlag;

//�����������ȼ���˳���������
struct ErrorInfo
{
    SystemErrorFlag MostSeriousError;
    SystemErrorFlag ConfigVersionrror;//������Ϣ�汾����
    SystemErrorFlag AudioFileError;//Flash�е��������ݴ���
    SystemErrorFlag BatteryNoneError;//�޹��ŵ�ش���
    SystemErrorFlag BatteryLowError;
    SystemErrorFlag BatteryHighError;
    SystemErrorFlag StartBatteryHighError;
};


extern struct ErrorInfo SystemError;

void SetSystmError(SystemErrorFlag ErrorType);
void ClearSystemError(SystemErrorFlag ErrorType);


#endif

