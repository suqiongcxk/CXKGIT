/**
 ******************************************************************************
 * @file    arena_config.h
 * @brief   比赛场地可调参数集中配置文件
 * @note    比赛现场只需修改此文件即可完成全部阈值调参。
 *          上电进入 ADC 模式观察传感器实际值，然后调整对应宏定义。
 ******************************************************************************
 */

#ifndef ARENA_CONFIG_H
#define ARENA_CONFIG_H

/*===========================================================================
 * 1. 灰度分类阈值 —— 比赛现场首要调整对象
 *    场地颜色变化时，根据 ADC 模式观察到的实际灰度值调整以下几组
 *===========================================================================*/

/*---- 台上/台下判定（freertos.c CAR_STATUS 判断逻辑）----*/
/* 注意：左右阈值不对称是已知的硬件差异（传感器安装位置/角度不同导致）*/
#define ARENA_GRAY_ON_STAGE_L_MIN    600     /* 台上判定：左灰度 >= 此值 */
#define ARENA_GRAY_ON_STAGE_R_MIN    700     /* 台上判定：右灰度 >= 此值（右侧比左侧高100）*/
#define ARENA_GRAY_OFF_STAGE_L_MAX   650     /* 台下判定：左灰度 < 此值 */
#define ARENA_GRAY_OFF_STAGE_R_MAX   700     /* 台下判定：右灰度 < 此值（右侧比左侧高50）*/
#define ARENA_GRAY_OFF_STAGE_MIN     300     /* 台下判定：灰度 > 此值（排除纯黑边缘/台下地面）*/

/*---- 区域类型判定（影响索敌灵敏度、速度策略）----*/
#define ARENA_GRAY_LIGHT_AREA       2500     /* 浅色区域阈值：灰度 > 此值时索敌阈值用宽松档 */
#define ARENA_GRAY_EDGE_DARK         200     /* 纯黑/边缘阈值：灰度 <= 此值视为台下边缘/被遮挡 */
#define ARENA_GRAY_BLUE             1850     /* 蓝色区域：左右灰度和 >= 此值*2 时加速通过 */
#define ARENA_GRAY_BACK_ON_STAGE    3400     /* 后退上台确认：L+R 灰度和 > 此值 = 已上台 */
#define ARENA_GRAY_FRONT_BACK_MID   1300     /* 前后灰度分界：前平均/2 < 此值 = 前面深后面浅（朝向擂台内）*/

/*---- 灰度补偿（修正左右传感器硬件差异）----*/
#define ARENA_GRAY_L_HUIDU_VALUE    1200     /* 左灰度正常范围阈值（边缘检测中判断是否"不在角落"）*/
#define ARENA_GRAY_R_HUIDU_VALUE    1200     /* 右灰度正常范围阈值 */
#define ARENA_GRAY_HUIDU_GAIN        1.4f    /* 边缘检测中右灰度补偿系数 */
#define ARENA_GRAY_COMPENSATION_R  1.333f    /* 漫游中右灰度补偿系数（标准化后与 L_huidu 比较）*/

/*---- 索敌自转角度衰减 ----*/
#define TURN_ANGLE_ATTEN_SPEED_THRESH   400     /* 直行速度 >= 此值时衰减转向角度（代替灰度判断）*/
#define ARENA_GRAY_TURN_ANGLE_ATTEN    0.75f    /* 浅色区域自转角度衰减系数 */


/*===========================================================================
 * 2. 灰度→速度映射参数（漫游速度计算）
 *===========================================================================*/
#define GRAY_SPEED_LIGHT_THRESH     1900     /* 高灰度切换点：超过此值用固定高速 */
#define GRAY_SPEED_LIGHT_MAX         440     /* 浅色区最大直行速度 ，在使用时会乘以1.7*/
#define GRAY_SPEED_DIVISOR          9.0f     /* 深色区速度除数（灰度/除数 = 速度）*/
#define GRAY_SPEED_AUTO_ROTATE      1900     /* 低于此灰度值强制自转模式 */
#define GRAY_TURN_HIGH_THRESH       2900     /* 转弯高速切换点 */
#define GRAY_TURN_MAX_SPEED          550     /* 转弯时最大速度 */
#define GRAY_TURN_DIVISOR_L        6.5f     /* 左轮优先时速度除数 */
#define GRAY_TURN_DIVISOR_R        7.0f     /* 右轮优先时速度除数（左右不对称）*/
#define GRAY_TURN_R_DIVISOR        110.0f    /* 转弯半径除数 */
#define GRAY_TURN_R_MIN              10     /* 最小转弯半径 */
#define GRAY_TURN_WHEEL_BASE        22.0f    /* 轮距参数（cm）*/
#define GRAY_TURN_R_VALUE            50     /* 高速区转弯半径固定值 */


/*===========================================================================
 * 3. 激光距离阈值 —— 浅色区（灰度 > ARENA_GRAY_LIGHT_AREA 时生效）
 *===========================================================================*/
