#include "driver/gpio.h"
#include <esp_log.h>

#include "pet_dog.h"
#include "application.h"
#include "board.h"
#include "iot/thing.h"

#define TAG "Action"


namespace iot {

class Action : public Thing {
private:
    PetDog dog_;
public:
    Action() : Thing("Action", "当前 AI 机器人的行为（站立，坐下，睡觉,左转，右转，前进，后退)") {
         dog_.InitializeDog(LEDC_OUTPUT_IO_1,LEDC_OUTPUT_IO_2,LEDC_OUTPUT_IO_3,LEDC_OUTPUT_IO_4);
        // 定义设备可以被远程执行的指令
        methods_.AddMethod("Walk", "前进(回复:前进了多少步)", ParameterList({
            Parameter("step", "0到100之间的整数", kValueTypeNumber, true)
        }), [this](const ParameterList& parameters) {
            dog_.Action(kActionStateWalk,STEP_TO_TIME_MS(parameters["step"].number()));
        });
        
        methods_.AddMethod("Walk back", "后退", ParameterList(), [this](const ParameterList& parameters) {
            dog_.Action(kActionStateWalkBack);
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
};

} // namespace iot

DECLARE_THING(Action);