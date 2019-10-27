/**
  ******************************************************************************
  * @file    BSP_EEPROM.c
  * @author  Hossein Bagherzade(@realhba)
	* @email	 hossein.bagherzade@gmail.com
  * @version V1.1.1
  * @date    08-March-2018
  * @brief   This file provides functions for running AT25160 EEPROM in SPI mode
  ******************************************************************************
	**/	
	
#include "BSP_EEPROM.h"	

//=======================================================================================
//====================== Functions for Hardware based SPI ===============================
//=======================================================================================
#if (USE_SOFTWARE_SPI == 0)

/**
  * @brief  spi low level function for initalizing interface
	* @retval HAL_StatusTypeDef enum, HAL_OK in case of successful operation
  */
//==============================================
HAL_StatusTypeDef EEPROM_HardSPI_Init(void)
//==============================================
{
	//Hard SPI should be inited in the main.
	return HAL_OK;
}

/**
  * @brief  spi low level function for sending single byte
  * @param  value: data value for write
	* @retval none
  */
//========================================================================================
__STATIC_INLINE HAL_StatusTypeDef EEPROM_HardSPI_SendByte(uint8_t* pByte, uint8_t ucSize)
//========================================================================================
{
	uint32_t uwTimeout = BSP_GetTick();
	
  while ((!__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_TXE)) && (BSP_GetTick() - uwTimeout < WRITE_TIMEOUT_MS)); //BSP_Delay(1, NONE_BLOCKING);
	if(BSP_GetTick() - uwTimeout >= 50) return HAL_ERROR;

  return HAL_SPI_Transmit(&hspi1, (uint8_t*)pByte, (uint16_t)ucSize, EEPROM_SPI_FLAG_TIMEOUT);
}

/**
  * @brief  spi low level function for receiving single byte
	* @retval received data
  */
//========================================================================================
__STATIC_INLINE HAL_StatusTypeDef EEPROM_HardSPI_RecvByte(uint8_t* pByte, uint8_t ucSize)
//========================================================================================
{
	uint32_t uwTimeout = BSP_GetTick();
	
  while ((!__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_RXNE)) && (BSP_GetTick() - uwTimeout < WRITE_TIMEOUT_MS)); //BSP_Delay(1, NONE_BLOCKING);
	if(BSP_GetTick() - uwTimeout >= 50) return HAL_ERROR;

  return HAL_SPI_Receive(&hspi1, (uint8_t*)pByte, (uint16_t)ucSize, EEPROM_SPI_FLAG_TIMEOUT);
}

/**
  * @brief  it checks if spi interface is not busy
	* @retval HAL_StatusTypeDef enum, HAL_OK in case of successful operation
  */
//===========================================================
HAL_StatusTypeDef EEPROM_HardSPI_IsReady(void)
//===========================================================	
{
	HAL_StatusTypeDef E2PStatus = HAL_ERROR;
  uint8_t sEEstatus[1] = { 0x00 };
  uint8_t command[1] = { CMD_RDSR };

  // Select the EEPROM: Chip Select low
  EEP_SPI_CS_LOW();

  // Send "Read Status Register" Instruction and Wait to Receive
  if(EEPROM_HardSPI_SendByte(command, 1) == HAL_OK){	
		for(uint8_t uCount = 0; uCount < 5; uCount++){
			if(HAL_SPI_Receive(&hspi1, (uint8_t*)sEEstatus, 1, 100) == HAL_OK){
				if((sEEstatus[0] & 0x01) == 0){
					E2PStatus = HAL_OK;
					break;
				}
			}
			BSP_Delay(1, NONE_BLOCKING);
		}
	}
			
	// Deselect the EEPROM: Chip Select high
	EEP_SPI_CS_HIGH();
  return E2PStatus;
}

/**
  * @brief  stores one byte to eeprom
  * @param  RegAdd: eeprom register address
  * @param  RegData: data for write
	* @retval HAL_StatusTypeDef enum, HAL_OK in case of successful operation
  */
