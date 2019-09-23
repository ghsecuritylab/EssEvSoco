#include "nrf52.h"
#include "nrf_drv_timer.h"
#include "nrf_gpio.h"
#include "TestModel.h"


const nrf_drv_timer_t TEST_MODEL_TIMER = NRF_DRV_TIMER_INSTANCE(1);  


uint32_t thisCycles, lastCycles;
uint32_t minCycles = 0xFFFFFFFF;
uint32_t idleCounter, oldIdleCounter;
uint32_t LastLastCycle;
float idlePercent;


void TestModelTimeIntHandlerNull(nrf_timer_event_t event_type, void* p_context)
{
    idlePercent = (float)(idleCounter - oldIdleCounter) / (16000000 / minCycles);
    oldIdleCounter = idleCounter;
}


void InitTestModelTimer()
{
    uint32_t err_code = NRF_SUCCESS;
    nrf_drv_timer_config_t timer_cfg;// = NRF_DRV_TIMER_DEFAULT_CONFIG;
    timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32;
    timer_cfg.frequency = NRF_TIMER_FREQ_16MHz;
    timer_cfg.interrupt_priority = 6;
    timer_cfg.mode = NRF_TIMER_MODE_TIMER;
    timer_cfg.p_context = NULL;
    err_code = nrf_drv_timer_init(&TEST_MODEL_TIMER, &timer_cfg, TestModelTimeIntHandlerNull);
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_extended_compare(
         &TEST_MODEL_TIMER, NRF_TIMER_CC_CHANNEL0, 16000000, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);


    nrf_drv_timer_enable(&TEST_MODEL_TIMER);    
}

/** 
 * [MeasureOneCycleMinTime description]����ѭ��whlie�е��ã�����һ��whileѭ����̵�ʱ��
 * @Author   tin39
 * @DateTime 2019��7��22��T14:20:44+0800
 * @param                             [description]
 */
void MeasureOneCycleMinTime()
{
    uint32_t cycles;
    TEST_MODEL_TIMER.p_reg->TASKS_CAPTURE[1] = 1;
    thisCycles = TEST_MODEL_TIMER.p_reg->CC[1];
    if(thisCycles < lastCycles)
    {
        cycles = 0xFFFFFFFF + thisCycles - lastCycles;
    }
    else
    {
        cycles = thisCycles - lastCycles;
    }
    LastLastCycle = lastCycles;
    lastCycles = thisCycles; 

    //�õ������while��ѭ�������ʱ�䣬������Ϊ��û�������жϣ�����������while�����е�ָ������
    if (cycles < minCycles)
        minCycles = cycles;


    idleCounter++;
}


//void TestHwInit()
//{
//    nrf_gpio_cfg_output(PIN_DEBUG);
//}




//void CalCpuLoad()
//{
//    //ÿ�����һ��CPU����
//    if(SystemTick % 100000 == 0)
//    {
//        //64000000 / minCycles���������CPU��ȫ����ʱ��1��������whileѭ���Ĵ���
//        //idleCounter - oldIdleCounter��ʵ��1���ڣ�����while�Ĵ���
//        
//    }
//}




