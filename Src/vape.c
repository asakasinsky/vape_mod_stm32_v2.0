#include "ssd1306.h"
#include <stdbool.h>
#include <math.h>
#include "eeprom.h"

RTC_DateTypeDef sdatestructureget;
RTC_TimeTypeDef stimestructureget;
RTC_TimeTypeDef setT;
extern RTC_HandleTypeDef hrtc;


uint8_t click=0; 
char ch_timestamp[6];
uint16_t timer_akum=4000;
extern volatile float read_values[2];
extern volatile float voltageOut[2];
extern float value[3];
extern int16_t PWM_OUT;
extern int16_t PWM_UP;
extern float volt_set;
extern float watt_set;
extern float volt_set_w;
extern char voltage2[5];
extern char watt2[6];
extern char tempP[6];
extern uint8_t status;
extern uint8_t old_status;
extern bool clearLCD;
extern float temp;
extern char vOut2[9];

extern float R_buff;
extern float R_vape;
extern char R_vape2[6];
extern bool noCoil;
extern uint8_t counterCoil;
uint16_t tik=0;
extern uint16_t time_ADC;
int i;

char *st[11][6]={"           "," ВАРИВОЛЬТ "," ВАРИВАТТ  "," НАСТРОЙКИ "," Инфо      ","           "};
char *st_settings[11][8]={"           "," ВРЕМЯ ВЫКЛ"," ВРЕМЯ OUT "," ЗАТЯЖКИ   "," СБРОС     "," УСТ ЧАСОВ "," НАЗАД     ","           "};

uint8_t m=2;
uint8_t m2=2;


extern bool FireButton;
extern uint16_t temp_tik;
extern bool charge;
extern bool clear;
extern uint32_t tick_delay;
extern uint32_t timeout;
extern int32_t setout;
char setout2[5];
char timeout2[5];
extern uint32_t volt_set_eeprom;
extern uint32_t watt_set_eeprom;
float amper=0;

char amper_print[6];
uint8_t low_batt=0;
bool coil_tik=true;
extern uint32_t puffs;
char puffs_print[8];
extern uint32_t powercount;

float Mn = 0.0;		 //Результат фильтра Калмана
float An = 0.0;		 //Исходное значение
float Mn1 = 0.0;   //результат преддущей итерации
float k = 0.15;     //Коэффициент 

volatile float temp_timestamp=0.0;  //Отсчет микросекунд нажатой кнопки Fire

bool read_om = false;
char showsec[3]={0};
char showtime[9]={0};
uint8_t status_timer = 0;
uint32_t clok =0;

uint8_t setTimeH=0;
uint8_t setTimeM=0;
uint8_t SetMin=0;
char timeoutM[4]={0};
extern bool set_RTC;

//uint16_t tempSecond=0;


void Out_Time()
{
	TIM1->CCR1=0;
	SSD1306_DrawFilledRectangle(1,1,85,30,Black);
	ssd1306_SetCursor(0,9);
	ssd1306_WriteString("Out Time",Font_11x18,White);
	ssd1306_UpdateScreen();	
	

}


 

void Print_Sec()
{
	
    
    
		
		sprintf((char *)ch_timestamp,"%0.2fs",(temp_timestamp/1000.0));
		ssd1306_SetCursor(4,7);
		ssd1306_WriteString(ch_timestamp,Font_16x25,White);

}






void Kalman ()
{
	Mn = k * An;
	An = 1 - k;
	Mn1 = Mn1 * An;
	Mn = Mn + Mn1;
	Mn1 = Mn;
}

void Info()
{
	temp_timestamp=0;
	SSD1306_DrawFilledRectangle(0,0,128,64,Black);
  ssd1306_SetCursor(21,1);
	ssd1306_WriteString2("Вейп мод V2.3",Font_7x9,White);
  ssd1306_SetCursor(28,13);
	ssd1306_WriteString2("Open Source",Font_7x9,White);
	ssd1306_SetCursor(28,26);
	ssd1306_WriteString2("работает на",Font_7x9,White);
	ssd1306_SetCursor(28,39);
	ssd1306_WriteString2("stm32f103c8",Font_7x9,White);
	ssd1306_SetCursor(24,52);
	ssd1306_WriteString2("Cvetaev 2018",Font_7x9,White);
	ssd1306_UpdateScreen();
	
 
  	if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13)&&temp_timestamp>1000)
    { status=0;
			old_status=status;
			SSD1306_DrawFilledRectangle(0,0,128,64,Black);
		}
}






uint8_t PowerOn()
{
	SSD1306_DrawRectangle(0,28,128,9,White);
	if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0))
	{
		SSD1306_DrawFilledRectangle(2,30,1+powercount,5,White);
		ssd1306_UpdateScreen();
		powercount=powercount+5;
		HAL_Delay(1);
		
		if (powercount>=125)
		{
      
      
			SSD1306_DrawFilledRectangle(0,0,128,64,Black);
			ssd1306_UpdateScreen();
      
    
      
			ssd1306_SetCursor(50,10);
			ssd1306_WriteString("Hi",Font_14x22,White);
			ssd1306_SetCursor(20,30);
			ssd1306_WriteString("Cvetaev",Font_14x22,White);
			ssd1306_UpdateScreen();
			HAL_Delay(600);
			return 0;
		}
		
		
	}
	else {powercount--;
		SSD1306_DrawFilledRectangle(1,30,125,5,Black);
		SSD1306_DrawFilledRectangle(2,30,1+powercount,5,White);
		ssd1306_UpdateScreen();
		//HAL_Delay(50);
	}
	
	return 1;
}

