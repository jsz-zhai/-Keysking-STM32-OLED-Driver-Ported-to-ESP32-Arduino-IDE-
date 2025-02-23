#include "IIC_1.h"

// 我们将创建一个 IIC_Pin 类，用于封装GPIO引脚的操作，
class IIC_Pin {
  private:
    uint8_t pin;  // 存储引脚号

  public:
    // 构造函数：设置引脚号并初始化引脚模式
    IIC_Pin(uint8_t pin) {
      this->pin = pin;
      pinMode(pin, OUTPUT);  // 将引脚设置为输出模式
      digitalWrite(pin, LOW);  // 初始化引脚为低电平
    }

    // 重载 = 运算符，实现高低电平控制
    void operator=(int level) {
      if (level == 0) {
        digitalWrite(pin, LOW);  // 设置引脚为低电平
      } else if (level == 1) {
        digitalWrite(pin, HIGH);  // 设置引脚为高电平
      }
    }
};

// 初始化并重载 operator= 运算符，使其能够直接通过 IIC_SCL = 1; 这样的语句来控制引脚电平。
IIC_Pin IIC_SCL(SCL_PIN);  // SCL引脚
IIC_Pin IIC_SDA(SDA_PIN);  // SDA引脚

void IIC_init(void) {
    SDA_OUT();      // 设置SDA为输出模式
    IIC_SCL = 0;    // 设置SCL为低电平
    IIC_SDA = 0;    // 设置SDA为低电平
}

// 精确延时（传入延时微秒数）
void sleep_us(uint32_t us) {
  uint64_t start_time = esp_timer_get_time();  // 获取当前时间（微秒）
  while (esp_timer_get_time() - start_time < us) {
    // 等待指定的微秒数
  }
}

// 开始传输: 发送 "起始位"
void IIC_Start(void) {
    SDA_OUT();      // 确保SDA为输出模式
    IIC_SCL = 1;    // 拉高SCL
    IIC_SDA = 1;    // 拉高SDA
    sleep_us(4);    // 等待SDA稳定处于高电平
    IIC_SDA = 0;    // 在SCL高电平时拉低SDA, 产生(1变0)信号
    sleep_us(4);    // 等待SDA处于稳定的低电平状态
    IIC_SCL = 0;    // 拉低SCL, 避免数据误差
}

// 发送从机地址，并附带读/写控制位
// addr 为 7 位从机地址，rw 为读写控制位（0 为写，1 为读）
// 示例：
//     IIC_SendAddress(0x50, 0);  // 向地址为 0x50 的设备发送写请求（0 为写）
//     IIC_SendAddress(0x50, 1);  // 向地址为 0x50 的设备发送读请求（1 为读）
void IIC_SendAddress(uint8_t addr, uint8_t rw) {
    uint8_t addressWithRW = (addr << 1) | (rw & 0x01);  // 将 7 位地址左移 1 位，并加入读/写位

    SDA_OUT();    // 设置SDA为输出
    IIC_SCL = 0;  // 确保SCL为低电平

    // 逐位发送地址和读/写位
    for (uint8_t i = 0; i < 8; i++) {
        // 发送每一位
        IIC_SDA = (addressWithRW >> (7 - i)) & 0x01;  // 获取地址和读写位的每一位
        sleep_us(2);                                  // 延时，保证时序稳定
        IIC_SCL = 1;                                  // SCL 拉高，发送数据
        sleep_us(2);                                  // 稳定延时
        IIC_SCL = 0;                                  // SCL 拉低，准备发送下一位
    }

    IIC_SDA = 1;
}

// 发送数据：发送一字节数据到从机
void IIC_SendByte(uint8_t dat) {
    uint8_t i = 0;

    SDA_OUT();    // 设置SDA为输出
    IIC_SCL = 0;  // 确保SCL为低电平

    for (i = 0; i < 8; i++) {
        // 发送数据的每一位
        IIC_SDA = (dat >> 7) & 0x01;  // 发送数据位
        sleep_us(2);                  // 延时，保证时序正确
        IIC_SCL = 1;                  // SCL为高电平，发送数据
        sleep_us(2);                  // 延时，保证时序正确
        IIC_SCL = 0;                  // 拉低SCL，准备下一个数据位
        dat <<= 1;                    // 左移数据，准备发送下一位
    }
}

