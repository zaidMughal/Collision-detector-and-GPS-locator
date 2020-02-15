/*
* main.c
*
* Created: 4/22/2019 5:00:59 PM
* Author : Zaid Mughal
*/

/*
* This product has three modes.
* 1. Collision mode: In this mode,the MPU6050 sensor reads your angle periodically and see if 
* 	 something is unusual e.g. a flipped car. In case of a collision, it first ask you if everything is ok 
* 	 by pressing a button. If you don't press it, it sends SOS signal along with your GPS location to the 
* 	 registered phone numbers. Currently only one number can be registered but it can be extended later on.
* 2. Non collision mode: It does not do the above mentioned operations.
* 3. GPS Location: It sends the current location of GPS module to the registered number.
*/

//send #* to turn on collision detection mode
//send ##* to turn off collision detection mode
//send ###* to see GPS location
#include <avr/io.h>
#include <math.h>
#include<string.h>
#include "i2c.h"
#include "USART.h"
#include "MPU6050.h"							/* Include I2C Master header file */

double a1,a2,a3;		//new angle values
long ax1,ax2,ax3;		//new values
long ax1p,ax2p,ax3p;	//previous values
char collisionMode = 1;

char onNumber[] = "+923444115978";
#define TEXTBUFFERSIZE 100
char textBuffer[TEXTBUFFERSIZE];
int textBufferIndex=0;

long abs(long x){
	if(x<0)
	return -x;
	else return x;
}
void flushTextBuffer()
{
	for (int i = 0 ; i < TEXTBUFFERSIZE ; i++)
	{
		textBuffer[i] = ' ';
	}
	textBufferIndex = 0;
}
void addtext(char str[],int startfrom,int endat)
{
	for(;startfrom<endat;startfrom++)
	{
		textBuffer[textBufferIndex++]=str[startfrom];
	}
}
char isCollided()
{
	static int angleError = 0;
	static int accError = 0;
	MPU6050_read(&ax1, &ax2, &ax3, &a1, &a2, &a3);
	long a=17000;
	long b=14000;
	long c=14000;
	char d=0;char e=0;char f=0;
	if((abs(ax1-ax1p)>a)||(abs(ax2-ax2p)>b))//||(abs(ax3-ax3p)>c)		//We only need x and y values
	{																	//Discomment it if needed
		if(++accError>=1)//can be change for sensitivity
		d=0xFF;
	}
	else
	accError = 0;
	
	if(((a1>30)&&(a1<330)) || ((a2>30)&&(a2<330)))  // || ((z>15)&&(z<350))
	{
		if(++angleError>=100)//can be change for sensitivity
		e=0xff;
	}
	else
	angleError = 0;
	
	f=d|e;
	ax1p = ax1;
	ax2p = ax2;
	ax3p = ax3;
	return f;
}
char checkIfManOK(void)
{
	DDRC |= 0x01;
	PORTC |= 0x01;
	char ok = 0;
	for(int i=0;i<50;i++)
	{
		TCNT1 = -1562; //(1/15625)*1562 = 0.1 = 100ms
		TCCR1B|= (1<<CS12) | (1<<CS10);	//1024 prescalar
		while((TIFR1&(1<<TOV1)) == 0)
		{
			if((PINC & (1<<PINC1)) ==(1<<PINC1))
			{
				ok = 0xFF;
				PORTC &= ~(0x01);
				collisionMode = 1;
				return ok;
			}
		}
		TIFR1 = (1<<TOV1);//clear TOV1 flag
		PORTC = PORTC ^ 0x01;
	}
	PORTC &= (~0x01);
	return ok;
}
int main(void)
{
	DDRB = 0xFF;
	DDRC = 0x01;
	PORTB = 0x00;
	
	usart_init();
	usart_sendString("AT",2);//initialize on desired baud rate
	_delay_ms(300);
	Serialflush();
	MPU6050_init();
	isCollided();
	isCollided();
	isCollided();
	isCollided();
	isCollided();
	while (1)
	{
		if(collisionMode)
		{
			PORTB = isCollided();
			if(PORTB)
			{
				flushTextBuffer();
				if(checkIfManOK()) continue;
				addtext("Collision detected\n",0,19);
				if(getGPSloc() && loccounter>20)
				{
					addtext("Latitude,Longitude: ",0,20);
					addtext(loc, 24, 33);//string from second, to third
					addtext(loc, 13, 23);//string from first, to second
				}
				else
					addtext(loc, 0, loccounter);
				sendsms(onNumber,textBuffer,textBufferIndex);
				collisionMode = 0;
			}
			
		}
		if((PINC & (1<<PINC2)) == (1<<PINC2))//Push button on pin A2
		{
			collisionMode = 1;
			sendsms(onNumber,"All OK",6);
		}
		
		if(smsReceived)
		{
			readSms();
			for (int i = 0; i < tx_Buffer_index; i++)
			{
				if (tx_Buffer[i] == '#')
				{
					if (tx_Buffer[i+1] == '*')//#*
					{
						collisionMode = 1;
						break;
					}
					else if(tx_Buffer[i+1] == '#')
					{
						if (tx_Buffer[i+2] == '*')//##*
						{
							collisionMode = 0;
							break;
						}
						else if(tx_Buffer[i+2] == '#')
						{
							if (tx_Buffer[i+3] == '*')//###*
							{
								PORTB = 0XFF;
								flushTextBuffer();
								getGPSloc();
								addtext("Latitude,Longitude: ",0,20);
								addtext(loc, 24, 33);//string from second, to third
								addtext(loc, 13, 23);//string from first, to second
								sendsms(onNumber,textBuffer,textBufferIndex);
								break;
							}
						}
					}
				}
			}
			Serialflush();
		}
	}
}
