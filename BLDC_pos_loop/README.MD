# 基于FAULHABER MC5010S 的驱动设置

## 之前的问题 
阅读手册 发现USB连接只能与Motion Manager进行通讯设置，如果使用RS232的话需要用X2 COM的三根线,即USB只能用于和他自己的软件连接
![图片](通讯说明.png)

/## 2022/07/04
首先如果要使用RS232作为通讯，需要一个RS232转usb，不是CH340，CH340是TTL
硬件购买完毕了
然后是通讯协议

|Byte|SOF|length|NODE|COMMAND|DATA|CRC|EOF|
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
|length|1|1|1|1|DataLenght|1|1|

后续的命令中忽略SOF和EOF为

|Byte|length|NODE|COMMAND|DATA|CRC|
|:---:|:---:|:---:|:---:|:---:|:---:|
|length|1|1|1|DataLenght|1|

现在的commucation settings 0x0038302

启动：
|Byte|length|NODE|COMMAND|DATA|CRC|
|:---:|:---:|:---:|:---:|:---:|:---:|
|length|1|1|1|DataLenght|1|
|content|length|NodeNum|0x00|Devicename|CRC|

??RS232用不了

启动流程
我这里只有一个NODE 所以NODENUM=1

如果要进行设置看的RS232通讯手册和驱动功能两个PDF


## 2022/07/05

需要设定单位换算
### *控制字*
控制器启动 控制字 0x6040 0x00 +U16
下面指的是U16
首先使能0x0006->0x0007->0x000F
退出控制 去使能->0x0007->0x0006
### *状态字*
0x6041 0x00 +U16 只读
若进入操作状态则为0x000 0010*111

### *配置驱动器*
这里不用管，因为已经通过Motion MAnager保存在机器里了
即到4.8因子群？目前的策略是都不管，按减速比60在上位机处处理

### *工作模式选择*
关键是要从这里读取需要的参数

0x6060 0x00 设置 +S8
0x6061 0x00 读取
应该在软件里设置过了，所以这里的读取一次确认就可以了
如果是CSV的话应该是9 CSP为8
CSP ：目标位置0x607A
      实际位置0x6064
      实际速度0x606C
      实际转矩0x6077

CSV： 目标速度0x60FF
      实际位置0x6064
      实际速度0x606C
      实际转矩0x6077
大致上就是这些

### *RS232*
SOF - S
EOF - E

CRC是计算2-N的 就是除了SOF和EOF的部分

用户数据长度（除去首位标识）不能超过62字节，也就是加上不能超过64字节

command 部分 controlword 0x04
             stateword   0x05
             tracelog    0x06
读取参数好像用了SDO，这个后面看，确实是用了SDO 在command=0x01和0x02时进行处理

然后关于stateword的读取，他不是请求发送的形式进行的，而是当state发生变化时自动发送的

### *CRC计算*
计算方法如图所示  
![CRC计算方法](CRC%E8%AE%A1%E7%AE%97%E6%96%B9%E6%B3%95.png)
```python
def calcCRC(temp):
    CRC=0xFF
    Polynom=0xD5
    CRC=CRC^temp
    for i in range(8):
        if CRC&0x01:
            CRC=CRC(CRC>>1)^Polynom
        else:
            CRC>>=1
    return CRC

```
这是单个字节的CRC计算方法，如果计算多个字节的结果也是比较简单的，先计算第一个字节的crc结果，然后把第一个字节的crc结果与第二个字节进行异或， 异或后的值再进行一次crc计算就可以了，多个字节也是反复这过程就好

这里在写C++程序的时候还有一个需要注意的问题是，数组数据的传递，指针什么的再看一眼

### *关于几种位置的程序示例*
在驱动功能的第五章对各个运行模式的介绍中有详细的示例，这里对需要使用的CSV和CSP进行介绍
#### CSV
![CSV](CSV.png)
#### CSP
![CSP](CSP.png)



## 2022/07/06

早上基本把收发都DEBUG完了，下一步在转换器到之前可以考虑先尝试做一下pybind

### *接线*
2 RX
3 Tx
5 GND

电机的控制突然不行了，不是接线的问题

## 2022/07/07 
查出来了，是编码器的问题，霍尔可以用，但是效果不是很好  
增加了得到actual data的request字符串生成函数  
发现CRC计算部分存在问题  
例如 需要得到 actual pos的request应为  
53 07 01 01 64 60 00 56 45  
而程序计算得到的为
53 07 01 01 64 60 00 9C 45  
CRC不同，考虑是不是CRC计算顺序出错了？？
CRC-8
CRC polynom 0xD5 = 1101 0101


## 2022/07/08
数据传输总是从最小数据开始的
CRC -8 DVB S2

## 2022/07/09
![RS232](RS232%E8%AE%BE%E7%BD%AE.png)

问题解决 抄漏等号了。。。。


## 2022/07/11
接收到的字符串是：53 06 01 05 21 14 C8 45 ，这里不太对
设置模式为PV，应该是
如果是设置速度为1000 应该字符串是53 0B 01 02 FF 60 00 E8 03 00 00 D6 45
好像是先选模式再使能  
直接按照软件记录的来好像不行，需要再看看怎么弄
先选模式PV 然后使能走 06 0F 然后设置速度1000，这样电机不转

Motion manger软件中的写控制字命令和通讯手册里面给出的不一样，经过尝试软件里的不对
初步判断是设置控制模式那里不对，因为发送数据之后没有反馈

## 2022/07/12 
不是数据的问题，是串口写的问题
有个同步异步收发，在drive function 中的 communication-general-communication settings中
设置transmit async messages via RS232
现在是取消异步串口
异步串口：以字符为单位传送，位之间是同步的，字节是异步的
同步串口：字符和位都同步