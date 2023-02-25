#include "User.h"
#include "bmp.h"
UI ui;
extern Ticker adc_max_temp_tic;

typedef enum
{
    temp_mode_menu_num,
    backflow_menu_num,
    set_temp_time,
    error_temp_fix_num,
    iot_menu_num,
    oled_light_menu_num,
    fan_set,
    use_menu_num,
    resume_factory,
    about_menu_num,
    page2_menu_num_max
} page2_menu_num_type;

static int8_t page_num = 0;

/*菜单页定义*/
static int8_t page2_menu_num = 0;
static int8_t page2_move_tmp = 0;
static int8_t page2_move_flg = 0;

const char *page2_str_ptr[] = {
    "温控模式",
    "回流参数",
    "恒温参数",
    "温度校准",
    "物联网",
    "屏幕亮度",
    "风扇设置",
    "使用说明",
    "恢复出厂",
    "关于",
};

const unsigned char *page2_bmp_ptr[] = {
    page2_bmp_mode,
    page2_bmp_line,
    page2_bmp_timer,
    page2_bmp_temp,
    page2_bmp_iot,
    page2_bmp_light,
    page2_bmp_fan,
    page2_bmp_about,
    page2_bmp_factory,
    page2_bmp_setting,
};
/************************/

/*设置页定义*/
static int8_t page3_switch_flg = 0;

static const char menu0_option0[] = {"回流焊模式"};
static const char menu0_option1[] = {"恒温模式"};

static const char menu1_option0[] = {"活性区"};
static const char menu1_option1[] = {"回流区"};
static const char menu1_option_s[] = {"秒"};
static const char menu1_option_t[] = {"摄"};
static int8_t temp_mode0_option = 0; // 回流曲线设置项 0：活性区温度 1：活性区时间 2：回流区温度 3：回流区时间

static const char menu2_option0[] = {"分钟"};

static const char menu3_option0_0[] = {"单击开启"};
static const char menu3_option0_1[] = {"单击关闭"};
static const char menu3_option1_0[] = {"单击配置网络密匙"};
static const char menu3_option1_1[] = {"请连接至热点"};
static const char menu3_option1_2[] = {"QF_HP"};

#if compile_style
static const char menu5_option0_0[] = {"只要微笑就好了"};

const char menu6_option0_0[] = {"汎用恒温加熱台"};
const char menu6_option0_1[] = {"量產機"};
const char menu6_option0_2[] = {"02"};
#else
static const char menu5_option0_0[] = {"编码器空位再上电"};
static const char menu6_option0_0[] = {"启凡科创智能物联网加热台"};
static const char menu6_option0_1[] = {"V1.62"};
#endif

static const char menu7_option0_0[] = {"实测最高"};
static const char menu7_option0_1[] = {"校准温度"};
static const char menu7_option0_2[] = {":   `"};
static const char menu7_option0_3[] = {"校准结束后将"};
static const char menu7_option0_4[] = {"自动返回"};

static const char app_fan_option0_0[] = {"开启自动降温"};
static const char app_fan_option0_1[] = {"关闭自动降温"};

static const char app_factory_option0_0[] = {"恢复出厂设置"};
static const char app_factory_option0_1[] = {"返回菜单"};

/************************/

/**
 * @brief 循环处理任务
 */
void UI::run_task()
{

    wake_sleep_page();

    if (oled_sleep_flg)
        return;

    switch (page_num)
    {
    case 1:
        temp_move();
        temp_mode_move();
        heat_move();
        temp_time_switch();
        if (show_warning_flg)
        {
            show_warning();
            show_warning_flg = 0;
        }
        break;
    case 2:
        page2_move();
        break;
    case 3:
        page3_switch();
        blinker_config();
        error_temp_fix_page_move();
        choose_options_move();
        break;
    }

    if (page2_move_flg)
        return;

    page_switch(switch_buf);

    if (!oled_flg)
        return;
    oled_flg = 0;
    oled.clr();

    show_page(0, 0, page_num);
    write_oled_light();

    oled.refresh();
}

/**
 * @brief 显示警告
 */
void UI::show_warning()
{
    for (int8_t i = 32; i >= 0; i--)
    {
        oled.clr();
        show_page(0, i, 4);
        show_page(0, i - 32, 1);
        oled.refresh();
        delay(1);
    }
    delay(1000);
    for (int8_t i = 0; i < 32; i++)
    {
        oled.clr();
        show_page(0, i, 4);
        show_page(0, i - 32, 1);
        oled.refresh();
        delay(1);
    }
}

