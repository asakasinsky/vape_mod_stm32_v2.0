

 
 #ifndef __VAPE_H
 #define __VAPE_H
 #include "stm32f1xx_hal.h"
 #include <stdbool.h>
void Kalman(void);
void Info(void);
void Power_click(void);
void PuffsPrint(void);
uint8_t PowerOn2(void);
uint8_t PowerOn(void);	
void Draw_Frame(void);
void Draw_Frame2(void);
void Draw_Acumulator(void);
void Read_ADC(void);
void Read_temperature(void);
//bool Get_Volt();
void Read_Amp(void);
void Varivolt(void);
void Varivatt(void);
void Set_Out(void);
void Set_Time_Out(void);
void Menu_settings(void);
void Menu(void);
void Read_Om_t(void);
void NoCoil(void);
void Print_Om(void);
void Print_Acum(void);
void Print_Acum_charge(void);
void Charge(void);
void PrintInfo(void);
void Power_off(void);
void Power_off2(void);
void Timer_off(void);
void Counter_Fire(void);







 #endif

