#include <stdio.h>
#include <string.h>

//#include <xdc/std.h>
//#include <ti/sysbios/BIOS.h>
//#include <ti/sysbios/knl/Semaphore.h>
//#include <ti/sysbios/knl/Queue.h>
//#include <ti/sysbios/knl/Task.h>
//#include <ti/sysbios/knl/Clock.h>
//#include <ti/sysbios/knl/Event.h>

#include "board.h"
#include <ti/drivers/uart/UARTCC26XX.h>
#include "uart.h"

#include "hci_tl.h"

#include "simple_gatt_profile.h"


#define SIZE_RX_BUFF			1000

#define CNT_SEND_CMD_POOL	21

#define FRAME_START 			0xFA
#define FRAME_END			0xFD
#define FRAME_TRANSFER		0xFE


struct SendCmdPoolInfo
{
    uint8_t Buff[CNT_SEND_CMD_POOL];
    uint8_t Cnt;
};


#define CNT_NOOD_BLE_NOTIFY_POOL        50
struct BleNotifyPoolNoodInfo
{
    uint8_t Buff[20];
    uint8_t Cnt;
};

struct BleNotifyPoolInfo
{
    struct BleNotifyPoolNoodInfo Node[CNT_NOOD_BLE_NOTIFY_POOL];
    uint8_t Head, Tail;
};

struct BleNotifyPoolInfo BleNotifyPool;



UART_Handle UARTHandle;
UART_Params UARTparams;
uint8_t SendBuff[50];
uint8_t ReadBuff[100];
uint8_t RxBuff[SIZE_RX_BUFF];


//��������أ�˲ʱ��BLE�յ����ݿ��ܿ��ڴ��ڷ������ݣ��ᵼ�����ݶ�ʧ������ͨ������صķ�ʽ��ֹ���ݶ�ʧ
struct SendCmdPoolInfo SendCmdPool[CNT_SEND_CMD_POOL];
uint8_t SendCmdPoolHead, SendCmdPoolTail;

bool UartSendFlag;



uint32_t RxBuffHead, RxBuffTail;


void Uart_ReadCallback(UART_Handle handle, void *rxBuf, size_t size);
void UartSendOnePacket(uint8_t *buff, uint32_t cnt);
void Uart_WriteCallback(UART_Handle handle, void *txBuf, size_t size);


void UartHwInit()
{
    UART_init();                                      //��ʼ��ģ��Ĵ��ڹ���
    UART_Params_init(&UARTparams);                    //��ʼ�����ڲ���
    UARTparams.baudRate = 115200;                     //���ڲ�����115200
    UARTparams.dataLength = UART_LEN_8;               //��������λ8
    UARTparams.stopBits = UART_STOP_ONE;              //����ֹͣλ1
    UARTparams.readDataMode = UART_DATA_BINARY;       //���ڽ������ݲ�������
    UARTparams.writeDataMode = UART_DATA_BINARY;        //���ڷ������ݲ�������
    UARTparams.readMode = UART_MODE_CALLBACK;         //�����첽��
    UARTparams.writeMode = UART_MODE_CALLBACK;        //�����첽д
    UARTparams.readEcho = UART_ECHO_OFF;              //���ڲ�����
    UARTparams.readReturnMode = UART_RETURN_FULL;  //�����յ����з�ʱ���ص�
    UARTparams.readCallback = Uart_ReadCallback;      //���ڶ��ص�
    UARTparams.writeCallback = Uart_WriteCallback;    //����д�ص�
    UARTparams.readTimeout   = 10000 / Clock_tickPeriod;
    
    UARTHandle = UART_open(Board_UART0, &UARTparams); //�򿪴���ͨ��
    UART_control(UARTHandle, UARTCC26XX_RETURN_PARTIAL_ENABLE,  NULL);   //������ղ��ֻص�
    
    UART_read(UARTHandle, ReadBuff, 100);       //��һ�����ڶ�
}

uint32_t StartTime;
uint32_t StopTime;
uint32_t UsedTime;
void UartSendPacket(uint8_t *buff, uint32_t cnt)
{
    //�����͵����ݷŵ����ͳ���
    SendCmdPool[SendCmdPoolTail].Cnt = cnt;
    memcpy(SendCmdPool[SendCmdPoolTail].Buff, buff, cnt);
    SendCmdPoolTail++;
    if(SendCmdPoolTail >= CNT_SEND_CMD_POOL)
        SendCmdPoolTail = 0;

    //ȷ�ϴ����Ƿ����ڷ�������
    if(UartSendFlag == false)
    {
        //����û�з������ݣ��������ݷ���
        UartSendFlag = true;
        UartSendOnePacket(SendCmdPool[SendCmdPoolHead].Buff, SendCmdPool[SendCmdPoolHead].Cnt);
        SendCmdPoolHead++;
        if(SendCmdPoolHead >= CNT_SEND_CMD_POOL)
            SendCmdPoolHead = 0;
    }
}