/**
 * @brief 主界面按键处理
 *
 * @param ec_type
 * @param ec_value
 */
void UI::page1_key(ec11_task_result_type ec_type, int16_t ec_value) // 主界面按键对应功能
{
    if (ec_type == ec11_task_is_key)
    {
        switch (ec_value)
        {
        case sw_click:
            if (pwm.power)
            {
                pwm.end();
                if (user_datas.fan_auto_flg == 1)
                    pwm.fan(1);
            }
            else
            {
                pwm.begin();
            }
            break;
        case sw_long:
            switch_buf = sure;
            page_switch_flg = 1;
            ec11.speed_up(false);
            break;
        case sw_double:
            pwm.fan_state = !pwm.fan_state;
            pwm.fan(pwm.fan_state);
            break;

        default:
            break;
        }
    }
    else
    {
        if (user_datas.pwm_temp_mode)
        {
            user_datas.pwm_temp_buf += ec_value;

            if (user_datas.pwm_temp_buf > user_datas.adc_hotbed_max_temp)
                user_datas.pwm_temp_buf = user_datas.adc_hotbed_max_temp;
            else if (user_datas.pwm_temp_buf < 40)
                user_datas.pwm_temp_buf = 40;
            eeprom.write_flg = 1;
            if (show_temp_mode != show_set_temp)
            {
                show_temp_mode = show_set_temp;
                temp_move_flg = 1;
            }
        }
        else
        {
            show_warning_flg = 1;
        }
    }
}

/**
 * @brief 菜单页案件处理
 *
 * @param ec_type
 * @param ec_value
 */
void UI::page2_key(ec11_task_result_type ec_type, int16_t ec_value) // 界面2按键对应功能
{
    if (ec_type == ec11_task_is_key)
    {
        switch (ec_value)
        {
        case sw_double:
            switch_buf = back;
            page_switch_flg = 1;
            ec11.speed_up(true);
            break;
        default:
            switch_buf = sure;
            page_switch_flg = 1;
            if (page2_menu_num == set_temp_time || page2_menu_num == oled_light_menu_num || page2_menu_num == backflow_menu_num)
                ec11.speed_up(true);
            ec11.double_click(false);
            if (page2_menu_num == fan_set)
                set_var_tmp = user_datas.fan_auto_flg;
            if (page2_menu_num == resume_factory)
                set_var_tmp = 0;
            break;
        }
    }
    else
    {
        if (page2_move_tmp < 0 && ec_value == 1)
            page2_move_tmp = -1;
        else if (page2_move_tmp > 0 && ec_value == -1)
            page2_move_tmp = 1;
        page2_move_tmp += ec_value;
        page2_move_flg = 1;
    }
}

void UI::page3_push_back()
{
    switch_buf = back;
    page_switch_flg = 1;
    ec11.speed_up(false);
    if (page2_menu_num != resume_factory && page2_menu_num != about_menu_num && page2_menu_num != use_menu_num)
        eeprom.write_flg = 1;
    ec11.double_click(true);
}

/**
 * @brief app页按键处理
 *
 * @param ec_type
 * @param ec_value
 */
