#include "Common.h"
#include "key.h"
#include "ActionTick.h"
#include "mixer.h"
#include "AgentTest.h"

#define MENU_BUTTON_NUM			1
#define BUTTON_MINUS				0
//#define BUTTON_PLUS					1

#define LONG_PRESS_TIME			100


typedef enum {KEY_IDLE = 0, KEY_DOWN = 1, KEY_UP = 2, KEY_FINISH = 3} KeyStatus;
//�ڴ���ĳ�ְ�����ı�״̬
typedef enum {NOACTION = 0, CLICK = 1, LONG_PRESS = 2, DOUBLE_CLICK = 3} KeyType;


struct MenyKeyInfo
{
    u16 PressTime;//��λ10MS
    u16 ReleaseTime;//��λ10MS
    u16 UnHandlerTime;//��������δ�����ʱ��
    KeyStatus status;
    KeyType type;
    u8 KeyLevelArray[10];
};

struct MenyKeyInfo MenuKey[MENU_BUTTON_NUM];


/******************keyӲ����ʼ��********************/
void KeyHwInit()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}


/**********************����״̬���ж�***************************/
//һ�������İ���״̬����
static u32 OneKeyHandler(struct MenyKeyInfo *ThisKey, u8 PressStatus)
{
    //�����ڰ���������֮��ż�����������İ���
    if(ThisKey->type != NOACTION)
    {
        //��������2S��δ����״̬�������
        if(ThisKey->UnHandlerTime <= 200)
        {
            ThisKey->UnHandlerTime++;
            return 1;
        }
        else
        {
            ThisKey->type = NOACTION;
        }
        
    }

    //����״̬�ж���ɣ��ȴ������ͷ�
    if(ThisKey->status == KEY_FINISH)
    {
        if(PressStatus == 0)
        {
            ThisKey->status = KEY_IDLE;
        }
        else
        {
            return 2;
        }
    }

    //��ʼ�����ĸ���״̬�ж����л�
    if(ThisKey->status == KEY_IDLE)
    {
        if(PressStatus == 1)
        {
            //����״̬�����£����������ĵ�һ���¼������������������ж��߼�
            ThisKey->status = KEY_DOWN;
            ThisKey->PressTime = 0;
            ThisKey->ReleaseTime = 0;
            //ThisKey->ClickCnt = 1;
        }
    }
    else if(ThisKey->status == KEY_DOWN)
    {
        if(PressStatus == 0)
        {
            //��������
            ThisKey->status = KEY_UP;
            ThisKey->ReleaseTime = 0;
        }
        else
        {
          //���������£����Ӱ���ʱ��
            ThisKey->PressTime++;
            // 2S����
            if(ThisKey->PressTime >= LONG_PRESS_TIME)
            {
                ThisKey->status = KEY_FINISH;
                ThisKey->type = LONG_PRESS;
                ThisKey->UnHandlerTime = 0;
            }
        }
    }
    else if(ThisKey->status == KEY_UP)
    {
        ThisKey->ReleaseTime++;
        if(ThisKey->ReleaseTime >= 5)
        {
            ThisKey->status = KEY_FINISH;
            ThisKey->type = CLICK;
            ThisKey->UnHandlerTime = 0;
        }
        //ȥ����˫������
        /*
        if(PressStatus == 0)
        {
            ThisKey->ReleaseTime++;
            //�������£��жϵ���ʱ��
            if(ThisKey->ReleaseTime >= 40)
            {
                //�ͷŴ���600ms�������¼����
                ThisKey->status = KEY_FINISH;
                //if(ThisKey->ClickCnt == 1)
                //{
                ThisKey->type = CLICK;
                ThisKey->UnHandlerTime = 0;
                //}
            }
        }
        else
        {
            ThisKey->status = KEY_FINISH;
            ThisKey->type = DOUBLE_CLICK;
            ThisKey->UnHandlerTime = 0;
        }*/
    }
    return 0;
}

