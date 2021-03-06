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


//变速箱最大档位数
#define MAX_GEAR_LEVEL					6
#define DEFAULT_GEAR_LEVEL			        4


//转把的各个位置对应的电压，单位mv
#define VOLTAGE_TURN_START      850//900//850
#define VOLTAGE_TURN_STOP       (VOLTAGE_TURN_START - 30)
#define VOLTAGE_TURN_IDLE       (VOLTAGE_TURN_START + 50)
#define VOLTAGE_TURN_MAX        3300//3700

//实时速度计算的倍频的位移。1 << 3 = 8   ;加速频率太低会由于AmplitudeFactor突变导致音频数据小幅突变，从而有芝麻粒声
//减速时不做修改，否则会影响减速响应
#define REAL_SPEED_CAL_MULT_SHIFT           3






struct GearBoxInfo
{
    FunctionalState GearBoxEnableFlag;
    GearBoxStatusFlag GearBoxStatus;
    
    uint8_t MaxGearLevel;//最大档位
    uint8_t GeraLevel;//当前档位
    uint16_t Ratio[MAX_GEAR_LEVEL + 1];//齿比概念，
    
    uint8_t UpGearSpeedPercent;
    uint8_t AfterUpGearSpeedPercent;
    uint16_t UpGearDelayTime;
    uint16_t DownGearDelayTime;

    uint32_t InGearDownTime;//进入降档的时间
    uint32_t InGearUpTime;//用作升档前的延时

    uint16_t UpGearThrottlePosition;//换挡的油门PWM位置
    uint16_t DownGearThrottlePosition;
    ThrottlePositionStatusFlag ThrottlePosition;
    
    int16_t ChangeGearSpeed;//换挡速度，加速过程速度达到该值进行升档
    int16_t AfterUpGearSpeed;//换挡完成后，发动机应有的速度
    
};



struct MotorSpeedInfo
{
    int16_t DistSpeed;
    int16_t DistSpeedRaw;
    int16_t RealSpeed;
    int16_t SlientSpeed;//静默时的速度，当熄火时记录该静默速度，当速度比这个速度大一些的时，开始启动
    
    BreakFunctionType BreakCfg;//刹车配置
    BreakStatusFlag BreakDirect;
    uint16_t BreakDetectTime;
    //unsigned short int BreakForwardOffset;//刹车向前配置值存储，用于和油门中立点计算偏移量
    //unsigned short int BreakBackwardOffset;
    uint16_t BreakForwardOutPosition;
    uint16_t BreakBackwardOutPosition;
    FunctionalState DetectBreakFlag;

    VehicleStatusFlag VehicleStatu;

    uint16_t UpGearKpDownPercent;


    int16_t EvMaxSpeed;//电动车最大的转速，随着档位的变化而变化
    int16_t EvHoldSpeed;//当超过HOLD速度后，持续一段时间，速度会逐渐回落到HOLD速度
    uint32_t EvAccRatio;
    EvSpeedLevelInfo EvSpeedLevel;

    SpeedModeInfo SpeedMode;

    uint8_t ThrottleCurveLevel;//HALL油门的曲线，采用EXP曲线，范围0-10，0为斜率为1的直线。

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


