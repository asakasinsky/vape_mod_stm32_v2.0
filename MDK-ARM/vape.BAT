SET PATH=C:\Keil_v5\ARM\ARMCC\Bin;C:\ProgramData\Oracle\Java\javapath;C:\Program Files (x86)\NVIDIA Corporation\PhysX\Common;C:\Program Files (x86)\Intel\iCLS Client\;C:\Program Files\Intel\iCLS Client\;C:\WINDOWS\system32;C:\WINDOWS;C:\WINDOWS\System32\Wbem;C:\WINDOWS\System32\WindowsPowerShell\v1.0\;C:\Program Files\Intel\Intel(R) Management Engine Components\DAL;C:\Program Files (x86)\Intel\Intel(R) Management Engine Components\DAL;C:\Program Files\Intel\Intel(R) Management Engine Components\IPT;C:\Program Files (x86)\Intel\Intel(R) Management Engine Components\IPT;C:\Program Files\Microsoft SQL Server\Client SDK\ODBC\110\Tools\Binn\;C:\Program Files (x86)\Microsoft SQL Server\120\Tools\Binn\;C:\Program Files\Microsoft SQL Server\120\Tools\Binn\;C:\Program Files\Microsoft SQL Server\120\DTS\Binn\;C:\Program Files (x86)\Windows Kits\8.1\Windows Performance Toolkit\;C:\WINDOWS\system32\config\systemprofile\.dnx\bin;C:\Program Files\Microsoft DNX\Dnvm\;C:\Program Files (x86)\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility;C:\Users\Виталий Цветаев\AppData\Local\Microsoft\WindowsApps;
SET CPU_TYPE=STM32F103CB
SET CPU_VENDOR=STMicroelectronics
SET UV2_TARGET=vape
SET CPU_CLOCK=0x044AA200
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via "vape\system_stm32f1xx.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmAsm" --Via "vape\startup_stm32f103xb._ia"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via "vape\stm32f1xx_it.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via "vape\main.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via "vape\stm32f1xx_hal_msp.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via "vape\fonts.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via "vape\ssd1306.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmCC" --Via "vape\vape.__i"
"C:\Keil_v5\ARM\ARMCC\Bin\ArmLink" --Via "vape\vape.lnp"
