#include <stdio.h>
#include <string.h>
#include "driver/uart.h"
#include <driver/gpio.h>
#include <esp_log.h>
#include "serial.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define TAG  "Serial"

Serial::Serial(int baud_rate)
{
    uart_config_t uart_config = {
        .baud_rate = baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    ESP_ERROR_CHECK(uart_param_config(UART1_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART1_PORT_NUM, UART1_TX_GPIO, UART1_RX_GPIO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART1_PORT_NUM, UART_BUF_SIZE, UART_BUF_SIZE, UART_QUEUE_SIZE, &uart_queue_, 0));

    xTaskCreate([](void* arg) {
    auto this_ = (Serial*)arg;
    this_->SerialRxDetectTask();
    vTaskDelete(NULL);
    }, "serial_rx_detection", 4096, this, 3, nullptr);
    ESP_LOGI(TAG, "create"); 
}

Serial::~Serial()
{
    ESP_LOGI(TAG, "delete\r\n");
}

void Serial::SerialTransmit(std::function<void()> callback)
{
    mutex_.lock();
    serial_transmit_ = callback;
    serial_transmit_();
    mutex_.unlock();
}

void Serial::SerialReceive(std::function<uint8_t(uint8_t*, int)> callback)
{
    mutex_.lock();
    serial_rx_tasks_.push_back(callback);
    mutex_.unlock();
}

// UART中断处理任务
void Serial::SerialRxDetectTask() {
    uart_event_t event;
    uint8_t data[UART_BUF_SIZE];
    BaseType_t ret = 0;
    while(1) {
        // 等待UART事件
        ret = xQueueReceive(uart_queue_, &event, portMAX_DELAY);
        if(ret != pdFAIL) {
            switch(event.type) {
                // 数据接收事件
                case UART_DATA:{
                    // 读取接收到的数据
                    int len = uart_read_bytes(UART1_PORT_NUM, data, event.size, portMAX_DELAY);
                    mutex_.lock();
                    for (auto& task : serial_rx_tasks_) {
                        task(data, len);
                    }
                    mutex_.unlock();
                    break; 
                }                               
                // FIFO溢出事件
                case UART_FIFO_OVF:{
                    ESP_LOGW(TAG, "HW FIFO Overflow");
                    uart_flush_input(UART1_PORT_NUM);
                    break;                
                }
                // 帧错误事件
                case UART_FRAME_ERR:{
                    ESP_LOGW(TAG, "Frame Error");
                    break;
                }
                // 其他未处理事件
                default:{
                    ESP_LOGW(TAG, "Unhandled event: %d", event.type);
                    break;
                }
            }
        }
    }
}
