#include <zephyr.h>
#include <init.h>
#include <drivers/i2c.h>
#include <helper.h>
#include "codetab.h"

/*
* void I2C_Configuration(void) -- 配置CPU的硬件I2C
* void I2C_WriteByte(uint8_t addr,uint8_t data) -- 向寄存器地址写一个byte的数据
* void WriteCmd(unsigned char I2C_Command) -- 写命令
* void WriteDat(unsigned char I2C_Data) -- 写数据
* void OLED_Init(void) -- OLED屏初始化
* void OLED_SetPos(unsigned char x, unsigned char y) -- 设置起始点坐标
* void OLED_Fill(unsigned char fill_Data) -- 全屏填充
* void OLED_CLS(void) -- 清屏
* void OLED_ON(void) -- 唤醒
* void OLED_OFF(void) -- 睡眠
* void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize) -- 显示字符串(字体大小有6*8和8*16两种)
* void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N) -- 显示中文(中文需要先取模，然后放到codetab.h中)
* void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]) -- BMP图片
*
*/

// iic_scl, PB8
// iic_sda, PB9
#define I2C_DEV_NAME CONFIG_I2C_1_NAME

#define OLED_ADDRESS	0x78 //通过调整0R电阻,屏可以0x78和0x7A两个地址 -- 默认0x78

static struct device *i2c_dev;

static u32_t i2c_cfg = I2C_SPEED_SET(I2C_SPEED_STANDARD) | I2C_MODE_MASTER;


int32_t I2C_Configuration(void)
{
	i2c_dev = device_get_binding(I2C_DEV_NAME);
	if (i2c_dev == NULL) {
		printk("can't get ic2_dev['%s']\n", I2C_DEV_NAME);
		return -1;
	}

	u32_t i2c_cfg = I2C_SPEED_SET(I2C_SPEED_FAST) | I2C_MODE_MASTER;

	if (i2c_configure(i2c_dev, i2c_cfg)) {
		printk("I2C config Fail.\n");
		return -2;
	}
}

void I2C_WriteByte(uint8_t addr,uint8_t data)
{
    i2c_reg_write_byte(i2c_dev, OLED_ADDRESS, addr, data);
}


void WriteCmd(unsigned char I2C_Command)//写命令
{
	I2C_WriteByte(0x00, I2C_Command);
}

void WriteDat(unsigned char I2C_Data)//写数据
{
	I2C_WriteByte(0x40, I2C_Data);
}

void OLED_Init(void)
{
	k_sleep(100); //这里的延时很重要

	WriteCmd(0xAE); //display off
	WriteCmd(0x20);	//Set Memory Addressing Mode
	WriteCmd(0x10);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	WriteCmd(0xb0);	//Set Page Start Address for Page Addressing Mode,0-7
	WriteCmd(0xc8);	//Set COM Output Scan Direction
	WriteCmd(0x00); //---set low column address
	WriteCmd(0x10); //---set high column address
	WriteCmd(0x40); //--set start line address
	WriteCmd(0x81); //--set contrast control register
	WriteCmd(0xff); //亮度调节 0x00~0xff
	WriteCmd(0xa1); //--set segment re-map 0 to 127
	WriteCmd(0xa6); //--set normal display
	WriteCmd(0xa8); //--set multiplex ratio(1 to 64)
	WriteCmd(0x3F); //
	WriteCmd(0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	WriteCmd(0xd3); //-set display offset
	WriteCmd(0x00); //-not offset
	WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
	WriteCmd(0xf0); //--set divide ratio
	WriteCmd(0xd9); //--set pre-charge period
	WriteCmd(0x22); //
	WriteCmd(0xda); //--set com pins hardware configuration
	WriteCmd(0x12);
	WriteCmd(0xdb); //--set vcomh
	WriteCmd(0x20); //0x20,0.77xVcc
	WriteCmd(0x8d); //--set DC-DC enable
	WriteCmd(0x14); //
	WriteCmd(0xaf); //--turn on oled panel
}

void OLED_SetPos(unsigned char x, unsigned char y) //设置起始点坐标
{
	WriteCmd(0xb0+y);
	WriteCmd(((x&0xf0)>>4)|0x10);
	WriteCmd((x&0x0f)|0x01);
}

void OLED_Fill(unsigned char fill_Data)//全屏填充
{
	unsigned char m,n;
	for(m=0;m<8;m++)
	{
		WriteCmd(0xb0+m);		//page0-page1
		WriteCmd(0x00);		//low column start address
		WriteCmd(0x10);		//high column start address
		for(n=0;n<128;n++)
			{
				WriteDat(fill_Data);
			}
	}
}

void OLED_CLS(void)//清屏
{
	OLED_Fill(0x00);
}

//--------------------------------------------------------------
// Prototype      : void OLED_ON(void)
// Calls          :
// Parameters     : none
// Description    : 将OLED从休眠中唤醒
//--------------------------------------------------------------
void OLED_ON(void)
{
	WriteCmd(0X8D);  //设置电荷泵
	WriteCmd(0X14);  //开启电荷泵
	WriteCmd(0XAF);  //OLED唤醒
}

//--------------------------------------------------------------
// Prototype      : void OLED_OFF(void)
// Calls          :
// Parameters     : none
// Description    : 让OLED休眠 -- 休眠模式下,OLED功耗不到10uA
//--------------------------------------------------------------
void OLED_OFF(void)
{
	WriteCmd(0X8D);  //设置电荷泵
	WriteCmd(0X10);  //关闭电荷泵
	WriteCmd(0XAE);  //OLED休眠
}

//--------------------------------------------------------------
// Prototype      : void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
// Calls          :
// Parameters     : x,y -- 起始点坐标(x:0~127, y:0~7); ch[] -- 要显示的字符串; TextSize -- 字符大小(1:6*8 ; 2:8*16)
// Description    : 显示codetab.h中的ASCII字符,有6*8和8*16可选择
//--------------------------------------------------------------
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
{
	unsigned char c = 0,i = 0,j = 0;
	switch(TextSize)
	{
		case 1:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 126)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<6;i++)
					WriteDat(F6x8[c][i]);
				x += 6;
				j++;
			}
		}break;
		case 2:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 120)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<8;i++)
					WriteDat(F8X16[c*16+i]);
				OLED_SetPos(x,y+1);
				for(i=0;i<8;i++)
					WriteDat(F8X16[c*16+i+8]);
				x += 8;
				j++;
			}
		}break;
	}
}

