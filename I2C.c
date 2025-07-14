#include <msp430.h>

#define SLAVE_ADDR 0x50       //I2C SLAVE的地址

unsigned char TXdata = 0x3A;  //要寫入的資料
unsigned char RXdata = 0;

int Write(unsigned char reg, unsigned char data);
int Read(unsigned char reg, unsigned char *data);

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
	P1DIR |= BIT1;
	P1OUT &= ~BIT1; 
	
	
    __delay_cycles(1000);

    P1SEL |= BIT6 + BIT7;
    P1SEL2 |= BIT6 + BIT7;

    UCB0CTL1 |= UCSWRST;	//RESET MODE
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC; // 設此為MASTER並使用 I2C
    UCB0CTL1 = UCSSEL_2 + UCSWRST;	//使用SMCLK 
    UCB0BR0 = 10;
    UCB0BR1 = 0;
    UCB0I2CSA = SLAVE_ADDR;
    UCB0CTL1 &= ~UCSWRST;	//離開RESET MODE 
    IE2 &= ~(UCB0RXIE + UCB0TXIE);	//不使用中斷 
    
    if (Write(0x00, TXdata) == 0)
    {
        __delay_cycles(1000);
        if (Read(0x00, &RXdata) == 0)
        {
            if(TXdata == RXdata)
            P1OUT |= BIT1;
            else
            P1OUT &= ~BIT1;
        }
    }

    while (1);
}



int Write(unsigned char reg, unsigned char data)
{
    while (UCB0CTL1 & UCTXSTP);		//確認BUS是空閒的 

    UCB0CTL1 |= UCTR + UCTXSTT;		//UCTR 為傳送模式, UCTXSTT傳送START傳完自動消除 
    while (UCB0CTL1 & UCTXSTT);		//確定START傳完 

    if (UCB0STAT & UCNACKIFG) goto error;	//檢查ACK 

    while (!(IFG2 & UCB0TXIFG));	//等待BUFFER準備 
    UCB0TXBUF = reg;
    while (!(IFG2 & UCB0TXIFG));	//等待傳送完一筆資料
    if (UCB0STAT & UCNACKIFG) goto error;
    UCB0TXBUF = data;
    
    while (!(IFG2 & UCB0TXIFG));
    if (UCB0STAT & UCNACKIFG) goto error;

    UCB0CTL1 |= UCTXSTP;
    while (UCB0CTL1 & UCTXSTP);
    return 0;

error:
    UCB0CTL1 |= UCTXSTP;
    while (UCB0CTL1 & UCTXSTP);
    return -1;
}


int Read(unsigned char reg, unsigned char *data)
{
    while (UCB0CTL1 & UCTXSTP);

    UCB0CTL1 |= UCTR + UCTXSTT;		//傳送模式並傳送START 
    while (UCB0CTL1 & UCTXSTT);
    if (UCB0STAT & UCNACKIFG) goto error;

    while (!(IFG2 & UCB0TXIFG));
    UCB0TXBUF = reg;
    while (!(IFG2 & UCB0TXIFG));
    if (UCB0STAT & UCNACKIFG) goto error;

    
    UCB0CTL1 &= ~UCTR;		//切換成讀取模式 
    UCB0CTL1 |= UCTXSTT;
    while (UCB0CTL1 & UCTXSTT);
    if (UCB0STAT & UCNACKIFG) goto error;

    UCB0CTL1 |= UCTXSTP;	//讀取最後一個BYTE前要先宣告STOP 

    while (!(IFG2 & UCB0RXIFG));
    *data = UCB0RXBUF;

    while (UCB0CTL1 & UCTXSTP);
    return 0;

error:
    UCB0CTL1 |= UCTXSTP;
    while (UCB0CTL1 & UCTXSTP);
    return -1;
}