void UI::page3_key(ec11_task_result_type ec_type, int16_t ec_value) // 界面3按键对应功能
{
    if (ec_type == ec11_task_is_key) // 按键事件
    {
        if (ec_value == sw_click) // 单击
        {
            if (page2_menu_num == backflow_menu_num)
            {
                if (!page3_switch_flg)
                {
                    page3_switch_flg = 1;
                    temp_mode0_option++;
                    if (temp_mode0_option == 4)
                        temp_mode0_option = 0;
                }
            }

            if (page2_menu_num == iot_menu_num)
            {
                if (!page3_switch_flg)
                {
                    if (miot_option_buf)
                    {
                        blinker_config_flg = 1;
                    }
                    else
                    {
                        page3_switch_flg = 1;
                        user_datas.miot_miot_able = !user_datas.miot_miot_able;
                    }
                }
            }

            if (page2_menu_num == error_temp_fix_num)
            {
                switch (error_temp_fix_page_buf)
                {
                case 0:
                    error_temp_fix_page_buf = 1;
                    break;
                case 1:
                    error_temp_fix_page_buf = 0;
                    break;
                case 2:
                    if (pwm.power == 0 && adc.now_temp < 150)
                    {
                        error_temp_fix_page_buf = 3;
                        error_temp_fix_page_move_buf = 2;
                    }
                    break;
                default:
                    error_temp_fix_page_buf = 2;
                    error_temp_fix_page_move_buf = 2;
                    break;
                }
            }

            if (page2_menu_num == resume_factory)
            {
                if (set_var_tmp == 1)
                    eeprom.resume_factory();
            }

            if (page2_menu_num == fan_set ||
                page2_menu_num == set_temp_time ||
                page2_menu_num == oled_light_menu_num ||
                page2_menu_num == use_menu_num ||
                page2_menu_num == resume_factory ||
                page2_menu_num == about_menu_num ||
                page2_menu_num == temp_mode_menu_num)
            {
                page3_push_back();
            }
        }
        else // 长按、双击
        {
            if (wifima.wifima_flg)
            {
                wifima.back_flg = 1;
            }
            else
            {
                if (error_temp_fix_page_buf == 1)
                    error_temp_fix_page_buf = 0;
                if (error_temp_fix_page_buf == 3)
                    error_temp_fix_page_buf = 2;

                page3_push_back();

                if (pwm.power && page2_menu_num == temp_mode_menu_num)
                {
                    pwm.end();
                }
                if (page2_menu_num == iot_menu_num)
                {
                    if (user_datas.miot_miot_able && !miot.open_flg)
                    {
                        user_datas.miot_miot_able = 1;
                        eeprom.write_flg = 2;
                        eeprom.write_task();
                        ESP.reset();
                    }
                }

                if (page2_menu_num == resume_factory)
                {
                    if (set_var_tmp == 1)
                        eeprom.resume_factory();
                }
            }
        }
    }
    else // 编码器事件
    {

        switch (page2_menu_num)
        {
        case temp_mode_menu_num: // 温控模式
            if (!page3_switch_flg)
                user_datas.pwm_temp_mode = !user_datas.pwm_temp_mode;
            break;

        case backflow_menu_num: // 回流参数
            if (!page3_switch_flg)
            {
                switch (temp_mode0_option)
                {
                case 0:
                    user_datas.curve_temp_buf[0] += ec_value;
                    if (user_datas.curve_temp_buf[0] < 110)
                        user_datas.curve_temp_buf[0] = 110;
                    else if (user_datas.curve_temp_buf[0] > 200)
                        user_datas.curve_temp_buf[0] = 200;
                    break;

                case 1:
                    user_datas.curve_temp_buf[1] += ec_value;
                    if (user_datas.curve_temp_buf[1] < 60)
                        user_datas.curve_temp_buf[1] = 60;
                    else if (user_datas.curve_temp_buf[1] > 120)
                        user_datas.curve_temp_buf[1] = 120;
                    break;

                case 2:
                    user_datas.curve_temp_buf[2] += ec_value;
                    if (user_datas.curve_temp_buf[2] < 220)
                        user_datas.curve_temp_buf[2] = 220;
                    else if (user_datas.curve_temp_buf[2] > user_datas.adc_hotbed_max_temp)
                        user_datas.curve_temp_buf[2] = user_datas.adc_hotbed_max_temp;
                    break;

                case 3:
                    user_datas.curve_temp_buf[3] += ec_value;
                    if (user_datas.curve_temp_buf[3] < 30)
                        user_datas.curve_temp_buf[3] = 30;
                    else if (user_datas.curve_temp_buf[3] > 90)
                        user_datas.curve_temp_buf[3] = 90;
                    break;
                }
            }
            break;

        case set_temp_time: // 恒温参数
            user_datas.pwm_temp_mode1_time += ec_value;
            if (user_datas.pwm_temp_mode1_time < 0)
                user_datas.pwm_temp_mode1_time = 0;
            else if (user_datas.pwm_temp_mode1_time > 520)
                user_datas.pwm_temp_mode1_time = 520;
            break;

        case iot_menu_num: // iot
            if (!page3_switch_flg)
                miot_option_buf = !miot_option_buf;
            break;

        case oled_light_menu_num: // 屏幕亮度
            user_datas.ui_oled_light += ec_value;
            if (user_datas.ui_oled_light < 0)
                user_datas.ui_oled_light = 0;
            else if (user_datas.ui_oled_light > 255)
                user_datas.ui_oled_light = 255;
            write_oled_flg = 1;
            break;

        case fan_set:
            user_datas.fan_auto_flg = !user_datas.fan_auto_flg;
            circle_move_buf = user_datas.fan_auto_flg | 0x2;
            break;

        case resume_factory:
            set_var_tmp = !set_var_tmp;
            circle_move_buf = set_var_tmp | 0x2;
            break;

        case error_temp_fix_num: // 温度校准
            switch (error_temp_fix_page_buf)
            {
            case 0:
                if (ec_value > 0)
                {
                    error_temp_fix_page_buf = 2;
                    error_temp_fix_page_move_buf = 1;
                }
                break;
            case 1:
            {
                user_datas.adc_hotbed_max_temp += ec_value;
                if (user_datas.adc_hotbed_max_temp < 240)
                    user_datas.adc_hotbed_max_temp = 240;
                else if (user_datas.adc_hotbed_max_temp > 270)
                    user_datas.adc_hotbed_max_temp = 270;
            }
            break;
            case 2:
                if (ec_value < 0)
                {
                    error_temp_fix_page_buf = 0;
                    error_temp_fix_page_move_buf = 1;
                }
                break;
            default:
                break;
            }
        default:
            break;
        }
        if (page2_menu_num == temp_mode_menu_num || page2_menu_num == iot_menu_num)
            page3_switch_flg = ec_value;
    }
}

