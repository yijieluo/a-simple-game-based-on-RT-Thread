#include <stdlib.h>
#include <math.h>
#include "pattern.h"
#include "oled.h"

/*箭头*/
void arrow(rt_uint8_t x,rt_uint8_t y)
{
		int i;
		for(i=0;i<10;i++)
		{
				OLED_DrawPoint(x-i, y, 1);//这里可以根据oled寄存器手册，快速画直线而不是这样描点，淘宝店家没有提供这个api，有点难受
		}
		for(i=0;i<2;i++)
		{
				OLED_DrawPoint(x-2+i, y+2-i, 1);
				OLED_DrawPoint(x-2+i, y-2+i, 1);
		}
}
/*					
						(x-11, y-2) 	  (x+8, y-2)
		直升机             (x,y)
            (x-11,y+10)     (x+8,y+10)    飞机四个角相对于(x,y)的位置，围成的矩形不能碰到obstacle
*/
void helicopter(rt_uint8_t x,rt_uint8_t y)
{
		int i;
		for(i=0;i<5;i++)
		{
				OLED_DrawPoint(x-2*i, y-(int)i*2/3, 1); 
				OLED_DrawPoint(x+2*i, y+(int)i*2/3, 1);
				OLED_DrawPoint(x+2*i, y-(int)i*2/3, 1);
				OLED_DrawPoint(x-2*i, y+(int)i*2/3, 1);
				OLED_DrawPoint(x,y+i,1);
		}
		for(i=0;i<13;i++)
		{
				OLED_DrawPoint(x-4+i,y+4,1);
				OLED_DrawPoint(x-4+i,y+10,1);
				OLED_DrawPoint(x-4,y+4+(int)i/2,1);
				OLED_DrawPoint(x+8,y+4+(int)i/2,1);
		}
		for(i=0;i<8;i++)
		{
				OLED_DrawPoint(x-4-i,y+7,1);
		}
}
void top_obstacle(rt_uint8_t x, rt_uint8_t height) //y=0
{
		int i;
		for(i=0;i<height;++i)
		{
				OLED_DrawPoint(x,i,1);
				OLED_DrawPoint(x+1,i,1);
				OLED_DrawPoint(x+2,i,1);
		}
		
}
void bottom_obstacle(rt_uint8_t x, rt_uint8_t height)//y=63
{
		int i;
		for(i=0;i<height;++i)
		{
				OLED_DrawPoint(x,64-i,1);
				OLED_DrawPoint(x+1,64-i,1);
				OLED_DrawPoint(x+2,64-i,1);
		}
}
void generate_map(rt_uint8_t level, rt_uint8_t h[][2])
{
		int i;
		rt_int16_t m,n;
		for(i=0;i<100;i++)
		{
				m = 15 + (int)15*sin(rand()); //top_obstacle的高度
				n = 28 - level + rand()%10/(level+1); //空余的高度  //飞机的高度为12
			  if((i>=1 && abs(m-h[i-1][0]) > 18+level) || 64-m-n <= 0) //控制两次的差值不能太大
				{
					i--;
					continue;
				}
				h[i][0] = m;
				h[i][1] = 64-m-n;
		}
}
//这个函数是碰撞的必要条件，不是充分条件
rt_bool_t check_crash(rt_uint8_t x, rt_uint8_t y, rt_uint8_t obstacle_x, rt_uint8_t th, rt_uint8_t bh, rt_int32_t n)
{
		if(y-2<0 || y+10>64) //出上下界
				return RT_FALSE;
		
		if(obstacle_x <= x)
		{
				if((x-8 >= obstacle_x && x-8 <= obstacle_x+2) && y-2 <= th)//左上角撞了左上障碍物
						return RT_FALSE;
				else if((x-4 >= obstacle_x && x+4 <= obstacle_x+2) && y+10 >= 64-bh)//左下角撞了左下障碍物
						return RT_FALSE;
				for(int j=0;j<13;j++)
				{
						if((x-11+j >= obstacle_x && x-11+j <= obstacle_x+2) && (y+7 <= th || y+7 >= 64-bh)) //尾部撞了左侧上下障碍物
								return RT_FALSE;
				}
		}
		else
		{
				if((x+8 >= obstacle_x && x+8 <= obstacle_x+2) && y-2 <= th)//右上角撞了右上障碍物
						return RT_FALSE;
				else if((x+8 >= obstacle_x && x+8 <= obstacle_x+2) && y+10 >= 64-bh)//右下角撞了右下障碍物
						return RT_FALSE;
		}
		return RT_TRUE;
}

