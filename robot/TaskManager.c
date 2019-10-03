#include "TaskManager.h"
#include "utilities.h"
#include "stm32f303xe.h"
#include "SysTick.h"
#include "LCD.h"
#include "DCMotor.h"
#include "UART.h"
#define PRESCALER_TIM20 899
#define ARR_TIM20 7999

extern volatile Instruction current_instructions;
extern volatile uint32_t left_period_width;			//location to read the left encoder period width
extern volatile uint32_t right_period_width;		//location to read the right encoder period width
extern volatile uint8_t command_num;

void TaskManagerInit(void){
	//set up TIM20 to check or finish task periodically
	//*******************************************Configure TIM20*****************************//
	//Enable timer 20 clock
	SET_BITS(RCC->APB2ENR, RCC_APB2ENR_TIM20EN);
	
	//set pre-scalar - 72MHz/(899+1) = 80KHz
	TIM20->PSC = PRESCALER_TIM20;
	
	//set counter frequency. 80/KHz/(7999+1) = 10Hz
	TIM20->ARR = ARR_TIM20;
	
	//Select Channel 1 as output																													   
	CLR_BITS(TIM20->CCMR1, TIM_CCMR1_CC1S);  									
													
	//Select active mode. When TIM20_CNT = TIM20_CCR1, CC1IF is set high.
  FORCE_BITS(TIM20->CCMR1, TIM_CCMR1_OC1M, TIM_CCMR1_OC1M_0);																															

	//Duty cycle. May not be important here.
	TIM20->CCR1 = 500;
	
	//Main output enable
	TIM20->BDTR |= TIM_BDTR_MOE;
	
	//Select output polarity: 0 = active high, 1 = active low
	TIM20->CCER &= ~TIM_CCER_CC1P;
	
	//Enable output for channel 1 output
	TIM20->CCER |= TIM_CCER_CC1E;	 
	
	//Enable capture/compare 1 interrupt
	SET_BITS(TIM20->DIER, TIM_DIER_CC1IE);
	
	//Enable timer 20
	TIM20->CR1 |= TIM_CR1_CEN;
	
	/********************************Enable Interrupt****************************************/
	//Set the interrupt priority of TIM20_IRQn 
	NVIC_SetPriority(TIM20_CC_IRQn, 3);															
	
	//Enable the interrupt TIM20_IRQn
	NVIC_EnableIRQ(TIM20_CC_IRQn);																	
	
}
void TIM20_CC_IRQHandler(void){
	//This interrupt happens at 10 Hz
	if((TIM20->SR & TIM_SR_CC1IF) != 0){ //Check if the compare 1 interrupt flag is set
		static uint8_t count = 0;	
		static uint8_t previous_cmd_num = 33;
		static uint8_t error_check = 0;
		count++;
		if(count >= 2){
			if(error_check > 5){
					//stop movement
					current_instructions.DCM_Left_SPD = 0;
				  current_instructions.DCM_Right_SPD = 0;
					error_check = 0;
			}
			else{
				if(command_num <= previous_cmd_num)
					error_check++;
				else
					error_check=0;
			}
			
			
			
			
			count = 0;
		}
		CLR_BITS(TIM20->SR, TIM_SR_CC1IF);												//clear the flag
	}
}

int32_t period_width_to_speed(int8_t direction, int32_t current_period_ticks){
	int32_t vane_frequency = 720000/current_period_ticks;
	
	if(vane_frequency <= MIN_VANE_FREQ)
			vane_frequency = MIN_VANE_FREQ;
		if(vane_frequency >= MAX_VANE_FREQ)
			vane_frequency = MAX_VANE_FREQ;
		
	return direction*mapValue(MIN_VANE_FREQ, MAX_VANE_FREQ, 0, 100, vane_frequency);
}