void UartSendOnePacket(uint8_t *buff, uint32_t cnt)
{
    int32_t i;
    int32_t TransFormCnt = 0;
    int32_t SendCnt;
    uint8_t Crc;
    uint8_t data;
    uint8_t *BuffPtr;

    //StartTime = Clock_getTicks();
    Crc = 0;
    for(i = 0; i < cnt; i++)
    {
        Crc ^= buff[i];
    }

    BuffPtr = SendBuff;
    *BuffPtr++ = 0xFA;
    for(i = 0; i < cnt; i++)
    {
        data = buff[i];
        if(data == 0xFA || data == 0xFE || data == 0xFD)
        {
            *BuffPtr++ = 0xFE;
            TransFormCnt++;
        }
        *BuffPtr++ = data;
    }

    if(Crc == 0xFA || Crc == 0xFE || Crc == 0xFD)
    {
        *BuffPtr++ = 0xFE;
        TransFormCnt++;
    }
    *BuffPtr++ = Crc;
    *BuffPtr = 0xFD;

    //�ܹ����͵�������Ϊ��������+ ת���ַ�����+ 3���ֽ�(��ʼ��������CRC)
    SendCnt = cnt + TransFormCnt + 3;
    UART_write(UARTHandle, SendBuff, SendCnt);
}


//���ڻص����������ݷ�����ɺ󣬽��б������Ļص�
void Uart_WriteCallback(UART_Handle handle, void *txBuf, size_t size)
{
    //����Ƿ���������Ҫ��������
    if(SendCmdPoolHead != SendCmdPoolTail)
    {
        //��Ȼ�����ݣ���������
        UartSendOnePacket(SendCmdPool[SendCmdPoolHead].Buff, SendCmdPool[SendCmdPoolHead].Cnt);
        SendCmdPoolHead++;
        if(SendCmdPoolHead >= CNT_SEND_CMD_POOL)
            SendCmdPoolHead = 0;
    }
    else
    {
        //�����������
        UartSendFlag = false;
    }
}

uint32_t ReadActCnt;
void Uart_ReadCallback(UART_Handle handle, void *rxBuf, size_t size)
{
    //int32_t ReadCnt;
    int32_t FirstCnt, SecondCnt;
    
    if(size != 0)
    {
        if(RxBuffTail + size <= SIZE_RX_BUFF)
        {
            memcpy(RxBuff + RxBuffTail, ReadBuff, size);
        }
        else
        {
            FirstCnt = SIZE_RX_BUFF - RxBuffTail;
            SecondCnt = size - FirstCnt;
            memcpy(RxBuff + RxBuffTail, ReadBuff, FirstCnt);
            memcpy(RxBuff, ReadBuff + FirstCnt, SecondCnt);
        }
        RxBuffTail += size;
        RxBuffTail = RxBuffTail % SIZE_RX_BUFF;
    }
    UART_read(UARTHandle, ReadBuff, 100);     
}

//void Uart_ReadCallback(UART_Handle handle, void *rxBuf, size_t size)
//{
//    int32_t ReadCnt;
//    int32_t FirstCnt, SecondCnt;
//    StartTime = Clock_getTicks();
//    ReadCnt = UART_read(UARTHandle, ReadBuff, 100);     
//    StopTime = Clock_getTicks();
//    UsedTime = StopTime - StartTime;
//    if(ReadCnt != 0)
//    {
//        ReadActCnt++;
//        if(RxBuffTail + ReadCnt <= SIZE_RX_BUFF)
//        {
//            memcpy(RxBuff + RxBuffTail, ReadBuff, ReadCnt);
//        }
//        else
//        {
//            FirstCnt = SIZE_RX_BUFF - RxBuffTail;
//            SecondCnt = ReadCnt - FirstCnt;
//            memcpy(RxBuff + RxBuffTail, ReadBuff, FirstCnt);
//            memcpy(RxBuff, ReadBuff + FirstCnt, SecondCnt);
//        }
//        RxBuffTail += ReadCnt;
//        RxBuffTail = RxBuffTail % SIZE_RX_BUFF;
//    }
//}



