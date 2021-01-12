/*
 * Project i2s_mic
 * Description:
 * Author:
 * Date:
 */

#include "stm32f2xx.h"
#include "stm32f2xx_rcc.h"
#include "stm32f2xx_gpio.h"
#include "stm32f2xx_spi.h"
#include "misc.h"

union
{
  int32_t I2S_Rx;
  struct
  {
    int16_t I2S_Rx_B1_B16;
    int16_t I2S_Rx_B17_B32;
  };
} Mic_data;

int16_t I2S_Buffer_Rx[2] = {0, 0};
int8_t RxIdx = 0;
/*
1. I2S3_WS could be on either PA15 which is pin# 7 on M18PTH (U$5) or PA4 which is pin# 14 on M18PTH (U$4)
2. I2S3_SCK could be on either PB3 which is pin# 8 on M18PTH (U$5) or PC10 which is pin# 15 on M18PTH (U$5)
3. I2S3_SD could be on either PB5 which is pin# 10 on M18PTH (U$5) or PC12 which is pin# 17 on M18PTH (U$5)
*/
static void I2S_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  I2S_InitTypeDef I2S_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;  

   /* Enable the I2S3 peripheral clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
  
  /* Enable I2S3 GPIO clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB, ENABLE);
  
  /* I2S3 pins configuration: WS, SCK and SD pins */
  /* I2S3_WS  --> PA15
     I2S3_SCK --> PB3
	   I2S3_SD  --> PB5
  */	 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;  
  GPIO_Init(GPIOA, &GPIO_InitStructure);   
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5; 
  GPIO_Init(GPIOB, &GPIO_InitStructure);   
  
  /* Connect pins to I2S3 peripheral  */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource15, GPIO_AF_SPI3);  
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource3,  GPIO_AF_SPI3);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource5,  GPIO_AF_SPI3);

  /* Disable I2S3 peripheral before configuration */
  SPI_I2S_DeInit(SPI3);

   /* I2S peripheral configuration */
  I2S_InitStructure.I2S_AudioFreq = I2S_AudioFreq_48k;
  I2S_InitStructure.I2S_Standard = I2S_Standard_MSB;
  I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_24b;
  I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;
  I2S_InitStructure.I2S_Mode = I2S_Mode_MasterRx;
  I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Disable;

  /* Initialize the I2S3 peripheral with the structure above */
  I2S_Init(SPI3, &I2S_InitStructure);  
  
  /* I2S DMA IRQ Channel configuration */
	NVIC_InitStructure.NVIC_IRQChannel = SPI3_IRQn;	             
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);  

  SPI_I2S_ITConfig(SPI3, SPI_I2S_IT_RXNE|SPI_I2S_IT_ERR, ENABLE);

  /* Enable the SPI3/I2S3 Master peripheral */
  I2S_Cmd(SPI3, ENABLE);
	

}

/**
* @brief  This function handles SPI1 global interrupt request
* @param  None
* @retval : None
*/
void SPI3_IRQHandler(void)
{
  uint32_t tmpreg = 0x00;
  tmpreg = SPI3->SR;

  if ((tmpreg & SPI_I2S_FLAG_RXNE) == SPI_I2S_FLAG_RXNE)
  {
    I2S_Buffer_Rx[RxIdx++] = SPI_I2S_ReceiveData(SPI3);     
  }
  else if (((tmpreg & I2S_FLAG_UDR) == 0x01) || ((tmpreg & SPI_I2S_FLAG_OVR) == 0x01))
  {
     RxIdx = -1;   
  }
}



// setup() runs once, when the device is first turned on.
void setup() {
  // Put initialization like pinMode and begin functions here.

  /* Initialize the I2S3 peripheral with the structure above */
  I2S_Config();
  
  Particle.variable("Mic", Mic_data.I2S_Rx);
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code will likely live here.

   /* handle error */
  if(RxIdx == -1)
  {    
    I2S_Config(); //re-init I2S
    RxIdx = 0;
  }
  else  if(RxIdx == 2)
  {
    // The Data Format is I2S, 24-bit, 2’s compliment, MSB first. 
    // The data precision is 18 bits; unused bits are zeros.
    Mic_data.I2S_Rx_B1_B16 = (int32_t)I2S_Buffer_Rx[0];
    Mic_data.I2S_Rx_B17_B32 =  (int32_t)I2S_Buffer_Rx[1];
    Particle.publish("Mic", String(Mic_data.I2S_Rx), PRIVATE);
    RxIdx = 0;
  }
}