void UI::blinker_config()
{
    int8_t y;
    if (!blinker_config_flg)
        return;
    blinker_config_flg = 0;

    y = -1;
    for (;;)
    {
        if (y == -33)
            break;
        oled.clr();
        show_page(0, y, 3);
        oled.chinese(16, y + 32, menu3_option1_1, 16, 1, 0);
        oled.str(44, y + 48, menu3_option1_2, 16, 1, 0);
        oled.refresh();
        y--;
        yield();
    }
    Serial.println("Start ap config");
    wifima.startConfigPortal("QF_HP");
    Serial.println("End ap config");

    y = 1;
    for (;;)
    {
        if (y == 32)
            break;
        oled.clr();
        show_page(0, y - 32, 3);
        oled.chinese(16, y, menu3_option1_1, 16, 1, 0);
        oled.str(44, y + 16, menu3_option1_2, 16, 1, 0);
        oled.refresh();
        y++;
        yield();
    }
}

bool UI::oled_display_set()
{
    if (ui.wake_sleep_change_flg)
        return 1;
    if (!ui.oled_sleep_flg)
    {
        oled_sleep_t = 0;
        return 0;
    }
    else
    {
        ui.wake_sleep_change_flg = 1;
        oled_sleep_t = 0;
        return 1;
    }
}

void ui_key_callb(ec11_task_result_type ec_type, int16_t ec_value) // 按键事件中断处理
{
    if (ui.oled_display_set())
        return;

    switch (page_num)
    {
    case 1:
        ui.page1_key(ec_type, ec_value);
        break;
    case 2:
        ui.page2_key(ec_type, ec_value);
        break;
    case 3:
        ui.page3_key(ec_type, ec_value);
        break;
    }
    eeprom.write_t = 0;
}

void UI::write_oled_light()
{
    if (write_oled_flg)
    {
        write_oled_flg = 0;
        oled.light(user_datas.ui_oled_light);
    }
}

bool UI::page_switch(uint8_t mode)
{
    if (!page_switch_flg)
        return 0;
    page_switch_flg = false;

    int8_t next_page;
    int8_t show_y = 0;
    int8_t next_y;
    if (mode == back)
    {
        next_page = page_num - 1;
        next_y = -32;
    }
    else if (mode == sure)
    {
        next_page = page_num + 1;
        next_y = 32;
    }
    else
        return 0;

    for (;;)
    {
        oled.clr();
        show_page(0, show_y, page_num);
        show_page(0, next_y, next_page);
        oled.refresh();

        if (mode == back)
        {
            show_y++;
            next_y++;
        }
        else
        {
            show_y--;
            next_y--;
        }
        if (show_y == 33 || show_y == -33)
            break;
        yield();
    }

    page_num = next_page;

    return 1;
}

