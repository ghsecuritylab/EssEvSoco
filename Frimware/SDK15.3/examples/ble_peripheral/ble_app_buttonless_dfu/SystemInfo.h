#ifndef __SYSTEM_INFO__
#define __SYSTEM_INFO__

#include "common.h"


//�̼��汾��Ϣ
#define ESS_ONE_PLUS_FIRMWARE_VERSION			"v1.0.0.0"
//��̼���Ӧ�����ð汾��Ϣ
#define ESS_ONE_PLUS_CONFIG_VERSION				0x00000001
//Ӳ���İ汾��Ϣ
#define ESS_ONE_PLUS_HARDWARE_VERSION		"ESS-P12-V2.2"
//Frog��������ģ������Ʒ���
//#define FROG_PRODUCT_ID				1



#define AMP_MAX_POWER					30


//#define ESS_ONE_PLUS_PRODUCT_ID    							0x010A
//#define AES_OFFSET											86

#define OFFSET_MODULUS										1
#define OFFSET_PUBLIC_EXPONENT								68
#define OFFSET_EXPONENT										168
#define OFFSET_PRIME0										250			
#define OFFSET_PRIME1										301
#define OFFSET_PRIME_EXPONENT0								364
#define OFFSET_PRIME_EXPONENT1								402
#define OFFSET_COEFFICIENT									479


#define OFFSET_SECRET										580




//#define MAIN_APP_START_ADDR			    0x00004000
#define OnChipAudioMenuAddr				0x00072000   
#define SYSTEM_INFO_ADDR				0x00073000
#define SECURE_START_ADDR				0x0007F000


//#define UNIQUE_ID_ADDR_051			    0x1FFFF7E8//0x1FFFF7AC
//#define ID0_ADDR						(UNIQUE_ID_ADDR_051)
//#define ID1_ADDR						(UNIQUE_ID_ADDR_051 + 4)
//#define ID2_ADDR						(UNIQUE_ID_ADDR_051 + 8)


#define FIRMWARE_VERSION_1			1
#define FIRMWARE_VERSION_2			0
#define FIRMWARE_VERSION_3			13
#define FIRMWARE_VERSION_4			0



#define MAX_INDEX_SYSTEM_INFO		14

#define PROTOCAL_VERSION_BLE4       (1 << 0)
#define PROTOCAL_VERSION_BLE42      (1 << 1)
#define PRODUCT_TYPE				0x0A06
#define PROTOCAL_VERSION			(PROTOCAL_VERSION_BLE4 | PROTOCAL_VERSION_BLE42)
#define MODEL						"ESS EV MICORMAX"
#define MANUFACTURE					"SenseHobby Inc."
#define HARDWARE_VERSION			"AAA"
#define BOOTLOADER_VERSION			0

#define BLOCK_SIZE_DATA_TRANSFER	1024
#define VEHICLE_CNT					4


#define LENGTH_DEVICE_ID                16
struct SystemsInfo
{
    unsigned char DeviceId[16];
    FunctionalState MassStorageWriteEnableFlag;
};


extern struct SystemsInfo SystemInformation;

void GetDeviceId(void);
int CheckSystemConfigVersion(unsigned int *ConfigVersion);
void ItIsASecret(void);
int IsDeviceProduced(void);


int32_t GetOneSystemInfo(uint8_t *buff, uint8_t Index);


#endif