//================================================================================
HAL_StatusTypeDef EEPROM_HardSPI_WriteByte(uint16_t RegAdd, uint8_t RegData)	
//=================================================================================
{
	HAL_StatusTypeDef E2PStatus = HAL_ERROR;
	uint32_t tw = BSP_GetTick();
	uint8_t ucByte;

	EEP_SPI_CS_LOW();
	ucByte = CMD_RDSR;
	E2PStatus = EEPROM_HardSPI_SendByte(&ucByte, 1);
	
	do
	{
		ucByte = 0;
		E2PStatus = EEPROM_HardSPI_RecvByte(&ucByte, 1);
		
	} while((bitRead(ucByte, BIT_WIP) == 1) && (BSP_GetTick() < tw + WRITE_TIMEOUT_MS));
	
	 EEP_SPI_CS_HIGH();

	if(bitRead(ucByte, BIT_WIP) == 1) return HAL_ERROR;

	EEP_SPI_CS_LOW();
	ucByte = CMD_WREN;
	E2PStatus = EEPROM_HardSPI_SendByte(&ucByte, 1);
	EEP_SPI_CS_HIGH();
	
	EEP_SPI_CS_LOW();
	uint8_t buf4[4];
	buf4[0]=CMD_WRITE;
	buf4[1]=(uint8_t)(RegAdd >> 8) & 0xFF;
	buf4[2]=(uint8_t)(RegAdd & 0xFF);
	buf4[3]=RegData;
	E2PStatus = EEPROM_HardSPI_SendByte(buf4, 4);
	EEP_SPI_CS_HIGH();

	return E2PStatus;
}

/**
  * @brief  receives one byte from eeprom
  * @param  RegAdd: eeprom register address
  * @param  pData: pointer to read variable
	* @retval HAL_StatusTypeDef enum, HAL_OK in case of successful operation
  */
//================================================================================
HAL_StatusTypeDef EEPROM_HardSPI_ReadByte(uint16_t ReadAddr, uint8_t* pData)	
//=================================================================================
{
  uint8_t header[3];
	HAL_StatusTypeDef E2PStatus = HAL_ERROR;
	
  header[0] = CMD_READ;    				 // Send "Read from Memory" instruction
  header[1] = ReadAddr >> 8;  		 // Send 16-bit address
  header[2] = ReadAddr;

  // Select the EEPROM: Chip Select low
  EEP_SPI_CS_LOW();

	// Send WriteAddr address byte to read from and Wait to Receive
  if(EEPROM_HardSPI_SendByte(header, 3) == HAL_OK){	
		for(uint8_t uCount = 0; uCount < 5; uCount++){
			if(EEPROM_HardSPI_RecvByte(pData, 1) == HAL_OK){
					E2PStatus = HAL_OK;
					break;
			}
			BSP_Delay(1, NONE_BLOCKING);
		}
	}

  // Deselect the EEPROM: Chip Select high
  EEP_SPI_CS_HIGH();

  return E2PStatus;	
}

/**
  * @brief  changes the value of status register of eeprom
  * @param  regval: value of data
	* @retval HAL_StatusTypeDef enum, HAL_OK in case of successful operation
  */
//=========================================================================
HAL_StatusTypeDef EEPROM_HardSPI_WriteStatusRegister(uint8_t regval)
//=========================================================================	
{
	HAL_StatusTypeDef E2PStatus = HAL_ERROR;
  uint8_t command[2];

  command[0] = CMD_WRSR;
  command[1] = regval;

  // Enable the write access to the EEPROM
  if(EEPROM_HardSPI_WriteByte(CMD_WREN, 1) == HAL_OK)
	{
		// Select the EEPROM: Chip Select low
		EEP_SPI_CS_LOW();
	
		// Send "Write Status Register" instruction and Regval in one packet
		E2PStatus = EEPROM_HardSPI_SendByte(command, 2);
		
		// Deselect the EEPROM: Chip Select high
		EEP_SPI_CS_HIGH();
	}
	if(E2PStatus != HAL_OK) return E2PStatus;
	
  E2PStatus = EEPROM_HardSPI_WriteByte(CMD_WRDI, 1);
	return E2PStatus;
}

/**
  * @brief  reads number of bytes from eeprom 
  * @param  pBuffer: pointer to the data for read out
  * @param  WriteAddr: read address of eeprom (16bit address space)
  * @param  NumByteToRead: number of bytes for read  
	* @retval HAL_StatusTypeDef enum, HAL_OK in case of successful operation
  */
//==============================================================================================================
HAL_StatusTypeDef EEPROM_HardSPI_ReadBuffer(uint8_t* pBuffer, uint16_t ReadAddr, uint16_t NumByteToRead)
//==============================================================================================================
{
  uint8_t header[3];
	HAL_StatusTypeDef E2PStatus = HAL_ERROR;
	
  header[0] = CMD_READ;    				 // Send "Read from Memory" instruction
  header[1] = ReadAddr >> 8;  		 // Send 16-bit address
  header[2] = ReadAddr;

  // Select the EEPROM: Chip Select low
  EEP_SPI_CS_LOW();

	// Send WriteAddr address byte to read from and Wait to Receive
  if(EEPROM_HardSPI_SendByte(header, 3) == HAL_OK){	
		for(uint8_t uCount = 0; uCount < 5; uCount++){
			if(EEPROM_HardSPI_RecvByte(pBuffer, 1) == HAL_OK){
					E2PStatus = HAL_OK;
					break;
			}
			BSP_Delay(1, NONE_BLOCKING);
		}
	}

  // Deselect the EEPROM: Chip Select high
  EEP_SPI_CS_HIGH();

  return E2PStatus;
}