void Power_off()
{			
      SSD1306_OFF();
			volt_set_eeprom=volt_set*100.0;
			watt_set_eeprom=watt_set*100.0;
			EE_Write(0,volt_set_eeprom);
			EE_Write(1,watt_set_eeprom);
			EE_Write(2,timeout);
			if (status==1||status==2){
			EE_Write(3,status);}
			EE_Write(4,puffs);
      EE_Write(5,setout);
			
			PWR->CSR   |= PWR_CSR_EWUP;
			PWR->CR    |= PWR_CR_CWUF;
			PWR->CR = PWR_CR_PDDS | PWR_CR_CWUF;
			HAL_PWR_EnterSTANDBYMode();	
}


void Power_off2()
{
			SSD1306_OFF();
			PWR->CSR   |= PWR_CSR_EWUP;
			PWR->CR    |= PWR_CR_CWUF;
			PWR->CR = PWR_CR_PDDS | PWR_CR_CWUF;
			HAL_PWR_EnterSTANDBYMode();	

}

void Draw_Frame()
{
	//--------------frame
	SSD1306_DrawLine(0,64,128,64,White);
	SSD1306_DrawLine(0,0,128,0,White);
	SSD1306_DrawLine(128,0,128,64,White);
	SSD1306_DrawLine(0,0,0,64,White);
  //SSD1306_DrawLine(0,15,92,15,White);
	SSD1306_DrawLine(89,33,128,33,White);
	SSD1306_DrawLine(88,0,88,64,White);
	
	//SSD1306_DrawLine(92,0,92,33,White);
	//---------------frame end
}

void Draw_Frame2()
{

SSD1306_DrawLine(89,64,128,64,White);
	SSD1306_DrawLine(89,0,128,0,White);
	SSD1306_DrawLine(128,0,128,64,White);
//	SSD1306_DrawLine(0,0,0,64,White);
  //SSD1306_DrawLine(0,15,92,15,White);
	SSD1306_DrawLine(89,33,128,33,White);
	SSD1306_DrawLine(88,0,88,64,White);

}

void Draw_Acumulator()
{
//---------------acum
	SSD1306_DrawLine(4,36,77,36,White);
	//SSD1306_DrawLine(5,35,46,35,White);
	SSD1306_DrawLine(4,60,77,60,White);
	SSD1306_DrawLine(3, 37, 3, 59,White);
	SSD1306_DrawLine(78, 37, 78, 59,White);
	SSD1306_DrawLine(79, 41, 79, 55,White);
	SSD1306_DrawLine(80, 41, 80, 55,White);
	SSD1306_DrawLine(81, 42, 81, 54,White);
//	SSD1306_DrawLine(52, 43, 52, 53,White);
	//---------------acum end
	//---------------acum2
/*	SSD1306_DrawLine(4,47,27,47,White);
	SSD1306_DrawLine(4,35,27,35,White);
	SSD1306_DrawLine(3, 36, 3, 46,White);
	SSD1306_DrawLine(28, 36, 28, 46,White);
	SSD1306_DrawLine(29, 38, 29, 44,White);
	SSD1306_DrawLine(30, 38, 30, 44,White);
	SSD1306_DrawLine(31, 39, 31, 43,White);
*/	//---------------acum2 end
}

void Read_ADC()
{
		
			voltageOut[1]=(value[1]*3.33)/4095;
			voltageOut[0]=(value[0]*3.33)/4095;
			
			
//			An=voltageOut[1]/(100000.0/(100000.0+33000.0));
//			Kalman();
//			read_values[1]=Mn;
			read_values[1]=voltageOut[1]/(100000.0/(100000.0+33000.0));
			read_values[0]=voltageOut[0]/(100000.0/(100000.0+33000.0));

}

void Read_temperature()
{
	//	temp=(read_values[1]*read_values[1])/R_vape;
	//	sprintf(tempP,"%.1fW",temp);
		
		
		//temp=value[2]/4095.0*3.27;
		temp=((1.39-(value[2]/4095.0*3.33))/0.0043)+25.0;
		sprintf((char *)tempP,"%.1f* ",temp);
	
	
		ssd1306_SetCursor(91,38);
		ssd1306_WriteString2(tempP,Font_7x9,White);
		//ssd1306_UpdateScreen();
	
	
}

void Read_sensor_charge()
{
		temp=value[2]/4095.0*3.3;
		temp=(1.34-temp)/0.0043+25;
		sprintf((char *)tempP,"%.1f*",temp);
		ssd1306_SetCursor(45,50);
		ssd1306_WriteString2(tempP,Font_7x9,White);
		ssd1306_UpdateScreen();
		
	
}

uint8_t Get_Watt()
{
	if(volt_set_w>(read_values[1]-0.06))
	{ 
	
		return 0;
	}
	else 
		return 1;
}


uint8_t Get_Volt()
{
	if (volt_set>(read_values[1]-0.1))
	{
		return 0;
	}else
		return 1;
}


