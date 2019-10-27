/**
  ******************************************************************************
  * @file    BSP_EEPROM.h
  * @author  Hossein Bagherzade(@realhba)
	* @email	 hossein.bagherzade@gmail.com
  * @version V1.1.1
  * @date    08-March-2018
  * @brief   Header file for BSP_EEPROM.c
  ******************************************************************************
	**/
	

#ifndef __BSP_EEPROM_H
#define __BSP_EEPROM_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "debugprobe.h"

	
#define EEP_DEBUG
	
#ifdef EEP_DEBUG
#include "stdio.h"	
#define EEP_LOG(...) 	aPrintOutLog( __VA_ARGS__) //Send Data on Stream...
#else
#define EEP_LOG(...)  
#endif	

#define USE_SOFTWARE_SPI								 (1)
	
#define EEP_SPI_CS_GPIO_CLK_ENABLE()   	 __HAL_RCC_GPIOA_CLK_ENABLE()
#define EEP_SPI_CS_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOA_CLK_DISABLE()
#define EEP_SPI_SCK_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOA_CLK_ENABLE()
#define EEP_SPI_SCK_GPIO_CLK_DISABLE()   __HAL_RCC_GPIOA_CLK_DISABLE()
#define EEP_SPI_CS_LOW()      					 HAL_GPIO_WritePin(EEP_CS_GPIO_Port, EEP_CS_Pin, GPIO_PIN_RESET)
#define EEP_SPI_CS_HIGH()      					 HAL_GPIO_WritePin(EEP_CS_GPIO_Port, EEP_CS_Pin, GPIO_PIN_SET)
#define EEP_SPI_SI_LOW()      					 HAL_GPIO_WritePin(EEP_MOSI_GPIO_Port, EEP_MOSI_Pin, GPIO_PIN_RESET)
#define EEP_SPI_SI_HIGH()      					 HAL_GPIO_WritePin(EEP_MOSI_GPIO_Port, EEP_MOSI_Pin, GPIO_PIN_SET)
#define EEP_SPI_CK_LOW()      					 HAL_GPIO_WritePin(EEP_CLK_GPIO_Port, EEP_CLK_Pin, GPIO_PIN_RESET)
#define EEP_SPI_CK_HIGH()      					 HAL_GPIO_WritePin(EEP_CLK_GPIO_Port, EEP_CLK_Pin, GPIO_PIN_SET)
#define EEP_SPI_SO_IS_HIGH()      			 (HAL_GPIO_ReadPin(EEP_MISO_GPIO_Port, EEP_MISO_Pin) == GPIO_PIN_SET)

#define EEPROM_SPI_FLAG_TIMEOUT          ((uint32_t) 200)			                               
#define WRITE_TIMEOUT_MS  			 				 (uint8_t)20 		// a write should only ever take 5 ms max

#define EEP_CLK_DELAY(x)						 		 for(int i = 0 ; i < (50 * x) ; i++) __NOP()
#define EEP_SEQ_DELAY(x)						 		 for(int i = 0 ; i < (20 * x) ; i++) __NOP()

#define	EEP_SPI_PAGESIZE								 (uint8_t)32
#define CMD_WRSR  							 				 (uint8_t)0x01  // write status register
#define CMD_WRITE 							 				 (uint8_t)0x02  // write to EEPROM
#define CMD_READ  							 				 (uint8_t)0x03  // read from EEPROM
#define CMD_WRDI  							 				 (uint8_t)0x04  // write disable
#define CMD_RDSR  							 				 (uint8_t)0x05  // read status register
#define CMD_WREN  							 				 (uint8_t)0x06  // write enable
											                   
#define BIT_WIP   							 				 (uint8_t)0	 		// write in progress
#define BIT_WEL   							 				 (uint8_t)1	 		// write enable latch
#define BIT_BP0   							 				 (uint8_t)2	 		// block protect 0
#define BIT_BP1   							 				 (uint8_t)3	 		// block protect 1
#define BIT_SRWD  							 				 (uint8_t)7	 		// status register write disable

#define bitRead(value, bit) 						 (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) 							 ((value) |= (1UL << (bit)))
#define bitClear(value, bit) 						 ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) 	 (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

#define EEP_RW_Delay										 (uint8_t)5


#if (USE_SOFTWARE_SPI == 0)
#define EEPROM_SPI_Init 								EEPROM_HardSPI_Init
#define EEPROM_SPI_SendByte 						EEPROM_HardSPI_SendByte
#define EEPROM_SPI_RecvByte 						EEPROM_HardSPI_RecvByte
#define EEPROM_SPI_WriteByte 						EEPROM_HardSPI_WriteByte
#define EEPROM_SPI_ReadByte 						EEPROM_HardSPI_ReadByte
#define EEPROM_SPI_IsReady 							EEPROM_HardSPI_IsReady
#define EEPROM_SPI_WriteStatusRegister 	EEPROM_HardSPI_WriteStatusRegister
#define EEPROM_SPI_ReadBuffer 					EEPROM_HardSPI_ReadBuffer
#define EEPROM_SPI_WritePage 						EEPROM_HardSPI_WritePage
#define EEPROM_SPI_WriteBuffer 					EEPROM_HardSPI_WriteBuffer
#elif (USE_SOFTWARE_SPI == 1)
#define EEPROM_SPI_Init 								EEPROM_SoftSPI_Init
#define EEPROM_SPI_SendByte 						EEPROM_SoftSPI_SendByte
#define EEPROM_SPI_RecvByte 						EEPROM_SoftSPI_RecvByte
#define EEPROM_SPI_WriteByte 						EEPROM_SoftSPI_WriteByte
#define EEPROM_SPI_ReadByte 						EEPROM_SoftSPI_ReadByte
#define EEPROM_SPI_IsReady 							EEPROM_SoftSPI_IsReady
#define EEPROM_SPI_WriteStatusRegister 	EEPROM_SoftSPI_WriteStatusRegister
#define EEPROM_SPI_ReadBuffer 					EEPROM_SoftSPI_ReadBuffer
#define EEPROM_SPI_WritePage 						EEPROM_SoftSPI_WritePage
#define EEPROM_SPI_WriteBuffer 					EEPROM_SoftSPI_WriteBuffer
#else
#error Wrong SPI Configuration
#endif


extern SPI_HandleTypeDef hspi1;	

HAL_StatusTypeDef EEPROM_HardSPI_Init(void);
HAL_StatusTypeDef EEPROM_SoftSPI_Init(void);

HAL_StatusTypeDef EEPROM_SPI_MultipleReadWriteTest(uint8_t eraseFlag);
HAL_StatusTypeDef EEPROM_SPI_SingleReadWriteTest(uint8_t eraseFlag);

uint8_t BSP_EEPROM_IsConnected(void);
HAL_StatusTypeDef BSP_EEPROM_Write(uint16_t reg_address, uint8_t data_buf[], uint16_t length);
HAL_StatusTypeDef BSP_EEPROM_Read(uint16_t reg_address, uint8_t data_buf[], uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_EEPROM_H */
	
