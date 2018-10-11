//  Nesterenko IU7-54


#include <LPC23xx.H>
#include "controller.h"

#define OUTPUT_PINS 0x38000000 // Bit mask for all output pins
#define PUMP_PIN 0x20000000 // Bit mask of Pump
#define HEATER_PIN 0x10000000 // Bit mask of Heater
#define ROTOR_PIN 0x8000000 //  Bit mask of rotor
#define BUTTON_PIN 0x4000000 // Button

struct tm1638 tm;

// Describes binary pin state
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
	tm1638_setadr(&tm, 0);
	tm1638_sendcmd(&tm, 0x46); // Отправка команды чтение сигнала с клавиатуры
	
	return tm1638_receivebyte(&tm) & (1); // Битовая маска - читаем только 1-ую кнопку (SEG1)
}

// Установить состояние выделенного выхода (или нескольких выходов)
void turn_heater(int heater_pin, enum pin_state state)
{
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
			//IOSET0 = heater_pin;
			set_led_enabled(led_num, 1);
			break;
		
		case DISABLED:
			//IOCLR0 = heater_pin;
			set_led_enabled(led_num, 0);
			break;
	}
}

// Считать сигнал с выхода pin
enum pin_state read_pin(int pin)
{
	if((IOPIN0 & pin) != 0)
		return ENABLED;
	return DISABLED;
}

int main(void)
{	
	// Предыдущее состояние кнопки
	enum pin_state prev_button_state;
	
	// Текущее состояние устройства (1 - включено, 0 - выключено)
	int device_enabled = 1;
	
	// Предыдущее состояние устройства, чтобы избежать отключение уже отключенного элемента
	int prev_enabled_state = 0;
	
	/*// Set controll to General Purpose Input/Outpu
	PINSEL0 = 0;
	
	/// Set output direction for heaters' pins
	IODIR0 = OUTPUT_PINS;*/
	
	tm.STB = 26;
	tm.CLK = 27;
	tm.DIO = 28;
	tm1638_init(&tm);

	while(1)
	{
		// Постоянно работающие элементы (двигатель и нагреватель)
		if (const_elements = 0)
		{
			turn_heater(HEATER_PIN, ENABLED);
			turn_heater(ROTOR_PIN, ENABLED);
			const_elements = 1;
		}

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
			
			// Активировать насос
			turn_heater(PUMP_PIN, ENABLED);
			
			// Сохранить состояние устройства
			prev_enabled_state = 1;
		}
		else // Устройство не активно
		{
			// Отключить насос
			if(prev_enabled_state == 1)
			{
				turn_heater(PUMP_PIN, DISABLED);
			}

			// Сохранить состояние устройства
			prev_enabled_state = 0;
			current_tick = 0; // Reset timer
		}		
	}
	
}