/**
  * @brief  writes number of bytes in case they are page aligned 
  * @param  pBuffer: pointer to the data for write in
  * @param  WriteAddr: write address of eeprom (16bit address space)
  * @param  NumByteToWrite: number of bytes for write  
	* @retval HAL_StatusTypeDef enum, HAL_OK in case of successful operation
  */
//==============================================================================================================
HAL_StatusTypeDef EEPROM_HardSPI_WritePage(uint8_t* pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite)
//==============================================================================================================	
{
	HAL_StatusTypeDef E2PStatus = HAL_ERROR;
	uint32_t uwTimeout = BSP_GetTick();
  uint8_t header[3];
		
  // Enable the write access to the EEPROM
  if(EEPROM_HardSPI_WriteByte(CMD_WREN, 1) == HAL_OK)
	{
    header[0] = CMD_WRITE;   				 // Send "Write to Memory" instruction
    header[1] = WriteAddr >> 8; 		 // Send 16-bit address
    header[2] = WriteAddr;

    // Select the EEPROM: Chip Select low
    EEP_SPI_CS_LOW();

    EEPROM_HardSPI_SendByte((uint8_t*)header, 3);

    // Make 5 attemtps to write the data
    for (uint8_t uCount = 0; uCount < 5; uCount++) {
       if(EEPROM_HardSPI_SendByte(pBuffer, NumByteToWrite) == HAL_OK){
					E2PStatus = HAL_OK;
					break;
			}
			BSP_Delay(5, NONE_BLOCKING);
    }

    // Deselect the EEPROM: Chip Select high
    EEP_SPI_CS_HIGH();

    // Wait the end of EEPROM writing
    while((EEPROM_HardSPI_IsReady() != HAL_OK) && (BSP_GetTick() - uwTimeout < 50)) BSP_Delay(5, NONE_BLOCKING);
		if(BSP_GetTick() - uwTimeout >= 50) return E2PStatus;
	}
	
	if(E2PStatus != HAL_OK) return E2PStatus;
	
  E2PStatus = EEPROM_HardSPI_WriteByte(CMD_WRDI, 1);
	return E2PStatus;
}

/**
  * @brief  writes any number of data to eeprom in PageWrite mode even if the buffer is not page aligned
  * @param  pBuffer: pointer to the data for write in
  * @param  WriteAddr: write address of eeprom (16bit address space)
  * @param  NumByteToWrite: number of bytes for write  
	* @retval HAL_StatusTypeDef enum, HAL_OK in case of successful operation
  */
//================================================================================================================
HAL_StatusTypeDef EEPROM_HardSPI_WriteBuffer(uint8_t* pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite)
//=================================================================================================================	
{
	HAL_StatusTypeDef E2PStatus = HAL_ERROR;
  uint16_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
  uint16_t sEE_DataNum = 0;

  Addr = WriteAddr % EEP_SPI_PAGESIZE;
  count = EEP_SPI_PAGESIZE - Addr;
  NumOfPage =  NumByteToWrite / EEP_SPI_PAGESIZE;
  NumOfSingle = NumByteToWrite % EEP_SPI_PAGESIZE;

  if(Addr == 0)
	{ // WriteAddr is EEPROM_PAGESIZE aligned 
      if(NumOfPage == 0)
			{ // NumByteToWrite < EEPROM_PAGESIZE
          sEE_DataNum = NumByteToWrite;
          E2PStatus = EEPROM_HardSPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);
          if (E2PStatus != HAL_OK) return E2PStatus;

      } 
			else
			{ // NumByteToWrite > EEPROM_PAGESIZE
          while (NumOfPage--)
					{
              sEE_DataNum = EEP_SPI_PAGESIZE;
              E2PStatus = EEPROM_HardSPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);
              if (E2PStatus != HAL_OK) return E2PStatus;

              WriteAddr +=  EEP_SPI_PAGESIZE;
              pBuffer += EEP_SPI_PAGESIZE;
          }

          sEE_DataNum = NumOfSingle;
          E2PStatus = EEPROM_HardSPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);

          if (E2PStatus != HAL_OK) return E2PStatus;
      }
  }
	else
	{ // WriteAddr is not EEPROM_PAGESIZE aligned
      if (NumOfPage == 0)
			{ // NumByteToWrite < EEPROM_PAGESIZE
          if (NumOfSingle > count)
					{ // (NumByteToWrite + WriteAddr) > EEPROM_PAGESIZE
              temp = NumOfSingle - count;
              sEE_DataNum = count;
              E2PStatus = EEPROM_HardSPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);
              if (E2PStatus != HAL_OK) return E2PStatus;

              WriteAddr +=  count;
              pBuffer += count;

              sEE_DataNum = temp;
              E2PStatus = EEPROM_HardSPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);
          } else
					{
              sEE_DataNum = NumByteToWrite;
              E2PStatus = EEPROM_HardSPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);
          }
          if (E2PStatus != HAL_OK) return E2PStatus;
      }
			else
			{ //NumByteToWrite > EEPROM_PAGESIZE
          NumByteToWrite -= count;
          NumOfPage =  NumByteToWrite / EEP_SPI_PAGESIZE;
          NumOfSingle = NumByteToWrite % EEP_SPI_PAGESIZE;

          sEE_DataNum = count;

          E2PStatus = EEPROM_HardSPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);
          if (E2PStatus != HAL_OK) return E2PStatus;

          WriteAddr +=  count;
          pBuffer += count;

          while (NumOfPage--)
					{
              sEE_DataNum = EEP_SPI_PAGESIZE;

							E2PStatus = EEPROM_HardSPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);
							if (E2PStatus != HAL_OK) return E2PStatus;

              WriteAddr +=  EEP_SPI_PAGESIZE;
              pBuffer += EEP_SPI_PAGESIZE;
          }

          if (NumOfSingle != 0)
					{
              sEE_DataNum = NumOfSingle;

              E2PStatus = EEPROM_HardSPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);
							if (E2PStatus != HAL_OK) return E2PStatus;
          }
      }
  }

  return HAL_OK;
}


