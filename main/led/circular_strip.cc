#include "circular_strip.h"
#include <stdlib.h>
#include <string.h>

#include "esp_err.h"
#include "driver/gpio.h"

// 前置声明 TimerCallback 为 CircularStrip 的友元
class CircularStrip;
static void TimerCallback(TimerHandle_t timer);

// HSV颜色空间转RGB颜色空间
// h: 色相 (0-255)
// s: 饱和度 (0-255) 
// v: 明度 (0-255)
// r,g,b: 输出的RGB值 (0-255)
static void hsv2rgb(uint8_t h, uint8_t s, uint8_t v, uint8_t* r, uint8_t* g, uint8_t* b) {
    if (s == 0) {
        *r = *g = *b = v;
        return;
    }

    uint8_t region = h / 43;
    uint8_t remainder = (h - (region * 43)) * 6; 

    uint8_t p = (v * (255 - s)) >> 8;
    uint8_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    uint8_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch (region) {
        case 0:
            *r = v; *g = t; *b = p;
            break;
        case 1:
            *r = q; *g = v; *b = p;
            break;
        case 2:
            *r = p; *g = v; *b = t;
            break;
        case 3:
            *r = p; *g = q; *b = v;
            break;
        case 4:
            *r = t; *g = p; *b = v;
            break;
        default:
            *r = v; *g = p; *b = q;
            break;
    }
}

// --- CircularStrip 类实现 ---

// 静态定时器回调，转发到对象成员
static void TimerCallback(TimerHandle_t timer) {
    auto* this_strip = static_cast<CircularStrip*>(pvTimerGetTimerID(timer));
    if (this_strip) {
        this_strip->OnTimer();
    }
}

CircularStrip::CircularStrip(uint16_t led_num, gpio_num_t pin)
    : led_num_(led_num), pin_(pin), brightness_(128), speed_ms_(50), effect_(0) {
    // 创建定时器
    timer_ = xTimerCreate("led_effect", pdMS_TO_TICKS(speed_ms_), pdTRUE, this, TimerCallback);
    // 初始化 WS2812 LED 灯带配置
    led_strip_config_t strip_config = {
        .strip_gpio_num = pin_,
        .max_leds = led_num_,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model = LED_MODEL_WS2812,
        .flags = {
            .invert_out = false,
        },
    };
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
        .flags = {
            .with_dma = false,
        },
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip_));
}

CircularStrip::~CircularStrip() {
    if (timer_) {
        xTimerDelete(timer_, 0);
    }
    // todo：如需释放 led_strip_，请根据 led_strip API 适配
}

void CircularStrip::SetEffect(int effect) {
    effect_ = effect;
}

void CircularStrip::SetBrightness(uint8_t brightness){
    brightness_ = brightness;
}

void CircularStrip::SetSpeed(uint16_t speed_ms) {
    speed_ms_ = speed_ms;
    if (timer_) {
        xTimerChangePeriod(timer_, pdMS_TO_TICKS(speed_ms_), 0);
    }
}

void CircularStrip::Start() {
    if (timer_) {
        xTimerStart(timer_, 0);
    }
}

void CircularStrip::Stop() {
    if (timer_) {
        xTimerStop(timer_, 0);
    }
}

void CircularStrip::OnStateChanged() {
    // 可根据设备状态自定义灯效切换
}

void CircularStrip::OnTimer() {
    switch (effect_) {
        case 1: { // CIRCULAR_STRIP_EFFECT_FLOW
            static uint8_t flow_pos = 0;
            static uint8_t hue = 0;
            
            // 清除所有LED
            for (int i = 0; i < led_num_; i++) {
                led_strip_set_pixel(led_strip_, i, 0, 0, 0);
            }
            // 创建流动效果，同时点亮多个LED
            for (int i = 0; i < led_num_ +1; i++) {
                int pos = (flow_pos - i + led_num_) % led_num_;
                uint8_t r, g, b;
                // 使用指数衰减来增加亮度差异
                uint8_t brightness = brightness_ * (1 << (4 - i)) / 16;
                // 使用HSV颜色空间，色相随时间变化
                hsv2rgb(hue, 255, brightness, &r, &g, &b);
                led_strip_set_pixel(led_strip_, pos, r, g, b);
            }
            
            flow_pos = (flow_pos + 1) % led_num_;
            // 降低色相变化速度
            if (flow_pos % 4 == 0) {  // 每4个位置才改变一次色相
                hue = (hue + 1) % 255;
            }
            break;
        }
        case 2: { // CIRCULAR_STRIP_EFFECT_RAINBOW
            static uint8_t hue = 0;
            for (int i = 0; i < led_num_; i++) {
                uint8_t h = (hue + i * 255 / led_num_) % 255;
                uint8_t r, g, b;
                hsv2rgb(h, 255, brightness_, &r, &g, &b);
                led_strip_set_pixel(led_strip_, i, r, g, b);
            }
            hue = (hue + 1) % 255;
            break;
        }
        case 3: { // CIRCULAR_STRIP_EFFECT_BREATH
            static uint8_t breath_val = 0;
            static bool increasing = true;
            for (int i = 0; i < led_num_; i++) {
                led_strip_set_pixel(led_strip_, i, breath_val, breath_val, breath_val);
            }
            if (increasing) {
                breath_val++;
                if (breath_val >= brightness_) increasing = false;
            } else {
                breath_val--;
                if (breath_val == 0) increasing = true;
            }
            break;
        }
        default: {
            for (int i = 0; i < led_num_; i++) {
                led_strip_set_pixel(led_strip_, i, 0, 0, 0);
            }
            break;
        }
    }
    led_strip_refresh(led_strip_);
}