void Read_Amp()
{
//	float amper_filter[6];
//	amper_filter[0]=(read_values[1]-read_values[0])/R_vape;	
//	amper_filter[1]=(read_values[1]-read_values[0])/R_vape;	
//	amper_filter[2]=(read_values[1]-read_values[0])/R_vape;	
//	amper_filter[3]=(read_values[1]-read_values[0])/R_vape;
//	amper_filter[4]=(read_values[1]-read_values[0])/R_vape;
//	amper_filter[5]=(read_values[1]-read_values[0])/R_vape;
//	amper=(amper_filter[0]+amper_filter[1]+amper_filter[2]+amper_filter[3]+amper_filter[4]+amper_filter[5])/6.0;
	switch (status)
		{
//			case 1:amper=volt_set/R_vape;break;
//			case 2:amper=volt_set_w/R_vape;break;
			case 1:amper=(read_values[1]-read_values[0])/R_vape;sprintf(amper_print,"%.1fA ",amper);break;
			case 2:amper=(read_values[1]-read_values[0])/R_vape;sprintf(amper_print,"%.1fA ",amper);break;
		}
		
	//sprintf(amper_print,"%.1fA",amper);
}

void Power_click()
{
  
  
  if(FireButton==false)
  {
    click++;
  }
  else  
   if(temp_timestamp>900)
      {
        click=0;
      }
 if(temp_timestamp>900)
  {
    click=0;
  }
  
  if(click>2)
    Power_off();
  
}


void Varivolt()
{
	Draw_Frame2();
    if(temp_timestamp>900)
        {
          click=0;
        }
if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)) 
		{
			if(FireButton==false){temp_timestamp=0;puffs++;}
									
        Power_click();
        FireButton=true;
			
						if (read_values[1]>3.30&&low_batt==0)
						{
							if (counterCoil==1)
								{
									noCoil=true;
                  if(temp_timestamp>setout)
										Out_Time();
									else
										{
											timer_akum=0;
											Print_Sec();
											FireButton=true;
											noCoil=true;
											TIM1->CCR1=PWM_OUT;
											tick_delay = HAL_GetTick();
											ssd1306_SetCursor(91,14);
											ssd1306_WriteString(amper_print,Font_7x10,White);
										}
								} 
							else 
									{
										SSD1306_DrawFilledRectangle(1,1,85,30,Black);
										ssd1306_SetCursor(6,9);
										ssd1306_WriteString("NO COIL",Font_11x18,White);
										ssd1306_UpdateScreen();
										FireButton=true;
										tick_delay = HAL_GetTick();
									}
						}
						else{		SSD1306_DrawFilledRectangle(1,1,85,30,Black);
										ssd1306_SetCursor(0,9);
										ssd1306_WriteString("LOW BATT",Font_11x18,White);
										ssd1306_UpdateScreen();	
										TIM1->CCR1=0;
										low_batt=1;
										noCoil=false;
										FireButton=true;
										tick_delay = HAL_GetTick();
								}
					}
//				else
//				{
//						SSD1306_DrawFilledRectangle(1,1,85,30,Black);
//						ssd1306_SetCursor(0,9);
//						ssd1306_WriteString("ERROR Om",Font_11x18,White);
//						ssd1306_UpdateScreen();
//						TIM1->CCR1=0;
//						low_batt=1;
//						noCoil=false;
//						FireButton=false;
//				
//				}
//    }
			else
				{
					TIM1->CCR1=0;
					noCoil=false;
					FireButton=false;
					SSD1306_DrawFilledRectangle(1,1,85,30,Black);
					sprintf(voltage2,"%0.2fV",volt_set);
					ssd1306_SetCursor(4,7);
					ssd1306_WriteString(voltage2,Font_16x25,White);
					ssd1306_SetCursor(91,14);
					ssd1306_WriteString("-.--A",Font_7x10,White);
					ssd1306_UpdateScreen();	
					coil_tik=true;
				}
				
if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14)&&Get_Volt()==1)
		{
			
			volt_set=volt_set+0.01;
			PWM_OUT = (PWM_UP*volt_set)/read_values[1];
			tick_delay = HAL_GetTick();
		}

if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12)&&volt_set>=0.01)
		{
			volt_set=volt_set-0.01;
			PWM_OUT = (PWM_UP*volt_set)/read_values[1];
			tick_delay = HAL_GetTick();

		}

			
//ssd1306_UpdateScreen();		
}


