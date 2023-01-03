#include <stm32f0xx.h>

volatile int time_out;
uint32_t time = 0;

void SysTick_Handler(void);
void set_leds_button(void);
void ODR_led(int);
void direction_leds(uint32_t*, int*);

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

void ODR_led(int led)
{
	GPIOC->ODR &= ~(GPIO_ODR_6 | GPIO_ODR_7 | GPIO_ODR_8 | GPIO_ODR_9);

	switch (led)
	{
	case 6:
		GPIOC->ODR |= GPIO_ODR_6;
		break;
	case 7:
		GPIOC->ODR |= GPIO_ODR_8;
		break;
	case 8:
		GPIOC->ODR |= GPIO_ODR_7;
		break;
	case 9:
		GPIOC->ODR |= GPIO_ODR_9;
		break;
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

void SysTick_Handler(void)
{
	if (time >= 50000) {
		time = 0;
		time_out = 1;
	}
	else {
		time_out = 0;
		time++;
	}
}

int main(void)
{
	set_leds_button();
	int state = 0;
	int led = 6;
	uint32_t prev_state = GPIOA->IDR & GPIO_IDR_0;

	SystemCoreClockUpdate();
	SysTick->LOAD = SystemCoreClock / 1000 - 1;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk;

	while (1)
	{
		direction_leds(&prev_state, &state);
		SysTick_Handler();
		if (time_out) {
			if (state) {
				ODR_led(led);
				led++;
				if (led == 10) { led = 5; }
			}
			else {
				ODR_led(led);
				led--;
				if (led == 4) { led = 9; }
			}
		}
	}
}
