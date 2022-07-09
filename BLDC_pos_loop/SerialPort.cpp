//
// Created by zz23ecl on 2022/5/10.
//
// SerialPort.cpp : 定义控制台应用程序的入口点。

//#include "StdAfx.h"
#include "SerialPort.h"
#include "MiniPID.h"

using namespace std;
/** 线程退出标志 */
bool CSerialPort::s_bExit = false;
/** 当串口无数据时,sleep至下次查询间隔的时间,单位:毫秒 */
const UINT SLEEP_TIME_INTERVAL = 5;
/**  static变量 在类外初始化,需要放在main外，用于保存接收的数据   **/
//string CSerialPort::RX_Data;
//string RX_BUF="";

CSerialPort::CSerialPort(void)
        : m_hListenThread(INVALID_HANDLE_VALUE) {
    m_hComm = INVALID_HANDLE_VALUE;
    m_hListenThread = INVALID_HANDLE_VALUE;
    InitializeCriticalSection(&m_csCommunicationSync);
}

CSerialPort::~CSerialPort(void) {
    CloseListenTread();
    ClosePort();
    DeleteCriticalSection(&m_csCommunicationSync);
}

//初始化串口函数
bool CSerialPort::InitPort(UINT portNo /*= 1*/, UINT baud /*= CBR_9600*/, char parity /*= 'N'*/,
                           UINT databits /*= 8*/, UINT stopsbits /*= 1*/, DWORD dwCommEvents /*= EV_RXCHAR*/) {

    /** 临时变量,将制定参数转化为字符串形式,以构造DCB结构 */
    char szDCBparam[50];
    sprintf_s(szDCBparam, "baud=%d parity=%c data=%d stop=%d", baud, parity, databits, stopsbits);

    /** 打开指定串口,该函数内部已经有临界区保护,上面请不要加保护 */
    if (!openPort(portNo)) {
        return false;
    }

    /** 进入临界段 */
    EnterCriticalSection(&m_csCommunicationSync);

    /** 是否有错误发生 */
    BOOL bIsSuccess = TRUE;

    /** 在此可以设置输入输出的缓冲区大小,如果不设置,则系统会设置默认值.
    *  自己设置缓冲区大小时,要注意设置稍大一些,避免缓冲区溢出
    */
    /*if (bIsSuccess )
    {
    bIsSuccess = SetupComm(m_hComm,10,10);
    }*/

    /** 设置串口的超时时间,均设为0,表示不使用超时限制 */
    COMMTIMEOUTS CommTimeouts;
    CommTimeouts.ReadIntervalTimeout = 0;
    CommTimeouts.ReadTotalTimeoutMultiplier = 0;
    CommTimeouts.ReadTotalTimeoutConstant = 0;
    CommTimeouts.WriteTotalTimeoutMultiplier = 0;
    CommTimeouts.WriteTotalTimeoutConstant = 0;
    if (bIsSuccess) {
        bIsSuccess = SetCommTimeouts(m_hComm, &CommTimeouts);
    }

    DCB dcb;
    if (bIsSuccess) {
        //// 将ANSI字符串转换为UNICODE字符串
        //DWORD dwNum = MultiByteToWideChar(CP_ACP, 0, szDCBparam, -1, NULL, 0);
        //wchar_t *pwText = new wchar_t[dwNum];
        //if (!MultiByteToWideChar(CP_ACP, 0, szDCBparam, -1, pwText, dwNum))
        //{
        //    bIsSuccess = TRUE;
        //}

        /** 获取当前串口配置参数,并且构造串口DCB参数 */
        bIsSuccess = GetCommState(m_hComm, &dcb) && BuildCommDCB(szDCBparam, &dcb);
        /** 开启RTS flow控制 */
        dcb.fRtsControl = RTS_CONTROL_ENABLE;

        /** 释放内存空间 */
        /*delete[] pwText;*/
    }

    if (bIsSuccess) {
        /** 使用DCB参数配置串口状态 */
        bIsSuccess = SetCommState(m_hComm, &dcb);
    }

    /**  清空串口缓冲区 */
    PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

    /** 离开临界段 */
    LeaveCriticalSection(&m_csCommunicationSync);

    return bIsSuccess == TRUE;
}