//=======================================================================================
//====================== Functions for Software based SPI ===============================
//=======================================================================================
#elif (USE_SOFTWARE_SPI == 1)

/**
  * @brief  spi low level function for initalizing interface
	* @retval HAL_StatusTypeDef enum, HAL_OK in case of successful operation
  */
//==============================================
HAL_StatusTypeDef EEPROM_SoftSPI_Init(void)
//==============================================
{
	EEP_SPI_CS_GPIO_CLK_ENABLE();
	EEP_SPI_SCK_GPIO_CLK_ENABLE();
	
	GPIO_InitTypeDef GPIO_InitStruct;

  GPIO_InitStruct.Pin = EEP_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(EEP_CS_GPIO_Port, &GPIO_InitStruct);
	
  GPIO_InitStruct.Pin = EEP_CLK_Pin|EEP_MOSI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = EEP_MISO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(EEP_MISO_GPIO_Port, &GPIO_InitStruct);	
	
	return HAL_OK;
}

/**
  * @brief  spi low level function for sending single byte
  * @param  value: data value for write
	* @retval none
  */
//=========================================================
__STATIC_INLINE void EEPROM_SoftSPI_SendByte(uint8_t value)
//=========================================================	
{
  uint8_t clk = 0x08;
  
  while(clk > 0)
  {
     if((value & 0x80) != 0)
     {					
			EEP_SPI_SI_HIGH();
     }
     else
     {
       EEP_SPI_SI_LOW();
     }
     
     EEP_SPI_CK_LOW();
     EEP_CLK_DELAY(1);				
     EEP_SPI_CK_HIGH();
     EEP_CLK_DELAY(1);
     
     value <<= 1;
     clk--;
  };
}

/**
  * @brief  spi low level function for receiving single byte
	* @retval received data
  */
//=====================================================
__STATIC_INLINE uint8_t EEPROM_SoftSPI_RecvByte(void)
//=====================================================	
{
  uint8_t clk = 0x08;
  uint8_t temp = 0x00;
  
  while(clk > 0)
  {
    EEP_SPI_CK_LOW();
    EEP_CLK_DELAY(1);				
    EEP_SPI_CK_HIGH();
    EEP_CLK_DELAY(1);
    
    temp <<= 1;
    
    if(EEP_SPI_SO_IS_HIGH())
    {
        temp |= 1;
    }
    
    clk--;
  };
  
  return temp;
}

/**
  * @brief  it checks if spi interface is not busy
	* @retval HAL_StatusTypeDef enum, HAL_OK in case of successful operation
  */
//========================================================
HAL_StatusTypeDef EEPROM_SoftSPI_IsReady(void)
//========================================================
{
	uint32_t tw = HAL_GetTick();
	uint8_t ucByte;

	EEP_SPI_CS_LOW();
	EEPROM_SoftSPI_SendByte(CMD_RDSR);

	do
	{
		ucByte = EEPROM_SoftSPI_RecvByte();
		
	} while((bitRead(ucByte, BIT_WIP) == 1) && (HAL_GetTick() < tw + WRITE_TIMEOUT_MS));
	
	EEP_SPI_CS_HIGH();
	
	EEP_SEQ_DELAY(20);
	
	if(HAL_GetTick() >= tw + WRITE_TIMEOUT_MS) return HAL_ERROR;

	return HAL_OK;
}

