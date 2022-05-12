//#include "StdAfx.h"  
#include "SerialPort.h"

using namespace std;

string CSerialPort::RX_Data;//static变量 在类外初始化,需要放在main外
int _tmain(int argc, _TCHAR *argv[]) {

    CSerialPort mySerialPort;//首先将之前定义的类实例化
    int length = 8;//定义传输的长度

    unsigned char *temp = new unsigned char[8];//动态创建一个数组

    if (!mySerialPort.InitPort(4, CBR_115200, 'N', 8, 1, EV_RXCHAR))//是否打开串口，4就是你外设连接电脑的com口，可以在设备管理器查看，然后更改这个参数
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

    /*这里发送的10进制，得到的是一个一个的16进制*/
    int test=12345;
    mySerialPort.DataConversion(test,1,temp);//将uint转换为temp16进制数组进行传送
    cout << mySerialPort.WriteData(temp, 8) << endl;//这个函数就是给串口发送数据的函数，temp就是要发送的数组。
    cout << mySerialPort.GetBytesInCOM() << endl;//这个函数就是显示返回值函数
    //delete temp[];
    //_sleep(1*1000);
//    for(int i=0;i<mySerialPort.RX_Data.size();i++)
//        cout<<mySerialPort.RX_Data[i]<<endl;
    cout<<mySerialPort.RX_Data<<endl;
    system("pause");
    return 0;
}