//初始化串口函数
bool CSerialPort::InitPort(UINT portNo, const LPDCB &plDCB) {
    /** 打开指定串口,该函数内部已经有临界区保护,上面请不要加保护 */
    if (!openPort(portNo)) {
        return false;
    }

    /** 进入临界段 */
    EnterCriticalSection(&m_csCommunicationSync);

    /** 配置串口参数 */
    if (!SetCommState(m_hComm, plDCB)) {
        return false;
    }

    /**  清空串口缓冲区 */
    PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

    /** 离开临界段 */
    LeaveCriticalSection(&m_csCommunicationSync);

    return true;
}

//关闭关闭串口
void CSerialPort::ClosePort() {
    /** 如果有串口被打开，关闭它 */
    if (m_hComm != INVALID_HANDLE_VALUE) {
        CloseHandle(m_hComm);
        m_hComm = INVALID_HANDLE_VALUE;
    }
}

//打开出串口
bool CSerialPort::openPort(UINT portNo) {
    /** 进入临界段 */
    EnterCriticalSection(&m_csCommunicationSync);

    /** 把串口的编号转换为设备名 */
    char szPort[50];
    sprintf_s(szPort, "COM%d", portNo);

    /** 打开指定的串口 */
    m_hComm = CreateFileA(szPort,  /** 设备名,COM1,COM2等 */
                          GENERIC_READ | GENERIC_WRITE, /** 访问模式,可同时读写 */
                          0,                            /** 共享模式,0表示不共享 */
                          NULL,                         /** 安全性设置,一般使用NULL */
                          OPEN_EXISTING,                /** 该参数表示设备必须存在,否则创建失败 */
                          0,
                          0);

    /** 如果打开失败，释放资源并返回 */
    if (m_hComm == INVALID_HANDLE_VALUE) {
        LeaveCriticalSection(&m_csCommunicationSync);
        return false;
    }

    /** 退出临界区 */
    LeaveCriticalSection(&m_csCommunicationSync);

    return true;
}

//打开监听线程
bool CSerialPort::OpenListenThread() {
    /** 检测线程是否已经开启了 */
    if (m_hListenThread != INVALID_HANDLE_VALUE) {
        /** 线程已经开启 */
        return false;
    }
    s_bExit = false;
    /** 线程ID */
    UINT threadId;
    /** 开启串口数据监听线程 */
    m_hListenThread = (HANDLE) _beginthreadex(NULL, 0, ListenThread, this, 0, &threadId);
    if (!m_hListenThread) {
        return false;
    }
    /** 设置线程的优先级,高于普通线程 */
    if (!SetThreadPriority(m_hListenThread, THREAD_PRIORITY_ABOVE_NORMAL)) {
        return false;
    }
    return true;
}

//关闭监听线程
bool CSerialPort::CloseListenTread() {
    if (m_hListenThread != INVALID_HANDLE_VALUE) {
        /** 通知线程退出 */
        s_bExit = true;

        /** 等待线程退出 */
        Sleep(10);

        /** 置线程句柄无效 */
        CloseHandle(m_hListenThread);
        m_hListenThread = INVALID_HANDLE_VALUE;
    }
    return true;
}

//获取串口缓冲区的字节数
UINT CSerialPort::GetBytesInCOM() {
    DWORD dwError = 0;  /** 错误码 */
    COMSTAT comstat;   /** COMSTAT结构体,记录通信设备的状态信息 */
    memset(&comstat, 0, sizeof(COMSTAT));

    UINT BytesInQue = 0;
    /** 在调用ReadFile和WriteFile之前,通过本函数清除以前遗留的错误标志 */
    if (ClearCommError(m_hComm, &dwError, &comstat)) {
        BytesInQue = comstat.cbInQue; /** 获取在输入缓冲区中的字节数 */
    }
    return BytesInQue;
}

