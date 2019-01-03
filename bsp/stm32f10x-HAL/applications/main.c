#include <rtthread.h>
#include <math.h>
#include <string.h>
#include "led.h"
#include "key.h"
#include "oled.h"
#include "pattern.h"

/* 邮箱控制块 */
static struct rt_mailbox mb;
/* 用于放邮件的内存池 */
static char mb_pool[64];
static char mb_str1[] = "once";//单击
static char mb_str2[] = "double";//双击

/* 线程相关 */
rt_thread_t tid1,tid2,tid3,tid4;
#define THREAD_PRIORITY      25
#define THREAD_STACK_SIZE1    256
#define THREAD_STACK_SIZE2    1024
#define THREAD_TIMESLICE     5
static void thread3_entry(void *parameter);
static void thread4_entry(void *parameter);

/* 游戏相关 */
rt_uint8_t level = EASY; //0,1,2分别对应easy, middle, hard
rt_uint32_t grades[MAX1] = {0}; //分数
void game_over(rt_uint32_t grade);
rt_uint8_t choose_level(rt_uint8_t j);
/* 线程 menu 入口 */
static void thread1_entry(void *parameter)
{
		rt_uint8_t i = 26,j;
		if(level == EASY)j=26;
		else if(level == MIDDLE)j=41;
		else j=56;
		while(1)
		{		
				OLED_Clear();
				arrow(10,i);
				OLED_ShowString(25, 0, "helicopter");
				OLED_ShowString(12, 20, "start");
				OLED_ShowString(12, 35, "level");
				OLED_ShowString(12, 50, "grade");
				helicopter(98,30);
				//
				//bottom_obstacle(98,10);
				OLED_Refresh_Gram();
				char *str;
				if (rt_mb_recv(&mb, (rt_uint32_t *)&str, RT_WAITING_FOREVER) == RT_EOK)
        {
            if (str == mb_str1)//切换箭头
						{
							i += 15;
							if(i > 26+15*2)i=26;
							OLED_Clear();
						}
						else if (str == mb_str2 && i == 26)//点击start
						{
								tid3 = rt_thread_create("thread3",
																				thread3_entry, RT_NULL,
																			  THREAD_STACK_SIZE2,
																			  THREAD_PRIORITY+1,
																			  THREAD_TIMESLICE);
								rt_thread_startup(tid3);
								tid4 = rt_thread_create("thread4",
																				thread4_entry, RT_NULL,
																				THREAD_STACK_SIZE1,
																				THREAD_PRIORITY+1,
																				THREAD_TIMESLICE);
								rt_thread_startup(tid4);
								rt_thread_mdelay(50);
						}
						else if (str == mb_str2 && i == 41)//点击level
						{
								level = choose_level(j);
						}
						else if (str == mb_str2 && i == 56)//点击grade
						{
								OLED_Clear();
								OLED_ShowString(20,0,"Top3 Grades");
								OLED_ShowString(20,20,"1: ");OLED_ShowNumber(40,20,grades[0],4,12);
								OLED_ShowString(20,30,"2: ");OLED_ShowNumber(40,30,grades[1],4,12);
								OLED_ShowString(20,40,"3: ");OLED_ShowNumber(40,40,grades[2],4,12);
								OLED_Refresh_Gram();
								if(rt_mb_recv(&mb, (rt_uint32_t *)&str, RT_WAITING_FOREVER) == RT_EOK);
								OLED_Clear();
						}
						else ;
        }
		}
}