void Varivatt()
{
	 if(temp_timestamp>900)
        {
          click=0;
        }
	Draw_Frame2();
	if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0))
		{
			if(FireButton==false){temp_timestamp=0;puffs++;}
			
			Power_click();
			FireButton=true;
			if (read_values[1]>3.30&&low_batt==0)
			{
				if(counterCoil==1)
				{
					//if(FireButton==false){temp_timestamp=0;puffs++;}
          
         // Power_click();
				// FireButton=true;
					noCoil=true;
					
					if(temp_timestamp>setout)
						Out_Time();
						
					else
						{
							timer_akum=0;
							TIM1->CCR1=PWM_OUT;
							tick_delay = HAL_GetTick();
							Print_Sec();
							ssd1306_SetCursor(91,14);
							ssd1306_WriteString(amper_print,Font_7x10,White);
						}
				} else 
						{
							SSD1306_DrawFilledRectangle(1,1,85,30,Black);
							ssd1306_SetCursor(6,9);
							ssd1306_WriteString("NO COIL",Font_11x18,White);
							ssd1306_UpdateScreen();
							FireButton=true;
							tick_delay = HAL_GetTick();
						}
			} else{	SSD1306_DrawFilledRectangle(1,1,85,30,Black);
							ssd1306_SetCursor(0,9);
							ssd1306_WriteString("LOW BATT",Font_11x18,White);
							ssd1306_UpdateScreen();	
							TIM1->CCR1=0;
							low_batt=1;
							noCoil=false;
							//FireButton=false;
							FireButton=true;
              tick_delay = HAL_GetTick();
				
						}
		}
		else {
				TIM1->CCR1=0;
				coil_tik=true;
				noCoil=false;
				FireButton=false;
				SSD1306_DrawFilledRectangle(1,1,85,30,Black);
				ssd1306_SetCursor(91,14);
				ssd1306_WriteString("-.--A",Font_7x10,White);
				if (watt_set>=9.9){
						sprintf(watt2,"%0.1fW",watt_set);
						ssd1306_SetCursor(4,7);
						ssd1306_WriteString(watt2,Font_16x25,White);
						ssd1306_UpdateScreen();}
			else {
						sprintf(watt2," %0.1fW",watt_set);
						ssd1306_SetCursor(4,7);
						ssd1306_WriteString(watt2,Font_16x25,White);
						ssd1306_UpdateScreen();
						}
				
if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14)&&Get_Watt()==1)
		{
			watt_set=watt_set+0.1;
			volt_set_w= sqrt(watt_set*R_vape);
			PWM_OUT = (PWM_UP*volt_set_w)/read_values[1];
			tick_delay = HAL_GetTick();
		}

if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12)&&volt_set_w>=0.2)
		{
			watt_set=watt_set-0.1;
			volt_set_w= sqrt(watt_set*R_vape);
			PWM_OUT = (PWM_UP*volt_set_w)/read_values[1];
			tick_delay = HAL_GetTick();
		}

			
		
			}
//	sprintf(puffs_print,"%d",puffs);
//	ssd1306_SetCursor(91,24);
//	ssd1306_WriteString2(puffs_print,Font_7x9,White);
	


}


void PuffsPrint()
{
	temp_timestamp=0;
  SSD1306_DrawFilledRectangle(0,0,128,22,Black);
  ssd1306_SetCursor(5,1);
	ssd1306_WriteString2("Общее количество",Font_7x9,White);
  ssd1306_SetCursor(40,11);
	ssd1306_WriteString2("затяжек",Font_7x9,White);
  sprintf((char *)puffs_print,"%d",puffs);
  ssd1306_SetCursor(50,34);
  ssd1306_WriteString2(puffs_print,Font_7x9,White);
	ssd1306_SetCursor(20,51);
	ssd1306_WriteString2("Fire - сброс",Font_7x9,White);
	ssd1306_UpdateScreen();
  if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)&&temp_timestamp>100)
  {
    SSD1306_DrawFilledRectangle(0,32,128,10,Black);
    puffs=0;
  }
  	if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13)&&temp_timestamp>100)
    { status=5;
			old_status=status;
		}
}




			
void Set_Out()
{
    SSD1306_DrawFilledRectangle(0,0,128,22,Black);
    ssd1306_SetCursor(5,1);
		ssd1306_WriteString2("Настройка времени",Font_7x9,White);
    ssd1306_SetCursor(20,11);
		ssd1306_WriteString2("работы койла",Font_7x9,White);
	if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14))
		{
			
			setout=setout+100;
					
		}

if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12))
		{
			setout=setout-100;
			
		}
    if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13))
      { status=5;}
if(setout>6000)
		{
     
      ssd1306_SetCursor(42,27);
      ssd1306_WriteString2("Опасно",Font_7x9,White);
      sprintf((char *)setout2,"%.1f Sec ",(float)(setout/1000.0));
      ssd1306_SetCursor(22,42);
      ssd1306_WriteString(setout2,Font_13x19,White);
      ssd1306_UpdateScreen();
					
		}
      else if(setout<=0)
        {
          setout=0;
          ssd1306_SetCursor(22,42);
          ssd1306_WriteString("Off",Font_13x19,White);
          ssd1306_UpdateScreen();
        }
        else
        {
            ssd1306_SetCursor(42,27);
            ssd1306_WriteString2("      ",Font_7x9,White);
            sprintf((char *)setout2,"%.1f Sec ",(float)(setout/1000.0));
						ssd1306_SetCursor(22,42);
						ssd1306_WriteString(setout2,Font_13x19,White);
						ssd1306_UpdateScreen();
        }


	
}


  void Set_Time()
{
  
    SSD1306_DrawFilledRectangle(0,0,128,22,Black);
    ssd1306_SetCursor(5,1);
		ssd1306_WriteString2("Настройка времени",Font_7x9,White);
    ssd1306_SetCursor(5,11);
		ssd1306_WriteString2("Fire - установка",Font_7x9,White);

	if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14)&&SetMin==0)
		{
      setTimeH=setTimeH+1;
			if(setTimeH>23)
        setTimeH=0;
			
					
		}

if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12)&&SetMin==0)
		{
      setTimeH=setTimeH-1;
      if(setTimeH>23)
        setTimeH=23;
			
			
		}