//串口监听线程
UINT WINAPI CSerialPort::ListenThread(void *pParam) {
    /** 得到本类的指针 */
    CSerialPort *pSerialPort = reinterpret_cast<CSerialPort *>(pParam);

    // 线程循环,轮询方式读取串口数据
    while (!pSerialPort->s_bExit) {
        UINT BytesInQue = pSerialPort->GetBytesInCOM();
        /** 如果串口输入缓冲区中无数据,则休息一会再查询 */
        if (BytesInQue == 0) {
            Sleep(SLEEP_TIME_INTERVAL);
            continue;
        }
        string getBUF;
        /** 读取输入缓冲区中的数据并输出显示 */
        unsigned char cRecved = 0x00;
        do {
            cRecved = 0x00;
            if (pSerialPort->ReadChar(cRecved) == true) {
                std::stringstream ss;
                string b;
                int tm = cRecved;
                ss << std::hex << std::setw(2) << std::setfill('0') << tm;
                //ss << " ";
                string a = ss.str();
                transform(a.begin(), a.end(), back_inserter(b), ::toupper);
                getBUF = getBUF + b;
                continue;
            }
        } while (--BytesInQue);

        //cout<<endl;
        //cout<<getBUF;
        //RX_Data.push_back(getBUF);
//        RX_BUF=getBUF;
//        cout<<&RX_BUF<<endl;
//        cout<<&getBUF<<endl;
//        cout<<RX_BUF<<endl;
        if (getBUF != "")
            RX_BUF = getBUF;
        cout << RX_BUF << endl;
    }
    return 0;
}

//读取串口接收缓冲区中一个字节的数据
bool CSerialPort::ReadChar(unsigned char &cRecved) {
    BOOL bResult = TRUE;
    DWORD BytesRead = 0;
    if (m_hComm == INVALID_HANDLE_VALUE) {
        return false;
    }

    /** 临界区保护 */
    EnterCriticalSection(&m_csCommunicationSync);

    /** 从缓冲区读取一个字节的数据 */
    bResult = ReadFile(m_hComm, &cRecved, 1, &BytesRead, NULL);
    if ((!bResult)) {
        /** 获取错误码,可以根据该错误码查出错误原因 */
        DWORD dwError = GetLastError();

        /** 清空串口缓冲区 */
        PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);
        LeaveCriticalSection(&m_csCommunicationSync);

        return false;
    }

    /** 离开临界区 */
    LeaveCriticalSection(&m_csCommunicationSync);

    return (BytesRead == 1);

}

// 向串口写数据, 将缓冲区中的数据写入到串口
bool CSerialPort::WriteData(unsigned char *pData, int length) {
    int *pData1 = new int;
    BOOL bResult = TRUE;
    DWORD BytesToSend = 0;
    if (m_hComm == INVALID_HANDLE_VALUE) {
        return false;
    }

    /** 临界区保护 */
    EnterCriticalSection(&m_csCommunicationSync);

    /** 向缓冲区写入指定量的数据 */
    bResult = WriteFile(m_hComm,/*文件句柄*/pData,/*用于保存读入数据的一个缓冲区*/ 8,/*要读入的字符数*/ &BytesToSend,/*指向实际读取字节数的指针*/ NULL);
    if (!bResult) {
        DWORD dwError = GetLastError();
        /** 清空串口缓冲区 */
        PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);
        LeaveCriticalSection(&m_csCommunicationSync);

        return false;
    }

    /** 离开临界区 */
    LeaveCriticalSection(&m_csCommunicationSync);

    return true;
}

//对发送的数据进行处理
bool CSerialPort::DataConversion(unsigned int Data, unsigned int Mode, unsigned char *temp) {
    if (sizeof(Data) > 4)
        return false;

    temp[0] = Mode;
    temp[6] = 0x0D;
    temp[7] = 0x0A;
    temp[1] = 0x00;
    temp[5] = Data & 0xff;
    temp[4] = Data >> 8 & 0xff;
    temp[3] = Data >> 16 & 0xff;
    temp[2] = Data >> 24 & 0xff;

    return true;
}
/**********************************************************/
/*                 faulhaber电机RS232通讯部分               */
/**********************************************************/

//CRC计算,这里是计算一个u8的crc，使用时需要用类似的循环
/*
uint8_t CRC=0xFF;
for(int i=0;i++;i<5)
    CRC=mySerialPort.CalcCRCByte(*temp+i,CRC);
 */

