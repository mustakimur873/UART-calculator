/**
 * @file UART0.c
 *
 * @brief Source code for the UART0 driver.
 *
 * This file contains the function definitions for the UART0 driver.
 *
 * @note For more information regarding the UART module, refer to the
 * Universal Asynchronous Receivers / Transmitters (UARTs) section
 * of the TM4C123GH6PM Microcontroller Datasheet.
 *   - Link: https://www.ti.com/lit/gpn/TM4C123GH6PM
 *
 * @note Assumes that the frequency of the system clock is 50 MHz.
 *
 * @author
 */

#include "UART0.h"

void UART0_Init(void)
{
	// Enable the clock to the uart module by setting the
	// R0 bit (Bit 0) in the RCGCUART register
	SYSCTL->RCGCUART |= 0x01;
	
	// Enable the clock to Port A by setting the 
	// R0 bit (Bit 0) in the RCGCGPIO register
	SYSCTL->RCGCGPIO |= 0x01;
	
	// Disable the UART0 module before configuration by clearing
	// the UARTEN bit (Bit 0) in the CTL register
	UART0->CTL &= ~0x0001;
	
	// Configure the UART0 module to use the system clock (50 MHz)
	// divided by 16 by clearing the HSE bit (Bit 5) in the CTL register
	UART0->CTL &= ~0x0020;
	
	
	UART0->IBRD = 325;
	UART0->FBRD = 33;
	
	// Configure the data length of the UART packet to be 8 bits by writing
	// a value of 0x03
	// to the WLEN field (Bits 6 to 5) in the LCRH register/
	UART0->LCRH |= 0x60;
	
	// Enable the Transmit FIFO and the Receive FIFO by setting the FEN bit (bit 4)
	// in the LCRH register.
	UART0->LCRH |= 0x10;
	
	// Select one stop bit to be transmitted at the end of a UART frame by clearing the 
	// STP2 bit (Bit 3) in the LCRH register.
	UART0->LCRH &= ~0x08;
	
	// Disable the parity bit by clearing the PEN bit (Bit 1) in the LCRH register.
	UART0->LCRH &= ~0x02;
	
	// Enable the UART0 module after configuration by setting the UARTEN bit (Bit 0)
	// in the CTL register
	UART0->CTL |= 0x01;
	
	// Select the alternate function for the PA1 and PA0 pins by setting Bits 1 to 0
	// in the AFSEL register. The pins will be configures as TX and RX pins, respectively
	GPIOA->AFSEL |= 0x03;
	
	
	GPIOA->PCTL &= ~0x000000FF;
	
	
	GPIOA->PCTL |= 0x00000010;
	

	GPIOA->PCTL |= 0x00000001;
	
	
	GPIOA->DEN |= 0x03;
	
}

char UART0_Input_Character(void)
{	
	while((UART0->FR & UART0_RECEIVE_FIFO_EMPTY_BIT_MASK) != 0);
	
	return (char)(UART0->DR & 0xFF);
	
}

void UART0_Output_Character(char data)
{
	while ((UART0->FR & UART0_TRANSMIT_FIFO_FULL_BIT_MASK) != 0);
	
	UART0->DR = data;
}

void UART0_Input_String(char *buffer_pointer, uint16_t buffer_size) 
{
	int length = 0;
	char character = UART0_Input_Character();
	
	while(character != UART0_CR)
	{
		if(character == UART0_BS)
		{
			if (length)
			{
				buffer_pointer--;
				length--;
				UART0_Output_Character(UART0_BS);
			}
		}
		else if(length < buffer_size)
		{
			*buffer_pointer = character;
			buffer_pointer++;
			length++;
			UART0_Output_Character(character);
		}
		character = UART0_Input_Character();
	}
	*buffer_pointer = 0;
}

void UART0_Output_String(char *pt)
{
	while(*pt)
	{
		UART0_Output_Character(*pt);
		pt++;
	}
}

uint32_t UART0_Input_Unsigned_Decimal(void)
{
	uint32_t number = 0;
	uint32_t length = 0;
	char character = UART0_Input_Character();
	
	//
	//
	//
	while(character != UART0_CR)
	{
		if ((character >= '0') && (character <= '9'))
		{
		//
		number = (10 * number) + (character - '0');
		length++;
		UART0_Output_Character(character);
	}
	
	
	else if ((character == UART0_BS) && length)
	{
		number /= 10;
		length--;
		UART0_Output_Character(character);
	}
	
	character = UART0_Input_Character();
}
	return number;
}

void UART0_Output_Unsigned_Decimal(uint32_t n)
{

	if (n >= 10)
	{
		UART0_Output_Unsigned_Decimal(n / 10);
		n = n % 10;
	}
	

	UART0_Output_Character(n + '0');
}

uint32_t UART0_Input_Unsigned_Hexadecimal(void)
{
	uint32_t number = 0;
	uint32_t digit = 0;
	uint32_t length = 0;
	char character = UART0_Input_Character();
	
	while(character != UART0_CR)
	{
		digit = 0x10;
		
		if ((character >= '0') && (character <= '9'))
		{
			digit = character = '0';
		}
		else if ((character>='A') && (character <= 'F'))
		{
			digit = (character - 'A') + 0xA;
		}
			else if ((character>='a') && (character <= 'f'))
		{
			digit = (character - 'a') + 0xA;
		}
		
		if (digit <= 0xF)
		{
			number = (number * 0x10) + digit;
			length++;
			UART0_Output_Character(character);
		}
		
		else if((character == UART0_BS) && length)
		{
			number /= 0x10;
			length--;
			UART0_Output_Character(character);
		}
		character = UART0_Input_Character();
	}
	return number;
}

void UART0_Output_Unsigned_Hexadecimal(uint32_t number)
{
	if (number >= 0x10)
	{
		UART0_Output_Unsigned_Hexadecimal(number / 0x10);
		UART0_Output_Unsigned_Hexadecimal(number % 0x10);
	}
	else
	{
		if (number < 0xA)
		{
			UART0_Output_Character(number + '0');
		}
		else
		{
			UART0_Output_Character((number - 0x0A) + 'A');
		}
	}
}

void UART0_Output_Newline(void)
{
	UART0_Output_Character(UART0_CR);
	UART0_Output_Character(UART0_LF);
}