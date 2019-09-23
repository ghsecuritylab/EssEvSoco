#include "ActionTick.h"
//#include "nrf_delay.h"
//#include "fs.h"

#include "SystemInfo.h"
#include "SpiFlash.h"
#include "ActionTick.h"
#include "key.h"
#include "mixer.h"
#include "engine.h"
#include "OnChipLed.h"

//#include "BleComHw.h"
#include "MotorSpeed.h"

#include "MotorSpeedHal.h"

#include "nrf_drv_gpiote.h"
#include "TestModel.h"




//����NFC�ر�
const uint32_t UICR_ADDR_0x20C    __attribute__((at(0x1000120C))) __attribute__((used)) = 0xFFFFFFFE;


void InitAllPeriph(void)
{
    uint32_t err_code;
    //ItIsASecret();
    //���ݼĴ����趨����������ʱָ��פ����BOOTLOADER����APP
    //SetBootData(BOOT_APP);
    

    SystickConfig();


    
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);
    //
    OnChipLedHwInit();
    //
    ////FLASH��ʼ��
    SpiFlashHwInit();

    KeyHwInit();



    AudioOutHwConfig();



    EvMotorSpeedHwInit();


    InitTestModelTimer();
    //TestHwInit();


    MotorSpeedHalHwInit();
    
}