void UI::show_page(short x, short y, uint8_t page)
{
    uint8_t mode_tmp = ui.show_temp_mode;
    switch (page)
    {
    case 1:
        if (show_temp_mode == show_now_temp)
            show_temp(x, y, 93, y + 18);
        else
            show_temp(x, y, 0, 0);
        oled.xy_set(0, 0, 128, 4);
        if (user_datas.pwm_temp_mode == Re_So)
            oled.chinese(69, y, "回流", 16, 1, 0);
        else
            oled.chinese(69, y, "恒温", 16, 1, 0);
        if (pwm.power)
            oled.BMP(95, y + 2, 32, 28, heating, 1);
        break;
    case 2:

        oled.xy_set(0, 0, 128, 4);
        oled.chinese(64, y + 8, page2_str_ptr[page2_menu_num], 16, 1, 0);
        oled.BMP(y, page2_bmp_ptr[page2_menu_num]);
        break;
    case 3:
        switch (page2_menu_num)
        {
        case temp_mode_menu_num: // 模式设置
            if (user_datas.pwm_temp_mode)
                oled.chinese(32, y + 8, menu0_option1, 16, 1, 0);
            else
                oled.chinese(24, y + 8, menu0_option0, 16, 1, 0);
            break;

        case backflow_menu_num: // 回流曲线
            if (page3_switch_flg)
            {
                show_curve(0, y);
            }
            else
            {
                show_curve(y, y);
            }
            break;

        case set_temp_time: // 恒温时长
            ui.show_temp_mode = show_temp_mode1_time;
            show_temp(12, y, 0, 0);
            oled.chinese(84, y + 16, menu2_option0, 16, 1, 0);
            ui.show_temp_mode = mode_tmp;
            break;

        case iot_menu_num: // 物联网
            if (miot_option_buf)
            {
                oled.chinese(0, y + 8, menu3_option1_0, 16, 1, 0);
            }
            else
            {
                if (user_datas.miot_miot_able)
                    oled.chinese(32, y + 8, menu3_option0_1, 16, 1, 0);
                else
                    oled.chinese(32, y + 8, menu3_option0_0, 16, 1, 0);
            }
            break;

        case oled_light_menu_num: // 屏幕亮度
            ui.show_temp_mode = show_set_light;
            show_temp(28, y, 0, 0);
            ui.show_temp_mode = mode_tmp;
            break;

        case use_menu_num: // 使用说明
#if compile_style
            oled.chinese(8, y + 10, menu5_option0_0, 16, 1, 0);
#else
            oled.chinese(0, y + 8, menu5_option0_0, 16, 1, 0);
#endif
            break;

        case fan_set: // 风扇设置
            oled.chinese(0, y, app_fan_option0_0, 16, 1, 0);
            oled.chinese(0, y + 16, app_fan_option0_1, 16, 1, 0);
            if (user_datas.fan_auto_flg == 1)
                oled.BMP(118, y + 4, circle_kong);
            else
                oled.BMP(118, y + 20, circle_kong);
            break;

        case resume_factory: // 恢复出厂设置
            oled.chinese(0, y, app_factory_option0_0, 16, 1, 0);
            oled.chinese(0, y + 16, app_factory_option0_1, 16, 1, 0);
            if (set_var_tmp == 1)
                oled.BMP(118, y + 4, circle_kong);
            else
                oled.BMP(118, y + 20, circle_kong);
            break;

        case about_menu_num: // 关于
#if compile_style
            oled.chinese(8, y, menu6_option0_0, 16, 1, 1);
            oled.chinese(28, y + 16, menu6_option0_1, 16, 1, 0);
            oled.str(84, y + 16, menu6_option0_2, 16, 1, 0);
#else
            oled.chinese(0, y, menu6_option0_0, 16, 1, 1);
            oled.str(88, y + 16, menu6_option0_1, 16, 1, 0);
#endif
            break;

        case error_temp_fix_num: // 温度校准

            oled.chinese(0, y, menu7_option0_0, 16, 1, 1);
            oled.chinese(0, y + 16, menu7_option0_1, 16, 1, 1);
            oled.str(64, y, menu7_option0_2, 16, 1, 0);
            oled.str(64, y + 16, menu7_option0_2, 16, 1, 0);
            oled.num(72, y, user_datas.adc_hotbed_max_temp, 3, 16, LEFT, 1);
            oled.num(72, y + 16, user_datas.adc_adc_max_temp, 3, 16, LEFT, 1);

            switch (error_temp_fix_page_buf)
            {
            case 0:
                oled.BMP(118, y + 4, circle_kong);
                break;
            case 1:
                oled.BMP(118, y + 4, circle_shi);
                break;
            default:
                oled.BMP(118, y + 20, circle_kong);
                break;
            }
            break;
        }

        break;
    case 4: // 显示提示
        oled.chinese(x + 8, y, "回流模式请到菜", 16, 1, 0);
        oled.chinese(x + 16, y + 16, "单内设置参数", 16, 1, 0);
        break;
    default:
        break;
    }
}

void UI::wake_sleep_page()
{
    if (wake_sleep_change_flg)
    {

        oled.display_on();
        if (oled_sleep_flg)
        {
            for (int8_t i = 32; i > 0; i--)
            {
                oled.clr();
                show_page(0, i, page_num);
                oled.refresh();
                yield();
            }
        }
        else
        {
            oled.roll(0, 0, 128, 4, 1, UP, 32);
            oled.display_off();
        }
        oled_sleep_flg = !oled_sleep_flg;
        wake_sleep_change_flg = 0;
    }
}

