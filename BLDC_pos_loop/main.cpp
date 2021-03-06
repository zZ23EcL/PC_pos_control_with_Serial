//#include "StdAfx.h"  
#include "SerialPort.h"
#include "MiniPID.h"
#include<bitset>

#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)

using namespace std;
const int bitnum=8;

int _tmain(int argc, _TCHAR *argv[]) {

    CSerialPort mySerialPort;//首先将之前定义的类实例化

    int length = 8;//定义传输的长度



//
//    unsigned char *temp = new unsigned char[8];//动态创建一个数组
//
    if (!mySerialPort.InitPort(6, CBR_115200, 'N', 8, 1, EV_RXCHAR))//是否打开串口，6就是你外设连接电脑的com口，可以在设备管理器查看，然后更改这个参数
    {
        std::cout << "initPort fail !" << std::endl;
    } else {
        std::cout << "initPort success !" << std::endl;
    }
    if (!mySerialPort.OpenListenThread())//是否打开监听线程，开启线程用来传输返回值
    {
        std::cout << "OpenListenThread fail !" << std::endl;
    } else {
        std::cout << "OpenListenThread success !" << std::endl;
    }

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

//    uint8_t test[7];
//    mySerialPort.writeControlWord(test,1);
//
//    uint8_t datatemp[13];
//    mySerialPort.writeData(datatemp,12345,2);
//
//    //readBuf test
//    uint8_t buftest[13];
//    buftest[0]='S';
//    buftest[1]=0x0A;
//    buftest[2]=0x01;
//    buftest[3]=0x01;
//    buftest[4]=0x64;
//    buftest[5]=0x60;
//    buftest[6]=0x00;
//    buftest[7]=0x39;
//    buftest[8]=0x30;
//    buftest[9]=0x00;
//    buftest[10]=0x00;
//    buftest[11]=0xFF;
//    buftest[12]='E';
//    uint8_t t=0;
//    uint32_t v=0;
//    t=mySerialPort.readBuffer(buftest,&v);
//    printf("%d\n",t);
//    printf("%d\n",v);
//    uint8_t stattst[8];
//    stattst[0]='S';
//    stattst[1]=8;
//    stattst[2]=0x01;
//    stattst[3]=0x05;
//    stattst[4]=0x41;
//    stattst[5]=0x60;
//    stattst[6]=0x00;
//    stattst[7]=0x21;
//    stattst[8]=0x00;
//    stattst[9]=0xFF;
//    stattst[10]='E';
//    t=mySerialPort.readBuffer(stattst,0);
//    printf("%d\n",t);

/* **************************/
/*    设置控制模式为PV         */
/* **************************/
//    printf("\nSet Operation Mode:  PV....\n");
//    uint8_t SetOpMode[10];
//    mySerialPort.writeOperationMode(SetOpMode,0x03);
//    for(int i=0;i<10;i++)
//        printf("%x ",SetOpMode[i]);
//    mySerialPort.WriteData(SetOpMode, 10);
//    printf("\nSet PV Mode Finished!\n");

    while(1){

        //暂停
        if(KEY_DOWN('S')){//需要大写
            system("pause");
        }
        //结束
        if(KEY_DOWN('E')){//需要大写
            break;
        }
        //启动，建立连接并进入控制模式,经过测试是可以的
        if(KEY_DOWN('G')){
            printf("\nStart Up! Connecting Motor...\n");
            uint8_t controlw[8];
            mySerialPort.writeControlWord(controlw,1);
            mySerialPort.WriteData(controlw, 8);
            mySerialPort.writeControlWord(controlw,5);
            mySerialPort.WriteData(controlw, 8);
            mySerialPort.writeControlWord(controlw,3);
            mySerialPort.WriteData(controlw, 8);
            printf("Connecting Finished!\n");
            printf("\nSet Velocity=1000....\n");
            uint8_t datatemp[13];
            mySerialPort.getData(datatemp,1000,1);
            mySerialPort.WriteData(datatemp, 13);
            Sleep(20);
        }
        if(KEY_DOWN('C')){
            int type;
            uint8_t Ctype;
            uint32_t Cvalue;
            uint8_t datatemp[13];
            printf("\n1: Speed_demand  2: Position_demand\nplease input control type  :");
            cin >> type;
            if(type==1)
                Ctype=0x01;
            else if (type==2)
                Ctype=0x03;
            else
                Ctype=0x00;
            printf("\nplease input control value  :");
            cin >> Cvalue;
            mySerialPort.getData(datatemp,Cvalue,Ctype);
            cout << mySerialPort.WriteData(datatemp, 13) << endl;
            if (!mySerialPort.OpenListenThread())//是否打开监听线程，开启线程用来传输返回值
            {
                std::cout << "OpenListenThread fail !" << std::endl;
            } else {
                std::cout << "OpenListenThread success !" << std::endl;
            }
        }
        //断开连接，退出控制使能
        if(KEY_DOWN('U')){
            printf("\nSwitch OFF! Disconnecting Motor...\n");
            uint8_t controlw[8];
            mySerialPort.writeControlWord(controlw,2);
            mySerialPort.WriteData(controlw, 8);
            Sleep(20);
            printf("Closed Connecting Finished!\n");
        }
        //重新启动
        if(KEY_DOWN('R')){
            printf("\nReStart Up! Connecting Motor...\n");
            uint8_t controlw[8];
            mySerialPort.writeControlWord(controlw,4);
            mySerialPort.WriteData(controlw, 8);
            Sleep(20);
            mySerialPort.writeControlWord(controlw,3);
            mySerialPort.WriteData(controlw, 8);
            printf("ReConnecting Finished!\n");
            Sleep(20);
        }
        if(KEY_DOWN('V')){
            printf("\nSet Operation Mode:  PV....\n");
            uint8_t SetOpMode[10];
            mySerialPort.writeOperationMode(SetOpMode,0x03);
            for(int i=0;i<10;i++)
                printf("%x ",SetOpMode[i]);
            mySerialPort.WriteData(SetOpMode, 10);
            printf("\nSet PV Mode Finished!\n");
            Sleep(20);
        }
        if(KEY_DOWN('P')){
            printf("\nSet Operation Mode:  PP....\n");
            uint8_t SetOpMode[10];
            mySerialPort.writeOperationMode(SetOpMode,0x01);
            mySerialPort.WriteData(SetOpMode, 10);
            printf("\nSet PP Mode Finished!\n");
            Sleep(20);
        }
        if(KEY_DOWN('B')){
            printf("\nSet Velocity=1000....\n");
            uint8_t datatemp[13];
            mySerialPort.getData(datatemp,1000,1);
            for(int i=0;i<13;i++)
                printf("%x ",datatemp[i]);
            mySerialPort.WriteData(datatemp, 13);
            Sleep(20);
        }
        if(KEY_DOWN('N')){
            printf("\nSet Velocity=0....\n");
            uint8_t datatemp[13];
            mySerialPort.getData(datatemp,0,1);
//            for(int i=0;i<13;i++)
//                printf("%x ",datatemp[i]);
            mySerialPort.WriteData(datatemp, 13);
            Sleep(20);
        }
        if(KEY_DOWN('T')){
            printf("\nOPERATION TEST\n");
            uint8_t modetemp[9];
            modetemp[0]='S';
            modetemp[1]=0x07;
            modetemp[2]=0x01;
            modetemp[3]=0x01;
            modetemp[4]=0x60;
            modetemp[5]=0x60;
            modetemp[6]=0x00;
            modetemp[7]=0x52;
            modetemp[8]='E';
            mySerialPort.WriteData(modetemp, 9);
            Sleep(20);
            modetemp[4]=0x61;
            modetemp[7]=0x53;
            mySerialPort.WriteData(modetemp, 9);
            Sleep(20);
        }
        //修改数据
        Sleep(20);//循环时间间隔，防止太占内存,这个值不能设置的太大，否则有问题，之前的200就有问题
    }
//    uint8_t temp[9];
//    mySerialPort.getDataRequest(temp,0x03);
//    for(int i=0;i<9;i++)
//        printf("%x ",temp[i]);
//    cout<<endl;
//    uint8_t posbuffer[13];
//    posbuffer[0]='S';
//    posbuffer[1]=0x0B;
//    posbuffer[2]=0x01;
//    posbuffer[3]=0x01;
//    posbuffer[4]=0x64;
//    posbuffer[5]=0x60;
//    posbuffer[6]=0x00;
//    posbuffer[7]=0x00;
//    posbuffer[8]=0x00;
//    posbuffer[9]=0x00;
//    posbuffer[10]=0x00;
//    posbuffer[11]=0x5A;
//    posbuffer[12]='E';
//    uint8_t type;
//    uint32_t value;
//    type=mySerialPort.readBuffer(posbuffer,&value);
//    printf("%x  %d",type,value);

//    uint8_t CRC=0xFF;
//    CRC = mySerialPort.CalcCRCByte(0x00, CRC);
//    printf("%x ",CRC);
//    CRC = mySerialPort.CalcCRCByte(0x60, CRC);
//    printf("%x ",CRC);
//    CRC = mySerialPort.CalcCRCByte(0x64, CRC);
//    printf("%x ",CRC);
//    CRC = mySerialPort.CalcCRCByte(0x01, CRC);
//    printf("%x ",CRC);
//    CRC = mySerialPort.CalcCRCByte(0x01, CRC);
//    printf("%x ",CRC);
//    CRC = mySerialPort.CalcCRCByte(0x07, CRC);
//    printf("%x ",CRC);
    return 0;
}