if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14)&&SetMin==1)
  {
    setTimeM=setTimeM+1;
    if(setTimeM>60)
      setTimeM=0;
    
        
  }

if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12)&&SetMin==1)
  {
    setTimeM=setTimeM-1;
    if(setTimeM>60)
      setTimeM=60;
    
    
  }    

if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13))
     {
      SetMin++;
       if(SetMin>=2)
         SetMin=0;
     }

         
    
    
//    
//if(timeout>9)
//				{
//          
//					sprintf((char *)timeout2,"%d",(setTime));
//					ssd1306_SetCursor(50,39);
//					ssd1306_WriteString(timeout2,Font_13x19,White);
//					ssd1306_UpdateScreen();
//				}else 
 if(setTimeH>9)
    {
      sprintf(timeout2,"%d:",(setTimeH));
      ssd1306_SetCursor(32,39);
      ssd1306_WriteString(timeout2,Font_13x19,White);
     // ssd1306_UpdateScreen();
    
    }else 
      {
        sprintf(timeout2,"0%d:",(setTimeH));
        ssd1306_SetCursor(32,39);
        ssd1306_WriteString(timeout2,Font_13x19,White);
        //ssd1306_UpdateScreen();
      }
      
 if(setTimeM>9)
  {
    sprintf(timeoutM,"%d ",(setTimeM));
    ssd1306_SetCursor(72,39);
    ssd1306_WriteString(timeoutM,Font_13x19,White);
    ssd1306_UpdateScreen();
  
  }else 
    {
      sprintf(timeoutM,"0%d ",(setTimeM));
      ssd1306_SetCursor(72,39);
      ssd1306_WriteString(timeoutM,Font_13x19,White);
      ssd1306_UpdateScreen();
    }    
    
if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0))
     {
       SSD1306_DrawFilledRectangle(0,0,128,64,Black);
      // ssd1306_UpdateScreen();
       set_RTC=false;
       setT.Hours = setTimeH;
       setT.Minutes = setTimeM;
       setT.Seconds = 0x00;
       HAL_RTC_SetTime(&hrtc, &setT, RTC_FORMAT_BIN);
       
       status=0;
       old_status=status;
     }
	
}
	
	


void Set_Time_Out()
{
  
    SSD1306_DrawFilledRectangle(0,0,128,22,Black);
    ssd1306_SetCursor(5,1);
		ssd1306_WriteString2("Настройка времени",Font_7x9,White);
    ssd1306_SetCursor(30,11);
		ssd1306_WriteString2("выключения",Font_7x9,White);
	
	if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14))
		{
			
			timeout=timeout+1000;
					
		}

if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12))
		{
			timeout=timeout-1000;
			
		}

if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13))
     { status=5;}
    
    
    
if(timeout>99000)
				{
          
					sprintf((char *)timeout2,"%d",(timeout/1000));
					ssd1306_SetCursor(50,39);
					ssd1306_WriteString(timeout2,Font_13x19,White);
					ssd1306_UpdateScreen();
				}else if(timeout>9000)
					{
						sprintf(timeout2," %d ",(timeout/1000));
						ssd1306_SetCursor(42,39);
						ssd1306_WriteString(timeout2,Font_13x19,White);
						ssd1306_UpdateScreen();
					
					}else 
						{sprintf(timeout2,"  %d ",(timeout/1000));
						ssd1306_SetCursor(42,39);
						ssd1306_WriteString(timeout2,Font_13x19,White);
						ssd1306_UpdateScreen();}

	
}


void Menu_settings()
{

	temp_timestamp=0;
	ssd1306_SetCursor(32,27);
	ssd1306_WriteString2(st_settings[0][m2-1],Font_7x9,White);
	SSD1306_DrawLine(32,38,108,38,White);
	ssd1306_SetCursor(32,39);
	ssd1306_WriteString2(st_settings[0][m2],Font_7x9,Black);
	ssd1306_SetCursor(32,52);
	ssd1306_WriteString2(st_settings[0][m2+1],Font_7x9,White);
	ssd1306_UpdateScreen();
	//tick_delay = HAL_GetTick();
	
	if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12)&&temp_timestamp>100)
		{if (m2>=6)m2=6;else m2++;}

if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14)&&temp_timestamp>100)
		{if(m2<=1)m2=1;else m2--;}

    
    
    
    
    if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13)&&m2==5&&temp_timestamp>100)
			{ 
				SSD1306_DrawFilledRectangle(0,24,128,40,Black);
				ssd1306_UpdateScreen();
       
        status=11;
				old_status=status;
       
			}
		
		if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13)&&m2==6&&temp_timestamp>100)
			{ 
				status=0;
				old_status=status; 
				m=3;
        HAL_Delay(5);
				return;
       
			}
			
			if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13)&&m2==1&&temp_timestamp>100)
			{ 
				SSD1306_DrawFilledRectangle(0,24,128,40,Black);
				ssd1306_UpdateScreen();
				status=6;
				old_status=status;
			}	
				if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13)&&m2==2&&temp_timestamp>100)
			{ 
        SSD1306_DrawFilledRectangle(0,24,128,40,Black);
				ssd1306_UpdateScreen();
				status=7;
				old_status=status;
				
       
			}	
      
      	if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13)&&m2==3&&temp_timestamp>100)
			{ 
        SSD1306_DrawFilledRectangle(0,24,128,40,Black);
				ssd1306_UpdateScreen();
       
        status=8;
				old_status=status;
        
			}
      

      
			
			if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13)&&m2==4)
			{ 
				volt_set=2.2;
				watt_set=21.0;
				puffs=0;
				timeout=160000;
				status=1;
				old_status=1;
        setout=3500;
				Power_off();
			}	

}

					