void UI::page3_switch()
{

    if (!page3_switch_flg)
        return;
    int8_t y;

    if (page3_switch_flg < 0)
    {
        oled.roll(0, 0, 128, 4, 2, DOWN, 16);
        y = -32;
    }
    else
    {
        if (page2_menu_num == backflow_menu_num)
            oled.roll(72, 0, 48, 4, 1, UP, 32);
        else
            oled.roll(0, 0, 128, 4, 2, UP, 16);
        y = 32;
    }

    for (;;)
    {
        if (y == 0)
            break;
        oled.clr();
        show_page(0, y, 3);
        oled.refresh();
        if (y < 0)
            y++;
        else
            y--;
        yield();
    }

    page3_switch_flg = 0;
}

void UI::page2_move()
{

    if (!page2_move_flg)
        return;
    int8_t num_tmp;
    int8_t now_y = 0;
    int8_t next_y;

    if (page2_move_tmp < 0)
    {
        num_tmp = page2_menu_num - 1;
        next_y = -32;
    }
    else
    {
        num_tmp = page2_menu_num + 1;
        next_y = 32;
    }
    if (num_tmp < 0)
        num_tmp = page2_menu_num_max - 1;
    else if (num_tmp == page2_menu_num_max)
        num_tmp = 0;

    for (;;)
    {
        if (page2_move_tmp == 0)
        {
            if (next_y < 0)
                page2_move_tmp = -1;
            else
                page2_move_tmp = 1;
        }
        now_y -= page2_move_tmp;
        next_y -= page2_move_tmp;
        if (now_y < -32 || now_y > 32)
            break;
        oled.clr();
        oled.chinese(64, now_y + 8, page2_str_ptr[page2_menu_num], 16, 1, 0);
        oled.BMP(now_y, page2_bmp_ptr[page2_menu_num]);

        oled.chinese(64, next_y + 8, page2_str_ptr[num_tmp], 16, 1, 0);
        oled.BMP(next_y, page2_bmp_ptr[num_tmp]);

        oled.refresh();
        yield();
    }
    if (page2_move_tmp < 0)
    {
        page2_move_tmp++;
    }
    else
    {
        page2_move_tmp--;
    }
    if (page2_move_tmp == 0)
        page2_move_flg = 0;
    page2_menu_num = num_tmp;
}

void UI::heat_move()
{
    if (!heat_flg)
        return;
    heat_flg = 0;
    int8_t y;
    if (pwm.power)
        y = 32;
    else
        y = 2;
    oled.xy_set(0, 0, 128, 4);
    for (;;)
    {
        if (pwm.power)
            y--;
        else
            y++;
        oled.choose_clr(101, 0, 26, 2);
        oled.choose_clr(95, 2, 32, 2);
        oled.BMP(95, y, 32, 28, heating, 1);
        oled.choose_refresh(95, 0, 32, 4);
        if (y == 2 || y == 32)
            return;
        yield();
    }
}

void UI::temp_mode_move()
{
    if (!temp_mode_flg)
        return;
    temp_mode_flg = 0;
    int8_t y = 0;
    oled.xy_set(0, 0, 128, 2);
    for (y = 0; y >= -16; y--)
    {
        oled.choose_clr(69, 0, 32, 2);
        if (user_datas.pwm_temp_mode == Re_So)
        {
            oled.chinese(69, y + 16, "回流", 16, 1, 0);
            oled.chinese(69, y, "恒温", 16, 1, 0);
        }
        else
        {
            oled.chinese(69, y, "回流", 16, 1, 0);
            oled.chinese(69, y + 16, "恒温", 16, 1, 0);
        }
        oled.choose_refresh(69, 0, 32, 2);
        delay(8);
    }
}

void UI::choose_options_move()
{
    if ((circle_move_buf & 0x2) == 0)
        return;
    uint8_t tmp;
    if (circle_move_buf == 0x2)
        tmp = 4;
    else
        tmp = 20;
    for (uint8_t x = 1; x < 17; x++)
    {
        oled.choose_clr(118, 0, 9, 4);
        oled.BMP(118, tmp, circle_kong);
        oled.choose_refresh(118, 0, 9, 4);
        if (circle_move_buf == 0x2)
            tmp++;
        else
            tmp--;
        delay(10);
    }
    circle_move_buf = 0;
}