/**
  * @brief  stores one byte to eeprom
  * @param  RegAdd: eeprom register address
  * @param  RegData: data for write
	* @retval HAL_StatusTypeDef enum, HAL_OK in case of successful operation
  */
//=================================================================================
HAL_StatusTypeDef EEPROM_SoftSPI_WriteByte(uint16_t RegAdd, uint8_t RegData)
//=================================================================================	
{
	if(EEPROM_SoftSPI_IsReady() != HAL_OK){
		if(EEPROM_SoftSPI_IsReady() != HAL_OK) return HAL_ERROR;
	}
	
	EEP_SPI_CS_LOW();
	EEPROM_SoftSPI_SendByte(CMD_WREN);
	EEP_SPI_CS_HIGH();

	EEP_SEQ_DELAY(20);
	
	EEP_SPI_CS_LOW();
	EEPROM_SoftSPI_SendByte(CMD_WRITE);
	EEPROM_SoftSPI_SendByte((uint8_t)(RegAdd >> 8) & 0xFF);
	EEPROM_SoftSPI_SendByte((uint8_t)RegAdd & 0xFF);
	EEPROM_SoftSPI_SendByte(RegData);
	EEP_SPI_CS_HIGH();

	EEP_SEQ_DELAY(20);
	
	EEP_SPI_CS_LOW();
	EEPROM_SoftSPI_SendByte(CMD_WRDI);
	EEP_SPI_CS_HIGH();

	EEP_SEQ_DELAY(20);
	
	return HAL_OK;
}

/**
  * @brief  receives one byte from eeprom
  * @param  RegAdd: eeprom register address
  * @param  pData: pointer to read variable
	* @retval HAL_StatusTypeDef enum, HAL_OK in case of successful operation
  */
//===============================================================================
HAL_StatusTypeDef EEPROM_SoftSPI_ReadByte(uint16_t RegAdd, uint8_t* pData)
//===============================================================================	
{
	EEP_SPI_CS_LOW();
	EEPROM_SoftSPI_SendByte(CMD_READ);
	EEPROM_SoftSPI_SendByte((uint8_t)(RegAdd >> 8) & 0xFF);
	EEPROM_SoftSPI_SendByte((uint8_t)RegAdd & 0xFF);
	*pData = EEPROM_SoftSPI_RecvByte();
	EEP_SPI_CS_HIGH();

	EEP_SEQ_DELAY(20);
	
  return HAL_OK;
}

/**
  * @brief  changes the value of status register of eeprom
  * @param  regval: value of data
	* @retval HAL_StatusTypeDef enum, HAL_OK in case of successful operation
  */
//=========================================================================
HAL_StatusTypeDef EEPROM_SoftSPI_WriteStatusRegister(uint8_t regval)
//=========================================================================	
{
	HAL_StatusTypeDef E2PStatus = HAL_OK;

	if(EEPROM_SoftSPI_IsReady() != HAL_OK){
		if(EEPROM_SoftSPI_IsReady() != HAL_OK) return HAL_ERROR;
	}
	
	EEP_SPI_CS_LOW();
	EEPROM_SoftSPI_SendByte(CMD_WREN);
	EEP_SPI_CS_HIGH();

	EEP_SEQ_DELAY(20);
	
	EEP_SPI_CS_LOW();
	EEPROM_SoftSPI_SendByte(CMD_WRSR);
	EEPROM_SoftSPI_SendByte(regval);
	EEP_SPI_CS_HIGH();

	EEP_SEQ_DELAY(20);
	
	EEP_SPI_CS_LOW();
	EEPROM_SoftSPI_SendByte(CMD_WRDI);
	EEP_SPI_CS_HIGH();

	EEP_SEQ_DELAY(20);
	
	return E2PStatus;
}

/**
  * @brief  reads number of bytes from eeprom 
  * @param  pBuffer: pointer to the data for read out
  * @param  WriteAddr: read address of eeprom (16bit address space)
  * @param  NumByteToRead: number of bytes for read  
	* @retval HAL_StatusTypeDef enum, HAL_OK in case of successful operation
  */
//==============================================================================================================
HAL_StatusTypeDef EEPROM_SoftSPI_ReadBuffer(uint8_t* pBuffer, uint16_t ReadAddr, uint16_t NumByteToRead)
//==============================================================================================================
{
	HAL_StatusTypeDef E2PStatus = HAL_OK;
	
	if(NumByteToRead == 0 || pBuffer == NULL) return HAL_ERROR;
	
	EEP_SPI_CS_LOW();
	EEPROM_SoftSPI_SendByte(CMD_READ);
	EEPROM_SoftSPI_SendByte((uint8_t)(ReadAddr >> 8) & 0xFF);
	EEPROM_SoftSPI_SendByte((uint8_t)(ReadAddr & 0xFF));
	
	for(uint16_t uCount = 0; uCount < NumByteToRead; uCount++, pBuffer++){
		*pBuffer = EEPROM_SoftSPI_RecvByte();
	}
	
	EEP_SPI_CS_HIGH();

	EEP_SEQ_DELAY(20);

  return E2PStatus;
}

