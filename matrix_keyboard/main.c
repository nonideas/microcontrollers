#include <stm32f0xx.h>
#include "keyboard.h"

#define SIZE 8

void set_led(int* cur_state);
void init(void);
void delay(int time);
void SysTick_Handler(void);
void work_with_buttons(void);

// init section
//-----------------------------------------------------------------------------------------------//

void spi_init(void);
void spi_init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;

	SPI2->CR1 = SPI_CR1_SSI | SPI_CR1_SSM | SPI_CR1_BR | SPI_CR1_MSTR; // spol, cpha

	SPI2->CR2 = SPI_CR2_DS; // ie
	SPI2->CR1 |= SPI_CR1_SPE;

	//PAS_GPO_LE
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER &= ~GPIO_MODER_MODER8;
	GPIOA->MODER |= GPIO_MODER_MODER8_0;

	// PS15_SPI2_MOSI, PS13_SPI2_SCK
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	// GPIOB->AFR[1] ~= GPIO_AFRH_AFRH
	// GPIOB->AFR[1] |= Nfunk << 4 * n; // Port n = 0..7
	// GPIOB->AFR[1] |= Nfunk << 4 * (n-8); // Port n = 8..15
	GPIOB->AFR[1] |= 0 << 4 * (13 - 8);
	GPIOB->AFR[1] |= 0 << 4 * (15 - 8);
	GPIOB->MODER &= ~(GPIO_MODER_MODER13 | GPIO_MODER_MODER15);
	GPIOB->MODER |= GPIO_MODER_MODER13_1 | GPIO_MODER_MODER15_1;
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

void sys_tick_init(void);
void sys_tick_init()
{
	SystemCoreClockUpdate();
	SysTick->LOAD = SystemCoreClock / 1000 - 1;
	SysTick->VAL = SystemCoreClock / 1000 - 1;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk;
}

void init(void)
{
	sys_tick_init();
	init_pins();
	spi_init();
}

//-----------------------------------------------------------------------------------------------//

static uint32_t x_axis[SIZE] = { 0x0001, 0x0001, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };
static uint32_t y_axis[SIZE] = { 0x0100 * 2, 0x0700, 0x0100 * 2, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };

void set_led(int* cur_state)
{
	if (cur_state[0] == 1)
	{
		cur_state[0] = 0;
		GPIOC->BSRR = GPIO_BSRR_BR_6;
		if (x_axis[0] == 0x0000) {
			for (int i = 0; i < SIZE - 1; i++) {
				x_axis[i] = x_axis[i + 1];
				y_axis[i] = y_axis[i + 1];
			}
			x_axis[SIZE - 1] = 0x0000;
			y_axis[SIZE - 1] = 0x0000;
		}
	}

	if (cur_state[1] == 1)
	{
		cur_state[1] = 0;
		GPIOC->BSRR = GPIO_BSRR_BR_7;
		if (x_axis[SIZE - 1] == 0x0000) {
			for (int i = SIZE - 1; i > 0; i--) {
				x_axis[i] = x_axis[i - 1];
				y_axis[i] = y_axis[i - 1];
			}
			x_axis[0] = 0x0000;
			y_axis[0] = 0x0000;
		}
	}

	if (cur_state[2] == 1)
	{
		cur_state[2] = 0;
		GPIOC->BSRR = GPIO_BSRR_BR_8;
		for (int i = 0; i < SIZE; i++) {
			if (y_axis[i] == 0x0100 * 2) { break; }
			y_axis[i] = y_axis[i] / 2;
		}
	}

	if (cur_state[3] == 1)
	{
		cur_state[3] = 0;
		GPIOC->BSRR = GPIO_BSRR_BR_9;
		GPIOA->BSRR = GPIO_BSRR_BR_8;
		for (int i = 0; i < SIZE; i++) {
			if (y_axis[i] == 0x0100 * 64) { break; }
			y_axis[i] = y_axis[i] * 2;
		}
	}
}

static volatile buttons* button;

void work_with_buttons(void)
{
	read_buttons(button->button_state);
	filter(button->button_state, button->stable_button);
	check_buttons(button->stable_button, button->front);
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
		for (int i = 0; i < 8; i++) {
			GPIOA->BSRR = GPIO_BSRR_BR_8;
			SPI2->DR = x_axis[i] * 1 << i | y_axis[i];
			while (SPI2->SR & SPI_SR_BSY);
			GPIOA->BSRR = GPIO_BSRR_BS_8;
		}
	}
}