//--------------------------------------------------------------
// Prototype      : void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N)
// Calls          :
// Parameters     : x,y -- 起始点坐标(x:0~127, y:0~7); N:汉字在codetab.h中的索引
// Description    : 显示codetab.h中的汉字,16*16点阵
//--------------------------------------------------------------
void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N)
{
	unsigned char wm=0;
	unsigned int  adder=32*N;
	OLED_SetPos(x , y);
	for(wm = 0;wm < 16;wm++)
	{
		WriteDat(F16x16[adder]);
		adder += 1;
	}
	OLED_SetPos(x,y + 1);
	for(wm = 0;wm < 16;wm++)
	{
		WriteDat(F16x16[adder]);
		adder += 1;
	}
}

//--------------------------------------------------------------
// Prototype      : void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]);
// Calls          :
// Parameters     : x0,y0 -- 起始点坐标(x0:0~127, y0:0~7); x1,y1 -- 起点对角线(结束点)的坐标(x1:1~128,y1:1~8)
// Description    : 显示BMP位图
//--------------------------------------------------------------
void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[])
{
	unsigned int j=0;
	unsigned char x,y;

  if(y1%8==0)
		y = y1/8;
  else
		y = y1/8 + 1;
	for(y=y0;y<y1;y++)
	{
		OLED_SetPos(x0,y);
    for(x=x0;x<x1;x++)
		{
			WriteDat(BMP[j++]);
		}
	}
}

void oled_thread_entry(struct device *unused)
{
    ARG_UNUSED(unused);

    int ret = 0;

    ret = I2C_Configuration();

    if (ret) {
        printk("run I2C_Configuration failed, ret: %d\n", ret);
        return;
    }

    OLED_Init();

    while(1)
	{
		OLED_Fill(0xFF);//全屏点亮
		k_sleep(2000);
		OLED_Fill(0x00);//全屏灭
		k_sleep(2000);
		for(int i=0;i<5;i++)
		{
			OLED_ShowCN(22+i*16,0,i);//测试显示中文
		}
		k_sleep(2000);
		OLED_ShowStr(0,3,"HelTec Automation",1);//测试6*8字符
		OLED_ShowStr(0,4,"Hello Tech",2);				//测试8*16字符
		k_sleep(2000);
		OLED_CLS();//清屏
		OLED_OFF();//测试OLED休眠
		k_sleep(2000);
		OLED_ON();//测试OLED休眠后唤醒
		OLED_DrawBMP(0,0,128,8,(unsigned char *)BMP1);//测试BMP位图显示
		k_sleep(2000);
	}
}

K_THREAD_DEFINE(oled_thread, 1024, oled_thread_entry, NULL, NULL, NULL, 1, 0, K_NO_WAIT);
