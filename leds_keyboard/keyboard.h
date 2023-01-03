#ifndef KEYBOARD
#define KEYBOARD

#include "stm32f0xx.h"
#include <stdlib.h>

typedef struct
{
	int* button_state;
	int* stable_button;
	int* front;
} buttons;

buttons* init_buttons(unsigned int n);
void read_buttons(int* cur_state);
void filter(int* cur_btn, int* stable_button);
void check_buttons(int* cur, int* front);

//----------------------------------------------------------------------------------------------------------------------//

buttons* init_buttons(unsigned int n)
{
	buttons* new_buttons = malloc(sizeof(buttons));
	new_buttons->button_state = malloc(sizeof(int) * n);
	new_buttons->stable_button = malloc(sizeof(int) * n);
	new_buttons->front = malloc(sizeof(int) * n);
	
	return new_buttons;
}

void read_buttons(int* cur_state)
{
	GPIOA->BSRR = GPIO_BSRR_BS_15;
	
	cur_state[0] = ( (GPIOA->IDR & GPIO_IDR_4) == GPIO_IDR_4 ) ? 1 : 0;
	cur_state[1] = ((GPIOA->IDR & GPIO_IDR_5) == GPIO_IDR_5 ) ? 1 : 0;

	GPIOA->BSRR = GPIO_BSRR_BR_15;
	
		//wait while bit will set to zero
	__NOP(); // No operation
  __NOP(); // No operation	
	__NOP();
	
	GPIOC->BSRR = GPIO_BSRR_BS_12;
	
	cur_state[2] = ((GPIOA->IDR & GPIO_IDR_5) == GPIO_IDR_5 ) ? 1 : 0;
	cur_state[3] = ((GPIOA->IDR & GPIO_IDR_4) == GPIO_IDR_4 ) ? 1 : 0;

	GPIOC->BSRR = GPIO_BSRR_BR_12;
}

void filter(int* cur_btn, int* stable_button)
{
	static int count[4] = {0,0,0,0};
	static int prev_btn[4] = {0,0,0,0};
	for (int i = 0; i<4; ++i)
	{
		++count[i];
		if (prev_btn[i] == cur_btn[i])
		{
			if(count[i] > 9)
			{
				count[i] = 0;
				stable_button[i] = cur_btn[i];
			}
		}
		else {
		prev_btn[i] = cur_btn[i];
		count[i] = 0;}
	}
}

void check_buttons(int* cur, int* front)
{
	static int prev[4] = {0,0,0,0};
	for(int i = 0; i<4; ++i)
	{
		if(prev[i] != cur[i] && cur[i] == 1)
		{	
			front[i] = 1;	
		}
		prev[i] = cur[i];
	}
}



#endif