void Menu()
{	
	temp_timestamp=0;
	//ssd1306_UpdateScreen();
	ssd1306_SetCursor(32,27);
	ssd1306_WriteString2(st[0][m-1],Font_7x9,White);
	SSD1306_DrawLine(32,38,108,38,White);
	ssd1306_SetCursor(32,39);
	ssd1306_WriteString2(st[0][m],Font_7x9,Black);
	ssd1306_SetCursor(32,52);
	ssd1306_WriteString2(st[0][m+1],Font_7x9,White);
	ssd1306_UpdateScreen();
	tick_delay = HAL_GetTick();
	
	if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12)&&temp_timestamp>100)
		{if (m>=4)m=4;else m++;}

if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14)&&temp_timestamp>100)
		{if(m<=1)m=1;else m--;}
		
if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13)&&m==1&&temp_timestamp>100)
{ status=1; 
	old_status=status; 
	SSD1306_DrawFilledRectangle(0,0,128,64,Black);
	Draw_Frame2();
	ssd1306_UpdateScreen();
	Draw_Acumulator();
	temp_tik=0;
	//SSD1306_DrawFilledRectangle(0,0,128,64,Black);
}	
if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13)&&m==2&&temp_timestamp>100)
{ 
	status=2; 
	old_status=status;
	SSD1306_DrawFilledRectangle(0,0,128,64,Black&&temp_timestamp>100);
	Draw_Frame2();
	ssd1306_UpdateScreen();
	Draw_Acumulator();
	temp_tik=0;}		

if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13)&&m==3&&temp_timestamp>100)
{
	status=5;
	old_status=status;
	m2=1;
	SSD1306_DrawFilledRectangle(0,24,128,40,Black&&temp_timestamp>100);
	ssd1306_UpdateScreen();
	Menu_settings();
	
	//Read_Om();
	
}

if (!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13)&&m==4&&temp_timestamp>100)
{
	//Power_off();
	status=9;
	old_status=status;
	Info();
}



if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0))
{
	
	//Read_Om();
//	SSD1306_ON();
	//HAL_PWR_DisableSEVOnPend();
}




}



void PrintInfo()
{
	SSD1306_DrawFilledRectangle(1,1,125,61,Black);
	Draw_Frame2();
	
}

void Print_Om()
{
	ssd1306_SetCursor(91,3);
	ssd1306_WriteString2(R_vape2,Font_7x9,White);
	//ssd1306_UpdateScreen();
}

void Read_Om_t()
	
{
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_9,GPIO_PIN_SET);
	Read_ADC();
	if (read_om==false)
	{
		read_om=true;
		temp_timestamp=0;
	}
	if (temp_timestamp<350)
	{
		//Read_ADC();
			if(read_values[0]>0.5)
				{
					R_buff = (read_values[1]/read_values[0])-1;
					R_vape = 33.0*R_buff;
				}
	}
	else
	{
		counterCoil=1;
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_9,GPIO_PIN_RESET);
	}
	
	
	
	
//	if(read_values[0]>1.0){
//	for(uint8_t i=0;i<85;i++)
//	{
//		Read_ADC();
//		//ssd1306_UpdateScreen();
//		R_buff = (read_values[1]/read_values[0])-1;
//	//	HAL_Delay(1);
//	}
//	//R_buff = (((read_values[1])+0.01)/(read_values[0])+0.01)-1;
//	}
//	R_vape = 22.0*R_buff;
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_9,GPIO_PIN_RESET);
	sprintf(R_vape2,"%0.2f@",R_vape);
	Print_Om();
}



void NoCoil()
{
	ssd1306_SetCursor(91,3);
	ssd1306_WriteString2("-.--@",Font_7x9,White);
	//ssd1306_UpdateScreen();
}


