/******************************************************************************
*UART.c -- Inplementation of setting up UART function and send a string to the port
*By Weipeng Tang
*Jan 17, 2019
********************************************************************************/
#include "stm32f303xe.h"
#include "utilities.h"
#include "SysTick.h"
#include "UART.h"
#include "LCD.h"
#include "StepperMotor.h"
#include "Servo.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

extern volatile Instruction current_instructions;
static uint8_t USART1_Buffer_Rx[MAX_UART_BUFSIZ];
static volatile uint32_t Rx1_Counter = 0;
volatile uint8_t input_frame[INPUT_FRAME_SIZE]; //1st byte: 1 - special command  0 - data
																 //2nd byte: type of data
																 //					 1 - stepper motor target angle
																 //					 2 - stepper motor action speed
																 //					 3 - Left DC motor direction
																 //					 4 - Left DC motor speed
																 //					 5 - right DC motor direction
																 //					 6 - right DC motor speed
																 //					 7 - servo target angle
																 //          type of command
																 //					 1 - stepper motor homing
																 //					 2 - rotate LCD display content
																 //3th, 4th, 5th, 6th byte: real command parameters or data

void UARTInit(void) {
	
// *******************************************
// *** turn on clocks for USART 1 - 72 MHz ***
// *******************************************
	
	// Enable the clock of USART 1
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;  // Enable USART 1 clock. Reference manual page 151	
		// Select the USART1 clock source
	// 00: PCLK selected as USART2 clock
	// 01: System clock (SYSCLK) selected as USART2 clock
	// 10: HSI16 clock selected as USART2 clock
	// 11: LSE clock selected as USART2 clock

	CLR_BITS(RCC->CFGR3, RCC_CFGR3_USART1SW);
// ***************************************************************************************	
// *** configure GPIO PC4, PC5, PA11, & PA12	
// ***************************************************************************************
//Pin assignment
//PC4 - USART1_TX(AF7) - RS232_TxD
//PC5 - USART1_RX(AF7) - RS232_RxD
//PA11 - USART1_CTS(AF7) - RS232_CTS
//PA12 - USART1_RTS(AF7) -RS232_RTS
// Alternate function, High Speed, Push pull, Pull up
// **********************************************************	
		
// Enable the peripheral clock of GPIO Port C and A 
	EN_CLK_PORT(RS232_TXD_PORT);
	EN_CLK_PORT(RS232_RXD_PORT);
	EN_CLK_PORT(RS232_CTS_PORT);
	EN_CLK_PORT(RS232_RTS_PORT);
	
// Input(00), Output(01), AlterFunc(10), Analog(11)

//Set all the pins as alternate function
	GPIO_PIN_MODE(RS232_TXD_PORT, RS232_TXD_PIN, AF);
	GPIO_PIN_MODE(RS232_RXD_PORT, RS232_RXD_PIN, AF);
	GPIO_PIN_MODE(RS232_CTS_PORT, RS232_CTS_PIN, AF);
	GPIO_PIN_MODE(RS232_RTS_PORT, RS232_RTS_PIN, AF);

//Select the needed alternate functions
	FORCE_BITS(GPIO(RS232_TXD_PORT)->AFR[0], GPIO_AFRL_AFRL4, (7UL<<(4*4)));
	FORCE_BITS(GPIO(RS232_RXD_PORT)->AFR[0], GPIO_AFRL_AFRL5, (7UL<<(4*5)));
	FORCE_BITS(GPIO(RS232_CTS_PORT)->AFR[1], GPIO_AFRH_AFRH3, (7UL<<(4*3)));
	FORCE_BITS(GPIO(RS232_RTS_PORT)->AFR[1], GPIO_AFRH_AFRH4, (7UL<<(4*4)));
	
// GPIO Speed: Low speed (00), Medium speed (01), Fast speed (10), High speed (11)

	SET_BITS(GPIO(RS232_TXD_PORT)->OSPEEDR, GPIO_OSPEEDER_OSPEEDR4);
	SET_BITS(GPIO(RS232_RXD_PORT)->OSPEEDR, GPIO_OSPEEDER_OSPEEDR5);
	SET_BITS(GPIO(RS232_CTS_PORT)->OSPEEDR, GPIO_OSPEEDER_OSPEEDR11);
	SET_BITS(GPIO(RS232_RTS_PORT)->OSPEEDR, GPIO_OSPEEDER_OSPEEDR12);

// GPIO Push-Pull: No pull-up, pull-down (00), Pull-up (01), Pull-down (10), Reserved (11)

	GPIO_PIN_PP_TYPE(RS232_TXD_PORT, RS232_TXD_PIN, PU);
	GPIO_PIN_PP_TYPE(RS232_RXD_PORT, RS232_RXD_PIN, PU);
	GPIO_PIN_PP_TYPE(RS232_CTS_PORT, RS232_CTS_PIN, PU);
	GPIO_PIN_PP_TYPE(RS232_RTS_PORT, RS232_RTS_PIN, PU);


// GPIO Output Type: Output push-pull (0, reset), Output open drain (1) 

	GPIO_PIN_DRV_TYPE(RS232_TXD_PORT, RS232_TXD_PIN, PP);
	GPIO_PIN_DRV_TYPE(RS232_RXD_PORT, RS232_RXD_PIN, PP);
	GPIO_PIN_DRV_TYPE(RS232_CTS_PORT, RS232_CTS_PIN, PP);
	GPIO_PIN_DRV_TYPE(RS232_RTS_PORT, RS232_RTS_PIN, PP);	

// ***************************************************************************************	
// *** Configure USART1 as:
// ***      No hardware flow control, 8 data bits, no parity, 1 start bit and 1 stop bit
// ***************************************************************************************	
	
	USART1->CR1 &= ~USART_CR1_UE;  // Disable USART
	
	// Configure word length to 8 bit
	USART1->CR1 &= ~USART_CR1_M;   // M: 00 = 8 data bits, 01 = 9 data bits, 10 = 7 data bits
	
	// Configure oversampling mode: Oversampling by 16 
	USART1->CR1 &= ~USART_CR1_OVER8;  // 0 = oversampling by 16, 1 = oversampling by 8
	
	// Configure stop bits to 1 stop bit
	//   00: 1 Stop bit;      01: 0.5 Stop bit
	//   10: 2 Stop bits;     11: 1.5 Stop bit
	USART1->CR2 &= ~USART_CR2_STOP; 
	//FORCE_BITS(USART1->CR2, USART_CR2_STOP, USART_CR2_STOP_1);
                                    
	// CSet Baudrate to 9600 using APB frequency (72,000,000 Hz)
	// If oversampling by 16, Tx/Rx baud = f_CK / USARTDIV,  
	// If oversampling by 8,  Tx/Rx baud = 2*f_CK / USARTDIV
  // When OVER8 = 0, BRR = USARTDIV
	// USARTDIV = 72MHz/9600 = 7500 = 0x1D4C
	//USART1->BRR  = 6923; // Limited to 16 bits. For unknown reason, the number needs to be adjusted to have the correct baud rate. 7500*6/6.5 = 6923.
	
	//The clock frequency being used here is about 68MHz(from the reaction of the servo). 
	//68MHz/9600 = 7083. RS232 can work at this rate.
	//75MHz/9600 = 7500.
  //USART1->BRR  = 7083;
	USART1->BRR  = 7500;
	USART1->CR1  |= (USART_CR1_RE | USART_CR1_TE);  	// Transmitter and Receiver enable
	
	USART1->CR1  |= USART_CR1_UE; // USART enable                 
	
	while ( (USART1->ISR & USART_ISR_TEACK) == 0); // Verify that the USART is ready for reception
	while ( (USART1->ISR & USART_ISR_REACK) == 0); // Verify that the USART is ready for transmission

//***********************************************************************************************
//	Configure USART recevie data with interrupt
//***********************************************************************************************
	SET_BITS(USART1->CR1, USART_CR1_RXNEIE);      //Enable Receive interrupt 
	
	CLR_BITS(USART1->CR1, USART_CR1_TXEIE);				//Diable transmit interrupt(because all USART interrupt will go to the same ISR)
	
	NVIC_SetPriority(USART1_IRQn, 2);							//set the highest urgency
	
	NVIC_EnableIRQ(USART1_IRQn);									//Enable NVIC interrupt
}


