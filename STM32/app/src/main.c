/*
 * main.c
 *
 *  Created on: 23 ao�t 2020
 *      Author: Arnaud
 */

#include "main.h"
#include "stm32f0xx.h"
#include "bsp.h"
#include "comSX1272.h"
#include "SX1272.h"
#include "appSX1272.h"
#include "appSACS.h"

static void SystemClock_Config();

// Défini juste pour tester le calcul et le check du CRC :
static uint8_t payload[] = {0b10101010,0b00010001,0b00000000,0b00000000,0b00000000};
static uint8_t sizePayload = 5;
static uint8_t state = 0;

int main()
{
	uint32_t curtime=0;
	uint32_t i=0;
	frameSACS_s frame;
	frame.data[0]=0b01100110; //f
	frame.data[1]=0b01110010; //r
	frame.data[2]=0b01100001; //a
	frame.data[3]=0b01101101; //m
	frame.data[4]=0b01100010; //b
	frame.data[5]=0b01101111; //o
	frame.data[6]=0b01101001; //i
	frame.data[7]=0b01110011; //s
	frame.data[8]=0b01100101; //e
	frame.size_data = 9;
	frame.sid = 0;
	frame.ack = 1;
	uint8_t error = 0;

	// Initialize System clock to 48MHz from external clock
	SystemClock_Config();
	// Initialize timebase
	BSP_TIMER_Timebase_Init();
	// Initialize NVIC
	BSP_NVIC_Init();
	// Initialize SPI interface
	BSP_SPI1_Init();
	// Initialize Debug Console
	BSP_Console_Init();

	my_printf("Console ready!\r\n");

	///////////////////////////////////////////
	//setup SX1272
	APP_SX1272_setup();

	while(1)
	{
		APP_SACS_setCRC(payload, sizePayload);
		state = APP_SACS_checkCRC(payload, sizePayload);

		curtime=BSP_millis();

		if((curtime%1000)==0 && error==0)//send every 1000ms
		{
			error=APP_SACS_send(frame);
			i++;
		}else if(error!=0)
		{
			my_printf("Error\r\n");
		} else
		{
			//Default: nothing happens
		}
	}
}

/*
 * 	Clock configuration for the Nucleo STM32F072RB board
 * 	HSE input Bypass Mode 			-> 8MHz
 * 	SYSCLK, AHB, APB1 				-> 48MHz
 *  	PA8 as MCO with /16 prescaler 		-> 3MHz
 *
 *  Laurent Latorre - 05/08/2017
 */

static void SystemClock_Config()
{
	uint32_t	HSE_Status;
	uint32_t	PLL_Status;
	uint32_t	SW_Status;
	uint32_t	timeout = 0;

	timeout = 1000000;

	// Start HSE in Bypass Mode
	RCC->CR |= RCC_CR_HSEBYP;
	RCC->CR |= RCC_CR_HSEON;

	// Wait until HSE is ready
	do
	{
		HSE_Status = RCC->CR & RCC_CR_HSERDY_Msk;
		timeout--;
	} while ((HSE_Status == 0) && (timeout > 0));

	// Select HSE as PLL input source
	RCC->CFGR &= ~RCC_CFGR_PLLSRC_Msk;
	RCC->CFGR |= (0x02 <<RCC_CFGR_PLLSRC_Pos);

	// Set PLL PREDIV to /1
	RCC->CFGR2 = 0x00000000;

	// Set PLL MUL to x6
	RCC->CFGR &= ~RCC_CFGR_PLLMUL_Msk;
	RCC->CFGR |= (0x04 <<RCC_CFGR_PLLMUL_Pos);

	// Enable the main PLL
	RCC-> CR |= RCC_CR_PLLON;

	// Wait until PLL is ready
	do
	{
		PLL_Status = RCC->CR & RCC_CR_PLLRDY_Msk;
		timeout--;
	} while ((PLL_Status == 0) && (timeout > 0));

        // Set AHB prescaler to /1
	RCC->CFGR &= ~RCC_CFGR_HPRE_Msk;
	RCC->CFGR |= RCC_CFGR_HPRE_DIV1;

	//Set APB1 prescaler to /1
	RCC->CFGR &= ~RCC_CFGR_PPRE_Msk;
	RCC->CFGR |= RCC_CFGR_PPRE_DIV1;

	// Enable FLASH Prefetch Buffer and set Flash Latency
	FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY;

	/* --- Until this point, MCU was still clocked by HSI at 8MHz ---*/
	/* --- Switching to PLL at 48MHz Now!  Fasten your seat belt! ---*/

	// Select the main PLL as system clock source
	RCC->CFGR &= ~RCC_CFGR_SW;
	RCC->CFGR |= RCC_CFGR_SW_PLL;

	// Wait until PLL becomes main switch input
	do
	{
		SW_Status = (RCC->CFGR & RCC_CFGR_SWS_Msk);
		timeout--;
	} while ((SW_Status != RCC_CFGR_SWS_PLL) && (timeout > 0));

	/* --- Here we go! ---*/

	/*--- Use PA8 as MCO output at 48/16 = 3MHz ---*/

	// Set MCO source as SYSCLK (48MHz)
	RCC->CFGR &= ~RCC_CFGR_MCO_Msk;
	RCC->CFGR |=  RCC_CFGR_MCOSEL_SYSCLK;

	// Set MCO prescaler to /16 -> 3MHz
	RCC->CFGR &= ~RCC_CFGR_MCOPRE_Msk;
	RCC->CFGR |=  RCC_CFGR_MCOPRE_DIV16;

	// Enable GPIOA clock
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

	// Configure PA8 as Alternate function
	GPIOA->MODER &= ~GPIO_MODER_MODER8_Msk;
	GPIOA->MODER |= (0x02 <<GPIO_MODER_MODER8_Pos);

	// Set to AF0 (MCO output)
	GPIOA->AFR[1] &= ~(0x0000000F);
	GPIOA->AFR[1] |=  (0x00000000);

	// Update SystemCoreClock global variable
	SystemCoreClockUpdate();
}