uint8_t CSerialPort::CalcCRCByte(uint8_t u8Byte, uint8_t u8CRC) {
    uint8_t i;
    u8CRC = u8CRC ^ u8Byte;
    for (i = 0; i <8 ; i++) {
        if (u8CRC & 0x01) {
            u8CRC = (u8CRC >> 1) ^ 0xD5;
        }
        else {
            u8CRC >>= 1;
        }
    }
    return u8CRC;
}

//生成通讯字符串
uint8_t *CSerialPort::GenerateBuffer(uint8_t NodeNum, uint8_t command, uint8_t *data, uint8_t datalength) {
    uint8_t *temp = new unsigned char[datalength + 4];
    uint8_t CRC;
    temp[0] = datalength + 4;
    temp[1] = NodeNum;
    temp[2] = command;
    for (uint8_t i = 0; i++; i < datalength) {
        temp[3 + i] = data[i];
    }
    temp[datalength + 3] = CalcCRCByte(*temp, CRC);
    return temp;
}

//读取接收到的字符串
uint8_t CSerialPort::readBuffer(uint8_t *temp, uint32_t *value) {

    uint8_t length = *(temp + 1);
    uint8_t node = *(temp + 2);
    uint8_t command = *(temp + 3);
    uint8_t crc = *(temp + length - 2);
    uint16_t index;
    uint8_t subindex;
    uint16_t statusword;
    uint8_t type;
    uint32_t v = 0;
    //这里只有结点1
    if (node != 0x01) {
        printf("Error node %x", node);
        return 0;
    }
    switch (command) {
        //比较关键的就是0x01和0x05 两个部分
        //0x01 Response of SDO read service
        //控制参数speed pos这些就在这里
        case 0x01: {
            index = ((*(temp + 5)) << 8) + (*(temp + 4));
            subindex = *(temp + 6);
            for (uint8_t i = 0; i < length - 7; i++)
                v += *(temp + 7 + i) << (i * 8);
            *value = v;
            switch (index) {
                //实时位置
                case 0x6064: {
                    type = 1;
                    break;
                }
                    //实时速度
                case 0x606C: {
                    type = 2;
                    break;
                }
                    //实时力矩
                case 0x6077: {
                    type = 3;
                    break;
                }
                    //目标位置
                case 0x607A: {
                    type = 4;
                    break;
                }
                    //目标速度
                case 0x60FF: {
                    type = 5;
                    break;
                }
            }
            break;
        }
            //0x02 Response of SDO write request
        case 0x02: {
            index = ((*(temp + 5)) << 8) + (*(temp + 4));
            subindex = *(temp + 6);
            //这里的返回的是他们的object entry，不需要使用
            break;
        }
            //0x04  Response of write controlword
        case 0x04: {
            if (*(temp + 4) == 0) {
                printf("Error code=0,is OK!");
            }
            break;
        }
            //0x05 Receive statusword  U16
        case 0x05: {
            subindex = *(temp + 6);
            statusword = (*(temp + 7)) + ((*(temp + 8)) << 8);
            statusword = statusword & 0x00FF;
            //闭锁
            if (statusword & 0x40)
                type = 6;
            else {
                //待机
                if (!(statusword - 0x21) & 0xF)
                    type = 7;
                //开机
                if (!(statusword - 0x23) & 0xF)
                    type = 8;
                //操作
                if (!(statusword & 0x27) & 0xF)
                    type = 9;
            }
            break;
        }
        default: {
            printf("unknow message!");
            break;
        }
    }
    return type;
}

//写控制字
void CSerialPort::writeControlWord(uint8_t temp[], uint8_t type) {
    uint8_t length = 0x06;
    uint8_t NodeNum = 1;
    uint8_t command = 0x04;
    uint8_t controlWordLB;
    uint8_t controlWordHB;

    switch (type) {
        //停车
        case 1: {
            controlWordLB = 0x06;
            controlWordHB = 0x00;
            break;
        }
            //开机
        case 2: {
            controlWordLB = 0x07;
            controlWordHB = 0x00;
            break;
        }
            //使能
        case 3: {
            controlWordLB = 0x0F;
            controlWordHB = 0x00;
            break;
        }
        default: {
            printf("ControlWord Type Error!");
            break;
        }
    }
    temp[0] = 'S';
    temp[1] = length;
    temp[2] = NodeNum;
    temp[3] = command;
    temp[4] = controlWordLB;
    temp[5] = controlWordHB;
    uint8_t CRC = 0xFF;
    for (int i = 1; i <length; i++){
        printf("%x",temp[i]);
        CRC = CalcCRCByte(temp[i], CRC);}
    temp[6] = CRC;
    temp[7] = 'E';
}

