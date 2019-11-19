#include "SystemHw.h"
#include "SystemInfo.h"
#include "Fs.h"
#include "engine.h"
#include "SystemConfig.h"
#include "MixerConfig.h"
#include "OnChipAudio.h"
#include "mixer.h"
#include "MotorSpeed.h"
#include "OnChipLed.h"
#include "key.h"
#include "ActionTick.h"
#include "BleTransportLayer.h"

#include "DownLoader.h"

#include "DecryDesKey.h"

#include "Authorize.h" 
#include "engine.h"
#include "CalibrationMode.h"


int main(void)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       
{  
	IWDG_ReloadCounter();
    InitAllPeriph();
    GetDeviceId();

    //ChipErase();
    InitFs();

    DownloaderInit();
    GetRsaPrivateKey();
    GetAuthorizeRSAPulbicKey();

    //������ʱ500ms����ֹ�ϵ��ͬʱ�����Ź����������ӿ�ջ�����
    mDelay(51);

    
    AnalyzeSystemConfig();
    AnalyzeDeviceConfig();

    bool IsCalibrationMode;
    IsCalibrationMode =  CalibrationCheck();
    if(IsCalibrationMode == true)
    {
        CalibrationMode();
    }
    
	
    //���õĽ���
    
    //TryChangeSoundIndex(mixer.SoundIndex);
    TryChangeVehileIndexLoop(mixer.SoundIndex, (bool)TRUE);
    

    IWDG_ReloadCounter();


    //For tests
    //PlaySystemStartBeep(0);
    //mDelay(1000);

    /*if(IsDeviceProduced() != 0)
    {
        AmplifierEnable();
        //�豸�ѱ��������߳�ʼ�������е����Ĳ���
        PlaySystemStartBeep(mixer.SoundIndex);
        //��ʱ1�룬��ֹ�ε�������������������һ��
        mDelay(1000);
    }*/



    //ConvertTest();

    
    //StartCheckAmplifierVoltage();
    //�ϵ������
    EngineStartHandler();

    //Md5Test();
    

    while(1)
    {
        EngineHandler();

        KeyTopLevelHandler();
        //SlientKeyHandler();
        

        IWDG_ReloadCounter();
        AmplifierControlHandler();



        BleAnalyzeTransportLayer();

        MixerBleTopLevelHandler();

        EvMotorSpeedHandler();



        VolumeHandler();

        OnChipLedHandler();

    }

}