/**
  * @brief  writes number of bytes in case they are page aligned 
  * @param  pBuffer: pointer to the data for write in
  * @param  WriteAddr: write address of eeprom (16bit address space)
  * @param  NumByteToWrite: number of bytes for write  
	* @retval HAL_StatusTypeDef enum, HAL_OK in case of successful operation
  */
//==============================================================================================================
HAL_StatusTypeDef EEPROM_SoftSPI_WritePage(uint8_t* pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite)
//==============================================================================================================	
{
	HAL_StatusTypeDef E2PStatus = HAL_OK;
	
	if(NumByteToWrite == 0 || pBuffer == NULL) return HAL_ERROR;
	
	if(EEPROM_SoftSPI_IsReady() != HAL_OK){
		if(EEPROM_SoftSPI_IsReady() != HAL_OK) return HAL_ERROR;
	}
	
	EEP_SPI_CS_LOW();
	EEPROM_SoftSPI_SendByte(CMD_WREN);
	EEP_SPI_CS_HIGH();
	
	EEP_SEQ_DELAY(20);
	
	EEP_SPI_CS_LOW();
	EEPROM_SoftSPI_SendByte(CMD_WRITE);
	EEPROM_SoftSPI_SendByte((uint8_t)(WriteAddr >> 8) & 0xFF);
	EEPROM_SoftSPI_SendByte((uint8_t)(WriteAddr & 0xFF));
	for(uint16_t uCount = 0; uCount < NumByteToWrite; uCount++, pBuffer++){
		EEPROM_SoftSPI_SendByte(*pBuffer);
	}
	EEP_SPI_CS_HIGH();

	EEP_SEQ_DELAY(20);	
	
	EEP_SPI_CS_LOW();
	EEPROM_SoftSPI_SendByte(CMD_WRDI);
	EEP_SPI_CS_HIGH();
	
	return E2PStatus;
}

/**
  * @brief  writes any number of data to eeprom in PageWrite mode even if the buffer is not page aligned
  * @param  pBuffer: pointer to the data for write in
  * @param  WriteAddr: write address of eeprom (16bit address space)
  * @param  NumByteToWrite: number of bytes for write  
	* @retval HAL_StatusTypeDef enum, HAL_OK in case of successful operation
  */
//================================================================================================================
HAL_StatusTypeDef EEPROM_SoftSPI_WriteBuffer(uint8_t* pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite)
//=================================================================================================================	
{
	HAL_StatusTypeDef E2PStatus = HAL_ERROR;
	
  uint16_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
  uint16_t sEE_DataNum = 0;

  Addr = WriteAddr % EEP_SPI_PAGESIZE;
  count = EEP_SPI_PAGESIZE - Addr;
  NumOfPage =  NumByteToWrite / EEP_SPI_PAGESIZE;
  NumOfSingle = NumByteToWrite % EEP_SPI_PAGESIZE;

	if(EEPROM_SoftSPI_IsReady() != HAL_OK){
		if(EEPROM_SoftSPI_IsReady() != HAL_OK) return HAL_ERROR;
	}
		
  if(Addr == 0)
	{ // WriteAddr is EEPROM_PAGESIZE aligned 
      if(NumOfPage == 0)
			{ // NumByteToWrite < EEPROM_PAGESIZE
          sEE_DataNum = NumByteToWrite;
          E2PStatus = EEPROM_SoftSPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);
          if (E2PStatus != HAL_OK) return E2PStatus;

      } 
			else
			{ // NumByteToWrite > EEPROM_PAGESIZE
          while (NumOfPage--)
					{
              sEE_DataNum = EEP_SPI_PAGESIZE;
              E2PStatus = EEPROM_SoftSPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);
              if (E2PStatus != HAL_OK) return E2PStatus;

              WriteAddr +=  EEP_SPI_PAGESIZE;
              pBuffer += EEP_SPI_PAGESIZE;
          }

          sEE_DataNum = NumOfSingle;
          E2PStatus = EEPROM_SoftSPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);

          if (E2PStatus != HAL_OK) return E2PStatus;
      }
  }
	else
	{ // WriteAddr is not EEPROM_PAGESIZE aligned
      if (NumOfPage == 0)
			{ // NumByteToWrite < EEPROM_PAGESIZE
          if (NumOfSingle > count)
					{ // (NumByteToWrite + WriteAddr) > EEPROM_PAGESIZE
              temp = NumOfSingle - count;
              sEE_DataNum = count;
              E2PStatus = EEPROM_SoftSPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);
              if (E2PStatus != HAL_OK) return E2PStatus;

              WriteAddr +=  count;
              pBuffer += count;

              sEE_DataNum = temp;
              E2PStatus = EEPROM_SoftSPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);
          } else
					{
              sEE_DataNum = NumByteToWrite;
              E2PStatus = EEPROM_SoftSPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);
          }
          if (E2PStatus != HAL_OK) return E2PStatus;
      }
			else
			{ //NumByteToWrite > EEPROM_PAGESIZE
          NumByteToWrite -= count;
          NumOfPage =  NumByteToWrite / EEP_SPI_PAGESIZE;
          NumOfSingle = NumByteToWrite % EEP_SPI_PAGESIZE;

          sEE_DataNum = count;

          E2PStatus = EEPROM_SoftSPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);
          if (E2PStatus != HAL_OK) return E2PStatus;

          WriteAddr +=  count;
          pBuffer += count;

          while (NumOfPage--)
					{
              sEE_DataNum = EEP_SPI_PAGESIZE;

							E2PStatus = EEPROM_SoftSPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);
							if (E2PStatus != HAL_OK) return E2PStatus;

              WriteAddr +=  EEP_SPI_PAGESIZE;
              pBuffer += EEP_SPI_PAGESIZE;
          }

          if (NumOfSingle != 0)
					{
              sEE_DataNum = NumOfSingle;

              E2PStatus = EEPROM_SoftSPI_WritePage(pBuffer, WriteAddr, sEE_DataNum);
							if (E2PStatus != HAL_OK) return E2PStatus;
          }
      }
  }

  return HAL_OK;
}

