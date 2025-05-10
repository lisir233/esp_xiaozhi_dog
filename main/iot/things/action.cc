#include "driver/gpio.h"
#include <esp_log.h>

#include "pet_dog.h"
#include "application.h"
#include "board.h"
#include "iot/thing.h"
#include "display/ssd1306_display.h"

#define TAG "Action"

namespace iot {

class Action : public Thing {
private:
    PetDog dog_;
    Ssd1306Display* display_;
    TimerHandle_t emotion_timer_ = nullptr;

    static void emotion_timer_callback(TimerHandle_t timer) {
        auto* action = static_cast<Action*>(pvTimerGetTimerID(timer));
        action->display_->hide_static_emotion();
    }

public:
    Action() : Thing("Action", "当前 AI 机器人的行为（站立，坐下，睡觉,左转，右转，前进，后退)") {
         dog_.InitializeDog(LEDC_OUTPUT_IO_1,LEDC_OUTPUT_IO_2,LEDC_OUTPUT_IO_3,LEDC_OUTPUT_IO_4);
         emotion_timer_ = xTimerCreate("emotion_timer", 1000, pdFALSE, this, emotion_timer_callback);
        // 定义设备可以被远程执行的指令
        methods_.AddMethod("Walk", "前进(回复:前进了多少步)", ParameterList({
            Parameter("step", "0到100之间的整数", kValueTypeNumber, true)
        }), [this](const ParameterList& parameters) {
            display_ = static_cast<Ssd1306Display*>(Board::GetInstance().GetDisplay());
            display_->show_static_emotion("");
            uint32_t move_time = STEP_TO_TIME_MS(parameters["step"].number());
            ESP_LOGI(TAG, "move_time: %lu", move_time);
            xTimerChangePeriod(emotion_timer_, pdMS_TO_TICKS(move_time), 0);
            xTimerStart(emotion_timer_, 0);
            dog_.Action(kActionStateWalk,move_time);
        });
        
        methods_.AddMethod("Walk back", "后退",ParameterList({
            Parameter("step", "0到100之间的整数", kValueTypeNumber, true)}), [this](const ParameterList& parameters)  {
                        display_ = static_cast<Ssd1306Display*>(Board::GetInstance().GetDisplay());
            display_->show_static_emotion("");
            uint32_t move_time = STEP_TO_TIME_MS(parameters["step"].number());
            ESP_LOGI(TAG, "move_time: %lu", move_time);
            xTimerChangePeriod(emotion_timer_, pdMS_TO_TICKS(move_time), 0);
            xTimerStart(emotion_timer_, 0);
            dog_.Action(kActionStateWalkBack,move_time);
        });
        methods_.AddMethod("stand", "站立", ParameterList(), [this](const ParameterList& parameters) {
            dog_.Action(kActionStateStand);
        });
        methods_.AddMethod("sitdown", "坐下", ParameterList(), [this](const ParameterList& parameters) {
            dog_.Action(kActionStateSitdown);
        });
        methods_.AddMethod("sleep", "睡觉", ParameterList(), [this](const ParameterList& parameters) {
            dog_.Action(kActionStateSleep);
        });

        methods_.AddMethod("turn left", "左转", ParameterList(), [this](const ParameterList& parameters) {
            dog_.Action(kActionStateTurnLeft);
        });

        methods_.AddMethod("turn right", "右转", ParameterList(), [this](const ParameterList& parameters) {
            dog_.Action(kActionStateTurnRight);
        });

        methods_.AddMethod("wave", "挥挥手(回复:哥哥你好呀)", ParameterList(), [this](const ParameterList& parameters) {
            dog_.Action(kActionStateWave);
        });

        methods_.AddMethod("stop", "停下来", ParameterList(), [this](const ParameterList& parameters) {
            dog_.Action(kActionStateStop);
        });
    }

    ~Action() {
        if (emotion_timer_ != nullptr) {
            xTimerDelete(emotion_timer_, 0);
        }

    }
};

} // namespace iot

DECLARE_THING(Action);