void UARTprintf(char *fmt, ...){										//UARTprintf() that uses similar format of parameters as a standard printf()
	va_list args;
	
	char buffer[MAX_UART_BUFSIZ];
	
	va_start(args, fmt);
	vsnprintf(buffer, MAX_UART_BUFSIZ, fmt, args);
	va_end(args);
	
	UARTputs(buffer);
}


void UARTputs(char *str){
	while(*str){
		UARTputc(*str++);
	}
	str--;
	if(*str == '\n')
		UARTputc(0x0D);
}

void UARTputc(char cx){
	 while(!(USART1->ISR & USART_ISR_TXE));
	USART1->TDR = (uint8_t)cx;
}

void USART1_IRQHandler(void){
	
	//Recive raw message
	receive(USART1, USART1_Buffer_Rx, &Rx1_Counter);//receive a 8-bit character

	if(UARTCheckEnter()){//update instructions
		update_instruction();
	}
}

void receive(USART_TypeDef *USARTx, uint8_t *buffer, volatile uint32_t *pCounter){
	if(USARTx->ISR & USART_ISR_RXNE) {  		//Check RXNE event
		
		if(USARTx->RDR == 0x0D){							//if a carriage return is found
			buffer[*pCounter] = 0x0A;						//replace the carriage return with a newline character
			(*pCounter)++;											//increment counter
		}
		else{																	//for normal cases
			buffer[*pCounter] = USARTx->RDR;		//Reading RDR clears the RXNE flag
			(*pCounter)++;											//Dereference and update memory value
		}
		if((*pCounter) >= MAX_UART_BUFSIZ){		//Check buffer overflow
			(*pCounter) = 0;										//Circular buffer
		}
	}
}

