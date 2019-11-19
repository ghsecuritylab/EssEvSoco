#include "common.h"
#include "SystemInfo.h"
#include "SpiFlash.h"
#include "ActionTick.h"
#include "key.h"
#include "mixer.h"
#include "engine.h"
#include "OnChipLed.h"

#include "BleComHw.h"
#include "MotorSpeed.h"

#include "MotorSpeedHal.h"





void RCC_Configuration_HSI()
{
        RCC_DeInit();

        RCC_HSEConfig(RCC_HSE_OFF);

        RCC_HSICmd(ENABLE);                        //���ڲ�ʱ��
        
        while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET)        
        {        
        }

        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

        FLASH_SetLatency(FLASH_Latency_2);
        /* HCLK = SYSCLK */
        RCC_HCLKConfig(RCC_SYSCLK_Div1);
        //APB2
        RCC_PCLK2Config(RCC_HCLK_Div1);
        //APB1
        RCC_PCLK1Config(RCC_HCLK_Div2);
        //PLL ��Ƶ
        RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_16);        //64M
        RCC_PLLCmd(ENABLE);                        //ʹ�ܱ�Ƶ
                                                                                                         
   //�ȴ�ָ���� RCC ��־λ���óɹ� �ȴ�PLL��ʼ���ɹ�
        while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
        {
        }

        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);        
        while(RCC_GetSYSCLKSource() != 0x08){}
}


void RccConfiguration(void)
{
    
    RCC_ClocksTypeDef RCC_Clocks;
    
    //������ RCC�Ĵ�������Ϊȱʡֵ
    
    ErrorStatus HSEStartUpStatus;
    RCC_DeInit();
    //�����ⲿ���پ���HSE��
    RCC_HSEConfig(RCC_HSE_ON);
  
    //�ȴ� HSE ���� 
    HSEStartUpStatus = RCC_WaitForHSEStartUp();
  
    if(HSEStartUpStatus == SUCCESS)
    {
        //Ԥȡָ����ʹ��
        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
    
         //���ô�����ʱֵ
        //FLASH_Latency_2  2 ��ʱ����
        FLASH_SetLatency(FLASH_Latency_2);
    
        //���� AHB ʱ�ӣ�HCLK��
        //RCC_SYSCLK_Div1  AHB ʱ�� =  ϵͳʱ�� 
        RCC_HCLKConfig(RCC_SYSCLK_Div1);
    
         //���ø��� AHB ʱ�ӣ�PCLK2��
        //RCC_HCLK_Div2  APB1 ʱ��  = HCLK / 2 
        RCC_PCLK2Config(RCC_HCLK_Div1);
    
        //���õ��� AHB ʱ�ӣ�PCLK1��
        //RCC_HCLK_Div2  APB1 ʱ��  = HCLK / 2 
        RCC_PCLK1Config(RCC_HCLK_Div2);
    
        // PLLCLK = 8MHz * 9 = 72 MHz 
        //���� PLL ʱ��Դ����Ƶϵ��
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_6);
    
        //ʹ�ܻ���ʧ�� PLL
        RCC_PLLCmd(ENABLE);
    
        //�ȴ�ָ���� RCC ��־λ���óɹ� �ȴ�PLL��ʼ���ɹ�
        while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
        {
        }
    
    
        //����ϵͳʱ�ӣ�SYSCLK�� ����PLLΪϵͳʱ��Դ
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    
        //�ȴ�PLL�ɹ�������ϵͳʱ�ӵ�ʱ��Դ
        //  0x00��HSI ��Ϊϵͳʱ�� 
        //  0x04��HSE��Ϊϵͳʱ�� 
        //  0x08��PLL��Ϊϵͳʱ��  
        while(RCC_GetSYSCLKSource() != 0x08)
        {
        }
    }
    else
    {
        RCC_Configuration_HSI();
    }
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA 
                             |RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC
                             |RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE
  						   |RCC_APB2Periph_ADC1  | RCC_APB2Periph_AFIO 
                             , ENABLE );
  
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    RCC_GetClocksFreq(&RCC_Clocks);
}




void NvicConfiguration(void)
{ 
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, MAIN_APP_START_ADDR - 0x08000000);   
    //NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0);   
    
    /* Configure the NVIC Preemption Priority Bits */  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}



void SetBootData(BootFromTypeInfo BootData)
{
    BKP_WriteBackupRegister(BKP_DR1, (unsigned int)BootData);
    //RTC_WriteBackupRegister(RTC_BKP_DR0, (unsigned int)BootData);
}

void CodeFlashProtect()
{
    
    if(FLASH_GetReadOutProtectionStatus() == RESET)
    {
        FLASH_Unlock();
        FLASH_ReadOutProtection(ENABLE);
        FLASH_Lock();
    }
    
}

void BkpHwInit()
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);
}

void IwdgInit(void)  
{
         
	  /* Enable write access to IWDG_PR and IWDG_RLR registers */
	  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	  /* IWDG counter clock: 40KHz(LSI) / 256 = 156.25Hz */
	  IWDG_SetPrescaler(IWDG_Prescaler_256);//8ms
	  /* Set counter reload value to 4095 */
	  IWDG_SetReload(1);//��ʱʱ��= (256/(40*1000))*313=2.0032s
	  /* Reload IWDG counter */
	  IWDG_ReloadCounter();
	  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
	  IWDG_Enable();
}





void InitAllPeriph(void)
{

    //CodeFlashProtect();
    //ϵͳʱ���趨
    RccConfiguration();
    NvicConfiguration();
    BkpHwInit();
    ItIsASecret();
    //���ݼĴ����趨����������ʱָ��פ����BOOTLOADER����APP
    SetBootData(BOOT_APP);
    

    SystemTickConfiguration();
    OnChipLedHwInit();
    
    //FLASH��ʼ��
    SpiFlashHwInit();


    

    KeyHwInit();
    //SlientKeyHwInit();

    
    EngineMixerChannelInit();
    AudioOutHwConfig();


    BleComHwInit();

    EvMotorSpeedHwInit();


    MotorSpeedHalHwInit();
    
}

/****************************RC PLUS���ݼ��*****************************/
/*void RunToRcPlusConnector()
{
    typedef  void (*pFunction)(void);
    unsigned int JumpAddress;
    pFunction Jump_To_Application;
    unsigned Addr;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM14 |RCC_APB1Periph_USART2 | RCC_APB1Periph_DAC | RCC_APB1Periph_SPI2, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_TIM1 | RCC_APB2Periph_USART1 | RCC_APB2Periph_TIM15 | RCC_APB2Periph_TIM16 | RCC_APB2Periph_TIM17, DISABLE);


    Addr = CONNECTOR_START_ADDR;

    JumpAddress = *(volatile unsigned int*) (Addr + 4);

      
    Jump_To_Application = (pFunction) JumpAddress;//Jump to user application 

    IWDG_ReloadCounter();
      
    __set_MSP(*(volatile unsigned int*) Addr);//Initialize user application's Stack Pointer 
    Jump_To_Application();   
}



void RcPlusConnectMonitor()
{
    if(RcCapture.ContinueRcPlusData >= 3)
    {
        RunToRcPlusConnector();
    }
}*/

