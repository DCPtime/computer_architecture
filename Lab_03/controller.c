#include "controller.h"
#include <LPC23xx.H>

// Delay
void delay(unsigned int ticks)
{
	unsigned int i;
	for(i = 0; i < ticks; i++)
	{
		
	}
}

void tm1638_sendbyte(struct tm1638* controller, unsigned int data)
{	
	int i;
	int outputDioPin = 1 << controller->DIO;
	int outputClkPin = 1 << controller->CLK; // Shifting 1 to pin with number CLK
	
	// Setting DIO to output
	IODIR1 |= (1 << outputClkPin);
	
	
	for(i = 0; i < 8; i++)
	{
		IOCLR1 = outputClkPin; // Clearing CLK pin
		delay(0xfff); // Delay
		
		if(data & 1) // Checking if current data's bit is 1
			IOSET1 = outputDioPin;
		else
			IOCLR1 = outputDioPin;
		
		delay(0xfff);
		
		data >>= 1; // Shifing data towards right;
		IOSET1 = outputClkPin; // Setting CLK pin
		
		delay(0x1fff);
	}
}

unsigned int tm1638_receivebyte(struct tm1638* controller)
{	
	unsigned int data = 0;
	int i;
	
	int outDioPin = 1 << controller->DIO;
	int outClkPin = 1 << controller->CLK;
	
	IODIR1 = IODIR1 & ~outDioPin; // Setting DIO pin to input;
	
	for(i = 0; i < 32; i++)
	{
		IOCLR1 = outClkPin; // Clear CLK
		
		delay(0xfff);
		
		if(IOPIN1 & outDioPin) // Reading DIO
		{
			data = data | (1 << i); // Setting bit
		}
		
		delay(0xfff);
		
		IOSET1 = outClkPin; // Setting CLK;
		delay(0x1fff);
	}
	
	return data;
}

void tm1638_sendcmd(struct tm1638* controller, unsigned int cmd)
{
	IOSET1=(1<<controller->STB);

	IODIR1 = (1<<controller->CLK)|(1<<controller->DIO)|(1<<controller->STB);

	IOCLR1=(1<<controller->STB);
	
	tm1638_sendbyte(controller, cmd);
}

void tm1638_setadr(struct tm1638* controller, unsigned int adr)
{
	// C0h == 1100 0000
	// 0xC0 | adr => 1100 XXXX
	tm1638_sendcmd(controller, 0xC0 | adr);
}

void tm1638_init(struct tm1638* controller)
{
	
	int i;

	tm1638_sendcmd(controller, 0x88); // Enable work of indication;
	
	tm1638_setadr(controller, 0); // Clear address
	
	for(i = 0; i < 0xf; i++)
		tm1638_sendbyte(controller, 0); // Setting fixed addressing mode
	
	tm1638_sendcmd(controller, 0x44);
}