#endif   


#include "string.h"
#include "stdlib.h"	

#define READ_WRITE_NUM 						 	(uint8_t)40
#define READ_WRITE_ADDRESS				 	(uint16_t)0
#define NVM_RANDOM_SEED				 			(uint16_t)0x1237

/**
  * @brief  testing eeprom operation in single read/write mode
  * @param  eraseFlag: select if erase write area
	* @retval HAL_StatusTypeDef enum, HAL_OK in case of successful operation
  */
//=================================================================
HAL_StatusTypeDef EEPROM_SPI_SingleReadWriteTest(uint8_t eraseFlag)
//=================================================================
{
	HAL_StatusTypeDef E2PStatus = HAL_ERROR;
	uint8_t ucBuf[READ_WRITE_NUM];
	
	//Init IO for soft SPI.
	EEPROM_SPI_Init();
	
	//Check if SPI is ready
	if(EEPROM_SPI_IsReady() == HAL_OK)
	{
		EEP_LOG("EEPROM Is Ready\r\n");
		
		//ReadOut NVM Data
		EEP_LOG("EEPROM Data ReadOut :\r\n\r\n");
		memset(ucBuf, 0, sizeof(ucBuf));
		for(uint16_t i = READ_WRITE_ADDRESS; i < READ_WRITE_NUM + READ_WRITE_ADDRESS ; i++) {
			E2PStatus = EEPROM_SPI_ReadByte((i - READ_WRITE_ADDRESS), &ucBuf[(i - READ_WRITE_ADDRESS)]);
			EEP_LOG("0x%X ", ucBuf[(i - READ_WRITE_ADDRESS)]);
		}
		EEP_LOG("\r\n\r\n");
		
		//WriteIn random data in NM
		EEP_LOG("EEPROM Data WriteIn :\r\n\r\n");
		srand(NVM_RANDOM_SEED);		
		memset(ucBuf, 0, sizeof(ucBuf));
		for(uint16_t j = READ_WRITE_ADDRESS; j < READ_WRITE_NUM + READ_WRITE_ADDRESS ; j++) 
		{
			if(eraseFlag == 0) ucBuf[(j - READ_WRITE_ADDRESS)] = rand() % 255;
			if(eraseFlag == 1) ucBuf[(j - READ_WRITE_ADDRESS)] = 0xFF;
			E2PStatus = EEPROM_SPI_WriteByte((j - READ_WRITE_ADDRESS), ucBuf[(j - READ_WRITE_ADDRESS)]);
			EEP_LOG("0x%X ", ucBuf[(j - READ_WRITE_ADDRESS)]);
		}
		EEP_LOG("\r\n\r\n");

		//ReadOut again data
		EEP_LOG("EEPROM Data ReadOut Again :\r\n\r\n");
		memset(ucBuf, 0, sizeof(ucBuf));
		for(uint16_t i = READ_WRITE_ADDRESS; i < READ_WRITE_NUM + READ_WRITE_ADDRESS ; i++) {
			E2PStatus = EEPROM_SPI_ReadByte((i - READ_WRITE_ADDRESS), &ucBuf[(i - READ_WRITE_ADDRESS)]);
			EEP_LOG("0x%X ", ucBuf[(i - READ_WRITE_ADDRESS)]);
		}
		EEP_LOG("\r\n\r\n");
	}	
	
	return E2PStatus;
}

