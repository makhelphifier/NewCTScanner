#ifndef IXRAYSOURCE_H
#define IXRAYSOURCE_H

// 硬件接口：X射线源
class IXRaySource {
public:
    // 虚析构函数
    virtual ~IXRaySource() {}

    // 定义标准操作
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool setVoltage(double kv) = 0;
    virtual bool turnOn() = 0;
    virtual bool turnOff() = 0;
};

#endif // IXRAYSOURCE_H
