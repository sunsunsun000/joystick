#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"   // DSP2833x Examples Include File
#include "public.h"
#include "PWM_ISR.h"
#include "ADprocessor.h"
#include "Filter_Alg.h"
#include "SPIprocess.h"

KeyValue gKeyValue = {0};
FeedbackVarBuf feedbackVarBuf;
void ForceAndDisplaceProcess(int count);

/**************************************************************
 *Name:						CalForceSpeedAccel
 *Function:
 *Input:					none
 *Output:					none
 *Author:					Simon
 *Date:						2018.10.28
 **************************************************************/
void CalForceSpeedAccel(void) {
	static int count = 0;

	ForceAndDisplaceProcess(count);
	//CalFuncPara(gSysMonitorVar.anolog.single.var[DisplacementValue].value,count);

	CalFuncPara(feedbackVarBuf.displacementbuf[count], count);

	count++;

	if(count >= 10){
		gKeyValue.motorSpeed = 2*funcPara.a*11 + funcPara.b;
		gKeyValue.motorAccel = 2*funcPara.a;
		count = 0;
	}
}
/**************************************************************
 *Name:						ReadHallValue
 *Function:
 *Input:					none
 *Output:					none
 *Author:					Simon
 *Date:						2018.10.31
 **************************************************************/
Uint16 ReadHallValue(void){

	Uint16 temp;
	Uint16 a;
	Uint16 b;
	Uint16 c;

	c = GpioDataRegs.GPADAT.bit.GPIO27;
	b = GpioDataRegs.GPBDAT.bit.GPIO48;
	a = GpioDataRegs.GPBDAT.bit.GPIO49;

	temp = ((c << 2) + (b << 1) + a)^0x07;

	if(temp < 1 || temp >6){
		//TODO if temp < 1 or >6 means program abnormal, need to do something
	}


	return temp;
}
/**************************************************************
 *Name:						SwitchDirection
 *Function:
 *Input:					none
 *Output:					none
 *Author:					Simon
 *Date:						2018.10.31
 **************************************************************/
void SwitchDirection(void){


}

/**************************************************************
 *Name:						Pwm_ISR_Thread
 *Function:					PWM interrupt function
 *Input:					none
 *Output:					none
 *Author:					Simon
 *Date:						2018.6.10
 **************************************************************/
void Pwm_ISR_Thread(void)
{
	//TODO
	//start spi
	//read 12bit AD value
	//read DI
	//specific channel check
	//换向，占空比输出
	//prepare output
	//read spi value
	/*struct FanKui_OUT {int32 LI, WEIYI , SPEED, ACCEL, LOCK }  LW_BUFF;
	LOCK=0：PWM线程已将数据输出，0.25ms中断尚未复制
	LOCK=1：0.25ms已完成复制
	在PWM中断中，中断第10次准备刷新LW_BUFF时，检查LOCK，若LOCK=1，则复制数据，并将LOCK=0；若LOCK=0，则软件报警（LOCK 001）
	在0.25ms中断中，进入中断判断LOCK，若LOCK=0，则复制数据（将全局变量以形参的形式调用函数，等于将全局变量压入堆栈，被调用的函数第一句话将全局变量的LOCK=1）；若LOCK=1，则报警（LOCK 002）
	 */

	//StartGetADBySpi();
	ReadAnalogValue();
	//ReadDigitalValue();

	if(IsSingleAnalogValueAbnormal() == True){
		//TODO
	}

	//TODO prepare output
	//ReadADBySpi();
	CalForceSpeedAccel();
}
/**************************************************************
 *Name:						forcebufProcess
 *Function:
 *Input:					none
 *Output:					force value
 *Author:					Simon
 *Date:						2018.10.28
 **************************************************************/
int32 forcebufProcess(void)
{
	return ((feedbackVarBuf.sumForce - feedbackVarBuf.maxForce - feedbackVarBuf.minForce) >> 3);
}
/**************************************************************
 *Name:						displacebufProcess
 *Function:
 *Input:					none
 *Output:					displacement value
 *Author:					Simon
 *Date:						2018.10.28
 **************************************************************/
int32 displacebufProcess(void)
{
	return ((feedbackVarBuf.sumDisplacement - feedbackVarBuf.maxDisplacement - feedbackVarBuf.minDisplacement) >> 3);
}
/**************************************************************
 *Name:						UpdateMaxAndMin
 *Function:
 *Input:					feedbackVarBuf
 *Output:					none
 *Author:					Simon
 *Date:						2018.10.28
 **************************************************************/
void UpdateMaxAndMin(FeedbackVarBuf* feedbackVarBuf) {
	if (gSysMonitorVar.anolog.single.var[ForceValue].value
			>= feedbackVarBuf->maxForce) {
		feedbackVarBuf->maxForce =
				gSysMonitorVar.anolog.single.var[ForceValue].value;
	}
	if (gSysMonitorVar.anolog.single.var[ForceValue].value
			<= feedbackVarBuf->minForce) {
		feedbackVarBuf->minForce =
				gSysMonitorVar.anolog.single.var[ForceValue].value;
	}
	if (gSysMonitorVar.anolog.single.var[DisplacementValue].value
			>= feedbackVarBuf->maxDisplacement) {
		feedbackVarBuf->maxDisplacement =
				gSysMonitorVar.anolog.single.var[DisplacementValue].value;
	}
	if (gSysMonitorVar.anolog.single.var[DisplacementValue].value
			<= feedbackVarBuf->minDisplacement) {
		feedbackVarBuf->minDisplacement =
				gSysMonitorVar.anolog.single.var[DisplacementValue].value;
	}
}
/**************************************************************
 *Name:						ForceAndDisplaceProcess
 *Function:					PWM interrupt function
 *Input:					int count
 *Output:					none
 *Author:					Simon
 *Date:						2018.6.10
 **************************************************************/
void ForceAndDisplaceProcess(int count){
	/*
	feedbackVarBuf.forcebuf[count] = gSysMonitorVar.anolog.single.var[ForceValue].value;
	feedbackVarBuf.displacementbuf[count] = gSysMonitorVar.anolog.single.var[DisplacementValue].value;
	*/

	feedbackVarBuf.sumForce = feedbackVarBuf.sumForce + feedbackVarBuf.forcebuf[count];
	feedbackVarBuf.sumDisplacement = feedbackVarBuf.sumDisplacement + feedbackVarBuf.displacementbuf[count];

	//UpdateMaxAndMin(&feedbackVarBuf);
	if(count >= 9){
		gKeyValue.lock = 1;//need to remove after debug
		if(gKeyValue.lock == 0)
		{
			//TODO generate alarm;
			return;
		}
		gKeyValue.displacement = displacebufProcess();
		gKeyValue.force = forcebufProcess();
		gKeyValue.lock = 0;
		feedbackVarBuf.sumForce = 0;
		feedbackVarBuf.sumDisplacement =0;
	}
}