void update_instruction(void){
		char temp_buffer[50];
		UARTString(temp_buffer);
		for(uint8_t i=0; i<6; i++){
			UARTprintf("%d: %c ||", i, temp_buffer[i]);
			temp_buffer[i] -= 48;
		}
	
		switch(temp_buffer[0]){		
			case 0: //data
				switch(temp_buffer[1]){			
					case 1: //stepper motor target angle
						current_instructions.stepper_target = (uint32_t)(framer_32bit(temp_buffer));
						break;
					case 2: //stepper motor action speed
						current_instructions.stepper_speed = (uint8_t)(framer_8bit(temp_buffer));
						break;
					case 3: //left DC motor direction
						current_instructions.DCM_Left_DIR = (int8_t)(framer_8bit(temp_buffer));
						break;
					case 4: //left DC motor speed
						current_instructions.DCM_Left_SPD = (uint32_t)(framer_32bit(temp_buffer));
						break;
					case 5: //right DC motor direction
						current_instructions.DCM_Right_DIR = (int8_t)(framer_8bit(temp_buffer));
						break;
					case 6:	//right DC motor speed
						current_instructions.DCM_Right_SPD = (uint32_t)(framer_32bit(temp_buffer));
						break;
					case 7: //servo target angle
						servoPosition((uint32_t)(framer_32bit(temp_buffer)));
						break;
				}
				break;
			case 1: //special command
				switch((int)(temp_buffer[1])){				
					case 1: //stepper motor homming
						stepperHoming();
						break;
					case 2: //increment LCD content index
						//LCD content list:
						//1 - DC motor target speed
						//2 - DC motor current speed
						//3 - ultrasonic distance sensor 1
					  //4 - ultrasonic distance sensor 2
					  //5 - stepper motor angle
						//6 - servo angle
						if(current_instructions.LCD_index == 6)
							current_instructions.LCD_index = 1;
						else
							current_instructions.LCD_index++;
						break;
				}
				break;
			default:
				break;
		}
		Rx1_Counter = 0; //clear receive buffer
		UARTprintf("Confirmed.\n");
}
uint32_t framer_32bit(char *buffer){
	uint32_t temp = (uint32_t)(((buffer[2]<<24)|(buffer[3]<<16)|(buffer[4]<<8)|buffer[5])&0xFFFF);
	return (temp);
}
uint16_t framer_16bit(char *buffer){
	return (uint16_t)(((buffer[2]<<24)|(buffer[3]<<16)|(buffer[4]<<8)|buffer[5])&0x00FF);
}
uint8_t framer_8bit(char *buffer){
	return (uint8_t)(((buffer[2]<<24)|(buffer[3]<<16)|(buffer[4]<<8)|buffer[5])&0x000F);
}

int8_t UARTDequeue(void){
	uint8_t temp;
	uint8_t i;
	if(Rx1_Counter != 0){
		temp = USART1_Buffer_Rx[0];													//pop the first item out from the buffer
		for(i = 0; i < Rx1_Counter; i++){										//shift the buffer one bit to the left
			USART1_Buffer_Rx[i] = USART1_Buffer_Rx[i+1]; 
		}
		Rx1_Counter--;																			//decrement counter
		return (temp);																			//return the item
	}
	else
		return (-1);
	
}
int8_t UARTNotEmpty(void){
	if(Rx1_Counter != 0)					//check if the buffer is empty. If not, return 1, or return -1
		return 1;
	else
		return -1;
	
}
int8_t UARTCheckEnter(void){
	if(USART1_Buffer_Rx[Rx1_Counter-1]==0x0A)				//check if the last character is a newline character. 
		return 1;
	else
		return -1;
}
void UARTString(char *cx){
	
		while(UARTNotEmpty()==1){
			(*cx) = UARTDequeue();
			cx++;
		}
		(*cx) = '\0';
}
void UART_receive_frame(char *buffer){
	uint8_t i;
	for(i=0; i<6; i++){
		buffer[i] = UARTDequeue();
	}
	
}