/**
  * @brief  testing eeprom operation in multiple read/write mode
  * @param  eraseFlag: select if erase write area
	* @retval HAL_StatusTypeDef enum, HAL_OK in case of successful operation
  */
//===================================================================
HAL_StatusTypeDef EEPROM_SPI_MultipleReadWriteTest(uint8_t eraseFlag)
//===================================================================
{
	HAL_StatusTypeDef E2PStatus = HAL_ERROR;
	uint8_t ucBuf[READ_WRITE_NUM];
	
	//Init IO for soft SPI.
	EEPROM_SPI_Init();
	
	//Check if SPI is ready
	if(EEPROM_SPI_IsReady() == HAL_OK)
	{
		EEP_LOG("EEPROM Is Ready\r\n");
		
		//ReadOut NVM Data
		EEP_LOG("EEPROM Data ReadOut :\r\n\r\n");
		memset(ucBuf, 0, sizeof(ucBuf));
		E2PStatus = EEPROM_SPI_ReadBuffer(ucBuf, READ_WRITE_ADDRESS, READ_WRITE_NUM);
		
		for(uint16_t i = 0; i < READ_WRITE_NUM; i++){
			EEP_LOG("0x%X ", ucBuf[i]);
		}
		EEP_LOG("\r\n\r\n");
		
		//WriteIn random data in NM
		EEP_LOG("EEPROM Data WriteIn :\r\n\r\n");
		srand(NVM_RANDOM_SEED);		
		memset(ucBuf, 0, sizeof(ucBuf));
		for(uint16_t j = 0; j < READ_WRITE_NUM ; j++) 
		{
			if(eraseFlag == 0) ucBuf[j] = rand() % 255;
			if(eraseFlag == 1) ucBuf[j] = 0xFF;

			EEP_LOG("0x%X ", ucBuf[j]);
		}
		EEP_LOG("\r\n\r\n");
		E2PStatus = EEPROM_SPI_WriteBuffer(ucBuf, READ_WRITE_ADDRESS, READ_WRITE_NUM);
		
		//ReadOut again
		EEP_LOG("EEPROM Data ReadOut Again :\r\n\r\n");
		memset(ucBuf, 0, sizeof(ucBuf));
		E2PStatus = EEPROM_SPI_ReadBuffer(ucBuf, READ_WRITE_ADDRESS, READ_WRITE_NUM);
		
		for(uint16_t i = 0; i < READ_WRITE_NUM; i++){
			EEP_LOG("0x%X ", ucBuf[i]);
		}
		EEP_LOG("\r\n\r\n");
	}	
	
	return E2PStatus;
}

/**
  * @brief  check if spi interface is functional
	* @retval value 1 in case of successful operation
  */
//====================================
uint8_t BSP_EEPROM_IsConnected(void)
//====================================
{
	EEPROM_SPI_Init();	
	
	for(uint8_t uCount = 0; uCount < 5; uCount++)
	{
		if(EEPROM_SPI_IsReady() == HAL_OK) return 1;
		HAL_Delay(50);
	}
	return 0;
}

/**
  * @brief  Writes multiple bytes to eeprom
  * @param  reg_address: eeprom register address starts from 0x0000
  * @param  data_buf: pointer to a user defined buffer for data to be copied
  * @param  length: number of bytes to be written statring from reg_address
	* @retval HAL_StatusTypeDef enum, HAL_OK in case of successful operation
  */
//=============================================================================================
HAL_StatusTypeDef BSP_EEPROM_Write(uint16_t reg_address, uint8_t data_buf[], uint16_t length)
//=============================================================================================
{
	HAL_StatusTypeDef E2PStatus = HAL_ERROR;
	E2PStatus = EEPROM_SPI_WriteBuffer(data_buf, reg_address, length);		
	return E2PStatus;	
}

/**
  * @brief  Reads multiple bytes from eeprom
  * @param  reg_address: eeprom register address starts from 0x0000
  * @param  data_buf: pointer to a user defined buffer for data to be copied
  * @param  length: number of bytes to be restored statring from reg_address
	* @retval HAL_StatusTypeDef enum, HAL_OK in case of successful operation
  */
//============================================================================================
HAL_StatusTypeDef BSP_EEPROM_Read(uint16_t reg_address, uint8_t data_buf[], uint16_t length)
//============================================================================================
{
	HAL_StatusTypeDef E2PStatus = HAL_ERROR;
	E2PStatus = EEPROM_SPI_ReadBuffer(data_buf, reg_address, length);			
	return E2PStatus;
}
