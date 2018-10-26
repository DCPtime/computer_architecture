struct tm1638
{
	int STB;
	int CLK;
	int DIO;
	
};


void delay(unsigned int ticks);

void tm1638_sendbyte(struct tm1638* controller, unsigned int data);

unsigned int tm1638_receivebyte(struct tm1638 *controller);

void tm1638_sendcmd(struct tm1638* controller, unsigned int cmd);

void tm1638_setadr(struct tm1638* controller, unsigned int adr);

void tm1638_init(struct tm1638* controller);

