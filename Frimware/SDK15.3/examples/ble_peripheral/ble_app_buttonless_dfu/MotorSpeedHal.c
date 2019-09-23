#include "common.h"
#include <stdint.h>
#include "engine.h"
#include "SystemHw.h"
#include "MotorSpeed.h"
#include "nrf_drv_gpiote.h"
#include "nrf_drv_timer.h"


#define PIN_HAL            30

const nrf_drv_timer_t MIXER_HAL_CAPTURE_TIMER = NRF_DRV_TIMER_INSTANCE(2);  




uint32_t TotalHalCnt;
uint32_t HalCntPerPeriod;
//EXTITrigger_TypeDef  TriggerState;

uint16_t HalCalibrate100KmH;//����100km/h�£�ÿ200ms��HAL������
uint16_t HalMaxSpeedCalibrate;
uint16_t MaxVehicleCalibrate;//��λkm/h
/*
uint16_t WheelDiameter;//��λcm
uint16_t MotorPoles;//����Լ���
uint32_t ReductionRatio;//���ٱ�*100
*/

void MotorSpeedHalIrqHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
void HalSpeedTimerIntHandler(nrf_timer_event_t event_type, void           * p_context);



//200ms��ȡһ��HAL����
void MotorSpeedHalHwInit()
{
    uint32_t err_code = NRF_SUCCESS;
    nrf_drv_timer_config_t timer_cfg;// = NRF_DRV_TIMER_DEFAULT_CONFIG;
    timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32;
    timer_cfg.frequency = NRF_TIMER_FREQ_125kHz;
    timer_cfg.interrupt_priority = IRQ_PROIRITY_TIMER_HAL;
    timer_cfg.mode = NRF_TIMER_MODE_TIMER;
    timer_cfg.p_context = NULL;
    err_code = nrf_drv_timer_init(&MIXER_HAL_CAPTURE_TIMER, &timer_cfg, HalSpeedTimerIntHandler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_extended_compare(
         &MIXER_HAL_CAPTURE_TIMER, NRF_TIMER_CC_CHANNEL0, (125000 / 5) + 1, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);//����125000 / 5��ʾ����ʱ��Ƶ��125K��5ΪHAL�ɼ�Ƶ��5HZ

    nrf_drv_timer_enable(&MIXER_HAL_CAPTURE_TIMER);    


    
    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    in_config.is_watcher = false;
    in_config.pull = NRF_GPIO_PIN_NOPULL;
    in_config.hi_accuracy = true;
    in_config.sense = NRF_GPIOTE_POLARITY_TOGGLE;
    in_config.skip_gpio_setup = false;


    err_code = nrf_drv_gpiote_in_init(PIN_HAL, &in_config, MotorSpeedHalIrqHandler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(PIN_HAL, true);
}


void MotorSpeedHalIrqHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    TotalHalCnt++;
}


void HalSpeedTimerIntHandler(nrf_timer_event_t event_type, void           * p_context)
{
    static uint32_t LastHalCnt;
    uint32_t HalCntPerPeriodTmp;
    HalCntPerPeriodTmp = TotalHalCnt - LastHalCnt;
    LastHalCnt = TotalHalCnt;

    //HalCntPerPeriod = (HalCntPerPeriod * 3 + HalCntPerPeriodTmp) >> 2;
    HalCntPerPeriod = HalCntPerPeriodTmp >> 1;//֮ǰ��STƽ̨���������زɼ���������˫���زɼ�����������������1�����������2
    
    
}


#define CHAN_MAX_VALUE 10000

#define KMAX 100


int expou(int x, unsigned short int k)
{
	// k*x*x*x + (1-k)*x
	// 0 <= k <= 100

	unsigned int val = (x * x / CHAN_MAX_VALUE * x / CHAN_MAX_VALUE * k
		+ (KMAX - k) * x + KMAX / 2) / KMAX;

	return val;
}

int expo(int value, int k_Ratio)
{
	int  y;
	int  k;

	unsigned neg = value < 0;

	k = neg ? k_Ratio : k_Ratio;


	if (k == 0)
		return value;

	if (neg)
		value = -value; //absval

	if (k < 0)
	{
		y = CHAN_MAX_VALUE - expou(CHAN_MAX_VALUE - value, -k);
	}
	else
	{
		y = expou(value, k);
	}

	return neg ? -y : y;
}

void CalHalMaxSpeedCalibrate()
{

    //���ݰٹ��ﳵ����HAL�ı궨ֵ�ͱ궨������ٽ��м��㣬�ó���������ٶȶ�Ӧ��HAL�궨ֵ
    HalMaxSpeedCalibrate = (uint32_t)HalCalibrate100KmH * MaxVehicleCalibrate / 100;
}


int16_t GetDistSpeedHal()
{
    int32_t speed;
    int32_t ThPercent;
    int32_t SpeedPercent;
    ThPercent = (HalCntPerPeriod * 10000) / HalMaxSpeedCalibrate;
    if(ThPercent > 10000)
        ThPercent = 10000;
    SpeedPercent = expo(ThPercent, -MotorSpeed.ThrottleCurveLevel * 10);
    speed = engine.ForwardLowSpeed + (engine.ForwardHighSpeed - engine.ForwardLowSpeed) * SpeedPercent / 10000;
    return (int16_t)speed;
}





