#include <p18f4620.h>
#include <stdio.h>
#include <math.h>
#include <usart.h>

#pragma config OSC = INTIO67
#pragma config WDT = OFF
#pragma config LVP = OFF
#pragma config BOREN = OFF

#define D1_RED PORTCbits.RC0
#define D1_GREEN PORTCbits.RC1
#define D1_BLUE PORTCbits.RC2

#define D2_RED PORTEbits.RE0
#define D2_GREEN PORTEbits.RE1
#define D2_BLUE PORTEbits.RE2

#define D3_RED PORTCbits.RC3
#define D3_GREEN PORTCbits.RC4
#define D3_BLUE PORTCbits.RC5

#define DECIMAL PORTDbits.RD7 

void Select_ADC_Channel(char channel)
{
    ADCON0 = channel * 4 + 1;
}

unsigned int get_full_ADC(void)
{
    int result;
        ADCON0bits.GO = 1;
        while(ADCON0bits.DONE==1);
        result = (ADRESH * 0x100) + ADRESL;
    return result;
}

void init_ADC(void)
{
    ADCON0 = 0x01;
    ADCON1 = 0x1B;
    ADCON2 = 0xA9;
}

void init_UART()
{
    OpenUSART (USART_TX_INT_OFF & USART_RX_INT_OFF & 
            USART_ASYNCH_MODE & USART_EIGHT_BIT & USART_CONT_RX &
            USART_BRGH_HIGH, 25);
    OSCCON = 0X60;
}

void putch(char c)
{
    while (!TRMT);
    TXREG = c;
}

void init_ADC(void);
unsigned int get_full_ADC(void);
void init_UART(void);
void Select_ADC_Channel(char);
void display_upper(char up);
void display_lower(char low);
int ssegdisplay[] = {0x40,0x79,0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10};
char array_with_dp[10] = {0x40,0x79,0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10};
float calcresistance (float volt);
void wait_one_sec(void);
void display_D1(int resistin);
void display_D2(int r);
void display_lower_dp(char digit);
void display_upper_dp(char digit);
void Activate_Buzzer(void);
void Deactivate_Buzzer(void);

void set_d2_off(void);                                         //
void set_d2_RED(void);                                         //
void set_d2_GREEN(void);                                       //
void set_d2_YELLOW(void);                                      //
void set_d2_BLUE(void);                                        //                                                                            //
void set_d2_PURPLE(void);                                      //
void set_d2_CYAN(void);                                        //
void set_d2_WHITE(void);                                       //

void set_d3_off(void);                                         //
void set_d3_RED(void);                                         //
void set_d3_GREEN(void);                                      //
void set_d3_BLUE(void);                                                             //
void set_d3_WHITE(void);
void main(void)
{
    init_UART();
    init_ADC();
    TRISA = 0x0F;
    TRISB = 0x00;
    TRISC = 0x00;
    TRISD = 0x00;
    TRISE = 0x00;
    
    while(1)
    {
        char U;
        char L;
        Select_ADC_Channel(1);
        int num_step = get_full_ADC();
        float voltage1 = (num_step * 4)/1000.0;
        float resistor = calcresistance(voltage1);
        int resistance = resistor;
        
        if (resistor >= 10.0)                            		// WITH DECIMAL FUNCTION
        {
            								
            U = (int)resistance /10;                     		// RL VALUE IN kOHMS, ONLY INTEGER
            L =(int)resistance%10;        		// ENTIRE RL VALUE, SUBTRACT U (INTEGER OF RL) - DISPLAYS DECIMAL
            							
            display_upper(U);
            display_lower(L);                		// DISPLAYS VALUE L IN LOWER DIGIT TO SEVEN-SEGMENT DISPLAY WITH DECIMAL
            DECIMAL = 1; 			                                // DECIMAL SET TO 0 (ON)					
        }
        else
        {
            U = (int)resistance;
            L = (int)((resistance-U)*10);
            
            display_upper(U);
            display_lower(L);
            DECIMAL = 0;
        }
        printf("RL  = %f\r\n\n\n",resistor);
        display_D1(resistor);
        display_D2(resistor);
        wait_one_sec();
    }
}

void wait_one_sec()
{
    for(int i = 0; i<0xffff;i++);
}

void display_upper(char up)
{
    int ssegu;
    ssegu = ssegdisplay[up];
    PORTB = ssegu;
}

void display_lower(char low)
{
    int ssegl;
    ssegl = ssegdisplay[low];
    PORTD = ssegl;
}

void display_lower_dp(char digit)                		// DISPLAYS TEMP VALUE TO SEVEN SEGMENT DISPLAY [LOWER] WITH DECIMAL
{
    PORTD = array_with_dp[digit];                           		//  LOWER VALUE DUAL SEVEN SEGMENT DISPLAY
}
void display_upper_dp(char digit)                		// DISPLAYS TEMP VALUE TO SEVEN SEGMENT DISPLAY [HIGHER] WITH DECIMAL
{
    PORTB = array_with_dp[digit];                           		//  HIGHER VALUE DUAL SEVEN SEGMENT DISPLAY
}

float calcresistance (float volt)
{
    float ref = 4.096;
    
    float resistance =  (volt/(ref - volt))*0.1  ;
    
    return resistance;
}

void display_D1(int resistin)
{
    int resistor = resistin /10;
    if(resistor>=0 && resistor < 8)
    {
        PORTC = resistor;
    }
    else
    {
        PORTC = 0x07;
    }
}

void display_D2(int r)
{
    if(r < 0.07){
        set_d2_WHITE();
        Activate_Buzzer();
    }
    else if(r>=0.07 && r<0.1) {
        set_d2_RED(); 
        Deactivate_Buzzer();
    }
    else if(r>=0.1 && r<0.2) 
    {
        set_d2_GREEN();
        Deactivate_Buzzer();
    }
    else if(r>=0.2 && r<0.3)
    {
        set_d2_BLUE();
        Deactivate_Buzzer();
    }
    else if(r>=0.3) 
    {
        set_d2_off();
        Deactivate_Buzzer();
    }
}

void Activate_Buzzer()
{
PR2 = 0b11111001 ;
T2CON = 0b00000101 ;
CCPR2L = 0b01001010 ;
CCP2CON = 0b00111100 ;
}

void Deactivate_Buzzer()
{
CCP2CON = 0x0;
 PORTCbits.RC1 = 0;
} 

void set_d2_off()
{
    D2_RED = 0;
    D2_GREEN = 0;
    D2_BLUE = 0;
}
void set_d2_RED()
{
    D2_RED = 1;
    D2_GREEN = 0;
    D2_BLUE = 0;
}
void set_d2_GREEN()
{
    D2_RED = 0;
    D2_GREEN = 1;
    D2_BLUE = 0;
}
void set_d2_BLUE()
{
    D2_RED = 0;
    D2_GREEN = 0;
    D2_BLUE = 1;
}
void set_d2_WHITE()
{
    D2_RED = 1;
    D2_GREEN = 1;
    D2_BLUE = 1;
}