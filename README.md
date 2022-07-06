# PC_pos_control_with_Serial
基于STM32的速度环在PC上做的位置环控制  
## 2022/5/12  
增加了RX_Data变量方便读取串口信息
## 2022/5/18
修改了RX_BUF显示错误的BUG，是由于主线程比子线程先结束导致的，增加了字符串到数字的显示，需要基于c++11标准以上的thread和QT进行开发
## 2022/7/6
增加了FaulHaber电机的RS232控制通讯信息，并且修复的BUG
