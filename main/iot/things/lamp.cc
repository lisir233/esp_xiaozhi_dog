#include "iot/thing.h"
#include "board.h"
#include "audio_codec.h"
#include <driver/gpio.h>
#include <esp_log.h>
#include "driver/rmt_tx.h"
#include "led_strip.h"
#include "led/circular_strip.h"
#define TAG "Lamp"

#define LED_NUM 4
#define RIGHT_BLINK_GPIO         GPIO_NUM_38
#define LEFT_BLINK_GPIO         GPIO_NUM_8

namespace iot {

// 这里仅定义 Lamp 的属性和方法，不包含具体的实现
class Lamp : public Thing {
private:
    CircularStrip* left_strip_;
    CircularStrip* right_strip_;
public:
    Lamp() : Thing("Lamp", "一个测试用的灯"){
        // 创建并初始化LED灯带
        left_strip_ = new CircularStrip(LED_NUM, LEFT_BLINK_GPIO);
        right_strip_ = new CircularStrip(LED_NUM, RIGHT_BLINK_GPIO);       
        if (left_strip_) {
            left_strip_->SetBrightness(10); // 设置亮度为中等
            left_strip_->SetSpeed(100);      // 设置动画速度为50ms
            left_strip_->SetEffect(1);      // 流水灯效果（CIRCULAR_STRIP_EFFECT_FLOW）
            left_strip_->Start();           // 启动动画
        }
        if (right_strip_) {
            right_strip_->SetBrightness(10); // 设置亮度为中等
            right_strip_->SetSpeed(100);      // 设置动画速度为50ms
            right_strip_->SetEffect(1);      // 流水灯效果（CIRCULAR_STRIP_EFFECT_FLOW）
            right_strip_->Start();           // 启动动画
        }
        // 定义设备可以被远程执行的指令
        methods_.AddMethod("SetBrightness", "设置灯光亮度", ParameterList({
            Parameter("brightness", "0到100之间的整数", kValueTypeNumber, true)
        }), [this](const ParameterList& parameters) {
            left_strip_->SetBrightness(static_cast<uint8_t>(parameters["brightness"].number()));
            right_strip_->SetBrightness(static_cast<uint8_t>(parameters["brightness"].number()));
        });

        methods_.AddMethod("SetEffect", "设置灯光效果", ParameterList({
            Parameter("effect", "灯光效果编号:1-流水灯,2-彩虹灯,3-呼吸灯,4-灯光关闭", kValueTypeNumber, true)
        }), [this](const ParameterList& parameters) {
                left_strip_->SetEffect(static_cast<int>(parameters["effect"].number()));
                right_strip_->SetEffect(static_cast<int>(parameters["effect"].number()));
        });
        methods_.AddMethod("SetSpeed", "设置灯光速度", ParameterList({
            Parameter("speed", "灯光切换速度,范围1-100,数值越大速度越慢", kValueTypeNumber, true)
        }), [this](const ParameterList& parameters) {
                int speed = static_cast<int>(parameters["speed"].number());
                if(speed < 1) speed = 1;
                if(speed > 100) speed = 100;
                // 将1-100的范围映射到100-1000毫秒
                uint16_t speed_ms = speed * 10;
                left_strip_->SetSpeed(speed_ms);
                right_strip_->SetSpeed(speed_ms);
        });

    }
};

} // namespace iot


DECLARE_THING(Lamp);