void KeyStatusHandler()
{
    static u8 FilterCnt = 0;
    int i;
    u8 KeyLevel;
    u8 KeyNum;
    static int LastHandleTime = 0;

    if(GetSystemTime() < LastHandleTime + 10)
        return;
    LastHandleTime = GetSystemTime();
    
    MenuKey[BUTTON_MINUS].KeyLevelArray[FilterCnt]  = (!GET_KEY_MINUS);
    //MenuKey[BUTTON_PLUS].KeyLevelArray[FilterCnt]  = (!GET_KEY_PLUS);
    
    FilterCnt++;
    //���ڰ���IO����10���˲���1MSһ�Σ�10MSһ�ΰ���״̬�ж�
    if(FilterCnt >= 10)
    {
        //��2���������в���
        for(KeyNum = 0; KeyNum < MENU_BUTTON_NUM; KeyNum++)
        {
            //��ȡ10���˲���İ���״̬
            KeyLevel = 0;
            for(i = 0; i < 10; i++)
            {
                KeyLevel += MenuKey[KeyNum].KeyLevelArray[i];
            }
            //10��������8�μ�⵽���µĵ�ƽ������Ϊ����������
            if(KeyLevel >= 8)
            {
                OneKeyHandler(&MenuKey[KeyNum], 1);
            }
            else
            {
                OneKeyHandler(&MenuKey[KeyNum], 0);
            }
        }

        FilterCnt = 0;
    }
}

/**************************��ȡ������״̬*************************/
MenuKeyType GetMenuKey()
{
    if(MenuKey[BUTTON_MINUS].type == NOACTION)
    {
        return NO_KEY;
    }

    if(MenuKey[BUTTON_MINUS].type == CLICK)
    {
        MenuKey[BUTTON_MINUS].type = NOACTION;
        return  LEFT_CLICK;
    }
    else if(MenuKey[BUTTON_MINUS].type == LONG_PRESS)
    {
        MenuKey[BUTTON_MINUS].type = NOACTION;
        return LEFT_LONG_PRESS;
    }


    if(MenuKey[BUTTON_MINUS].type != NOACTION)
    {
        MenuKey[BUTTON_MINUS].type = NOACTION;
        return NO_KEY;
    }

    return NO_KEY;

    
}



/*************************ϵͳ���㰴������*************************/
void KeyTopLevelHandler()
{
    MenuKeyType key;
    KeyStatusHandler();
    key = GetMenuKey();

    if(key == NO_KEY)
    {
        return;
    }
    else if(key == LEFT_CLICK)
    {
        MinusVolume();
    }
    else if(key == LEFT_LONG_PRESS)
    {

        TryChangeVehileIndexLoop((mixer.SoundIndex + 1) % VEHICLE_CNT, (bool)FALSE);
        
    }
}


//#define GET_KEY_SLIENT		GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_10)

//void SlientKeyHwInit()
//{
//    GPIO_InitTypeDef GPIO_InitStructure;
//    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
//    GPIO_Init(GPIOA, &GPIO_InitStructure);

//    mDelay(10);
//    if(GET_KEY_SLIENT == RESET)
//    {
//        mixer.Slinet = FALSE;
//    }
//    else
//    {
//        mixer.Slinet = TRUE;
//    }
//}

//void SlientKeyHandler()
//{

//    static uint32_t LastHandleTime;
//    static uint32_t HighCnt;
//    static uint32_t LowCnt;
//    //FlagStatus Level;

//    if(GetSystemTime() < LastHandleTime + 1 * 10)
//        return;

//    LastHandleTime = GetSystemTime();

//    
//    if(GET_KEY_SLIENT == RESET)
//    {
//        LowCnt++;
//        HighCnt = 0;
//        if(LowCnt >= 100)
//        {
//            mixer.Slinet = FALSE;
//        }
//        
//    }
//    else
//    {
//        HighCnt++;
//        LowCnt = 0;
//        if(HighCnt >= 100)
//        {
//            mixer.Slinet = TRUE;
//        }
//        //mixer.Slinet = TRUE;
//    }
//}

