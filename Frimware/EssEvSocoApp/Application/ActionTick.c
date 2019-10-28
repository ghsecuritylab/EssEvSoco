#include "common.h"
#include "key.h"


//ÿ100ns��1
volatile unsigned int SystemTick;


void SystemTickConfiguration(void)
{
    /* Disable SysTick Counter */
    SysTick_CounterCmd(SysTick_Counter_Disable);
  
    /* Disable the SysTick Interrupt */
    SysTick_ITConfig(DISABLE);
  
    /* Configure HCLK clock as SysTick clock source */
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);  //ʱ�ӳ���8

    SysTick_SetReload(800);  //�������ڳ���
   
    SysTick_CounterCmd(SysTick_Counter_Enable);   //������ʱ�� 
    SysTick_ITConfig(ENABLE);  //���ж�

}

//û1ms���а����¼�����
void SystemTickInterruptHandler()
{
    SystemTick++;
}

u32 GetSystemTime()
{
    return SystemTick;
}

void mDelay(int ms)
{
    volatile u32 InTime;
    InTime = GetSystemTime();
    while(GetSystemTime() - InTime <= ms * 10)
    {
        KeyStatusHandler();
        IWDG_ReloadCounter();
    }
}

void uDelay(int us)
{
    volatile int i, j;
    for(i = 0; i < us; i++)
        for(j = 0; j < 0x5; j++);
}


