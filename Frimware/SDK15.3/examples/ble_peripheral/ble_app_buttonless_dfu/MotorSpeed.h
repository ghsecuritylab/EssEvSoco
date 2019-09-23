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
    
    uint8_t MaxGearLevel;//���λ
    uint8_t GeraLevel;//��ǰ��λ
    uint16_t Ratio[MAX_GEAR_LEVEL + 1];//�ݱȸ��
    
    uint8_t UpGearSpeedPercent;
    uint8_t AfterUpGearSpeedPercent;
    uint16_t UpGearDelayTime;
    uint16_t DownGearDelayTime;

    uint32_t InGearDownTime;//���뽵����ʱ��
    uint32_t InGearUpTime;//��������ǰ����ʱ

    uint16_t UpGearThrottlePosition;//����������PWMλ��
    uint16_t DownGearThrottlePosition;
    ThrottlePositionStatusFlag ThrottlePosition;
    
    int16_t ChangeGearSpeed;//�����ٶȣ����ٹ����ٶȴﵽ��ֵ��������
    int16_t AfterUpGearSpeed;//������ɺ󣬷�����Ӧ�е��ٶ�
    
};



struct MotorSpeedInfo
{
    int16_t DistSpeed;
    int16_t DistSpeedRaw;
    int16_t RealSpeed;
    int16_t SlientSpeed;//��Ĭʱ���ٶȣ���Ϩ��ʱ��¼�þ�Ĭ�ٶȣ����ٶȱ�����ٶȴ�һЩ��ʱ����ʼ����
    
    BreakFunctionType BreakCfg;//ɲ������
    BreakStatusFlag BreakDirect;
    uint16_t BreakDetectTime;
    //unsigned short int BreakForwardOffset;//ɲ����ǰ����ֵ�洢�����ں��������������ƫ����
    //unsigned short int BreakBackwardOffset;
    uint16_t BreakForwardOutPosition;
    uint16_t BreakBackwardOutPosition;
    FunctionalState DetectBreakFlag;

    VehicleStatusFlag VehicleStatu;

    uint16_t UpGearKpDownPercent;


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
    volatile int16_t TurnAdcValue;
    int16_t TurnVoltage;  
};

extern struct MotorSpeedInfo MotorSpeed;
extern struct TurnInfo Turn;


int MotorSpeedSetByMixerConfig(unsigned char *BreakCfg, uint16_t *BreakDetectTime,
                                                          unsigned short int *BreakForwardOffset, unsigned short int *BreakBackwardOffset);
int GearBoxSetByMixerConfig(FunctionalState *GearBoxEnableFlag, uint8_t *MaxGearLevel, uint8_t *UpGearSpeedPercent, uint8_t *AfterUpGearSpeedPercent,
                                                                 uint16_t *UpGearDelayTime, uint16_t *DownGearDelayTime);
int16_t GetMotorRealSpeedWithoutGearbox(int16_t DistSpeed, int16_t RealSpeed);
int16_t GetMotorRealSpeedWithGearBox(int16_t DistSpeed, int16_t RealSpeed);
int16_t BreakEventHandler(int16_t DistSpeed);

void GearBoxInit(void);
void GearBoxSpeedReset(void);
void GetGearBoxAccelerateArray(void);

void EvMotorSpeedHwInit(void);
void EvMotorSpeedHandler(void);
uint16_t CalDistSpeed(void);
bool IsTurnOverStart(void);
bool IsTurnBelowStop(void);
void ThrottleAdcSample(void);





#endif


