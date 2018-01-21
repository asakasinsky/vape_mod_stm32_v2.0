/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal.h"

/* USER CODE BEGIN Includes */
#include "ssd1306.h"
#include "fonts.h"
#include "vape.h"
#include <stdbool.h>
#include <math.h>
#include "eeprom.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim1;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
uint32_t adc_buffer[3];
float value[3];
float read_values[2];
float voltageOut[2];

char vOut2[9];  // vOut1 - Om; vOut1 - bat;
int voltage=400;
char voltage2[5];
char watt2[6];

float V_25 = 1.34;
float Slope = 4.3e-3;
float Vref = 3.36;
float V_sense;
float temp;
char tempP[6];


int16_t PWM_UP = 3800;
float volt_set = 2.2;
float watt_set=16.0;
float volt_set_w=0.0;
int16_t PWM_OUT = 0;

float R_vape = 0.00;
//float V_vape = 0.0;
float R_buff = 0.0;
//int raw = 0;
char R_vape2[6];

uint8_t status=2;
uint8_t old_status=1;

bool clearLCD = false;


bool noCoil=true;
uint8_t counterCoil=0;
bool coilTest=true;
bool FireButton=false;
uint16_t temp_tik=0;
bool charge = false;
bool clear =false;
uint32_t tick_delay=0;
uint32_t timeout=10000;
uint32_t volt_set_eeprom;
uint32_t watt_set_eeprom;
uint32_t status_eeprom;
uint32_t puffs=0;
uint8_t powercount=1;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
                                

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
		if (hadc->Instance == ADC1)
			for (int i=0; i<3; i++)
		{
					value[i]=adc_buffer[i];
		}

}
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

int main(void)

