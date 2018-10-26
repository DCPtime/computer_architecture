//  Nesterenko IU7-54


#include <LPC23xx.H>
#include "controller.h"

#define OUTPUT_PINS 0x38000000 // Bit mask for all output pins
#define PUMP_PIN 0x20000000 // Bit mask of Pump
#define HEATER_PIN 0x10000000 // Bit mask of Heater
#define ROTOR_PIN 0x8000000 //  Bit mask of rotor
#define BUTTON_PIN 0x4000000 // Button

struct tm1638 tm;

// Описание состояние выхода
enum pin_state
{
	ENABLED = 1, DISABLED = 0
};

int const_elements = 0;

void set_led_enabled(int led_num, int enabled)
{	
	int n = led_num * 2 + 1;
	tm1638_setadr(&tm, n);
	
	if(enabled)		
		tm1638_sendbyte(&tm, n);
	else
		tm1638_sendbyte(&tm, 0);
}

int read_key_state()
{
	
	int received = 0;
#ifdef USE_TM
	tm1638_sendcmd(&tm, 0x46); // Sending READ Command
	
	received = tm1638_receivebyte(&tm);
#else
	int dir_backup = IODIR1; // Что это и зачем	
	return (IOPIN1 & BUTTON_PIN) != 0;
#endif
	return received; // We need only first button (SEG1)
}

// Установить состояние выделенного выхода (или нескольких выходов)
void turn_heater(int heater_pin, enum pin_state state)
{
#ifdef USE_TM
    int dir_backup = IODIR1; // Что это и зачем	
	int led_num;
	switch(heater_pin)
	{
		case PUMP_PIN:
			led_num = 0;
			break;
		case HEATER_PIN:
			led_num = 1;
			break;
		case ROTOR_PIN:
			led_num = 2;
			break;
		
		default: led_num = 0;
	}
	
	switch(state)
	{
		case ENABLED:
			set_led_enabled(led_num, 1);
			break;
		
		case DISABLED:
			set_led_enabled(led_num, 0);
			break;
	}
	
	IODIR1 = dir_backup;
}
#else
	switch(state)
	{
		case ENABLED:
			IOSET1 = heater_pin;
			break;
		
		case DISABLED:
			IOCLR1 = heater_pin;
			break;
	}
#endif
}	

// Считать сигнал с выхода pin
enum pin_state read_pin(int pin)
{
	if((read_key_state() & 1) != 0)
		return ENABLED;
	return DISABLED;
}

void timer0_init(void)
{
	T0PR = 15000; // Timer PREDDELITEL

	T0TCR = 0x00000002; // Reset counter and divider

	T0MCR = 0x00000006; // Stop timer if match

	T0MR0 = 1000; // Match registry
}

int main(void)
{	
	// Предыдущее состояние кнопки
	enum pin_state prev_button_state;
	
	// Текущее состояние устройства (1 - включено, 0 - выключено)
	int device_enabled = 0;
	
	// Предыдущее состояние устройства, чтобы избежать отключение уже отключенного элемента
	int prev_enabled_state = 0;
	
		// Time to switch heater
#if USE_TM
	int cycle_tick_count = 1; // ~~ 1 second 1000000
#else
	int cycle_tick_count = 20000;
#endif

    int stage = 0;
	int prev_stage = -1;

#ifdef USE_TM	
	tm.STB = 26;
	tm.CLK = 27;
	tm.DIO = 28;
	tm1638_init(&tm);
#else
	PINSEL1 = 0;
	IODIR1 = OUTPUT_PINS;
#endif
    timer0_init();

	
	// Turning off all LEDS
	turn_heater(HEATER_PIN, DISABLED);
	turn_heater(ROTOR_PIN, DISABLED);
	turn_heater(PUMP_PIN, DISABLED);
	
	T0MR0 = cycle_tick_count;
	
	// Постоянно работающие элементы (двигатель и нагреватель)
	/*
	turn_heater(HEATER_PIN, ENABLED);
	turn_heater(ROTOR_PIN, ENABLED);*/

	while(1)
	{
		// Проверка нажатия на кнопку
		enum pin_state button_state = read_pin(BUTTON_PIN);			
		
		if(button_state == DISABLED && prev_button_state == ENABLED)
		{
			// Изменить состояние устройсва
			device_enabled = !device_enabled;
		}
		
		// Сохранить состояние текущей кнопки
		prev_button_state = button_state;
		
		// Устройство активно?
		if(device_enabled)
		{
			if(stage == 0) // 2 heaters are working
		    {
				if(prev_stage != stage)
				{
					T0TC = 0;
					T0TCR = 1;
					T0MR0 = cycle_tick_count / 2; // TODO 10 SECONDS
					
					turn_heater(HEATER_PIN, ENABLED);
					turn_heater(ROTOR_PIN, ENABLED);
					turn_heater(PUMP_PIN, ENABLED);
					
				}
				
				if((T0TCR & 0x1) == 0) //  && prev_enabled_state == 1
				{
					stage = 1;
					turn_heater(HEATER_PIN, DISABLED);
					turn_heater(ROTOR_PIN, DISABLED);
					current_heater = PUMP_PIN;
					
				}
				
				prev_stage = 0;
		    }
			
			if(stage == 1)
			{
				
			}
			
		}
			
			
	}	
	
	
}