//写目标数据
void CSerialPort::writeData(uint8_t temp[], int data, uint8_t type) {
    uint8_t length = 0x0a;
    uint8_t command = 0x02;
    uint8_t NodeNum = 0x01;
    uint8_t indexHB;
    uint8_t indexLB;
    uint8_t subindex;
    switch (type) {
        //速度
        case 1: {
            subindex = 0x00;
            indexLB = 0xFF;
            indexHB = 0x60;
            break;
        }
            //位置
        case 2: {
            subindex = 0x00;
            indexLB = 0x7A;
            indexHB = 0x60;
            break;
        }
        default: {
            printf("Erorr Data Type!");
            break;
        }
    }

    uint8_t dataBuf[4];
    dataBuf[3] = data & 0xff;
    dataBuf[2] = data >> 8 & 0xff;
    dataBuf[1] = data >> 16 & 0xff;
    dataBuf[0] = data >> 24 & 0xff;

    temp[0] = 'S';
    temp[1] = length;
    temp[2] = NodeNum;
    temp[3] = command;
    temp[4] = indexLB;
    temp[5] = indexHB;
    temp[6] = subindex;
    /********************************************/
    /*  这里有个问题是写入数据是按低位写还是高位优先写？ */
    /*               问题解决，按低位写            */
    /********************************************/
    //先按照低位先的来
    temp[7] = data & 0xff;
    temp[8] = data >> 8 & 0xff;
    temp[9] = data >> 16 & 0xff;
    temp[10] = data >> 24 & 0xff;
    uint8_t CRC = 0xFF;
    for (int i = 1; i < length; i++)
        CRC = CalcCRCByte(temp[i], CRC);
    //printf("%x\n",CRC);
    temp[11] = CRC;
    temp[12] = 'E';
}

//得到发送希望得到数据的请求的字符串
void CSerialPort::getDataRequest(uint8_t temp[], uint8_t type) {
    uint8_t indexLB;
    uint8_t indexHB;
    uint8_t CRC = 0xFF;
    uint8_t length = 0x07;
    //实际转矩
    if (type == 1) {
        indexLB = 0x77;
        indexHB = 0x60;
    }
        //实际速度
    else if (type == 2) {
        indexLB = 0x6C;
        indexHB = 0x60;
    }
        //实际位置
    else if (type == 3) {
        indexLB = 0x64;
        indexHB = 0x60;
    }
    temp[0] = 'S';
    temp[1] = length;
    temp[2] = 0x01;
    temp[3] = 0x01;
    temp[4] = indexLB;
    temp[5] = indexHB;
    temp[6] = 0x00;
    for (int i = 1; i <length; i++)
        CRC = CalcCRCByte(temp[i], CRC);
    temp[7] = CRC;
    temp[8] = 'E';
}

// 十六进制字符转换数字
int CSerialPort::str2data(string str) {
    int Data;
    size_t cnt = str.size();
    for (int i = 0; cnt; i++, cnt--) {
        if (!str[i]) {
            return false;
        }
        int v = 0;
        if (is_hex(str[i], v)) {
            Data = Data * 16 + v;
        }
    }
    return Data;
}

//判断是否为十六进制字符
bool CSerialPort::is_hex(char c, int &v) {
    if (0x20 <= c && isdigit(c)) {
        v = c - '0';
        return true;
    } else if ('A' <= c && c <= 'F') {
        v = c - 'A' + 10;
        return true;
    } else if ('a' <= c && c <= 'f') {
        v = c - 'a' + 10;
        return true;
    }
    return false;
}

//单字节按比特顺序取反
uint8_t CSerialPort::swap(uint8_t a) {
    uint8_t ans = 0;
    uint8_t temp = a;
    uint8_t x = 0x80;
    while (temp) {
        ans |= (temp & 0x01) ? x : 0;
        x >>= 1;
        temp >>= 1;
    }
    return ans;
}