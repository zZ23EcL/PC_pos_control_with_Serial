//
// Created by zz23ecl on 2022/5/10.
//

#ifndef BLDC_POS_LOOP_SERIALPORT_H
#define BLDC_POS_LOOP_SERIALPORT_H

#include <process.h>
#include "TChar.h"
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include <cctype>
#include <Windows.h>
#include <stdlib.h>
#include <vector>


using namespace std;
/** 串口通信类
*
*  本类实现了对串口的基本操作
*  例如监听发到指定串口的数据、发送指定数据到串口
*/

typedef enum AxisType
{
    AXIS_XX = 2,
    AXIS_YY = 3,
    AXIS_ZZ = 1,
    AXIS_OO = 4,
}AXIS_TYPE;

extern string RX_BUF;

class CSerialPort
{
public:
    CSerialPort(void);
    ~CSerialPort(void);

public:

    static string RX_Data;
    string test_Data;
    //vector<int> RX_Mode;

    /** 初始化串口函数
    *
    *  @param:  UINT portNo 串口编号,默认值为1,即COM1,注意,尽量不要大于9
    *  @param:  UINT baud   波特率,默认为9600
    *  @param:  char parity 是否进行奇偶校验,'Y'表示需要奇偶校验,'N'表示不需要奇偶校验
    *  @param:  UINT databits 数据位的个数,默认值为8个数据位
    *  @param:  UINT stopsbits 停止位使用格式,默认值为1
    *  @param:  DWORD dwCommEvents 默认为EV_RXCHAR,即只要收发任意一个字符,则产生一个事件
    *  @return: bool  初始化是否成功
    *  @note:   在使用其他本类提供的函数前,请先调用本函数进行串口的初始化
    *　　　　　   /n本函数提供了一些常用的串口参数设置,若需要自行设置详细的DCB参数,可使用重载函数
    *           /n本串口类析构时会自动关闭串口,无需额外执行关闭串口
    *  @see:
    */
    bool InitPort(UINT  portNo = 4, UINT  baud = CBR_115200, char  parity = 'N', UINT  databits = 8, UINT  stopsbits = 1, DWORD dwCommEvents = EV_RXCHAR);
    //bool InitPort(UINT  portNo, UINT  baud, char  parity, UINT  databits, UINT  stopsbits, DWORD dwCommEvents);


    /** 串口初始化函数
    *
    *  本函数提供直接根据DCB参数设置串口参数
    *  @param:  UINT portNo
    *  @param:  const LPDCB & plDCB
    *  @return: bool  初始化是否成功
    *  @note:   本函数提供用户自定义地串口初始化参数
    *  @see:
    */
    bool InitPort(UINT  portNo, const LPDCB& plDCB);

    /** 开启监听线程
    *
    *  本监听线程完成对串口数据的监听,并将接收到的数据打印到屏幕输出
    *  @return: bool  操作是否成功
    *  @note:   当线程已经处于开启状态时,返回flase
    *  @see:
    */
    bool OpenListenThread();

    /** 关闭监听线程
    *
    *
    *  @return: bool  操作是否成功
    *  @note:   调用本函数后,监听串口的线程将会被关闭
    *  @see:
    */
    bool CloseListenTread();

    /** 向串口写数据
    *
    *  将缓冲区中的数据写入到串口
    *  @param:  unsigned char * pData 指向需要写入串口的数据缓冲区
    *  @param:  unsigned int length 需要写入的数据长度
    *  @return: bool  操作是否成功
    *  @note:   length不要大于pData所指向缓冲区的大小
    *  @see:
    */
    bool WriteData(unsigned char *pData, int length);

    /** 串口数据处理
    *
    *  将需要发送字符转换为16进制
    *  @param:  UINT DataMode 该种数据的类型
    *  @param:  UINT Data 数据大小
    *  @param:  unsigned char* temp
    *  @return: bool  转换成功
    *  @note:   数据大小范围为4个字节
    *  @see:
    */
    bool DataConversion(UINT Data, UINT Mode, unsigned char* temp);