// 传输结束：数据发送完成，发送结束位(在SCL为高电平时发送 "0变1" 信号)
void IIC_Stop(void) {
    SDA_OUT();        // 设置SDA为输出
    IIC_SCL = 0;      // 确保SCL为低电平
    IIC_SDA = 0;      // 确保SDA为低电平
    sleep_us(4);      // 延时，确保信号稳定

    IIC_SCL = 1;      // 将SCL拉高，准备发送停止条件
    sleep_us(2);      // 稍微延时，确保SCL稳定

    IIC_SDA = 1;      // 将SDA拉高，生成停止信号（SDA在SCL高电平时，发送 "0变1" 信号）
    sleep_us(4);      // 延时，确保停止信号稳定
}


// 等待应答:从机接收数据后需要应答,此时需要接收应答信号判断发送是否正常
uint8_t IIC_Wait_Ask(void) {  
    uint8_t time = 0;  

    IIC_SCL = 1;     // 拉高 SCL，等待应答
    sleep_us(2);
    IIC_SDA = 1;     // 确保 SDA 为高电平
    sleep_us(2);
    SDA_IN();        // 设置 SDA 为输入模式，等待从设备应答

    // 检查 SDA 是否被拉低，从设备是否应答
    while (digitalRead(SDA_PIN) == 1) {
        time++;
        if (time >= 100) {  // 如果超过 200 次循环，认为超时
            IIC_Stop();    // 停止信号
            return 1;      // 返回 1 表示超时，未收到应答
        }
    }
    
    IIC_SCL = 0;     // 拉低 SCL，结束此周期
    return 0;         // 返回 0 表示成功收到应答
}


//主机应答函数
void IIC_Ack(void) {
    IIC_SCL = 0;          // 拉低 SCL，准备发送数据
    SDA_OUT();            // 设置 SDA 为输出模式
    IIC_SDA = 0;          // 设置 SDA 为低电平来表示应答
    sleep_us(2);          // 延时 2 微秒，确保信号稳定
    IIC_SCL = 1;          // 拉高 SCL，完成 ACK 信号
    sleep_us(2);          // 延时 2 微秒，确保 SCL 稳定
    IIC_SCL = 0;          // 拉低 SCL，结束此周期
}

//主机读数据函数
uint8_t IIC_ReadByte(void) {
    uint8_t i = 0;
    uint8_t tmp = 0;

    SDA_IN();  // 设置 SDA 为输入模式，准备读取数据

    for (i = 0; i < 8; i++) {
        IIC_SCL = 0;            // 拉低 SCL，准备读取数据
        sleep_us(2);            // 延时确保 SCL 拉低稳定
        IIC_SCL = 1;            // 拉高 SCL，准备读取数据
        sleep_us(2);            // 延时确保 SCL 拉高稳定

        // 读取 SDA 状态并将其保存到 tmp 中
        tmp |= (digitalRead(SDA_PIN) << (7 - i));  // 按位读取 SDA
    }

    IIC_Ack();  // 发送 ACK 应答
    return tmp;  // 返回读取的字节数据
}


//综合上面所有函数,进行数据发送
void myiic_go(uint8_t slave_address, uint8_t *data, uint16_t length) {

  // 发送起始位
  IIC_Start();

  // 发送从机地址，0 表示写操作
  IIC_SendAddress(slave_address, 0);

  // 等待从机应答
  if (IIC_Wait_Ask() == 1) { // 1 表示未收到应答
    IIC_Stop(); // 如果未应答,发送停止位,串口打印错误信息
    Serial.println("地址发送:从机无应答,发送结束位");
    return;
  }
  for (uint16_t i = 0; i < length; i++) {
    IIC_SendByte(data[i]);
    if (IIC_Wait_Ask() == 1) {
      IIC_Stop(); // 如果未应答,发送停止位,串口打印错误信息
      Serial.println("数据发送:从机无应答,发送结束位");
      return;
    }
  }
  IIC_Stop(); // 信息发送完毕,发送停止位
}