#include "main.h"
//������������

u32 SystemClk;


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



void RCC_Configuration(void)
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
  						     | RCC_APB2Periph_AFIO 
                             , ENABLE );
  
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, DISABLE);
    DMA_Cmd(DMA1_Channel1, DISABLE);

    RCC_GetClocksFreq(&RCC_Clocks);
    SystemClk = RCC_Clocks.SYSCLK_Frequency;
  
}




void CodeFlashProtect()
{
    
    if(RESET == FLASH_GetReadOutProtectionStatus())
    {
        FLASH_Unlock();
        FLASH_ReadOutProtection(ENABLE);
        FLASH_Lock();
        
    }
    
}


/*void KeyHwInit()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1 | GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}*/


void IwdgInit(void)  //ι��ʱ������Ϊ���ֵ
{
         
	  /* Enable write access to IWDG_PR and IWDG_RLR registers */
	  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	  /* IWDG counter clock: 40KHz(LSI) / 256 = 156.25Hz */
	  IWDG_SetPrescaler(IWDG_Prescaler_256);//8ms
	  /* Set counter reload value to 4095 */
	  IWDG_SetReload(313);//��ʱʱ��= (256/(40*1000))*313=2.0032s
	  /* Reload IWDG counter */
	  IWDG_ReloadCounter();
	  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
	  IWDG_Enable();
}




BootFromTypeInfo BootCheck()
{
    int ret;
    unsigned char FirmwareCompleteFlag;
    FirmwareCompleteFlag = *(unsigned char *)(SYSTEM_INFO_ADDR + FIRMWARE_COMPLETE_FLAG_OFFSET);
    ret = BKP_ReadBackupRegister(BKP_DR1);
    if(ret == BOOT_APP && FirmwareCompleteFlag == FIRMWARE_COMPLETE)
    {
        return BOOT_APP;
    }
    else
    {
        return BOOT_BOOTLOADER;
    }
}



void SetBootData(BootFromTypeInfo BootData)
{
    BKP_WriteBackupRegister(BKP_DR1, (unsigned int)BootData);
}




/*#if(defined ( __CC_ARM ))
__IO uint32_t VectorTable[48] __attribute__((at(0x20000000)));
#elif (defined (__ICCARM__))
#pragma location = 0x20000000
__no_init __IO uint32_t VectorTable[48];
#elif defined (__GNUC__)
__IO uint32_t VectorTable[48] __attribute__((section(".RAMVectorTable")));
#elif defined ( __TASKING__ )
__IO uint32_t VectorTable[48] __at(0x20000000);
#endif



void NvicConfigurate()
{
    uint32_t i = 0;
    for(i = 0; i < 48; i++)
    {
        VectorTable[i] = *(__IO uint32_t*)(LOADER_ADDRESS + (i << 2));
    }
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    SYSCFG_MemoryRemapConfig(SYSCFG_MemoryRemap_SRAM);

}*/

void BkpHwInit()
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);
    //RTC_WriteBackupRegister(RTC_BKP_DR0, 1);
}

void Init_All_Periph(void)
{
       
    CodeFlashProtect();
    RCC_Configuration();
    
    
    
    
    BkpHwInit();
       
	

	

}





