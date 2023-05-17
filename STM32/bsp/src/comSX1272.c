/*
 * comSX1272.c
 *
 *  Created on: 24 août 2020
 *      Author: Arnaud
 */


#include "comSX1272.h"

static uint8_t BSP_SPI_SendReceive(uint8_t tx_byte);

void BSP_SPI1_Init()
{
	// SPI_SCK  -> PA5 (AF0)
	// SPI_MISO -> PA6 (AF0)
	// SPI_MOSI -> PA7 (AF0)
	// CS pin	-> PB6 (GPIO)

	/////////////////////////////////////////////////
	// Configure PB6 as CS pin
	//
	// Enable GPIOB clock
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

	// Configure PB6 as output
	GPIOB->MODER &= ~GPIO_MODER_MODER6_Msk;
	GPIOB->MODER |= (0x01 <<GPIO_MODER_MODER6_Pos);

	// Configure PB6 as Push-Pull output
	GPIOB->OTYPER &= ~GPIO_OTYPER_OT_6;

	// Configure PB6 as High-Speed Output
	GPIOB->OSPEEDR &= ~GPIO_OSPEEDR_OSPEEDR6_Msk;
	GPIOB->OSPEEDR |= (0x03 <<GPIO_OSPEEDR_OSPEEDR6_Pos);

	// Disable PB6 Pull-up/Pull-down
	GPIOB->PUPDR &= ~GPIO_PUPDR_PUPDR6_Msk;

	// Idle state is high
	GPIOB->BSRR = GPIO_BSRR_BS_6;

	///////////////////////////////////////////////////
	// Configure PA5, PA6, PA7 as SPI1 pins (AF0)
	//
	// Enable GPIOA clock
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

	// Configure PA5, PA6, PA7 as AF mode
	GPIOA->MODER &= ~(GPIO_MODER_MODER5 | GPIO_MODER_MODER6 | GPIO_MODER_MODER7);
	GPIOA->MODER |= (0x02 <<10U) |(0x02 <<12U) |(0x02 <<14U);

	// Connect to SPI1 (AF0)
	GPIOA->AFR[0] &= ~(0xFFF00000);
	GPIOA->AFR[0] |=  (0x00000000);

	// Enable SPI1 Clock
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

	// Configure SPI with default config
	// 4-wire = full-duplex (MISO/MOSI on separated lines)
	// 8-bit, no CRC, MSB first
	// CPHA = 0, CPOL = 0 -> SPI mode 0 (first clock rising edge)
	SPI1->CR1 = 0x0000;
	SPI1->CR2 = 0x0000;

	// Set the baudrate to 48MHz /128 = 375kHz (slow, but easy to debug)
	SPI1->CR1 |= 0x06 <<SPI_CR1_BR_Pos;

	// Set data size to 8-bit
	SPI1->CR2 |= 0x07 <<SPI_CR2_DS_Pos;

	// Set as master (SSI must be high), with software managed NSS
	SPI1->CR1 |= SPI_CR1_MSTR | SPI_CR1_SSI | SPI_CR1_SSM;

	// Enable SPI1
	SPI1->CR1 |= SPI_CR1_SPE;
}

uint8_t BSP_SX1272_Read(uint8_t register_address)
{
	uint8_t data=0;
	// Set FIFO threshold to 1-byte
	SPI1->CR2 |= SPI_CR2_FRXTH;

	// Select slave (CS -> low)
	GPIOB->BSRR = GPIO_BSRR_BR_6;

	// Send register address to read from
	BSP_SPI_SendReceive(register_address);

	data = BSP_SPI_SendReceive(0x00);

	// Release slave (CS -> High)
	GPIOB->BSRR = GPIO_BSRR_BS_6;

	return data;
}

void BSP_SX1272_Write(uint8_t register_address, uint8_t data)
{
	// Set FIFO threshold to 1-byte
	SPI1->CR2 |= SPI_CR2_FRXTH;

	// Select slave (CS -> low)
	GPIOB->BSRR = GPIO_BSRR_BR_6;

	// Send register address to write to
	BSP_SPI_SendReceive(register_address | 0x80);

	// Send data to write
	BSP_SPI_SendReceive(data);

	// Release slave (CS -> High)
	GPIOB->BSRR = GPIO_BSRR_BS_6;
}

static uint8_t BSP_SPI_SendReceive(uint8_t tx_byte)
{
	uint8_t	rx_byte;

	// Make sure TXE is set before sending data
	while((SPI1->SR & SPI_SR_TXE_Msk) == 0);

	// Send tx_byte
	*(__IO uint8_t *)&SPI1->DR = tx_byte;

	// Wait until incoming data has arrived
	while((SPI1->SR & SPI_SR_RXNE_Msk) == 0);

	// Read data
	rx_byte = *(__IO uint8_t *)&SPI1->DR;

	return rx_byte;
}
