#ifndef CIRCULAR_STRIP_H
#define CIRCULAR_STRIP_H

#include "led.h"
#include <driver/gpio.h>
#include <led_strip.h>
#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#include <cstdint>
#include <cstddef>

class CircularStrip : public Led {
public:
    CircularStrip(uint16_t led_num, gpio_num_t pin);
    virtual ~CircularStrip();

    void OnStateChanged() override;
    // 灯效控制接口
    void SetEffect(int effect);
    void SetBrightness(uint8_t brightness);
    void SetSpeed(uint16_t speed_ms);
    void Start();
    void Stop();
    void OnTimer();
protected:
    

private:
    uint16_t led_num_;
    gpio_num_t pin_;
    uint8_t brightness_;
    uint16_t speed_ms_;
    int effect_;
    TimerHandle_t timer_ = nullptr; // FreeRTOS 软件定时器句柄
    led_strip_handle_t led_strip_ = nullptr; // LED 灯带句柄
};

#endif // CIRCULAR_STRIP_H