void Print_Acum()
{
	if (timer_akum>1000)
	{
		timer_akum=0;
	bool readACUM=true;
	if (!HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)){
	
	
	
		SSD1306_DrawFilledRectangle(5,38,71,20, White);
	if (read_values[1]<3.45&&readACUM==true)
	{
		SSD1306_DrawFilledRectangle(5,38,70,20, Black);
		readACUM=false;
	}
	if (read_values[1]<3.50&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(6,38,70,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.55&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(7,38,70,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.58&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(8,38,69,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.59&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(10,38,67,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.60&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(14,38,63,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.63&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(19,38,58,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.70&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(29,38,48,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.75&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(39,38,38,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.80&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(48,38,28,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.90&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(55,38,22,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.98&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(66,38,10,20, Black);
			readACUM=false;
	}
	if (read_values[1]<4.00&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(69,38,7,20, Black);
			readACUM=false;
	}
	if (read_values[1]<4.05&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(72,38,4,20, Black);
			readACUM=false;
	}
	if (read_values[1]<4.10&&readACUM==true)
	{
			//SSD1306_DrawFilledRectangle(76,38,0,20, Black);
			readACUM=false;
	}
	}
}
}


void Print_Acum_charge()
{
	bool readACUM=true;

	
		SSD1306_DrawFilledRectangle(25,22,71,20, White);
		
		if (read_values[1]<3.46&&readACUM==true)
	{
		SSD1306_DrawFilledRectangle(27,22,70,20, Black);
		readACUM=false;
	}
		if (read_values[1]<3.47&&readACUM==true)
	{
		SSD1306_DrawFilledRectangle(28,22,69,20, Black);
		readACUM=false;
	}
		if (read_values[1]<3.48&&readACUM==true)
	{
		SSD1306_DrawFilledRectangle(29,22,68,20, Black);
		readACUM=false;
	}
		if (read_values[1]<3.49&&readACUM==true)
	{
		SSD1306_DrawFilledRectangle(30,22,67,20, Black);
		readACUM=false;
	}
	if (read_values[1]<3.50&&readACUM==true)
	{
		SSD1306_DrawFilledRectangle(31,22,66,20, Black);
		readACUM=false;
	}
	if (read_values[1]<3.51&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(32,22,65,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.52&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(33,22,64,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.53&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(34,22,63,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.54&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(35,22,62,20, Black);
			readACUM=false;
	}
		if (read_values[1]<3.55&&readACUM==true)
	{
		SSD1306_DrawFilledRectangle(36,22,61,20, Black);
		readACUM=false;
	}
		if (read_values[1]<3.56&&readACUM==true)
	{
		SSD1306_DrawFilledRectangle(37,22,60,20, Black);
		readACUM=false;
	}
		if (read_values[1]<3.57&&readACUM==true)
	{
		SSD1306_DrawFilledRectangle(38,22,59,20, Black);
		readACUM=false;
	}
		if (read_values[1]<3.58&&readACUM==true)
	{
		SSD1306_DrawFilledRectangle(39,22,58,20, Black);
		readACUM=false;
	}
		if (read_values[1]<3.59&&readACUM==true)
	{
		SSD1306_DrawFilledRectangle(40,22,57,20, Black);
		readACUM=false;
	}
	if (read_values[1]<3.60&&readACUM==true)
	{
		SSD1306_DrawFilledRectangle(41,22,56,20, Black);
		readACUM=false;
	}
	if (read_values[1]<3.61&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(42,22,55,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.62&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(43,22,54,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.63&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(44,22,53,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.64&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(45,22,52,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.65&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(46,22,51,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.66&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(47,22,50,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.67&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(48,22,49,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.68&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(49,22,48,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.69&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(50,22,47,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.70&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(51,22,46,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.71&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(52,22,45,20, Black);
			readACUM=false;
	}
	
	if (read_values[1]<3.72&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(53,22,44,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.73&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(54,22,43,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.74&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(55,22,42,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.75&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(56,22,41,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.76&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(57,22,40,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.77&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(58,22,39,20, Black);
			readACUM=false;
	}

	if (read_values[1]<3.78&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(59,22,38,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.79&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(60,22,37,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.80&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(61,22,36,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.81&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(62,22,35,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.82&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(63,22,34,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.83&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(64,22,33,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.84&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(65,22,32,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.85&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(66,22,31,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.86&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(67,22,30,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.87&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(68,22,29,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.88&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(69,22,28,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.89&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(70,22,27,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.90&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(71,22,26,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.91&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(72,22,25,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.92&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(73,22,24,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.93&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(74,22,23,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.94&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(75,22,22,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.95&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(76,22,21,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.96&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(77,22,20,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.97&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(78,22,19,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.98&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(79,22,18,20, Black);
			readACUM=false;
	}
	if (read_values[1]<3.99&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(80,22,17,20, Black);
			readACUM=false;
	}
	if (read_values[1]<4.00&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(81,22,16,20, Black);
			readACUM=false;
	}
	if (read_values[1]<4.01&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(82,22,15,20, Black);
			readACUM=false;
	}
	if (read_values[1]<4.02&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(83,22,14,20, Black);
			readACUM=false;
	}
	if (read_values[1]<4.03&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(84,22,13,20, Black);
			readACUM=false;
	}
	if (read_values[1]<4.04&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(85,22,12,20, Black);
			readACUM=false;
	}
	if (read_values[1]<4.05&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(86,22,11,20, Black);
			readACUM=false;
	}
	if (read_values[1]<4.06&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(87,22,10,20, Black);
			readACUM=false;
	}
	if (read_values[1]<4.07&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(88,22,9,20, Black);
			readACUM=false;
	}
	if (read_values[1]<4.08&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(89,22,8,20, Black);
			readACUM=false;
	}
	if (read_values[1]<4.09&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(90,22,7,20, Black);
			readACUM=false;
	}
	if (read_values[1]<4.10&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(91,22,6,20, Black);
			readACUM=false;
	}
	if (read_values[1]<4.11&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(92,22,5,20, Black);
			readACUM=false;
	}
	if (read_values[1]<4.12&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(93,22,4,20, Black);
			readACUM=false;
	}
	if (read_values[1]<4.13&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(94,22,3,20, Black);
			readACUM=false;
	}
	if (read_values[1]<4.14&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(95,22,2,20, Black);
			readACUM=false;
	}
	if (read_values[1]<4.15&&readACUM==true)
	{
			SSD1306_DrawFilledRectangle(96,22,1,20, Black);
			readACUM=false;
	}
	
}




void Charge()
{
	
	SSD1306_DrawLine(24,20,97,20,White);
	//SSD1306_DrawLine(5,35,46,35,White);
	SSD1306_DrawLine(24,44,97,44,White);
	SSD1306_DrawLine(23, 21, 23, 43,White);
	SSD1306_DrawLine(98, 21, 98, 43,White);
	SSD1306_DrawLine(99, 25, 99, 39,White);
	SSD1306_DrawLine(100, 25, 100, 39,White);
	SSD1306_DrawLine(101, 26, 101, 38,White);
	//ssd1306_SetCursor(4,4);
	//ssd1306_WriteString("charging",Font_11x18,White);
	//ssd1306_UpdateScreen();
	
//	SSD1306_DrawLine(4,36,77,36,White);
//	//SSD1306_DrawLine(5,35,46,35,White);
//	SSD1306_DrawLine(4,60,77,60,White);
//	SSD1306_DrawLine(3, 37, 3, 59,White);
//	SSD1306_DrawLine(78, 37, 78, 59,White);
//	SSD1306_DrawLine(79, 41, 79, 55,White);
//	SSD1306_DrawLine(80, 41, 80, 55,White);
//	SSD1306_DrawLine(81, 42, 81, 54,White);
	Print_Acum_charge();
	sprintf(vOut2,"%.2fV",read_values[1]);
					
		
					ssd1306_SetCursor(45,6);
					ssd1306_WriteString(vOut2,Font_7x10,White);
	//Read_sensor_charge();
	ssd1306_UpdateScreen();
	clear = true;
	low_batt=0;
	
	
}



void Timer_off()
{
	//tick_delay = HAL_GetTick();
	
	if((HAL_GetTick()-tick_delay)>=timeout)
	{
		
		Power_off();
	}
	
}




void Low_batt()
{
	
	


}

void Counter_Fire()
{
	if (FireButton==true&&coil_tik==true)
	{
	//	puffs++;
		coil_tik=false;
	}
}

uint8_t PowerOn2()
{
//      char print2[2]={" "};
      //SSD1306_DrawFilledRectangle(0,0,128,64,Black);
//		//	ssd1306_UpdateScreen();
			ssd1306_SetCursor(12,27);
//			ssd1306_WriteString2("НАЖМИ ТРИ РАЗА",Font_7x9,White);
//      sprintf(print2,"%.d",click);
//			ssd1306_SetCursor(55,30);
//			ssd1306_WriteString(print2,Font_14x22,White);
//			ssd1306_UpdateScreen();
  
//     if (click==0)
//       ssd1306_WriteString2("НАЖМИ ТРИ РАЗА",Font_7x9,White);
//      if (click==1)
//        ssd1306_WriteString2("НАЖМИ ДВА РАЗА",Font_7x9,White);
//      if (click==2)
//        ssd1306_WriteString2("НАЖМИ ОДИН РАЗ",Font_7x9,White);
	if (click==0)
       ssd1306_WriteString2(" ",Font_7x9,White);
      if (click==1)
        ssd1306_WriteString2(" ",Font_7x9,White);
      if (click==2)
        ssd1306_WriteString2(" ",Font_7x9,White);
      
      ssd1306_UpdateScreen();  
      powercount++;
  
if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)&&FireButton==false)
      { 
        FireButton=true;
         click++;
        powercount=0;
        temp_timestamp=0;
       // HAL_Delay(100);
        if(click>2)
        {
            click=0;
            return 0;
        }
       
      }
if (powercount>1&&!HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)) FireButton=false;    
if (powercount>15)
    {
      click=0;
      powercount=0;
//      ssd1306_SetCursor(40,16);
//      ssd1306_WriteString2("БЫСТРО",Font_7x9,White);
    }      

  
    
if (temp_timestamp>10000)
       Power_off2();

//  return 1;

//HAL_Delay(100);
return 1;
}





void Screensaver()
{
  
	HAL_RTC_GetTime(&hrtc, &stimestructureget,  RTC_FORMAT_BCD);
	sprintf((char *)showtime, "%02X:%02X", stimestructureget.Hours, stimestructureget.Minutes);
  sprintf((char *)showsec, "%02X", stimestructureget.Seconds);
	SSD1306_DrawFilledRectangle(0,0,128,64,Black);
	ssd1306_SetCursor(25,23);
  ssd1306_WriteString(showtime,Font_16x26,White);
  ssd1306_SetCursor(106,36);
  ssd1306_WriteString(showsec,Font_7x10,White);
  
  uint8_t tempSecond = stimestructureget.Seconds % 2;
   if (tempSecond==0){
    SSD1306_DrawFilledRectangle(57,23,11,26,Black);
    clok = temp_timestamp;
    ssd1306_UpdateScreen();
    }
	if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0))
		{
			status=status_timer;
			old_status=status_timer;
			SSD1306_DrawFilledRectangle(0,0,128,64,Black);
			tick_delay=0;
      timer_akum=6000;
		}



}

void Timer_screensaver()
{
	//tick_delay = HAL_GetTick();
	
	if((HAL_GetTick()-tick_delay)>=30000)
	{
		
		status_timer=status;
		status=10;
		old_status=status; 
		
		Screensaver();
	}
	
}
