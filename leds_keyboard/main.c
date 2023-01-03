#include <stm32f0xx.h>
#include "keyboard.h"

void set_led(int* cur_state);
void init(void);
void delay(int time);
void SysTick_Handler(void);
void work_with_buttons(void);

static volatile buttons* button;

void delay(int time)
{
	for (int i = 0; i < time; ++i);
}

void set_led(int* cur_state)
{
	if (cur_state[0])
	{
		cur_state[0] = 0;
		if ((GPIOC->ODR & GPIO_ODR_6) == GPIO_ODR_6)
		{
			GPIOC->BSRR = GPIO_BSRR_BR_6;
		}
		else
		{
			GPIOC->BSRR = GPIO_BSRR_BS_6;
		}
	}

	if (cur_state[1] == 1)
	{
		cur_state[1] = 0;
		if ((GPIOC->ODR & GPIO_ODR_7) == GPIO_ODR_7)
		{
			GPIOC->BSRR = GPIO_BSRR_BR_7;
		}
		else
		{
			GPIOC->BSRR = GPIO_BSRR_BS_7;
		}
	}

	if (cur_state[2] == 1)
	{
		cur_state[2] = 0;
		if ((GPIOC->ODR & GPIO_ODR_8) == GPIO_ODR_8)
		{
			GPIOC->BSRR = GPIO_BSRR_BR_8;
		}
		else
		{
			GPIOC->BSRR = GPIO_BSRR_BS_8;
		}
	}

	if (cur_state[3] == 1)
	{
		cur_state[3] = 0;
		if ((GPIOC->ODR & GPIO_ODR_9) == GPIO_ODR_9)
		{
			GPIOC->BSRR = GPIO_BSRR_BR_9;
		}
		else
		{
			GPIOC->BSRR = GPIO_BSRR_BS_9;
		}
	}
}

void sys_tick_init(void);
void sys_tick_init()
{
	SystemCoreClockUpdate();
	SysTick->LOAD = SystemCoreClock / 1000 - 1;
	SysTick->VAL = SystemCoreClock / 1000 - 1;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk;
}

void init_pins(void);
void init_pins(void)
{
	RCC->AHBENR |= (RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIOAEN); // AHB peripheral clock enable register - RCC_AHBENR

	// init for leds
	GPIOC->MODER &= ~(GPIO_MODER_MODER6 | GPIO_MODER_MODER7 | GPIO_MODER_MODER8 | GPIO_MODER_MODER9);
	GPIOC->MODER |= (GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0 | GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0);

	GPIOC->MODER &= ~(GPIO_MODER_MODER12);
	GPIOC->MODER |= (GPIO_MODER_MODER12_0);

	GPIOA->MODER &= ~(GPIO_MODER_MODER15 | GPIO_MODER_MODER4 | GPIO_MODER_MODER5);
	GPIOA->MODER |= (GPIO_MODER_MODER15_0);

	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR4 | GPIO_PUPDR_PUPDR5);
	GPIOA->PUPDR |= (GPIO_PUPDR_PUPDR4_1 | GPIO_PUPDR_PUPDR5_1);
}

void work_with_buttons(void)
{
	read_buttons(button->button_state);
	filter(button->button_state, button->stable_button);
	check_buttons(button->stable_button, button->front);
}

void init(void)
{
	sys_tick_init();
	init_pins();
}

void SysTick_Handler(void)
{
	work_with_buttons();
}

int main(void)
{
	// init section
	init();
	button = init_buttons(4);

	while (1)
	{
		set_led(button->front);
	}
}
