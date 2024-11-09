#include "key_lptouch.h"
#include "app_config.h"
#include "lpctmu.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"


#if KEY_LPTOUCH_EN

static u32 lptouch_pre_value[LPCTMU_CH_SIZE] = {0};
static u32 lptouch_normal_value[LPCTMU_CH_SIZE] = {0};
static u32 lptouch_calibrate_cnt[LPCTMU_CH_SIZE] = {0};
static u32 lptouch_calibrate_tmp_value[LPCTMU_CH_SIZE] = {0};

static const struct lpctmu_platform_data lpctmu_data = {
    .clk_sel = LPCTMU_LRC_200K,
    .sample_time_ms = 2,
    .aim_vol_delta = 800,
    .aim_charge_khz = 2500,
    .ctl_mode = HW_NORMAL_POLL_MODE,
    .ch_en.cfg.ch0 = LPCTMU_CH0_EN,
    .ch_en.cfg.ch1 = LPCTMU_CH1_EN,
    .ch_en.cfg.ch2 = LPCTMU_CH2_EN,
    .ch_en.cfg.ch3 = LPCTMU_CH3_EN,
    .ch_en.cfg.ch4 = LPCTMU_CH4_EN,
    .ch_en.cfg.ch5 = LPCTMU_CH5_EN,
    .ch_en.cfg.ch6 = LPCTMU_CH6_EN,
    .ch_en.cfg.ch7 = LPCTMU_CH7_EN,
};


void lptouch_key_init(void)
{
    lpctmu_init(&lpctmu_data);

    for (u8 ch = 0; ch < LPCTMU_CH_SIZE; ch ++) {
        lptouch_pre_value[ch] = 0;
        lptouch_normal_value[ch] = 0;
        lptouch_calibrate_cnt[ch] = 0;
    }
}

//获取加权平均值
u32 lptouch_key_weighted_averag(u32 old_data, u32 new_data, float factor)
{
    u32 data;
    if (old_data == 0) {
        data = new_data;
    } else if (old_data > new_data) {
        data = old_data - (u32)((old_data - new_data) * factor);
    } else {
        data = old_data + (u32)((new_data - old_data) * factor);
    }
    return data;
}

//只返回两种状态 0：没被按下  1：被按下
u8 lptouch_key_algo(u8 ch, u32 ch_res)
{
    u8 key_state = 0;
    //简单滤波
    lptouch_pre_value[ch] = lptouch_key_weighted_averag(lptouch_pre_value[ch], ch_res, 0.2f);

    //处理滤波之后的值
    if ((lptouch_normal_value[ch]) && (lptouch_pre_value[ch] < (lptouch_normal_value[ch] - LPTOUCH_DELDA))) {
        lptouch_calibrate_cnt[ch] = 0;
        key_state = 1;
    } else {
        lptouch_calibrate_cnt[ch] ++;
    }
    //定期标定常态下的基准值
    if (lptouch_calibrate_cnt[ch] > LPTOUCH_VAL_CALIBRATE_CYCLE) {
        lptouch_calibrate_tmp_value[ch] = lptouch_calibrate_tmp_value[ch] / 10;
        lptouch_normal_value[ch] = lptouch_key_weighted_averag(lptouch_normal_value[ch], lptouch_calibrate_tmp_value[ch], 0.2f);
        lptouch_calibrate_tmp_value[ch] = 0;
        lptouch_calibrate_cnt[ch] = 0;
    } else if (lptouch_calibrate_cnt[ch] >= (LPTOUCH_VAL_CALIBRATE_CYCLE / 2)) {
        if (lptouch_calibrate_cnt[ch] < ((LPTOUCH_VAL_CALIBRATE_CYCLE / 2) + 10)) {
            lptouch_calibrate_tmp_value[ch] += lptouch_pre_value[ch];
        }
    } else {
        lptouch_calibrate_tmp_value[ch] = 0;
    }
    return key_state;
}


u8 get_lptouch_key_value(void)
{
    lpctmu_kstart();//触发当前采集

    u32 key_or = 0;
    u32 cur_val = 0;
    for (u8 ch = 0; ch < LPCTMU_CH_SIZE; ch ++) {
        cur_val = get_lpctmu_value(ch);
        if ((cur_val > 1000) && (cur_val < 20000)) {
            /* log_info("ch%d: %d ", ch, cur_val); */
            if (lptouch_key_algo(ch, cur_val)) {
                key_or |=  BIT(ch);
            }
        }
    }
    static u8 pre_ch = 0;
    u8 key_num = NO_KEY;
    if (key_or) {
        if (key_or & BIT(pre_ch)) {
            key_num = pre_ch;
        } else {
            for (u8 ch = 0; ch < LPCTMU_CH_SIZE; ch ++) {
                if (key_or & BIT(ch)) {
                    key_num = ch;
                    pre_ch = ch;
                    break;
                }
            }
        }
        /* log_info("after %d ", key_num); */
    }
    return key_num;
}

const key_interface_t key_lptouch_info = {
    .key_type = KEY_TYPE_LPTOUCH,
    .key_init = lptouch_key_init,
    .key_get_value = get_lptouch_key_value,
};

#endif

