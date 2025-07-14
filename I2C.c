#include <msp430.h>

#define SLAVE_ADDR 0x50       //I2C SLAVE���a�}

unsigned char TXdata = 0x3A;  //�n�g�J�����
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
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC; // �]����MASTER�èϥ� I2C
    UCB0CTL1 = UCSSEL_2 + UCSWRST;	//�ϥ�SMCLK 
    UCB0BR0 = 10;
    UCB0BR1 = 0;
    UCB0I2CSA = SLAVE_ADDR;
    UCB0CTL1 &= ~UCSWRST;	//���}RESET MODE 
    IE2 &= ~(UCB0RXIE + UCB0TXIE);	//���ϥΤ��_ 
    
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
    while (UCB0CTL1 & UCTXSTP);		//�T�{BUS�O�Ŷ��� 

    UCB0CTL1 |= UCTR + UCTXSTT;		//UCTR ���ǰe�Ҧ�, UCTXSTT�ǰeSTART�ǧ��۰ʮ��� 
    while (UCB0CTL1 & UCTXSTT);		//�T�wSTART�ǧ� 

    if (UCB0STAT & UCNACKIFG) goto error;	//�ˬdACK 

    while (!(IFG2 & UCB0TXIFG));	//����BUFFER�ǳ� 
    UCB0TXBUF = reg;
    while (!(IFG2 & UCB0TXIFG));	//���ݶǰe���@�����
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

    UCB0CTL1 |= UCTR + UCTXSTT;		//�ǰe�Ҧ��öǰeSTART 
    while (UCB0CTL1 & UCTXSTT);
    if (UCB0STAT & UCNACKIFG) goto error;

    while (!(IFG2 & UCB0TXIFG));
    UCB0TXBUF = reg;
    while (!(IFG2 & UCB0TXIFG));
    if (UCB0STAT & UCNACKIFG) goto error;

    
    UCB0CTL1 &= ~UCTR;		//������Ū���Ҧ� 
    UCB0CTL1 |= UCTXSTT;
    while (UCB0CTL1 & UCTXSTT);
    if (UCB0STAT & UCNACKIFG) goto error;

    UCB0CTL1 |= UCTXSTP;	//Ū���̫�@��BYTE�e�n���ŧiSTOP 

    while (!(IFG2 & UCB0RXIFG));
    *data = UCB0RXBUF;

    while (UCB0CTL1 & UCTXSTP);
    return 0;

error:
    UCB0CTL1 |= UCTXSTP;
    while (UCB0CTL1 & UCTXSTP);
    return -1;
}