/* 按键处理线程 单击和双击 */
static void thread2_entry(void *parameter)
{
		int key;
		while(1)
		{
				key = key_scan1();
				if(key == 1)
				{
						rt_mb_send(&mb, (rt_uint32_t)&mb_str1);
				}
				else if(key == 2)
				{
						rt_mb_send(&mb, (rt_uint32_t)&mb_str2);
				}
				else 
					;
				rt_thread_mdelay(50);
		}
}
/* 游戏界面线程 */
//x:0~127
//y:0~63
static void thread3_entry(void *parameter)
{
		rt_thread_delete(tid2);  
		rt_thread_delete(tid1);
		tid2 = RT_NULL;  
		tid1 = RT_NULL;
		rt_thread_delay(10);
	
		OLED_Clear();
	
		rt_uint8_t h[MAX2][2];
		generate_map(level, h);
	
		rt_uint32_t click_time;
		rt_tick_t start_time = rt_tick_get();
	
		rt_int32_t x = 63,y = 20;
		rt_int32_t m=0,n=0;
		while(1)
		{
				OLED_Clear();
				if (rt_mb_recv(&mb, (rt_uint32_t *)&click_time, RT_WAITING_NO) == RT_EOK)  //若有按键按下，随push的时间上升
				{
						y = y + 1 - (rt_uint8_t)click_time / 20; //20相当于控制按键的灵敏度
						helicopter(x,y);
				}
				else//否则，随时间下降
				{
						y = y + 1;
						helicopter(x,y);
				}
				for(n=0; n<MAX2; n++)  //显示障碍物；n是第几个障碍物，m是随时间移位的距离
				{	
						rt_int32_t tmp = 124-m + n*40;
						if(tmp<127) //障碍物宽度有3，40是两个障碍物之间的距离
						{
								if(tmp>= 0)
								{
										top_obstacle(tmp, h[n][0]);
										bottom_obstacle(tmp, h[n][1]);
									
										if((tmp >= x && tmp - x < 25) || (tmp < x && x-tmp< 25)) //只用检测飞机前后两个障碍物是否碰撞
										{
												//rt_kprintf("\nh[%d][0] = %d h[%d][1] = %d\n",n,h[n][0],n,h[n][1]);
												if(check_crash(x,y,tmp,h[n][0],h[n][1],n) == RT_FALSE)
												{
														game_over((rt_uint32_t)(rt_tick_get() - start_time)/10);
														rt_thread_mdelay(1500);
														tid1 = rt_thread_create("thread1",
																										thread1_entry, RT_NULL,
																										THREAD_STACK_SIZE1,
																										THREAD_PRIORITY,
																										THREAD_TIMESLICE);
														rt_thread_startup(tid1);
														tid2 = rt_thread_create("thread2",
																										thread2_entry, RT_NULL,
																										THREAD_STACK_SIZE1,
																										THREAD_PRIORITY,
																										THREAD_TIMESLICE);	
														rt_thread_startup(tid2);
goto end;//很舒服
												}
										}
								}
						}
						else break;
				}
				m++;
				OLED_Refresh_Gram();				
				rt_thread_mdelay(50);//50ms刷新界面
		};
end:
		rt_thread_delete(tid4);
		rt_thread_delay(10);
		tid3 = RT_NULL;
		rt_thread_delay(10);
}
/*按键处理线程2 处理按键时间*/
static void thread4_entry(void *parameter)
{
		rt_uint32_t click_time;
		while(1)
		{
				click_time = key_scan2();
				if(click_time != 0)
				{
						rt_mb_send(&mb, (rt_uint32_t)&click_time);
				}
				rt_thread_mdelay(20);
		}
}
int main()
{
		oled_init();
		led_init();
		key_init();
		rt_err_t result;
		/* 初始化一个mailbox */
    result = rt_mb_init(&mb,
                        "mbt",                      /* 名称是mbt */
                        &mb_pool[0],                /* 邮箱用到的内存池是mb_pool */
                        sizeof(mb_pool) / 4,        /* 邮箱中的邮件数目，因为一封邮件占4字节 */
                        RT_IPC_FLAG_FIFO);          /* 采用FIFO方式进行线程等待 */
    if (result != RT_EOK)
    {
        rt_kprintf("init mailbox failed.\n");
        return -1;
    }
	
    tid1 = rt_thread_create("thread1",
                           thread1_entry, RT_NULL,
                           THREAD_STACK_SIZE1,
                           THREAD_PRIORITY,
                           THREAD_TIMESLICE);
		rt_thread_startup(tid1);
		
    tid2 = rt_thread_create("thread2",
                           thread2_entry, RT_NULL,
                           THREAD_STACK_SIZE1,
                           THREAD_PRIORITY,
                           THREAD_TIMESLICE);	
    rt_thread_startup(tid2);
		
		return 0;
}
void game_over(rt_uint32_t grade)
{
		OLED_Clear();
		OLED_ShowString(25,10,"Game Over!");
		OLED_ShowString(25,30,"grade:");
		OLED_ShowNumber(75,30,grade,4,12);
		OLED_Refresh_Gram();
		int i,j;
		for(i=0;i<MAX1;i++) //按序插入新成绩
		{
				if(grade > grades[i])
				{
						for(j = MAX1-1;j>i;j--)
						{
								grades[j] = grades[j-1];
						}
						grades[i] = grade;
						break;
				}
		}
}
rt_uint8_t choose_level(rt_uint8_t j)
{
		OLED_Clear();

		arrow(10,j);
		OLED_ShowString(25, 0, "Level");
		OLED_ShowString(12, 20, "easy");
		OLED_ShowString(12, 35, "middle");
		OLED_ShowString(12, 50, "hard");
		OLED_Refresh_Gram();
		char *str;
		while(1)
		{
				if (rt_mb_recv(&mb, (rt_uint32_t *)&str, RT_WAITING_FOREVER) == RT_EOK)
				{
						if (str == mb_str1)//切换箭头
						{
								j += 15;
								if(j > 26+15*2)j=26;
						}
						if(str == mb_str2 && j == 26)
						{
								return 0;
						}
						if(str == mb_str2 && j == 41)
						{
								return 1;
						}
						if(str == mb_str2 && j == 56)
						{
								return 2;
						}
						OLED_Clear();
						arrow(10,j);
						OLED_ShowString(25, 0, "Level");
						OLED_ShowString(12, 20, "easy");
						OLED_ShowString(12, 35, "middle");
						OLED_ShowString(12, 50, "hard");
						OLED_Refresh_Gram();
						rt_thread_mdelay(50);
				}
		}
}
