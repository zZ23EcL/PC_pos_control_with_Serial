# PC_pos_control_with_Serial
基于STM32的速度环在PC上做的位置环控制  
## 2022/5/12  
增加了RX_Data变量方便读取串口信息
## 2022/5/18
修改了RX_BUF显示错误的BUG，是由于主线程比子线程先结束导致的，增加了字符串到数字的显示，需要基于c++11标准以上的thread和QT进行开发
## 2022/7/6
增加了FaulHaber电机的RS232控制通讯信息，并且修复的BUG
## 2022/7/7
增加了读取实际参数的request命令字符数组生成函数，并且发现了CRC计算存在问题需要修复
## 2022/7/11
CRC问题修复完毕，现能与FaulHaber电机正常通讯，使能功能完成，速度控制存在问题
## 2022/7/11
传输数据的构成确认无误，串口本身的调用的程序存在问题
