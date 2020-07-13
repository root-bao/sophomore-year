/**********单片机原理机器应用B***********************************************    
*       多功能电子时钟课设
***********************************************************************************/
/***********电子秒表功能************************************************************
*       按键start 开始计时
*       按键stop  停止
*       按键reset 复位
*       最多计时60分钟
***********************************************************************************/
/***********电子时钟功能************************************************************
*       按键可以修改时分秒
***********************************************************************************/

#include<reg51.h>
#include<absacc.h>       					//定义绝对地址访问
#include<intrins.h>
#define  uchar  unsigned  char
#define  uint  unsigned  int

//**********初始化时分秒毫秒的值**********
uint sec10=0;   //10ms
uint sec=55;    //1s次数的计数值
uint min =59;   //1min
uint hour =12;  //1h

//*********定义保存变量的四个参数*********
uint save_sec10;  //用来保存毫秒
uint save_sec;	  //用来保存秒
uint save_min;	  //用来保存分
uint sec_hour;	  //用来保存小时

//**********显示模式标志位初始化0表示时钟模式，1表示秒表模式********
uint flag=0;

//*********模式选择********************
sbit model_clock = P3^0;                    //时钟功能
sbit model_miao= P3^7;		                  //秒表功能

//**********电子秒表按键设置*************
sbit start = P1^7;	                        //K1按键表示开始 
sbit stop = P1^6;							              //K2按键表示停止
sbit reset = P1^5;							            //K3按键表示复位

//**********电子时钟按键******************
sbit sec_up= P3^1;                          //时钟秒增加
sbit sec_down = P3^2; 					           	//时钟秒减少
sbit min_up = P3^3; 						            //时钟分钟增加
sbit min_down = P3^4; 						          //时钟分钟减少
sbit hour_up = P3^5; 						            //时钟小时增加
sbit hour_down =P3^6;						            //时钟小时减少

//**************时间整点指示灯*******************
sbit led = P1^0;                            //LED整点报时位定义

//**************各个函数声明*********************
void  delay(uint);          				        //声明延时函数
void  Inital();                             //声明定时器初始函数
void  count_KeyScan();                      //声明计数器三独立按键功能函数
void  time_KeyScan();                       //声明时钟按键调整
void  display();        			              //声明时钟显示函数
void  clear();                              //清零函数

uchar  codevalue[12]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x00,0x40};	 //0～9\灭\小短横的字段码表
uchar  chocode[8]={0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f};                         //位选码表
//************定义时间显示缓冲区公用一个时间缓冲区************
uchar disbuffer[8] = {0x00,0x00,0x0b,0x00,0x00,0x0b,0x00,0x00};

//************主程序**************
void  main(void)
{
    led = 0;//led是默认关闭
    Inital();//初始化定时器
	
    while(1)
    {	
        time_KeyScan();                         //一开始就显示时钟				
	display();				//初始显示的是时间12-59-55
        count_KeyScan();		        //秒表按键功能函数
	         	
        if(model_clock==0)
        {
             delay(10);
	     if(model_clock==0)
             { 	
	         while(!model_clock){
	         //将清零前的数值保存下来，按键转换后从这个地方开始	
		    sec10=save_sec10;
                    sec=save_sec;
                    min=save_min;
                    hour=sec_hour;
		    flag=0;                    //时钟打开标志位				
		    TR0=1;ET0=1;	       //打开定时器，走表,打开总中断	
	          }					  
	      }		 
	}
		    
        else if(model_miao==0)
	{     
	    delay(10);
	    if(model_miao==0)
            { 	
	        while(!model_miao){
		    TR0=0;ET0=0;               	      //定时停止，停止走表
	            flag=1;			      //秒表打开标志位
                    clear();                          //清零，初始化00-00-00
                }
             }
	}  
    }
}

//**************初始化函数*********
void Inital()	 
{
    TMOD=0x01;              //选择定时器0的工作方式1
    TH0=0xD8;               //装初值（定时10ms）,晶振12MHz
    TL0=0xF0;
    EA=1;		    //打开总中断
    ET0=1;	            //打开定时器中断
    TR0=1;	            //先不要启动定时器    
}

//*********中断********
void time0_int(void)  interrupt 1   //定时器中断服务函数
{
    TH0=0xD8;                       //重装初值
    TL0=0xF0; 
    sec10++;		            //让进入中断次数值加1，用于判断是否达到0.1s
    if(sec10==100)                  //100*10ms=1s
    {
        sec10=0;
        sec++;
        if(sec==60)			    //sec=60 is 1minute
        {
            sec=0;
            min++;
            if(min==60)                 //min=60 is 1hour
	    {
                min=0;
	        hour++;
	        if(hour==24)            //hour=24 is max
		{
	             clear();           //时分秒毫秒都清零，重新开始
	        }
	     }
          }
      }
	   
     if(hour!=0)
     {//**********将清零前的数值保存下来，按键转换后从这个地方开始**********
	 save_sec10=sec10;	              //保存毫秒
         save_sec=sec;		              //保存秒
         save_min=min;		              //保存分
         sec_hour=hour;		              //保存小时
      }
}

