#ifndef _TIMER_ISR_H
#define _TIMER_ISR_H

void rs422aPack();
void rs422bPack();
void rs422aTx();
void rs422bTx();


#define MAXQSIZE 128
typedef struct _RS422TX{
	char txBuff[MAXQSIZE];
	int front;
	int rear;
}RS422TX;

typedef struct _RS422TXDATA{
	int front;
}RS422TXDATA;


void Timer0_ISR_Thread(void);
void Timer1_ISR_Thread(void);
#endif