#define JG_FRONT_LIGHT             800     /* 前方探测（索敌）*/
#define JG_SIDE_LIGHT              800     /* 侧面探测 */
#define JG_SIDE_30_LIGHT           800     /* 30度侧面探测 */
#define JG_SIDE_60_LIGHT           800     /* 60度侧面探测 */
#define JG_BACK_LIGHT              600     /* 后方探测 */


/*===========================================================================
 * 4. 激光距离阈值 —— 深色区（灰度 <= ARENA_GRAY_LIGHT_AREA 时生效）
 *===========================================================================*/
#define JG_FRONT_DARK              650     /* 前方探测 */
#define JG_SIDE_DARK               600     /* 侧面探测 */
#define JG_SIDE_30_DARK            600     /* 30度侧面探测 */
#define JG_SIDE_60_DARK            600     /* 60度侧面探测 */
#define JG_BACK_DARK               600     /* 后方探测 */


/*===========================================================================
 * 5. 激光距离阈值 —— 通用（与场地布局相关，调整可能较少）
 *===========================================================================*/

/*---- 边缘检测 ----*/
#define JG_EDGE_DETECT_ENTRY       550     /* 边缘检测入口：QIAN >= 此值才启用（不在障碍物后面）*/
#define JG_BOTH_BLOCKED             650     /* 双前遮挡判断：QIAN <= 此值 = 被遮挡 */

/*---- 索敌 ----*/
#define JG_SEARCH_ENTRY_FRONT       600     /* 进入索敌：B_JG < 此值 且 QIAN > 此值 → 180调头 */

/*---- 后方判断 ----*/
#define JG_BACK_ENEMY              650     /* 后方有敌人/障碍（反追击触发）*/
#define JG_BACK_CLEAR              700     /* 后方空旷（可直接上台）*/
#define JG_BACK_WALL               700     /* 背后有墙 */

/*---- 近端对正 ----*/
#define JG_ROX_FRONT_CLOSE         300     /* 对正目标：前方双激光都 < 此值 */
#define JG_ROX_ALIGN_TOLERANCE      25     /* 对正完成：|QIAN_L - QIAN_R| < 此值 */
#define JG_ROX_FRONT_NEAR          500     /* 超时后判断：> 此值 = 离墙太远 */
#define JG_ROX_BACK_TIME         300000    /* 对正超时后退时间(us) */
#define JG_ROX_BACK_SPEED_FACTOR   0.3f    /* 对正超时后退速度系数（*RPM_MAX）*/
#define JG_ROX_FWD_SPEED_FACTOR   0.15f    /* 对正超时前进速度系数 */
#define JG_ROX_TURN_SPEED           150     /* 对正转向速度 */

/*---- 远端对正（已废弃）----*/
#define JG_FAR_TARGET             2400     /* 远端对正完成：QIAN_L + QIAN_R > 此值 */
#define JG_FAR_TURN_SPEED          200     /* 远端对正转向速度 */

/*---- 台下导航 ----*/
#define JG_TAIXIA_CORRIDOR_SIDE    900     /* 过道判定：左右侧距都 > 此值 = 在过道中间 */
#define JG_TAIXIA_FRONT_SUM       1000     /* 过道前方判定：QIAN_L+QIAN_R < 此值 */
#define JG_TAIXIA_BACK_TIME    1400000     /* 后退上台时间(us) */
#define JG_TAIXIA_BACK_SPEED      0.40f    /* 后退上台速度系数 */
#define JG_TAIXIA_ROX_TIMEOUT   1400000     /* 台下近端对正超时(us) */
#define JG_TAIXIA_SIDE_SUM_LARGE  1700     /* 两侧距离之和 > 此值 → 直接调头180 */
#define JG_TAIXIA_SIDE_NEAR       1000     /* 侧面有障碍：< 此值 → 向前移动 */
#define JG_TAIXIA_NEAR_WALL        250     /* 侧面贴墙（需后退微调）*/
#define JG_TAIXIA_SIDE_MID         900     /* 侧面中等距离（需侧移对正）*/
#define JG_TAIXIA_ALIGN_CLOSE      200     /* 台下对齐完成：前方双激光 <= 此值 */
#define JG_TAIXIA_ALIGN_DIFF        25     /* 台下对齐完成：|差值| < 此值 */

/*---- 出发区检测（freertos.c CAR_STATUS 判定）----*/
#define JG_START_ZONE_SIDE        1000     /* 出发区侧面：<= 此值 */
#define JG_START_ZONE_FRONT        180     /* 出发区前方（双出发区）：QIAN <= 此值 */
#define JG_START_ZONE_FRONT_ALT    300     /* 出发区前方（单侧出发区）：QIAN <= 此值 */
#define JG_START_ZONE_SIDE_60      600     /* 出发区60度（双出发区）*/
#define JG_START_ZONE_SIDE_60_ALT  650     /* 出发区60度（单侧出发区）*/