//***********时钟功能函数***************
void  time_KeyScan()
{
    if(start==0)                              //设置时间之后，按下start键时钟继续走
    {
        delay(10);                //消抖
        if(start == 0)            //确实被按下
        {
            TR0=1;               //打开定时器，开始打节拍计数
        }
     }

//***************秒增加*********
    if(sec_up==0)        
    {
	 delay(6000);
         if(sec_up==0)
	 {
             TR0=0;
	     if(sec!=59)
	     {
	         sec++;
             }
	     else{sec=0;}
             delay(6000);
			      
             if(sec_up==1){TR0=1;}
          }
     }
//*******************秒减少*********	 
    if(sec_down==0)			  
    {
	  delay(6000);
          if(sec_down==0)
	  {
              TR0=0;
	      if(sec!=0)
              { 
                  sec--; 
              }
	      else{sec=59;}
	      delay(6000);
	      
	      if(sec_down==1){TR0=1;}
	   }
      }
//*****************分钟增加*********	 
    if(min_up==0)
    {
	delay(6000);
        if(min_up==0)
	{
            TR0=0;
            if(min!=59)
	    {
	        min++;
	    }
	    else{min=0;}
            delay(6000);
            
            if(min_up==1){TR0=1;}
         }
    }
//***************分钟减少**********	 
    if(min_down==0)
    {
        delay(6000);
        if(min_down==0)
	{
	    TR0=0;
	    if(min!=0)
	    {
	        min--;
	    }
	    else{min=59;}
	    delay(6000);
	    
            if(min_down==1){TR0=1;}
	 }
    }
//*****************小时增加********	 
    if(hour_up==0)
    {
	delay(6000);
        if(hour_up==0)
	{
            TR0=0;
	    if(hour!=23)
            {
                hour++;
	    }
	    else{hour=23;}
	    delay(6000);
			
	    if(hour_up==1){TR0=1;}
	 }
    }
//******************小时减少********	 
    if(hour_down==0)
    {
	delay(6000);
        if(hour_down==0)
        {
	    TR0=0;
	    if(hour!=0)
	    {
		hour--;
	    }
	    else{hour=23;}
			
	    delay(6000);
	    if(hour_down==1){TR0=1;}
	 }
    }
//****************整点灯亮**********
     if((sec==0)&(min==0))
     {
         led=1;
	 delay(100);
         led=0;
     }
}

//********秒表功能函数***********
void count_KeyScan()
{
    if(start==0)          //计时开始检测，按下start键
    {
        delay(10);       //消抖
	if(start == 0)   //确实被按下
	{
            TR0=1;ET0=1;
	}
    }
    
    if(stop==0)		     //暂停键
    {
	delay(10);
	if(stop==0)      //确实被按下
	{
	    TR0=0;ET0=0;
	}
    }
    
    if(reset==0)	     //复位键
    {
	delay(10);
	if(reset==0)     //确实被按下
	{
            clear();	 //都清零
	    TR0=0;ET0=0;
	}
     } 
}

//***********清零函数*********
void clear()
{
    sec10=0;        //10ms
    sec=0;          //1s
    min =0;         //1min
    hour=0;	    //1h
}

//************不精准延时函数************
void  delay(uint i)        				
{
    uint  j;
    for(j=0;j<i;j++){}
}

//************显示函数******************
void  display(void)        				//定义显示函数
{
    uchar  i,p,temp;

    if(flag==0){//*************clock功能
	disbuffer[7] = sec%10;              //秒个位
        disbuffer[6] = sec/10;	            //秒十位

	disbuffer[4] = min%10;		     	//分个位
	disbuffer[3] = min/10;			    //分十位

	disbuffer[1] = hour%10;			    //小时个位
	disbuffer[0] = hour/10;			    //小时十位
     }

     if(flag==1){//************秒表功能
	disbuffer[7] = (sec10%10)%10;   //毫秒个位
        disbuffer[6] = (sec10/10)%10;	//毫秒十位

	disbuffer[4] = sec%10;			//秒个位
	disbuffer[3] = sec/10;			//秒十位

	disbuffer[1] = min%10;			//分个位
	disbuffer[0] = min/10;			//分十位
    }
    
    for(i=0;i<8;i++)
    {
        temp=chocode[i];                        //取当前的位选码
        P2=temp;        	                //送出位选码
        p=disbuffer[i];                         //取当前显示的字符
        temp=codevalue[p];                      //查得显示字符的字段码
        P0=temp;        		        //送出字段码
        delay(20);                              //延时1ms
    }	
} 
