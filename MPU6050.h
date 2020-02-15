/*
 * MPU6050.h
 *
 * Created: 4/29/2019 5:49:06 PM
 *  Author: Zaid Mughal
 */ 


#ifndef MPU6050_H_
#define MPU6050_H_



#include <avr/io.h>
#include <math.h>
#include"i2c.h"

#define RAD_TO_DEG 57.30
#define PI 3.14
#define minVal 265//by testing
#define maxVal 402
#define SDA 4
#define SCL 5


long map(long x, long in_min, long in_max, long out_min, long out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
void MPU6050_init(void){
	_delay_ms(150);
	begin(0x68);
	beginTransmission(0x68);
	write(0x6B);
	write(0x00);
	endTransmission();
	
	beginTransmission(0x68);
	write(0x1C);
	write(0x10);
	endTransmission();
	
	beginTransmission(0x68);
	write(0x1B);
	write(0x08);
	endTransmission();
}

void MPU6050_read(long* Acx, long* Acy, long* Acz, double* x, double* y, double* z){
	  beginTransmission(0x68);                                        //Start communicating with the MPU-6050
	  write(0x3B);                                                    //Send the requested starting register
	  endTransmission();                                              //End the transmission
	  requestFrom(0x68,14,0,0,1);                                     //Request 14 bytes from the MPU-6050
	  while(available() < 14);                                        //Wait until all the bytes are received
	  *Acx = read()<<8|read();
	  *Acy = read()<<8|read();
	  *Acz = read()<<8|read();
	  long Tmp = read()<<8|read();
	  long GyX = read()<<8|read();
	  long GyY = read()<<8|read();
	  long GyZ = read()<<8|read();
	 int xAng = map(*Acx,minVal,maxVal,-90,90);
	 int yAng = map(*Acy,minVal,maxVal,-90,90);
	 int zAng = map(*Acz,minVal,maxVal,-90,90);
	 *x= RAD_TO_DEG * (atan2(-yAng, -zAng)+PI);
	 *y= RAD_TO_DEG * (atan2(-xAng, -zAng)+PI);
	 *z= RAD_TO_DEG * (atan2(-yAng, -xAng)+PI);
	

}



#endif /* MPU6050_H_ */