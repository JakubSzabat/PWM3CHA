/**
  ******************************************************************************
  * @file    PWM3CHA/main.c 
  * @author  Jakubsza
  * @brief   Config of 3 channels pwm to control RGB diode
  *
**/

#include "stm32f0xx.h"

#define TIMx	TIM1			//defined constants
#define PRES_VAL	48000
#define ARR_VAL	20
#define RED	0
#define GREEN	20
#define BLUE	0

typedef struct RGB_ValS{	//struct having RGB values in
	uint16_t R_C:11;
	uint16_t G_C:11;
	uint16_t B_C:11;
}RGB_ValS;

volatile uint32_t tick = 0;

__INLINE void PWM_Init(void);	//PWN init declaration

int main(void)	//main function
{
	SystemCoreClockUpdate();	//check if clock freq is right
	if(SystemCoreClock == 48000000){
  PWM_Init();
	}
	else return -1;
  while (1)  
  {  
    __WFI();
  }        
}


__INLINE void PWM_Init(void)
{ 
	RGB_ValS RGB;	//color value struct declaration
	RGB.R_C = RED;
	RGB.B_C = GREEN;
	RGB.G_C = BLUE;
	
  RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;	//enable clock for timer1
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN;	//enable clock for gpioa
	
  GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODER8 | GPIO_MODER_MODER9
								 | GPIO_MODER_MODER10)) | GPIO_MODER_MODER8_1
								 | GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1;	// select alternate function mode on gpioa 8,9,10
  GPIOA->AFR[1] |= 0x2 << ((8-8)*4)
								 |(0x2 << ((9-8)*4))
								 |(0x2 << ((10-8)*4));	//AF2 on PA8,9,10 in AFRH for TIM1_CH1,2,3

  TIMx->PSC = PRES_VAL - 1;	//set prescaler to get 48MHz/PRES_VAL i.e 1kHz
  TIMx->ARR = ARR_VAL - 1;	//set auto-reload register to ARR_VAL i.e period is 20ms
	
  TIMx->CCR1 = RGB.R_C;
	TIMx->CCR2 = RGB.G_C;
	TIMx->CCR3 = RGB.B_C;	//set compere register of 3 channels i.e period of RGB colors on/off
	
	TIM1->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE; //select PWM mode 1 on channels 1,2,3,
	TIM1->CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2PE;	//channelx is active as long as CNT< CCRx, otherwise it is inactive,
	TIM1->CCMR2 |= TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3PE;	//enable preload register on them
	
	//TIMx->CCER  |= TIM_CCER_CC1P;	//1B select type of polarity on channels 1,2,3 i.e BR high, G low
	//TIMx->CCER  |= TIM_CCER_CC2P;	//2R
	TIMx->CCER  |= TIM_CCER_CC3P;	//3G
  TIMx->CCER  |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E;	//enable channels 1,2,3
	
	TIM1->DIER |= TIM_DIER_UIE;
	NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, 0);	//NVIC enable for uart
  NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
	
  TIMx->BDTR  |= TIM_BDTR_MOE;	//enable output
  TIMx->CR1   |= TIM_CR1_CEN;		//enable counter, direction upcounter - default, edge aligned mode - default
  TIMx->EGR   |= TIM_EGR_UG;		//force update generation
	
}

void TIM1_BRK_UP_TRG_COM_IRQHandler(void){
	tick++;
	if(tick == 50){
		 TIMx->CCR1 = 20;
		 TIMx->CCR2 = 20;
		 TIMx->CCR3 = 20;
	}
	if(tick == 100){
		 TIMx->CCR1 = 0;
		 TIMx->CCR2 = 0;
		 TIMx->CCR3 = 20;
		tick = 0;
	}
	TIM1->SR &= ~TIM_SR_UIF;
}