{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
	PWR->CSR   |= PWR_CSR_EWUP;
	
 // HAL_PWR_PVDCallback();
	//if ((PWR->CSR & PWR_CSR_WUF) == PWR_CSR_WUF) // ???? ?????, ?? ?????????? ????      
    //     PWR->CR |= PWR_CR_CWUF;
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();

  /* USER CODE BEGIN 2 */
	HAL_ADC_Start_DMA(&hadc1,adc_buffer,3);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	ssd1306_Init();
	
	while(PowerOn())
	{
		if(powercount<=0)
		{
	Power_off2();}
		//if(powercount>=60)
		//{break;}
	}
	
	SSD1306_DrawFilledRectangle(0,0,128,64,Black);
	ssd1306_UpdateScreen();
	
	Draw_Frame2();
	Draw_Acumulator();
	
	
	ssd1306_UpdateScreen();
	PWM_OUT = (PWM_UP*volt_set)/read_values[1];
	
	
	DBGMCU->CR |= DBGMCU_CR_DBG_STANDBY;

	tick_delay = HAL_GetTick();
	
		EE_Read(0,&volt_set_eeprom);
		EE_Read(1,&watt_set_eeprom);
		EE_Read(2,&timeout);
		EE_Read(3,&status_eeprom);
		EE_Read(4,&puffs);
		status=status_eeprom;
		old_status=status;
		volt_set=volt_set_eeprom/100.0;
		watt_set=watt_set_eeprom/100.0;
		
		//tick_delay = HAL_GetTick();
		
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
		//tick_delay = HAL_GetTick();
		Read_ADC();
		temp_tik++;
		Timer_off();
		if (clear==true&&charge==false)
		{
			clear=false;
			clearLCD=true;
		}
		

		
		if (clearLCD==true)	
		{
			SSD1306_DrawFilledRectangle(0,0,128,64,Black);
			ssd1306_UpdateScreen();
			clearLCD=false;
		}
		
		
		if (status==0)	
		{
			ssd1306_SetCursor(51,9);
			ssd1306_WriteString("MENU",Font_7x10,White);
			SSD1306_DrawLine(0,23,128,23,White);
			ssd1306_UpdateScreen();

			Menu();
			
			
		}
		if (status==5)	
		{
			Menu_settings();
		}
		
		if (status==6)	
		{
			Set_Time_Out();
		}
		
		
		
		if (status==1)
		{
			
			Draw_Acumulator();
			PWM_OUT = (PWM_UP*volt_set)/read_values[1]; 
			
			
			if(read_values[0]>0.8&&counterCoil==0)
					{ 
						Read_Om_t();
						counterCoil=1;
						Print_Om();
					}
					
			if(read_values[0]<0.8&&FireButton==false)
					{
						
						counterCoil=0;
						NoCoil();
					}
			if (counterCoil==1)
					{
							Print_Om();
					}else {NoCoil();}
					
				Varivolt();
				Print_Acum();
				Counter_Fire();
			Vout();
			Read_Amp();
				if (temp_tik>=6)
						{	
							Read_temperature();
							temp_tik=0;
						}
		}
		
		
		if (status==2)
		{
			Draw_Acumulator();
			volt_set_w= sqrt(watt_set*R_vape);
			PWM_OUT = (PWM_UP*volt_set_w)/read_values[1];
			Vout();
			Read_Amp();
			if(read_values[0]>1.0&&counterCoil==0)
					{ 
						Read_Om_t();
						counterCoil=1;
					}
			if(read_values[0]<1.0&&FireButton==false)
					{
						counterCoil=0;
						NoCoil();
					}
			if (counterCoil==1)
					{
						Print_Om();
					}
			Varivatt();
			Print_Acum();
			Counter_Fire();
			if (temp_tik>=6)
					{	
						Read_temperature();
						temp_tik=0;
					}
					
							//ssd1306_UpdateScreen();

		}

		
		if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13)&&status!=0)
			{
				clearLCD=true;
				
				status=0;
				old_status=status;
				temp_tik=0;
				tick_delay = HAL_GetTick();
				
				
		//HAL_DBGMCU_EnableDBGStopMode();
	//	HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
			//SystemClock_Config();
		//	HAL_PWR_EnterSTANDBYMode();
				//HAL_PWR_EnableSleepOnExit();
			//	HAL_PWR_EnterSTANDBYMode();
			//	SystemClock_Config();
				
	//	PWR->CSR   |= PWR_CSR_EWUP1;   
	//	PWR->CR    |= PWR_CR_CWUF;
		//		SSD1306_OFF();
		//DBGMCU->CR |= DBGMCU_CR_DBG_STANDBY;
		//HAL_PWR_EnterSTANDBYMode();
		//	SystemClock_Config();	
			
			//HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
			//SystemClock_Config();
			}
		
			
		if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3)||!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4))
			{
				//old_status=status;
				status=3;
				charge=true;
				//clearLCD=true;
			SSD1306_DrawFilledRectangle(0,0,128,64,Black);
			}	
			else
					{
						
						status=old_status;
						charge=false;
						
						
					}		
			if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4)&&charge==true)
			{
				ssd1306_SetCursor(49,50);
				ssd1306_WriteString("FULL",Font_7x10,White);
				Charge();
				puffs=0;
			}
			
			if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3)&&charge==true)
			{
				
				Charge();
				tick_delay = HAL_GetTick();
			}
	
		ssd1306_UpdateScreen();

			
}
		
	
			
		


  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV2;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* ADC1 init function */
static void MX_ADC1_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Common config 
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 3;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = 3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* I2C1 init function */
static void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM1 init function */
static void MX_TIM1_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = PWM_UP;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim1);

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB13 PB14 PB3 
                           PB4 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_3 
                          |GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PA9 */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

}

/* USER CODE BEGIN 4 */
void Vout(void)
{
				
					sprintf(vOut2,"%.2fV",read_values[1]);
					
		
					ssd1306_SetCursor(91,51);
					ssd1306_WriteString(vOut2,Font_7x10,White);
				
				//	ssd1306_UpdateScreen();
}








/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
