#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"   // DSP2833x Examples Include File
#include "Timer_ISR.h"
#include "SCI_TX.h"
#include <stdio.h>

#define N (3)
void Timer0_ISR_Thread(void){

	static unsigned char count = 0;

	int i = 0;
	for(i = 0; i < 100; ++i){
		++i;
	}
	++count;
	if(count > N){
		testrs422tx();
		count = 0;
	}
}


void Timer1_ISR_Thread(void){
	//printf("tiemr0 interrupt function\r\n");
	int i;
	for(i = 0; i < 16; ++i){
		while(ScicRegs.SCIFFTX.bit.TXFFST != 0){

		}
		ScicRegs.SCITXBUF = Rx4225TxBuf[i];

	}

}
