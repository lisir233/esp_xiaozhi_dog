#ifndef SSD1306_DISPLAY_H
#define SSD1306_DISPLAY_H

#include "display.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "time.h"

#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>

class Ssd1306Display : public Display {
private:
    esp_lcd_panel_io_handle_t panel_io_ = nullptr;
    esp_lcd_panel_handle_t panel_ = nullptr;

    TaskHandle_t blink_task_handle;
    TaskHandle_t idle_task_handle;

    bool mirror_x_ = false;
    bool mirror_y_ = false;

    lv_obj_t* status_bar_ = nullptr;
    lv_obj_t* content_ = nullptr;
    lv_obj_t* container_ = nullptr;
    lv_obj_t* side_bar_ = nullptr;

    lv_obj_t *square1 = NULL;   //眼睛
    lv_obj_t *square2 = NULL;
    lv_obj_t *emojy_lable = NULL;

    int cur_square1_x = 0;
    int cur_square1_y = 0;
    int cur_square2_x = 0;
    int cur_square2_y = 0;

    bool show_emotion_label_ = false;  // 默认不显示emotion_label_
    bool show_emojy_label_ = false;     // 默认显示emojy_lable
    bool show_eyes_ = false;           // 默认不显示眼睛

    virtual bool Lock(int timeout_ms = 0) override;
    virtual void Unlock() override;

    void SetupUI_128x64();
    void SetupUI_128x32();

    void to_any_position(int tar1x,int tar1y,int tarx2,int tary2);
    int bresenham_line(int x1, int y1, int x2, int y2,int ret[]);

    void close_eyes();
    void open_eyes();
    void blink_eyes();

    void blink_task();
    void eye_move_emtion_task();
    

public:
    Ssd1306Display(void* i2c_master_handle, int width, int height, bool mirror_x = false, bool mirror_y = false);
    ~Ssd1306Display();

    virtual void start_emtion() override;
    virtual void stop_emtion()  override;
    virtual void idle_emtion()  override;

    void SetShowEmotionLabel(bool show) { 
        show_emotion_label_ = show;
        if (emotion_label_) {
            if (show) {
                lv_obj_clear_flag(emotion_label_, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(emotion_label_, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
    bool GetShowEmotionLabel() const { return show_emotion_label_; }

    void SetShowEmojyLabel(bool show) {
        show_emojy_label_ = show;
        if (emojy_lable) {
            if (show) {
                lv_obj_clear_flag(emojy_lable, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(emojy_lable, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
    bool GetShowEmojyLabel() const { return show_emojy_label_; }

    void SetShowEyes(bool show) {
        show_eyes_ = show;
        if (square1 && square2) {
            if (show) {
                lv_obj_clear_flag(square1, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_flag(square2, LV_OBJ_FLAG_HIDDEN);
                // 启动眼睛动画任务
                if (blink_task_handle == NULL) {
                    xTaskCreate([](void* arg) {
                        auto this_ = (Ssd1306Display*)arg;
                        this_->blink_task();
                        vTaskDelete(NULL);
                    }, "blink_task", 2048, this, 5, &blink_task_handle);
                }
                if (idle_task_handle == NULL) {
                    xTaskCreate([](void* arg) {
                        auto this_ = (Ssd1306Display*)arg;
                        this_->eye_move_emtion_task();
                        vTaskDelete(NULL);
                    }, "eye_move_emtion_task", 3096, this, 5, &idle_task_handle);
                }
            } else {
                lv_obj_add_flag(square1, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(square2, LV_OBJ_FLAG_HIDDEN);
                // 停止眼睛动画任务
                if (blink_task_handle != NULL) {
                    vTaskDelete(blink_task_handle);
                    blink_task_handle = NULL;
                }
                if (idle_task_handle != NULL) {
                    vTaskDelete(idle_task_handle);
                    idle_task_handle = NULL;
                }
            }
        }
    }
    bool GetShowEyes() const { return show_eyes_; }

    void show_static_emotion(const char* emotion);
    void hide_static_emotion();
};

#endif // SSD1306_DISPLAY_H
