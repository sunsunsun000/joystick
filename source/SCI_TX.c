#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"   // DSP2833x Examples Include File
#include "public.h"
#include "GlobalVarAndFunc.h"
#include "SCI_TX.h"
#include <stdio.h>
#include "ADprocessor.h"
#include "PWM_ISR.h"

GRX422TX gRx422TxVar[TOTAL_TX_VAR] = {0};
Uint16 gRx422TxEnableFlag[TOTAL_TX_VAR] = {0};
RS422TXQUE gRS422TxQue = {0};
#define S (0)


/***************************************************************
 *Name:						RX422TXEnQueue
 *Function:
 *Input:				    char e, come from tx queue
 *Output:					1 or 0, 1 means success, 0 means the queue is full already
 *Author:					Simon
 *Date:						2018.10.21
 ****************************************************************/
int RX422TXEnQueue(char e){
	if((gRS422TxQue.rear + 1) % TXMAXQSIZE == gRS422TxQue.front){
		asm ("      ESTOP0");
		return 0;
	}

	gRS422TxQue.txBuf[gRS422TxQue.rear] = e;
	gRS422TxQue.rear = (gRS422TxQue.rear + 1) % TXMAXQSIZE;
	return 1;
}
/***************************************************************
 *Name:						RX422TXDeQueue
 *Function:
 *Input:				    none
 *Output:					1 or 0, 1 means success, 0 means the queue is empty already
 *Author:					Simon
 *Date:						2018.10.21
 ****************************************************************/
int RX422TXDeQueue(void){
	if(gRS422TxQue.front == gRS422TxQue.rear){
		return 0;
	}

	gRS422TxQue.front = (gRS422TxQue.front + 1) % TXMAXQSIZE;
	return 1;
}
/***************************************************************
 *Name:						RS422RxQueLength
 *Function:
 *Input:				    none
 *Output:					none
 *Author:					Simon
 *Date:						2018.10.21
 ****************************************************************/
int RS422TxQueLength(){
	int length;
	length = (gRS422TxQue.rear - gRS422TxQue.front + TXMAXQSIZE) % TXMAXQSIZE;
	return length;
}
/***************************************************************
 *Name:						CalCrc
 *Function:
 *Input:				    rs422 rx data
 *Output:					int, should be zero
 *Author:					Simon
 *Date:						2018.10.21
 ****************************************************************/
int calCrc(int crc, const char *buf, int len) {
	int x;
	int i;

	for(i = 0; i < len; ++i){
		x = ((crc >> 8) ^ buf[i]) & 0xff;
		x ^= x >> 4;
		crc = (crc << 8) ^ (x  << 12) ^ (x << 5) ^ x;
		crc &= 0xffff;
	}
	return crc;
}
/**************************************************************
 *Name:		   updateTxEnableFlag
 *Comment:
 *Input:	   void
 *Output:	   void
 *Author:	   Simon
 *Date:		   2018.11.14
 **************************************************************/
void updateTxEnableFlag(void) {
	int i;
	for (i = 0; i < TOTAL_TX_VAR; ++i) {
		gRx422TxVar[i].isTx = gRx422TxEnableFlag[i];
	}
}

/***************************************************************
 *Name:						PackRS422TxData
 *Function:					pack the data that need to be sent
 *Input:				    none
 *Output:					none
 *Author:					Simon
 *Date:						2018.10.21
 ****************************************************************/