//�����ݽ���У�飬ȷ�����������֪ͨ
void IndicateOneFrame(uint8_t *buff, uint8_t len)
{
    uint8_t i;
    uint8_t crc = 0;
    for(i = 0; i < len; i++)
    {
        crc ^= buff[i];
    }

    if(crc != 0)
        return;

    //SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR3, len - 1, buff);
    memcpy(BleNotifyPool.Node[BleNotifyPool.Tail].Buff, buff, len - 1);
    BleNotifyPool.Node[BleNotifyPool.Tail].Cnt = len - 1;
    BleNotifyPool.Tail++;
    if(BleNotifyPool.Tail >= CNT_NOOD_BLE_NOTIFY_POOL)
        BleNotifyPool.Tail = 0;
}


//���ڽ��յ��Ĵ������ݽ��д���
//������ȥ�����ڴ����Э�飬
//Ȼ����APP����֪ͨ
#define MAX_BUFF_SIZE_PACKET			21 //20�ֽ�����+1�ֽڵ�CRC
void UartReceivePacketHandler()
{
    static uint8_t RxPacketBuff[MAX_BUFF_SIZE_PACKET];//���21�ֽ�
    static bool IsGetPacketHead;
    static bool TransferFlag;
    static uint8_t PacketLength;
    uint8_t data;
    while(RxBuffHead != RxBuffTail)
    {
        data = RxBuff[RxBuffHead];
        if(IsGetPacketHead != false)
        {
            if(TransferFlag == false)
            {
                if(data == FRAME_START)
                {
                    //���������淢����ʼλ������Ϊǰһ֡��Ϣû���յ�����λ�����¿�ʼ���ݵļ�¼
                    PacketLength = 0;
                }
                else if(data == FRAME_END)
                {
                    //�յ�������������һ֡���ݵ�֪ͨ
                    IndicateOneFrame(RxPacketBuff, PacketLength);
                    IsGetPacketHead = false;
                }
                else if(data == FRAME_TRANSFER)
                {
                    //�յ�ת���ַ�������¼��RxPacketBuff�У�PacketLength������
                    TransferFlag = true;
                }
                else
                {
                    RxPacketBuff[PacketLength] = data;
                    PacketLength++;
                    if(PacketLength > MAX_BUFF_SIZE_PACKET)
                    {
                        //���ݳ��ȳ�����������ĳ��ȣ���Ȼ��ͬʱ��©����ʼλ�ͽ���λ
                        IsGetPacketHead = false;
                    }
                }
            }
            else
            {
                //��һ�ֽ���ת���ַ�
                if(data == FRAME_START || data == FRAME_END || data == FRAME_TRANSFER)
                {
                    RxPacketBuff[PacketLength] = data;
                    PacketLength++;
                    if(PacketLength > MAX_BUFF_SIZE_PACKET)
                    {
                        //���ݳ��ȳ�����������ĳ��ȣ���Ȼ��ͬʱ��©����ʼλ�ͽ���λ
                        IsGetPacketHead = false;
                    }
                }
                else
                {
                    //ת���ַ�������ַ�������Ч�ַ�����֡����
                    IsGetPacketHead = false;
                }
                TransferFlag = false;
            }
        }
        else
        {
             if(TransferFlag == false)
             {
                 if(data == FRAME_START)
                 {
                     //�ҵ�֡ͷ
                      IsGetPacketHead = true;
                      PacketLength = 0;
                 }
                 else if(data == FRAME_TRANSFER)
                 {
                     //����ת�壬�����ת���ж�
                     TransferFlag = true;
                 }
             }
             else
             {
                 TransferFlag = false;
             }
        }
        
        RxBuffHead++;
        if(RxBuffHead >= SIZE_RX_BUFF)
        {
            RxBuffHead = 0;
        }
    }
}


void BleNotifyHandler()
{
    static uint32_t HandleCnt = 0;
    HandleCnt++;
    if(HandleCnt >= 25)
    {
        HandleCnt = 0;
    }
    else
    {
        return;
    }

    if(BleNotifyPool.Head == BleNotifyPool.Tail)
        return;

    
    SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR3, BleNotifyPool.Node[BleNotifyPool.Head].Cnt, BleNotifyPool.Node[BleNotifyPool.Head].Buff);
    BleNotifyPool.Head++;
    if(BleNotifyPool.Head >= CNT_NOOD_BLE_NOTIFY_POOL)
        BleNotifyPool.Head = 0;
}


