#ifndef PATTERN_H
#define PATTERN_H
#include <rtthread.h>

void arrow(rt_uint8_t x,rt_uint8_t y);
void helicopter(rt_uint8_t x,rt_uint8_t y);
void top_obstacle(rt_uint8_t x, rt_uint8_t height);
void bottom_obstacle(rt_uint8_t x, rt_uint8_t height);
void generate_map(rt_uint8_t level, rt_uint8_t h[][2]);

//飞机x，y坐标，飞机左障碍x坐标及上下障碍高度，飞机左障碍右坐标及上下障碍高度,障碍物个数
rt_bool_t check_crash(rt_uint8_t x, rt_uint8_t y, rt_uint8_t obstacle_x, rt_uint8_t th, rt_uint8_t bh,rt_int32_t n);

#define EASY 0
#define MIDDLE 1
#define HARD 2

#define MAX1 3  //grade 个数,并排序
#define MAX2 100 //obstacle个数

#endif