void PackRS422TxData(void){
	//TODO need do some test, because we sync the tx enable flag here
	int i;
	char crcl;
	char crch;
	static unsigned char count = 0;
	static int crc = 0;
	char tmp[3] = {0};
	int lenPosition = 0;
	Uint16 total =0;
	static Uint16 testdata = 0;
	static Uint16 alcount = 0;

	if(testdata >= 150){
		//testdata = 0;
		return;
	}

	if(count == 0){
		if(RX422TXEnQueue(0x5a) == 0){
			asm ("      ESTOP0");
			return;
		}
		if(RX422TXEnQueue(0x5a) == 0){
			asm ("      ESTOP0");
			return;
		}
		lenPosition = gRS422TxQue.rear;
		if(RX422TXEnQueue(0x05) == 0){
			asm ("      ESTOP0");
			return;
		}
//		if(RX422TXEnQueue(0xff) == 0){
//			asm ("      ESTOP0");
//			return;
//		}
//		if(RX422TXEnQueue(0xff) == 0){
//			asm ("      ESTOP0");
//			return;
//		}
		updateTxEnableFlag();
	}

	for(i = 0; i < TOTAL_TX_VAR; ++i){
		if(gRx422TxVar[i].isTx){
			++total;
			if(i == 0){
				//gRx422TxVar[i].value =(int16)(gKeyValue.displacement * 100);
				//gRx422TxVar[i].value = gSysMonitorVar.anolog.single.var[DisplacementValue].value;
				gRx422TxVar[i].value = test_data[testdata];

//				if(testdata >= 150){
//					//testdata = 0;
//					return;
//				}
			}
			else if(i == 1){
				//gRx422TxVar[i].value =(int16)(gKeyValue.motorAccel * 100);
				//gRx422TxVar[i].value =(int16)(gKeyValue.displacement * 100);
//				gRx422TxVar[i].value = al[alcount];
//				if(testdata % 10 == 0)
//				{
//					++alcount;
//				}
				gRx422TxVar[i].value = al[testdata];
				++testdata;
			}
			else if(i == 2){
				gRx422TxVar[i].value =(int16)(gKeyValue.motorSpeed * 10000);
			}
			else if(i == 3){
				gRx422TxVar[i].value =(int16)(gKeyValue.motorAccel * 100);
			}

			tmp[0] = gRx422TxVar[i].index;
			tmp[1] = gRx422TxVar[i].value >> 8;
			tmp[2] = gRx422TxVar[i].value;
			if(RX422TXEnQueue(gRx422TxVar[i].index) == 0){
				asm ("      ESTOP0");
				return;
			}
			if(RX422TXEnQueue(gRx422TxVar[i].value >> 8) == 0){
				asm ("      ESTOP0");
				return;
			}
			if(RX422TXEnQueue(gRx422TxVar[i].value) == 0){
				asm ("      ESTOP0");
				return;
			}
			crc = calCrc(crc, tmp, 3);
		}

		//错误信息，报警信息一直发送
	}

	if(count == 0){
		gRS422TxQue.txBuf[lenPosition] = total * (S + 1);//timer0 interrupt isr can not be interrupted by TX, so we can set length value here
	}

	++count;

	if(count > S){

		crcl = (char)crc;
		crch = (char)(crc >> 8);
		crc = 0;
		count = 0;
		if(RX422TXEnQueue(crch) == 0){
			asm ("      ESTOP0");
			return;
		}
		if(RX422TXEnQueue(crcl) == 0){
			asm ("      ESTOP0");
			return;
		}
		if(RX422TXEnQueue(0xa5) == 0){
			asm ("      ESTOP0");
			return;
		}
		if(RX422TXEnQueue(0xa5) == 0){
			asm ("      ESTOP0");
			return;
		}
	}
}
/**************************************************************
 *Name:		   ScibTxByte
 *Comment:
 *Input:	   one byte to send by sci B
 *Output:	   none
 *Author:	   Simon
 *Date:		   2018.11.14
 **************************************************************/
void ScibTxByte(Uint16 t){

	ScibRegs.SCITXBUF = t;

}
/**************************************************************
 *Name:		   ScicTxByte
 *Comment:
 *Input:	   one byte to send by sci C
 *Output:	   none
 *Author:	   Simon
 *Date:		   2018.11.14
 **************************************************************/
void ScicTxByte(Uint16 t){

	ScicRegs.SCITXBUF = t;

}
/**************************************************************
 *Name:		   DisableScicTxInterrupt
 *Comment:
 *Input:	   void
 *Output:	   void
 *Author:	   Simon
 *Date:		   2018.11.14
 **************************************************************/
void DisableScicTxInterrupt(void){

	ScicRegs.SCIFFTX.bit.TXFFIENA = 0;

}
/***************************************************************
 *Name:						rs422 tx interrupt isr
 *Function:			  		none
 *Input:				  	none
 *Output:					none
 *Author:					Simon
 *Date:						2018.11.3
 ****************************************************************/
void RS422A_Transmit(void){

	if(gRS422TxQue.front == gRS422TxQue.rear){
		DisableScicTxInterrupt();//disable the tx interrupt when tx fifo empty
		return;
	}

	//while((ScicRegs.SCIFFTX.bit.TXFFST != 16)
	//			&& (ScibRegs.SCIFFTX.bit.TXFFST != 16)){
	while((ScibRegs.SCIFFTX.bit.TXFFST != 16)){
		if(RS422TxQueLength() == 0)
		{
			return;
		}
		ScibTxByte(gRS422TxQue.txBuf[gRS422TxQue.front]);//printf by Scic
		//ScicTxByte(gRS422TxQue.txBuf[gRS422TxQue.front]);

		if(RX422TXDeQueue() == 0){
			DisableScicTxInterrupt();
			return;
		}
	}
}
/**************************************************************
 *Name:		   TransmitRS422ShakeHandMsg
 *Comment:
 *Input:	   void
 *Output:	   void
 *Author:	   Simon
 *Date:		   2018年11月15日下午9:02:53
 **************************************************************/
void TransmitRS422ShakeHandMsg(void){

	int len;
	int index;
	char rs422ShakeHandMsg[] = {
		//TODO use the real shake hand msg in the future
		0x5a,//head1
		0x5a,//head2
		0x01,//length
		0x00,//serial number
		0x00,//serial number
		0xff,//shake hand
		0xff,//shake hand
		0xff,//shake hand
		0x00,//crc1
		0x00,//crc2
		0xa5,//tail1
		0xa5 //tail2
	};

	len = sizeof(rs422ShakeHandMsg);

	for(index = 0; index < len; ++index){
		while(ScibRegs.SCIFFTX.bit.TXFFST != 0){

		}
		ScibRegs.SCITXBUF = rs422ShakeHandMsg[index];
	}
}
/**************************************************************
 *Name:		   ShakeHandWithUpperComputer
 *Comment:
 *Input:	   void
 *Output:	   void
 *Author:	   Simon
 *Date:		   2018年11月15日下午9:03:08
 **************************************************************/
void ShakeHandWithUpperComputer(void){

	if(FAIL == gRS422Status.shakeHand){
		TransmitRS422ShakeHandMsg();
	}
}