void UI::error_temp_fix_page_move()
{
    if (error_temp_fix_page_move_buf == 0)
        return;

    int8_t tmp;

    if (error_temp_fix_page_move_buf == 1)
    {
        if (error_temp_fix_page_buf)
            tmp = 4;
        else
            tmp = 20;
        for (uint8_t x = 1; x < 17; x++)
        {
            oled.choose_clr(118, 0, 9, 4);
            oled.BMP(118, tmp, circle_kong);
            oled.choose_refresh(118, 0, 9, 4);
            if (error_temp_fix_page_buf)
                tmp++;
            else
                tmp--;
            delay(10);
        }
    }
    else
    {

        if (error_temp_fix_page_buf == 2)
            tmp = -32;
        else
            tmp = 0;
        for (uint8_t x = 0; x < 33; x++)
        {
            oled.clr();
            show_page(0, tmp, 3);
            oled.chinese(0, tmp + 32, menu7_option0_3, 16, 1, 1);
            oled.chinese(0, tmp + 48, menu7_option0_4, 16, 1, 1);
            oled.BMP(95, tmp + 34, 32, 28, heating, 1);
            oled.num(64, tmp + 48, adc.now_temp, 3, 16, LEFT, 1);
            oled.refresh();
            if (error_temp_fix_page_buf == 2)
                tmp++;
            else
                tmp--;
            yield();
        }
        if (error_temp_fix_page_buf == 3)
        {
            adc.adc_max_temp_auto_flg = 0;
            digitalWrite(PWM_IO, HIGH);
            adc_max_temp_tic.attach(20, adc_max_temp_auto_feed);
            error_temp_fix_page_buf = 4;
            while (error_temp_fix_page_buf == 4)
            {
                if (adc.adc_max_temp_auto())
                {
                    digitalWrite(PWM_IO, LOW);
                    break;
                }
                oled.choose_clr(64, 2, 24, 2);
                oled.num(64, 16, adc.now_temp, 3, 16, LEFT, 1);
                oled.refresh();
                delay(1000);
            }
            adc_max_temp_tic.detach();
            error_temp_fix_page_buf = 2;
            adc.adc_max_temp_auto_flg = 1;
            digitalWrite(PWM_IO, LOW);
            tmp = -32;
            for (uint8_t x = 0; x < 33; x++)
            {
                oled.clr();
                show_page(0, tmp, 3);
                oled.chinese(0, tmp + 32, menu7_option0_3, 16, 1, 1);
                oled.chinese(0, tmp + 48, menu7_option0_4, 16, 1, 1);
                oled.BMP(95, tmp + 34, 32, 28, heating, 1);
                oled.num(64, tmp + 48, adc.now_temp, 3, 16, LEFT, 1);
                oled.refresh();
                tmp++;
                yield();
            }
        }
    }
    error_temp_fix_page_move_buf = 0;
}

void UI::temp_move()
{
    if (!temp_move_flg)
        return;
    temp_move_flg = 0;
    int8_t temp_x;
    uint8_t temp_y;
    uint8_t small_x;
    if (show_temp_mode == show_now_temp)
    {
        temp_x = 0;
    }
    else
    {
        oled.roll(0, 0, 68, 4, 2, UP, 16);
        temp_x = 68;
    }
    for (;;)
    {
        if (show_temp_mode == show_now_temp)
        {
            temp_x += 4;
        }
        else if (show_temp_mode == show_set_temp)
        {
            temp_x -= 4;
        }

        temp_y = temp_x * 1000 / 2125;
        small_x = temp_x * 100 / 283 + 69;

        oled.choose_clr(0, 0, 68, 4);
        oled.choose_clr(68, 2, 24, 2);
        show_temp(temp_x, temp_y, small_x, 18);

        oled.choose_refresh(0, 0, 68, 4);
        oled.choose_refresh(68, 2, 24, 2);
        if (temp_x >= 68 || temp_x <= 0)
            break;
        yield();
    }
    if (show_temp_mode == show_now_temp)
    {
        for (temp_y = 32; temp_y > 0; temp_y -= 2)
        {
            oled.choose_clr(0, 0, 68, 4);
            show_temp(0, temp_y, 0, 18);
            oled.choose_refresh(0, 0, 68, 4);
            yield();
        }
    }
}