    /** 接收串口数据处理
    *
    *  将接收到的16进制字符进行转换
    *  @param:  UINT *DataMode 该种数据的类型
    *  @param:  UINT *Data 数据大小
    *  @param:  unsigned char* temp
    *  @return: bool  转换成功
    *  @note:   数据大小范围为4个字节
    *  @see:
    */
    bool AntiDataConversion(unsigned int *Data, unsigned int *Mode, unsigned char* temp);

    /** 获取串口缓冲区中的字节数
    *
    *
    *  @return: UINT  操作是否成功
    *  @note:   当串口缓冲区中无数据时,返回0
    *  @see:
    */
    UINT GetBytesInCOM();
    /*UINT WriteData1(unsigned long long int *pData1, unsigned int length);*/

    /** 读取串口接收缓冲区中一个字节的数据
    *
    *
    *  @param:  char & cRecved 存放读取数据的字符变量
    *  @return: bool  读取是否成功
    *  @note:
    *  @see:
    */
    bool ReadChar(unsigned char &cRecved);


    unsigned char *MotorMoveXY(unsigned char x, unsigned char y);//xy相对移动
    unsigned char *StopMotor(unsigned char sm1);
    unsigned char *SetSpeed(AXIS_TYPE enAxisType, int speed);
    unsigned char *SetRunSpeed(int TY, int speed);

    /*******************************************/
    /*        以下用于与FAULHABER驱动通讯          */
    /*******************************************/


    /** CRC计算
    *
    *
    *  @param:  u8 Byte CRC
    *  @return: u8  CRC
    *  @note:
    *  @see:
    */
    uint8_t CalcCRCByte(uint8_t u8Byte,uint8_t u8CRC);

    uint8_t* GenerateBuffer(uint8_t NodeNum,uint8_t command,uint8_t* data,uint8_t datalenght);
    /*******************************************/
    /** 生成通讯的字符串
    *
    *
    *  @param:  u8 NodeNum，command，data,datalenth
    *  @return: u8*  Buffer
    *  @note:
    *  @see:
    */

    bool readBuffer(uint8_t *temp);
    /*******************************************/
    /** 读取FaulHaber发送的字符串
    *
    *
    *  @param:  u8*  Buffer 即接收到的字符串
    *  @return: bool 1为成功 0为识别失败
    *  @note:
    *  @see:
    */

    void writeControlWord(uint8_t temp[],uint8_t type);
    /*******************************************/
    /** 生成向FaulHaber写控制字的字符串
    *
    *
    *  @param:
    *  u8* temp 储存生成的数组
    *  u8  type controlword的种类
    *               1：停机
    *               2：开机
    *               3：使能
    *  @return:
    *  @note:
    *  @see:
    */


    void writeData(uint8_t temp[],int data,uint8_t type);
    /*******************************************/
    /** 生成向FaulHaber写数据的字符串
    *
    *
    *  @param:
    *  u8* temp 储存生成的数组
    *  int data 需要写入的数据
    *  u8  type 数据类型
    *               1：目标速度
    *               2：目标位置
    *  @return:
    *  @note:
    *  @see:
    */



    /*********************************/
    /*       faulhaber通讯部分结束     */
    /*********************************/

private:

    /** 打开串口
    *
    *
    *  @param:  UINT portNo 串口设备号
    *  @return: bool  打开是否成功
    *  @note:
    *  @see:
    */
    bool openPort(UINT  portNo);

    /** 关闭串口
    *
    *
    *  @return: void  操作是否成功
    *  @note:
    *  @see:
    */
    void ClosePort();

    /** 串口监听线程
    *
    *  监听来自串口的数据和信息
    *  @param:  void * pParam 线程参数
    *  @return: UINT WINAPI 线程返回值
    *  @note:
    *  @see:
    */
    static UINT WINAPI ListenThread(void* pParam);

private:

    /** 串口句柄 */
    HANDLE  m_hComm;

    /** 线程退出标志变量 */
    static bool s_bExit;

    /** 线程句柄 */
    volatile HANDLE    m_hListenThread;

    /** 同步互斥,临界区保护 */
    CRITICAL_SECTION   m_csCommunicationSync;       //!< 互斥操作串口

    /**  返回的字符串 */


};



#endif //BLDC_POS_LOOP_SERIALPORT_H
