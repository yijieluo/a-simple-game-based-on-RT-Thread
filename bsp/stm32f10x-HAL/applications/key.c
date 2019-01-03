#include <rtthread.h>
#include <rtdevice.h>
#include "key.h"
#include "led.h"

int key_init(void)
{
    rt_pin_mode(KEY_PIN, PIN_MODE_INPUT);
		return 0;
}
int key_scan1()
{
		rt_tick_t tick;
		if(rt_pin_read(KEY_PIN) == 0)//如果按键按下
		{
				rt_thread_mdelay(20);//消抖
				if(rt_pin_read(KEY_PIN) == 0)//确认按键按下
				{
						tick = rt_tick_get(); //获取当前tick
						while (rt_tick_get() - tick < (RT_TICK_PER_SECOND / 3))//333ms内  //可以适当减少这个延时，可以快一点确定单击
						{
								if(rt_pin_read(KEY_PIN) == 1)//按键松开
								{
										tick = rt_tick_get();
										while (rt_tick_get() - tick < (RT_TICK_PER_SECOND / 2))//333ms内
										{
												if(rt_pin_read(KEY_PIN) == 0)//如果按键再次按下
												{
														rt_thread_mdelay(20);//消抖
														if(rt_pin_read(KEY_PIN) == 0)//再次确认按键按下
														{
																while(rt_pin_read(KEY_PIN) == 0);//等到松手再return，以免影响下一次
																return 2;//双击
														}
												}
										}
										return 1;
								}
						}
						return 1;//单击
				}
		}
		return 0;
}
rt_uint32_t key_scan2()
{
		rt_tick_t tick;
		if(rt_pin_read(KEY_PIN) == 0)//如果按键按下
		{
				rt_thread_mdelay(20);//消抖
				if(rt_pin_read(KEY_PIN) == 0)//确认按键按下
				{
						tick = rt_tick_get();
						while(rt_pin_read(KEY_PIN) == 0)
						{
								led_on();
								if(rt_tick_get()-tick >= 5) //超过50ms则返回
									return rt_tick_get()-tick;
						};					
						return rt_tick_get()-tick;
				}
		}
		led_off();
		return 0;
}
