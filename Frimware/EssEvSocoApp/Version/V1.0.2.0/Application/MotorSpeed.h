#ifndef __MOTOR_SPEED__
#define __MOTOR_SPEED__


typedef enum {GO_FORWARD = 0, GO_BACKWARD = 1, BREAK = 2} VehicleStatusFlag;
typedef enum {FORWARD_BREAK = 0, BACKWARD_BREAK = 1} BreakStatusFlag;
typedef enum {BREAK_CLOSE = 0, BREAK_DUAL_DIRECT = 1, BREAK_SINGLE_DIRECT = 2} BreakFunctionType;
typedef enum {DOWN_GEAR_THROTTLE_POSITION = 0, HOLD_GEAR_THROTTLE_POSITION = 1, 
                         UP_GEAR_THROTTLE_POSITION =2, REVERSE_THROTTLE_POSITION = 3} ThrottlePositionStatusFlag;
typedef enum {NORMAL_DRIVE = 0, GEAR_UP = 1, GEAR_UPING = 2, GEAR_DOWN = 3, GEAR_UP_WAITING = 4} GearBoxStatusFlag;
typedef enum {EV_SPEED_HOLD = 1, EV_SPEED1 = 2, EV_SPEED2 = 3, EV_SPEED3 = 4} EvSpeedLevelInfo;
typedef enum {SPEED_MODE_HAL = 1, SPEED_MODE_TURN = 0} SpeedModeInfo;


//���������λ��
#define MAX_GEAR_LEVEL					6
#define DEFAULT_GEAR_LEVEL			        4


//ת�ѵĸ���λ�ö�Ӧ�ĵ�ѹ����λmv
#define VOLTAGE_TURN_START      850//900//850
#define VOLTAGE_TURN_STOP       (VOLTAGE_TURN_START - 30)
#define VOLTAGE_TURN_IDLE       (VOLTAGE_TURN_START + 50)
#define VOLTAGE_TURN_MAX        3300//3700

//ʵʱ�ٶȼ���ı�Ƶ��λ�ơ�1 << 3 = 8   ;����Ƶ��̫�ͻ�����AmplitudeFactorͻ�䵼����Ƶ����С��ͻ�䣬�Ӷ���֥������
//����ʱ�����޸ģ������Ӱ�������Ӧ
#define REAL_SPEED_CAL_MULT_SHIFT           3






struct GearBoxInfo
{
    FunctionalState GearBoxEnableFlag;
    GearBoxStatusFlag GearBoxStatus;
    
    u8 MaxGearLevel;//���λ
    u8 GeraLevel;//��ǰ��λ
    u16 Ratio[MAX_GEAR_LEVEL + 1];//�ݱȸ��
    
    u8 UpGearSpeedPercent;
    u8 AfterUpGearSpeedPercent;
    u16 UpGearDelayTime;
    u16 DownGearDelayTime;

    u32 InGearDownTime;//���뽵����ʱ��
    u32 InGearUpTime;//��������ǰ����ʱ

    u16 UpGearThrottlePosition;//����������PWMλ��
    u16 DownGearThrottlePosition;
    ThrottlePositionStatusFlag ThrottlePosition;
    
    s16 ChangeGearSpeed;//�����ٶȣ����ٹ����ٶȴﵽ��ֵ��������
    s16 AfterUpGearSpeed;//������ɺ󣬷�����Ӧ�е��ٶ�
    
};



struct MotorSpeedInfo
{
    s16 DistSpeed;
    s16 DistSpeedRaw;
    s16 RealSpeed;
    s16 SlientSpeed;//��Ĭʱ���ٶȣ���Ϩ��ʱ��¼�þ�Ĭ�ٶȣ����ٶȱ�����ٶȴ�һЩ��ʱ����ʼ����
    
    BreakFunctionType BreakCfg;//ɲ������
    BreakStatusFlag BreakDirect;
    u16 BreakDetectTime;
    //unsigned short int BreakForwardOffset;//ɲ����ǰ����ֵ�洢�����ں��������������ƫ����
    //unsigned short int BreakBackwardOffset;
    u16 BreakForwardOutPosition;
    u16 BreakBackwardOutPosition;
    FunctionalState DetectBreakFlag;

    VehicleStatusFlag VehicleStatu;

    u16 UpGearKpDownPercent;


    int16_t EvMaxSpeed;//�綯������ת�٣����ŵ�λ�ı仯���仯
    int16_t EvHoldSpeed;//������HOLD�ٶȺ󣬳���һ��ʱ�䣬�ٶȻ��𽥻��䵽HOLD�ٶ�
    uint32_t EvAccRatio;
    EvSpeedLevelInfo EvSpeedLevel;

    SpeedModeInfo SpeedMode;

    uint8_t ThrottleCurveLevel;//HALL���ŵ����ߣ�����EXP���ߣ���Χ0-10��0Ϊб��Ϊ1��ֱ�ߡ�

    struct GearBoxInfo GearBox;
};

struct TurnInfo
{
    volatile uint16_t TurnAdcValue;
    uint16_t TurnVoltage;  
};

extern struct MotorSpeedInfo MotorSpeed;
extern struct TurnInfo Turn;


int MotorSpeedSetByMixerConfig(unsigned char *BreakCfg, u16 *BreakDetectTime,
                                                          unsigned short int *BreakForwardOffset, unsigned short int *BreakBackwardOffset);
int GearBoxSetByMixerConfig(FunctionalState *GearBoxEnableFlag, u8 *MaxGearLevel, u8 *UpGearSpeedPercent, u8 *AfterUpGearSpeedPercent,
                                                                 u16 *UpGearDelayTime, u16 *DownGearDelayTime);
s16 GetMotorRealSpeedWithoutGearbox(s16 DistSpeed, s16 RealSpeed);
s16 GetMotorRealSpeedWithGearBox(s16 DistSpeed, s16 RealSpeed);
s16 BreakEventHandler(s16 DistSpeed);

void GearBoxInit(void);
void GearBoxSpeedReset(void);
void GetGearBoxAccelerateArray(void);

void EvMotorSpeedHwInit(void);
void EvMotorSpeedHandler(void);
uint16_t CalDistSpeed(void);
bool IsTurnOverStart(void);
bool IsTurnBelowStop(void);




#endif


