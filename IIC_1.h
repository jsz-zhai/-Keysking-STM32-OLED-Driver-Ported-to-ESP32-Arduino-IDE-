#ifndef IIC_1_H
#define IIC_1_H

#include <stdint.h>
#include <Arduino.h>


#define SDA_PIN 19    // D5 引脚作为 SDA 数据线
#define SCL_PIN 20   // D17 引脚作为 SCL 数据线

// 通过宏定义, 在后续程序中便捷控制SDA输入与输出切换
#define SDA_OUT()  pinMode(SDA_PIN, OUTPUT)   // 设置SDA为输出
#define SDA_IN()   pinMode(SDA_PIN, INPUT)    // 设置SDA为输入

void IIC_init(void);                                // 初始化
void sleep_us(uint32_t us);                         // 精确延时（传入延时微秒数）
void IIC_Start(void);                               // 开始传输: 发送 "起始位"
void IIC_SendAddress(uint8_t addr, uint8_t rw);     // 发送从机地址，并附带读/写控制位
void IIC_SendByte(uint8_t dat);                     // 发送数据：发送一字节数据到从机
void IIC_Stop(void);                                // 传输结束：发送 "结束位"
uint8_t IIC_Wait_Ask(void);                         // 等待应答
void IIC_Ack(void);                                 // 主机应答函数
uint8_t IIC_ReadByte(void);                         // 主机读数据函数

//综合上面所有函数,进行数据发送
void myiic_go(uint8_t slave_address, uint8_t *data, uint16_t length);

#endif  //IIC_1_H
