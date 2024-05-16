// PIC16F877A Configuration Bit Settings
// 'C' source line config statements

// CONFIG
#pragma config FOSC = HS     // Oscillator Selection bits (RC oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = ON     // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#include <xc.h>

#define _XTAL_FREQ 20000000    
#define RS RC2              
#define EN RC1                 

// Function prototypes
void init(void);
void i2c_init(const unsigned long);
void i2c_write(unsigned char);
int i2c_read(int);
void i2c_start();
void i2c_wait();
void i2c_stop();
void lcd_command(unsigned char);
void lcd_data(unsigned char);
int bcd_2_dec(int);
int dec_2_bcd(int);
void settime(void);
void update(void);


char msg1[5]={"TIME:"};       
char msg2[5]={"DAT:"};         
char hr[2]= {"AM"};             
char hr1[2]= {"PM"};            
int i,j,k,l,m,v,w;              
int sec=55;                     
int min=59;                     
int hour=11;                    
int date=28;                    
int day = 1;                   
int month=12;                  
int year =20;                   
char sec1,sec2,min1,min2,hour1,hour2,date1,date2,month1,month2,year1,year2; 


const char* daysOfWeek[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};

void main()
{
    init();                     // Initialize ports and LCD
    i2c_init(100);              // Initialize I2C communication
    settime();                  // Set initial time and date
    while(1)
    {
        update();               // Update time and date from RTC module
        
        
        sec1=sec/10;
        sec2=sec%10;
        min1=min/10;
        min2=min%10;
        hour1=hour/10;
        hour2=hour%10;
        date1=date/10;
        date2=date%10;
        month1=month/10;
        month2=month%10;
        year1=year/10;
        year2=year%10;

        
        lcd_command(0x80);      // Set cursor to beginning of first line
        for(i=0;i<5;i++) 
            lcd_data(msg1[i]);  
        lcd_data(hour1+'0');    // Display tens digit of hour
        lcd_data(hour2+'0');    // Display units digit of hour
        lcd_data(0x2D);         
        lcd_data(min1+'0');     // Display tens digit of minute
        lcd_data(min2+'0');     // Display units digit of minute
        lcd_data(0x2D);         
        lcd_data(sec1+'0');     // Display tens digit of second
        lcd_data(sec2+'0');     // Display units digit of second
        lcd_data(0x2D);       
        
        
        if(hour >= 12 )
        {
            for (m=0 ; m<2; m++)
                lcd_data(hr[m]); // Display AM
        }
        else
        {
            for (l=0 ; l<2; l++)
                lcd_data(hr1[l]); // Display PM
        }
        
        
        lcd_command(0xC0);      // Set cursor to beginning of second line
        for(i=0;i<4;i++) 
            lcd_data(msg2[i]);  // Display "DAT:"
        lcd_data(date1+'0');    // Display tens digit of date
        lcd_data(date2+'0');    // Display units digit of date
        lcd_data(0x2D);         
        lcd_data(month1+'0');   // Display tens digit of month
        lcd_data(month2+'0');   // Display units digit of month
        lcd_data(0x2D);        
        lcd_data(year1+'0');    // Display tens digit of year
        lcd_data(year2+'0');    // Display units digit of year
        lcd_data(0x2D);        
        
        
        lcd_data(daysOfWeek[day - 1][0]); // First letter of the day
        lcd_data(daysOfWeek[day - 1][1]); // Second letter of the day
        lcd_data(daysOfWeek[day - 1][2]); // Third letter of the day
    }
}


void init(void)
{
    TRISD=0x00;                 //  PORTD as output
    TRISC=0x18;                 //  RC3 and RC4 as inputs (for I2C)
    PORTD=0x00;                 //  PORTD as output
    lcd_command(0x38);         
    __delay_ms(5);
    lcd_command(0x0C);          // Display on, cursor off
    __delay_ms(5);
    lcd_command(0x06);          // Right shift
    __delay_ms(5);
    lcd_command(0x01);          // Clear display
    __delay_ms(5);
}


