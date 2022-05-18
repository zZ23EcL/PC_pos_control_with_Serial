# 基于STM32速度环的上位机位置环控制

## MiniPID的用法
``` cpp
MiniPID pid=MiniPID(1,0,0);
//set any other PID configuration options here. 

while(true){
  //get some sort of sensor value
  //set some sort of target value
  double output=pid.getOutput(sensor,target);
  //do something with the output
  delay(50);
}
```

## 串口通讯数据的定义
mode：00 stop
01 start
02 target
03 sensor
