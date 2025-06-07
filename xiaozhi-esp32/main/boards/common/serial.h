//#define SERIAL_H_

#ifndef SERIAL_H_
#define SERIAL_H_

#include "driver/uart.h"
#include <functional>
#include <list>
#include <mutex>

#define UART1_TX_GPIO GPIO_NUM_17
#define UART1_RX_GPIO GPIO_NUM_18
//#define UART1_PORT_NUM UART_NUM_1
#define UART1_PORT_NUM (uart_port_t)1
#define UART_BUF_SIZE       1024
#define UART_QUEUE_SIZE     20

class Serial{
public:
    Serial(int baud_rate=115200);
    ~Serial();
    void SerialTransmit(std::function<void()> callback);
    void SerialReceive(std::function<uint8_t(uint8_t*, int)> callback);
private:
    std::mutex mutex_;
    QueueHandle_t uart_queue_;
    int baud_rate_;
    std::function<void()> serial_transmit_;
    std::list<std::function<void(uint8_t*, int)>> serial_rx_tasks_;
    void SerialRxDetectTask();
};

#endif