void lcd_command(unsigned char i)
{
    RS=0;                       // Set RS low 
    PORTD=i;                    // Send command
    EN=1;                       // Enable LCD
    EN=0;                       // Disable LCD
    __delay_ms(5);              // Delay for LCD operation
}


void lcd_data(unsigned char i)
{
    RS=1;                       // Set RS high for data mode
    PORTD=i;                    // Send data
    EN=1;                       // Enable LCD
    EN=0;                       // Disable LCD
    __delay_ms(5);              // Delay for LCD operation
}


void i2c_init(const unsigned long feq_k)
{
    SSPCON=0x28;                // Enable I2C Master mode, clock = FOSC/(4 * (SSPADD+1))
    SSPSTAT=0x00;               
    SSPCON2=0x00;               
    SSPADD = (_XTAL_FREQ/(4*feq_k*100))-1; // Set clock frequency
}


void i2c_wait()
{
    while(SSPCON2 & 0x1F || SSPSTAT & 0x04); // Wait for I2C operation to finish
}


void i2c_start()
{
    i2c_wait();
    SEN=1;                      // Start condition on SDA and SCL pins
}


void i2c_stop()
{
    i2c_wait();
    PEN=1;                      // Stop condition on SDA and SCL pins
}


void i2c_write(unsigned char temp)
{
    i2c_wait();
    SSPBUF=temp;                // Load data into SSPBUF register
}


int i2c_read(int ack)
{
    int value;
    i2c_wait();
    RCEN=1;                     // Enable receive mode for I2C
    i2c_wait();
    value=SSPBUF;               // Read data from SSPBUF register
    i2c_wait();
    ACKDT=(ack)?0:1;            // Send ACK or NACK
    ACKEN=1;                    // Enable Acknowledge sequence
    return value;
}


int dec_2_bcd(int temp)
{ 
    return ((temp/10)<<4)+(temp%10); // Convert decimal to BCD
}


int bcd_2_dec(int temp)
{ 
    return ((temp>>4)*10)+(temp&0x0F); // Convert BCD to decimal
}


void settime(void)
{
    i2c_start();   // Start I2C communication
    i2c_write(0xD0);         
    i2c_write(0);     // Read address 0            
    i2c_write(dec_2_bcd(sec));  // Send seconds in BCD format
    i2c_write(dec_2_bcd(min));  // Send minutes in BCD format
    i2c_write(dec_2_bcd(hour) | 0x60 ); // Send hours in BCD format (assuming 12-hour mode)
    i2c_write(dec_2_bcd(day));  // Send day in BCD format
    i2c_write(dec_2_bcd(date)); // Send date in BCD format
    i2c_write(dec_2_bcd(month)); // Send month in BCD format
    i2c_write(dec_2_bcd(year)); // Send year in BCD format
    i2c_stop();                 // Stop I2C communication
}


void update(void)
{
    i2c_start();// Start I2C communication
    i2c_write(0xD0);           
    i2c_write(0); // Read address 0
    i2c_stop(); // Stop I2C communication
    i2c_start(); // Start I2C communication
    i2c_write(0xD1);     //Read address 1
    
    sec=(bcd_2_dec(i2c_read(1))); // Read seconds and convert from BCD to decimal
    min=(bcd_2_dec(i2c_read(1))); // Read mins and convert from BCD to decimal
    int hour_am = i2c_read(1); // Read hours
    
    
    hour = bcd_2_dec(((hour_am & 0x1F) | ((hour_am >> 5) & 0x60))); // Read hours and convert from BCD to decimal
    day=(bcd_2_dec(i2c_read(1))); // Read day and convert from BCD to decimal
    
    date=(bcd_2_dec(i2c_read(1))); // Read date and convert from BCD to decimal
    month=(bcd_2_dec(i2c_read(1))); // Read month and convert from BCD to decimal
    year=(bcd_2_dec(i2c_read(1))); // Read year and convert from BCD to decimal
    
    i2c_stop(); // Stop I2C communication
    i2c_start();// Start I2C communication
    i2c_write(0xD1); 
    i2c_read(1); 
    i2c_stop(); // Stop I2C communication
}
