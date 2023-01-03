#include <stm32f0xx.h>

static int leds[] = { 6, 8, 7, 9 };

void wait_time(void);
void set_leds_button(void);
void counter_clockwise(void);
void clockwise(void);
void ODR_led(int);
void direction_leds(uint32_t*, int*);

void wait_time(void)
{
	for (int i = 0; i < 50000; i++);
}

void set_leds_button(void)
{
	// PC6 - PC9 - leds
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
	GPIOC->MODER &= ~(GPIO_MODER_MODER6 | GPIO_MODER_MODER7 | GPIO_MODER_MODER8 | GPIO_MODER_MODER9);
	GPIOC->MODER |= (GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0 | GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0);

	//PA0 - USER_BUTTON
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER &= ~(GPIO_MODER_MODER0);
}

void ODR_led(int j)
{
	GPIOC->ODR |= 1 << leds[j];
	wait_time();
	GPIOC->ODR &= ~(1 << leds[j]);
}

void clockwise(void)
{
	for (int j = 3; j > -1; j--)
	{
		ODR_led(j);
	}
}

void counter_clockwise(void)
{
	for (int j = 0; j < 4; j++)
	{
		ODR_led(j);
	}
}

void direction_leds(uint32_t* prev_state, int* state)
{
	uint32_t cur_state = GPIOA->IDR & GPIO_IDR_0;
	if (cur_state == GPIO_IDR_0 && cur_state != *prev_state)
	{
		*state = (*state + 1) % 2;
	}
	*prev_state = cur_state;
}

int main(void)
{
	set_leds_button();
	int state = 0;
	uint32_t prev_state = GPIOA->IDR & GPIO_IDR_0;

	while (1)
	{
		direction_leds(&prev_state, &state);
		if (state)
		{
			clockwise();
		}
		else
		{
			counter_clockwise();
		}
	}
}
