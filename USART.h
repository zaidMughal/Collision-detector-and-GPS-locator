/*
* USART.h
*
* Created: 4/23/2019 2:52:42 AM
*  Author: Zaid Mughal
*/

/*
* This library functions are used to serially communicate between the GPS module and the microcontroller
*/


#ifndef USART_H_
#define USART_H_
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include<string.h>

#define tx_Buffer_size 100
char tx_Buffer[tx_Buffer_size];
int tx_Buffer_index=0;

#define locLen 55
char loc[locLen];
int loccounter=0;

char smsReceived=0;
char smsIndex[4];
int smsIndexCounter=0;


void usart_init(void)
{
	UCSR0B = (1<<TXEN0) | (1<<RXEN0) | (1<<RXCIE0);
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
	UBRR0L = 0x67;
	sei();//set external interrupt flag
}

void usart_send(unsigned char ch)
{
	while( !(UCSR0A & (1<<UDRE0)));
	UDR0 = ch;
}
void usart_sendint(int ch)
{
	while( !(UCSR0A & (1<<UDRE0)));
	UDR0 = ch>>8;
	while( !(UCSR0A & (1<<UDRE0)));
	UDR0 = ch;
	while( !(UCSR0A & (1<<UDRE0)));
	UDR0 = '\n';
}
void usart_sendString(char* str,char strLength){
	int i = 0;
	while (1)
	{
		if (str[i]=='\0')
		{
			break;
		}
		usart_send(str[i++]);
	}
}

int readStatus(void)
{
	char status_buffer[40]="                                        ";
	int status_buffer_index=0;
	do
	{
		if(UCSR0A & (1<<RXC0))
		{
			if(status_buffer_index<40){
			status_buffer[status_buffer_index++] = UDR0;}
			else
			{status_buffer[status_buffer_index-1] = UDR0;}
		}
	}while(status_buffer[status_buffer_index-1]!='\n');
	if(status_buffer[0]=='E')
	return 0;
	else
	return 1;
}
int getGPSloc(void)
{
	int errnot=1;
	cli();//clear wxternal interrupt flag
	char str[]="AT+SAPBR=3,1,\"Contype\",\"GPRS\"\n";
	int strlength = 30;
	usart_sendString(str,strlength);
	readStatus();
	readStatus();
	
	char str2[] ="AT+SAPBR=3,1,\"APN\",\"www\"\n";
	int strlength2 = 25;
	usart_sendString(str2,strlength2);
	readStatus();
	readStatus();
	
	char str3[] ="AT+SAPBR=1,1\n";
	int strlength3 = 13;
	usart_sendString(str3,strlength3);
	readStatus();
	if(!readStatus())
	{
		for(int i = 0;i<locLen;i++)
		loc[i] = ' ';//flush
		
		char mystr[] = "Low Signal,Location not found";
		for(loccounter=0;loccounter<29;loccounter++)
		loc[loccounter] = mystr[loccounter];
		errnot = 0;
	}
	else//Find location
	{
		char str4[] ="AT+CIPGSMLOC=1,1\n";
		int strlength4 = 17;
		usart_sendString(str4,strlength4);
		readStatus();
		
		for(int i = 0;i<locLen;i++)
		loc[i] = ' ';//flush
		loccounter=0;
		do
		{
			if(UCSR0A & (1<<RXC0))
			{
				if(loccounter<=locLen)
				loc[loccounter++] = UDR0;
				else
				loc[loccounter-1] = UDR0;//store in last
			}
		}while((loc[loccounter-1]!='\n'));
		
		readStatus();
		readStatus();
	}
	
char str4[] ="AT+SAPBR=0,1\n";//disconnect from network
int strlength4 = 13;
usart_sendString(str4,strlength4);
readStatus();
readStatus();

	sei();
	return errnot;
}
int index(char str[], char key,int i)
{
	while(str[i]!='\0')
	{
		if(str[i]==key)
			return key;
		i++;
	}
	return -1;
}
void Serialflush()
{
	for(int i=0;i<tx_Buffer_size;i++)
	{
		tx_Buffer[i]=0;
	}
	tx_Buffer_index=0;
}
void sendsms(char number[],char text[],int textlen)
{
	usart_sendString("AT+CMGF=1\r",10);
	_delay_ms(200);
	Serialflush();
	usart_sendString("AT+CMGS=\"",9);
	usart_sendString(number,13);
	usart_sendString("\"\r",2);
	usart_sendString(text,textlen);
	if(text[textlen-1]!='\r')
	usart_sendString("\r",1);
	_delay_ms(200);
	Serialflush();
	usart_send(0x1A);//sms terminate
	_delay_ms(2000);
	Serialflush();
}
void readSms()
{
	usart_sendString("AT+CMGF=1\r",10);
	_delay_ms(1000);//can take upto 5 seconds
	Serialflush();
	usart_sendString("AT+CMGR=",8);
	usart_sendString(smsIndex,smsIndexCounter);
	usart_send('\r');
	cli();
	readStatus();
	readStatus();
	Serialflush();
	sei();
	_delay_ms(4000);
	smsReceived = 0;
}


ISR(USART_RX_vect)//Interrupt service routine to handle serial receive interrupts
{
	do
	{
		if(UCSR0A & (1<<RXC0))
		{
			if(tx_Buffer_index!=tx_Buffer_size)
			tx_Buffer[tx_Buffer_index++] = UDR0;
			else
			tx_Buffer[tx_Buffer_index-1] = UDR0;//store in last
		}
	}while(UCSR0A & (1<<RXC0));
	
	
	for (int i = 0; i < tx_Buffer_index; i++)
	{
		if (tx_Buffer[i] == '\"')
		{
			//if (tx_Buffer[i + 1] == 'S')//this can vary to work for for specific sims. No condition means every sim
			{
			//	if (tx_Buffer[i + 2] == 'M')// ^
				{
					if (tx_Buffer[i + 3] == '\"')
					{
						if (tx_Buffer[i + 4] == ',')
						{
							_delay_ms(1000);
							if(tx_Buffer[i + 6] == '\n' || tx_Buffer[i + 7] == '\n' || tx_Buffer[i + 8] == '\n')//only if whole text is received
							{
								tx_Buffer[i+4] = 'n';//prevents loop from entring again
								smsIndexCounter = 0;
								do
								{
									if(tx_Buffer[i + 5]!='\r')
									{
									smsIndex[smsIndexCounter++]=tx_Buffer[i + 5];
									i++;
									}
								}while(smsIndex[smsIndexCounter - 1]!='\n');
								smsIndexCounter--;
								smsReceived=1;
							}
						}
					}
				}
			}
		}
	}
}
#endif