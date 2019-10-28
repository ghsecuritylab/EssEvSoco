#include "SystemError.h"
struct ErrorInfo SystemError;


SystemErrorFlag GetTheMostSeriousError()
{
    int i;
    SystemErrorFlag *ErrorPtr;
    int TotalErrorItemCnt;
    TotalErrorItemCnt = (sizeof(SystemError) / sizeof(SystemErrorFlag)) - 1;
    ErrorPtr = &SystemError.ConfigVersionrror;
    for(i = 0; i < TotalErrorItemCnt; i++)
    {
        if(*ErrorPtr != NO_ERROR)
        {
            return *ErrorPtr;
        }
        ErrorPtr++;
    }

    return NO_ERROR;
}

void SetSystmError(SystemErrorFlag ErrorType)
{
    
    //SWITCH��������迼�Ǵ������ȼ�
    switch(ErrorType)
    {
        case CONFIG_VERSION_ERROR:
            SystemError.ConfigVersionrror = CONFIG_VERSION_ERROR;
            break;
        case BATTERY_NONE_ERROR:
            SystemError.BatteryNoneError = BATTERY_NONE_ERROR;
            break;
        case AUDIO_FILE_ERROR:
            SystemError.AudioFileError = AUDIO_FILE_ERROR;
            break;
        case BATTERY_LOW_ERROR:
            SystemError.BatteryLowError = BATTERY_LOW_ERROR;
            break;
        case BATTERY_HIGH_ERROR:
            SystemError.BatteryHighError = BATTERY_HIGH_ERROR;
            break;
        default:  
            break;
    }

    //�ó����ȼ���ߵĴ����
    

   

         //�����д���������
    SystemError.MostSeriousError = GetTheMostSeriousError();
    
}

void ClearSystemError(SystemErrorFlag ErrorType)
{
    switch(ErrorType)
    {
        case CONFIG_VERSION_ERROR:
            SystemError.ConfigVersionrror = NO_ERROR;
            break;
        case BATTERY_NONE_ERROR:
            SystemError.BatteryNoneError = NO_ERROR;
            break;
        case AUDIO_FILE_ERROR:
            SystemError.AudioFileError = NO_ERROR;
            break;
        case BATTERY_LOW_ERROR:
            SystemError.BatteryLowError = NO_ERROR;
            break;
        case BATTERY_HIGH_ERROR:
            SystemError.BatteryHighError = NO_ERROR;
            break;
        default:  
            break;
    }

    SystemError.MostSeriousError = GetTheMostSeriousError();
}
