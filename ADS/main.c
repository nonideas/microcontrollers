#include <stm32f0xx.h>

static const uint8_t update_time = 10; // in millisecs
static const uint16_t scanTime = 100; //in millisecs time of scan

static uint8_t READ = 0;
static uint32_t TIME = 0;

static uint32_t max_voltage = 1100;
static uint32_t cur_voltage = 0;
static uint32_t voltage_arr[8];
static uint32_t prev_col = 4;

void spi_init(void);
uint32_t bitSet(uint32_t value, uint32_t position);
void sys_tick_init(void);
void SysTick_Handler(void);
void SPI2_IRQHandler(void);
void initADC(void);
void updateVoltage(void);
uint32_t getPattern(void);

void spi_init() {	
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOAEN;
	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
	GPIOB->AFR[1] = 0 << (15 - 8) * 4;
	GPIOB->AFR[1] = 0 << (13 - 8) * 4;
	SPI2->CR1 = 
		  SPI_CR1_SSM 
		| SPI_CR1_SSI 
		| SPI_CR1_BR 
		| SPI_CR1_MSTR 
		| SPI_CR1_CPOL 
		| SPI_CR1_CPHA;
	GPIOB->MODER |= GPIO_MODER_MODER13_1 | GPIO_MODER_MODER15_1;
	GPIOA->MODER |= GPIO_MODER_MODER8_0;
	GPIOB->PUPDR |= GPIO_PUPDR_PUPDR13_1 | GPIO_PUPDR_PUPDR15_1;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR8_1;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR4_1; 
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR5_1;
	GPIOB->AFR[1] |= (0 << (4 * (13 - 8))) | (0 << (4 * (15 - 8)));
	SPI2->CR2 = SPI_CR2_DS | SPI_CR2_RXNEIE;
	NVIC_EnableIRQ(SPI2_IRQn);
	SPI2->CR1 |= SPI_CR1_SPE;
	SPI2->DR = 0;
}

void sys_tick_init()
{	
 	SystemCoreClockUpdate();
	SysTick->LOAD = SystemCoreClock/100 - 1;
	SysTick->VAL = 0;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk; // the last bit means make on interrupt system
}

void initADC()
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER |= GPIO_MODER_MODER1;
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	ADC1->CR |= ADC_CR_ADCAL;
	while (ADC1->CR & ADC_CR_ADCAL) {}
	ADC1->CR |= ADC_CR_ADEN;
	while((ADC1->ISR & ADC_ISR_ADRDY) == 0);
	ADC1->CHSELR |= ADC_CHSELR_CHSEL1;					
	ADC1->CFGR1 |= ADC_CFGR1_RES_0;
	ADC1->CFGR1 |= ADC_CFGR1_CONT;
	ADC1->CR |= ADC_CR_ADSTART;
}

void init(void);
void init(void)
{
	sys_tick_init();
	initADC();
	spi_init();
}

uint32_t bitSet(uint32_t value, uint32_t position)
{
	return (value | (1 << position));
}

void SysTick_Handler()
{
	TIME += 1;
	if(TIME == (scanTime/update_time))
	{
		TIME = 0;
		READ = 1;
	}
}

uint32_t getPattern()
{
	uint32_t cur_col = (prev_col + 1) == 8 ? 0 : (prev_col + 1);
	uint32_t pattern = 0;
	pattern = bitSet(pattern,8 + cur_col);
	uint32_t line_to_show = voltage_arr[cur_col] * 8 / max_voltage;
	line_to_show = 7 - line_to_show;
	pattern = bitSet(pattern,line_to_show);
	return pattern;
}

void updateVoltage()
{
	cur_voltage = ADC1->DR;
	for(int i = 0; i < 7; ++i) {
		voltage_arr[7-i] = voltage_arr[6-i];
	}
	voltage_arr[0] = cur_voltage;
}

void SPI2_IRQHandler(void)
{
	if((SPI2->SR && SPI_SR_RXNE) == SPI_SR_RXNE) 
	{
		GPIOA->BSRR = GPIO_BSRR_BS_8;
		SPI2->DR;
		prev_col = ((prev_col + 1) == 8) ? 0 : (prev_col + 1);
		SPI2->DR = getPattern();
		GPIOA->BSRR = GPIO_BSRR_BR_8;
	}
}

int main(void)
{
	init();
	
	while(1)
	{
		if(READ)
		{
			updateVoltage();
			READ = 0;
		}
	}
}
