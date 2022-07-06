//#include "StdAfx.h"  
#include "SerialPort.h"
#include "MiniPID.h"

using namespace std;

void ntest(uint32_t *num){
    *num=100;
}

int _tmain(int argc, _TCHAR *argv[]) {

    CSerialPort mySerialPort;//首先将之前定义的类实例化

//    int length = 8;//定义传输的长度
//    int DataMode;//数据类型
//    int Data;//数据
//    MiniPID PID=MiniPID(1,0,0);
//    /**PID变量*/
//    double target;
//    double sensor;
//    double control2stm32;
//    bool start;
//
//    unsigned char *temp = new unsigned char[8];//动态创建一个数组
//
//    if (!mySerialPort.InitPort(4, CBR_115200, 'N', 8, 1, EV_RXCHAR))//是否打开串口，4就是你外设连接电脑的com口，可以在设备管理器查看，然后更改这个参数
//    {
//        std::cout << "initPort fail !" << std::endl;
//    } else {
//        std::cout << "initPort success !" << std::endl;
//    }
//    if (!mySerialPort.OpenListenThread())//是否打开监听线程，开启线程用来传输返回值
//    {
//        std::cout << "OpenListenThread fail !" << std::endl;
//    } else {
//        std::cout << "OpenListenThread success !" << std::endl;
//    }
//
//    /*这里发送的10进制，得到的是一个一个的16进制*/
//    int test=12345;
//    mySerialPort.DataConversion(test,1,temp);//将uint转换为temp16进制数组进行传送
//    cout << mySerialPort.WriteData(temp, 8) << endl;//这个函数就是给串口发送数据的函数，temp就是要发送的数组。
//    cout << mySerialPort.GetBytesInCOM() << endl;//这个函数就是显示返回值函数
//    //delete temp[];
//
//    cout<<RX_BUF<<endl;
//
//
//    system("pause");
//    cout<<RX_BUF<<endl;
//    cout<<PID.str2data(RX_BUF)<<endl;
//
//    /*
//    while(start){
//        //get some sort of sensor value
//
//        //set some sort of target value
//        double output=pid.getOutput(sensor,target);
//        //do something with the output
//        mySerialPort.WriteData(temp, 8);
//        Sleep(50);//等于Delay_ms(50)
//    }
//     */

    uint8_t test[7];
    mySerialPort.writeControlWord(test,1);

    uint8_t datatemp[13];
    mySerialPort.writeData(datatemp,12345,2);

    //readBuf test
    uint8_t buftest[13];
    buftest[0]='S';
    buftest[1]=0x0A;
    buftest[2]=0x01;
    buftest[3]=0x01;
    buftest[4]=0x64;
    buftest[5]=0x60;
    buftest[6]=0x00;
    buftest[7]=0x39;
    buftest[8]=0x30;
    buftest[9]=0x00;
    buftest[10]=0x00;
    buftest[11]=0xFF;
    buftest[12]='E';
    uint8_t t=0;
    uint32_t v=0;
    t=mySerialPort.readBuffer(buftest,&v);
    printf("%d\n",t);
    printf("%d\n",v);
    uint8_t stattst[8];
    stattst[0]='S';
    stattst[1]=8;
    stattst[2]=0x01;
    stattst[3]=0x05;
    stattst[4]=0x41;
    stattst[5]=0x60;
    stattst[6]=0x00;
    stattst[7]=0x21;
    stattst[8]=0x00;
    stattst[9]=0xFF;
    stattst[10]='E';
    t=mySerialPort.readBuffer(stattst,0);
    printf("%d\n",t);
    return 0;
}