void UI::show_temp(int8_t x, int8_t y, int8_t xx, int8_t yy)
{
    uint8_t dat_buf[3];
    uint16_t tmp;
    if (show_temp_mode == show_now_temp)
    {
        tmp = adc.now_temp;
        if (adc.now_temp <= 38)
            oled.BMP(x + 2, y, less);
    }
    else if (show_temp_mode == show_set_temp)
    {
        tmp = user_datas.pwm_temp_buf;
    }
    else if (show_temp_mode == show_set_light)
    {
        tmp = user_datas.ui_oled_light;
    }
    else if (show_temp_mode == show_temp_mode1_time)
    {
        tmp = user_datas.pwm_temp_mode1_time;
    }
    else
        return;
    oled.xy_set(68, 0, 128, 4);
    if (user_datas.pwm_temp_mode)
    {
        if (pwm.temp_reached_flg)
            oled.num(xx, yy, temp_time_buf, 3, 16, RIGHT, 1);
        else
            oled.num(xx, yy, user_datas.pwm_temp_buf, 3, 16, RIGHT, 1);
    }
    else
        oled.num(xx, yy, pwm.percent, 3, 16, RIGHT, 1);

    dat_buf[0] = tmp / 100 % 10;
    dat_buf[1] = tmp / 10 % 10;
    dat_buf[2] = tmp % 10;
    if (show_temp_mode < 3)
        oled.xy_set(0, 0, 68, 4);
    else
        oled.xy_set(0, 0, 128, 4);
    if (dat_buf[0])
        oled.BMP(x, y, 20, 32, number[dat_buf[0]], 1);
    if (dat_buf[0] || dat_buf[1])
        oled.BMP(x + 24, y, 20, 32, number[dat_buf[1]], 1);
    oled.BMP(x + 48, y, 20, 32, number[dat_buf[2]], 1);
}

void UI::show_curve(int8_t y, int8_t data_y)
{
    int8_t y_tmp;
    int8_t i;
    int8_t y_buf;
    int8_t x_tmp;
    int8_t x_buf;

    y_tmp = -((user_datas.curve_temp_buf[0] - 40) * 100000 / 671875) + 32;
    y_buf = y_tmp;

    oled.point(11, y_buf + 1 + y, 1);

    for (i = 10; i > 0; i--) // 升温曲线1
    {
        oled.point(i, ++y_buf + y, 1);
    }
    x_tmp = user_datas.curve_temp_buf[1] / 5;
    x_buf = user_datas.curve_temp_buf[3] / 5;

    for (i = 0; i < x_tmp; i++) // 保温曲线
    {
        oled.point(12 + i, y_tmp + y, 1);
    }
    x_tmp += 12;

    y_tmp--;

    oled.point(x_tmp++, y_tmp + y, 1);

    y_buf = -((user_datas.curve_temp_buf[2] - 40) * 100000 / 671875) + 32;

    for (i = 10; i > 0; i--) // 升温曲线2
    {
        oled.point(x_tmp++, y_tmp + y, 1);
        y_tmp--;
        if (y_tmp == y_buf)
            break;
        if (y_tmp < 6)
        {
            x_buf -= 2;
        }
    }

    oled.point(x_tmp++, y_tmp + 1 + y, 1);
    x_buf -= 2;

    for (i = 0; i < x_buf; i++)
    {
        oled.point(x_tmp++, y_tmp + y, 1);
    }
    y_tmp++;
    oled.point(x_tmp++, y_tmp + y, 1);

    for (i = 0; i < 10; i++)
    {
        oled.point(x_tmp++, y_tmp + y, 1);
        y_tmp++;
        if (x_tmp == 72)
            break;
    }

    if (temp_mode0_option < 2)
        oled.chinese(72, data_y, menu1_option0, 16, 1, 0);
    else
        oled.chinese(72, data_y, menu1_option1, 16, 1, 0);

    oled.num(100, data_y + 16, user_datas.curve_temp_buf[temp_mode0_option], 3, 16, RIGHT, 1);

    if (temp_mode0_option == 0 || temp_mode0_option == 2)
        oled.chinese(100, data_y + 16, menu1_option_t, 16, 1, 0);
    else
        oled.chinese(100, data_y + 16, menu1_option_s, 16, 1, 0);
}

void UI::temp_time_switch()
{
    if (temp_time_switch_flg == false)
        return;
    if (pwm.temp_reached_flg == false)
        min_count = 0;
    Serial.println(pwm.temp_reached_flg);
    pwm.temp_reached_flg = !pwm.temp_reached_flg;
    oled.roll(69, 2, 24, 2, 1, UP, 16);

    for (int8_t i = 32; i > 18; i--)
    {
        oled.choose_clr(69, 2, 24, 2);
        if (pwm.temp_reached_flg)
            oled.num(93, i, temp_time_buf, 3, 16, RIGHT, 1);
        else
            oled.num(93, i, user_datas.pwm_temp_buf, 3, 16, RIGHT, 1);
        oled.choose_refresh(69, 2, 24, 2);
        delay(10);
    }
    temp_time_switch_flg = false;
}
