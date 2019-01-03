#ifndef KEY_H
#define KEY_H

#include <rtthread.h>

#define KEY_PIN 15

int key_init(void);                     //按键初始化
int key_scan1(void); //确认单击还是双击
rt_uint32_t key_scan2(void); //长按返回时间
#endif