/*---- 反追击 ----*/
#define JG_ANTI_PURSUIT_FRONT      700     /* 逃离后前方空旷：QIAN >= 此值 → 执行上台 */
#define JG_ARENA_EDGE_DIST        2500     /* 擂台边缘参考距离（计算后退上台用：(2500-JG)/10 cm）*/

/*---- 挥手启动 ----*/
#define JG_RECOVERY_TRIGGER         400     /* 挥手启动触发：L_JG <= 此值 && R_JG <= 此值 */


/*===========================================================================
 * 6. 边缘检测
 *===========================================================================*/
#define EDGE_JG_L_VALUE            460     /* 左侧边缘激光阈值 */
#define EDGE_JG_R_VALUE            460     /* 右侧边缘激光阈值 */
#define EDGE_VAlue                 500     /* 边缘触发后的自转计数 */


/*===========================================================================
 * 7. 姿态/过渡态判断
 *===========================================================================*/
#define IMU_ROLL_BETWEEN_STAGE      15     /* roll >= 此值累计 → 过渡态 */
#define IMU_ROLL_RECOVERY            5     /* roll <= 此值 → 恢复 */
#define BETWEEN_STAGE_COUNT_THRES  400     /* 过渡态确认计数（*5ms = 2s）*/


/*===========================================================================
 * 8. 运动/行为参数
 *===========================================================================*/

/*---- 追击 ----*/
#define ZHUIJI_TIME_DEFAULT        1500     /* 追击持续时间(ms) */
#define ZHUIJI_TIME_STEP             5     /* 追击倒计时每次减量5ms */
#define ZHUIJI_TURN_BACK_COUNT     2000     /* 遇到炸弹块后禁止调头的冷却时间 */

/*---- 自转/搜索 ----*/
#define XUANZHUAN_VALUE           2000     /* 自转模式默认持续时间 */
#define XUANZHUAN_LOW_GRAY         1500     /* 低灰度区自转时间 */
#define XUANZHUAN_EDGE             500     /* 边缘触发后自转时间 */
#define SEARCH_SILENCE_CNT          100     /* 转向后索敌静默时间（*5ms）*/

/*---- 状态确认 ----*/
#define OFF_STAGE_CNT_THRES        100     /* 台下确认：连续 N 次采样（*5ms = 500ms）*/

/*---- 上台等待 ----*/
#define ON_STAGE_WAIT_TIME          300     /* 上台后等待时间(ms) */

/*---- 自转 PID ----*/
#define ZIZHUAN_OPEN_LOOP_SPEED     700     /* 自转开环阶段 RPM */
#define ZIZHUAN_FIRST_TIME_A     -0.001667f  /* 开环时间二次项系数 */
#define ZIZHUAN_FIRST_TIME_B      1.8167f   /* 开环时间一次项系数 */
#define ZIZHUAN_FIRST_TIME_C        15      /* 开环时间常数项 */
#define ZIZHUAN_PID_TIMEOUT      600000     /* 台下自转 PID 超时(us) */
#define ZIZHUAN_PID_TOLERANCE        1      /* 自转 PID 收敛容差(度) */
#define ZIZHUAN_SPEED_LIMIT        0.6f     /* 自转闭环速度上限（*RPM_MAX）*/
#define ZIZHUAN_TAIXIA_SPEED_LIMIT 0.4f     /* 台下慢速自转速度上限 */
#define ZIZHUAN_TAIXIA_MIN_SPEED  0.005f    /* 台下慢速自转速度下限 */

/*---- 偏转 ----*/
#define PIANZHUAN_SPEED_FAST        600     /* 偏转外侧轮速 */
#define PIANZHUAN_SPEED_SLOW        150     /* 偏转内侧轮速 */
#define PIANZHUAN_LARGE_ANGLE        50     /* 大角度偏转分界(度) */
#define PIANZHUAN_LARGE_TIME     700000     /* 大角度偏转超时(us) */
#define PIANZHUAN_SMALL_TIME     350000     /* 小角度偏转超时(us) */
#define PIANZHUAN_BLOCK_THRESH      600     /* 偏转中遇障碍物中断阈值 */

/*---- 低速开环转向 ----*/
#define LOWV_90_SPEED               500     /* 90度转向基础速度 */
#define LOWV_90_TIME                170     /* 90度转向时间(ms) */
#define LOWV_60_SPEED               400     /* 60度转向速度 */
#define LOWV_60_TIME                130     /* 60度转向时间(ms) */

/*---- 反追击 ----*/
#define ANTI_PURSUIT_ESCAPE_SPEED   250     /* 反追击逃离速度 */
#define ANTI_PURSUIT_ESCAPE_TIME    550     /* 反追击逃离时间(ms) */
#define ANTI_PURSUIT_BACK_SPEED   0.40f     /* 反追击后退速度系数 */
#define ANTI_PURSUIT_BACK_TIME  1600000     /* 反追击后退时间(us) */

/*---- 假上台限速 ----*/
#define FAKE_ON_STAGE_SPEED_LIMIT   150     /* 假上台（路过出发区）直行速度上限 */

#endif /* ARENA_CONFIG_H */
