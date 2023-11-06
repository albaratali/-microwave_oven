/*
 * File:   main.c
 * Author: faizan
 *
 * Created on November 3, 2023, 6:47 PM
 */

#include "main.h"

#pragma config WDTE =OFF // watchdog timer disabled

unsigned char sec=0,min=0, flag=0; //0
int operation_flag=POWER_ON_SCREEN ; //power on screen

static void init_config(void) {
    // initialization of CLCD Module
    init_clcd();
    
    //Initialization of MKP module
    init_matrix_keypad();
    
    //RC2 pic as output 
    FAN_DDR=0;
    FAN=OFF; // turn off the fan
    
    BUZZER_DDR=0; //RC1 pin as a output pin for Buzzer
    BUZZER=OFF;
    
    // initialization of TIMER2 Module
    init_timer2();
    
    PEIE=1;
    GIE=1;
}

void main(void) {
    init_config(); // calling initializing function
    
    //variable declaration
    
    unsigned char key;
    int reset_flag ;
     
    while (1) {
        // write application code here
        key=read_matrix_keypad(STATE);
        
        if(operation_flag == MENU_DISPLAY_SCREEN)
        {
            if(key==1)
            {
                operation_flag= MICRO_MODE;
                reset_flag=MODE_RESET;
                clear_screen();
                clcd_print(" Power = 900W  ",LINE2(0));
                __delay_ms(3000);
                clear_screen();

            }
            else if(key==2)//Grill mode
            {
                operation_flag = GRILL_MODE ;
                reset_flag = MODE_RESET;
                clear_screen();
            }
            else if(key==3)// convection
            {
                operation_flag = CONVECTION_MODE ;
                reset_flag = MODE_RESET;
                clear_screen();

            }
            else if(key==4)// start mode
            {
                sec=30;
                min=0;
                FAN =ON;
                TMR2ON= ON;
                clear_screen();
                operation_flag = TIME_DISPLAY;
            } 
        }
        else if (operation_flag == TIME_DISPLAY)
        {
            if(key==4) // start/resume
            {
                sec= sec + 30;
                if(sec > 59)
                {
                    min++;
                    sec=sec-60;
                }
            }
            else if (key == 5) // Pause
            {
                operation_flag= PAUSE;
            }
            else if (key == 6) //Stop
            {
                operation_flag= STOP;
                clear_screen();
                
            }
        }
        else if (operation_flag == PAUSE)
        {
            if(key==4){
                FAN=ON;
                TMR2ON=ON;
                operation_flag = TIME_DISPLAY;
            }
        }
        

        
        switch(operation_flag)
        {
            case POWER_ON_SCREEN :
                power_on_screen();
                operation_flag=MENU_DISPLAY_SCREEN;
                clear_screen();
                 break;
            case MENU_DISPLAY_SCREEN :
               menu_display_screen();
               break;
            case MICRO_MODE:
                set_time(key,reset_flag);
                break;
            case GRILL_MODE:
                set_time(key,reset_flag);
                break;
            case CONVECTION_MODE:
                
                if(flag ==0)
                {
                    set_temp(key,reset_flag);
                    if(flag == 1)
                    {
                        clear_screen();
                        reset_flag = MODE_RESET;
                        continue;
                    }
                      
                }
                else if (flag == 1)
                {
                    set_time(key,reset_flag);
                }
                break;
            case TIME_DISPLAY:
                time_display_screen();
                break;
            case PAUSE:
                FAN=OFF;
                TMR2ON=OFF;
                break;
            case STOP:
                FAN=OFF;
                TMR2ON=OFF;
                operation_flag= MENU_DISPLAY_SCREEN;
                break;
            
        }
        reset_flag=RESET_NOTHING;
    
        }
        

}

void power_on_screen(void)
{
    unsigned char i;
    // line1-> printing bar on line 1
    for(i=0;i<16;i++)
    {
        clcd_putch(BAR,LINE1(i));
        __delay_ms(100);
    }
    //printing power on message on line 2 and 3
    clcd_print("  Powering On   ",LINE2(0));
    clcd_print(" Microwave oven ",LINE3(0));
    
    // line1-> printing bar on line 4
    for(i=0;i<16;i++){
        clcd_putch(BAR,LINE4(i));
        __delay_ms(100);
    }
    __delay_ms(3000); //3sec deley
}

void time_display_screen(void)
{
    clcd_print("TIME =  ",LINE1(0));
    
    //min
    clcd_putch(min/10 + '0',LINE1(9));
    clcd_putch(min%10 + '0',LINE1(10));
    clcd_putch(':',LINE1(11));
    // sec
    clcd_putch(sec/10 + '0',LINE1(12));
    clcd_putch(sec%10 + '0',LINE1(13));
    
    //print option
    clcd_print(" 4.Start/Resume",LINE2(0));
    clcd_print(" 5.Pause",LINE3(0));
    clcd_print(" 6.Stop",LINE4(0));
    
    if (sec ==0 && min ==0)
    {
        clear_screen();
        clcd_print(" Time Up!!",LINE2(0));
        BUZZER=ON;
        __delay_ms(3000); // 3 sec delay
        clear_screen();
        BUZZER=OFF;
        FAN=OFF ; // Turn OFF the fan
        /* Switching OFF the Timer2 */
        TMR2ON=OFF;
        
        operation_flag=MENU_DISPLAY_SCREEN;
        
    }
}

void clear_screen(void)
{
    clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
    __delay_us(500);
}



void menu_display_screen(void)
{
    
    clcd_print("1.Micro",LINE1(0));
    clcd_print("2.Grill",LINE2(0));
    clcd_print("3.Convection",LINE3(0));
    clcd_print("4.Start",LINE4(0));
}

void set_temp(unsigned char key,int reset_flag)
{
    static unsigned char key_count,blink,temp;
    static int wait;
    if(reset_flag==MODE_RESET ) // Initialization for screen
    {
        key_count=0;
        wait=0;
        blink=0;
        temp=0;
        flag=0;
        key=ALL_RELEASED;
        clcd_print(" SET TEMP(oC)  ",LINE1(0));
        clcd_print(" TEMP = ",LINE2(0));
        //sec -> 0 to 59
        clcd_print("*:CLEAR  #:ENTER",LINE4(0)); 
    }
    
    //reading the temp 
    if((key != '*') && (key != '#') && (key != ALL_RELEASED))
    {
        //key=1 2 3 4
        key_count++;
        
        //120
        if (key_count<=3)// reading number of sec
        {
            temp = temp*10 + key; //temp =120
            
        }
    }
    
    else if (key=='*')
    {
        temp=0;
        key_count=0;
        
    }
    else if (key=='#')
    {
        clear_screen();
        clcd_print(" Pre-Heating ",LINE1(0));
        clcd_print("Time Rem.= ",LINE3(0));
        TMR2ON =1;
        sec=18;
        while( sec != 0)
        {
            //clcd_putch((sec/100)+'0',LINE3(11)); // 123/100 ->1
            clcd_putch((sec/10)%10+'0',LINE3(12)); // 123/100=12%10 ->2
            clcd_putch((sec%10)+'0',LINE3(13)); // 123%10 ->3
        }
        
        if(sec==0)
        {
            flag = 1;
            TMR2ON=0;
            // Set time screen exactly like Grill mode
            
            //operation_flag = GRILL_MODE;
            reset_flag=MODE_RESET;
            flag=1;
            clear_screen();
            set_time(key,reset_flag);
            
            
        }
    }
    
    if (wait++ == 15) // make blinking pheonmenon
    {
        wait=0;
        blink =! blink;
        
        //Printing Temp on the set temp screen 
         ///temp=123
         clcd_putch((temp/100)+'0',LINE2(8)); // 123/100 ->1
         clcd_putch((temp/10)%10+'0',LINE2(9)); // 123/100=12%10 ->2
         clcd_putch((temp%10)+'0',LINE2(10)); // 123%10 ->3  
    }
    
    if(blink)
    {
         clcd_print("   ",LINE2(8));
    }
   
}

void set_time(unsigned char key,int reset_flag)
{
   
    static unsigned char key_count,blink_pos,blink;
    static int wait;
    
    if(reset_flag==MODE_RESET )
    {
        key_count=0;
        sec=0;
        min=0;
        blink_pos=0;
        wait=0;
        blink=0;
        key=ALL_RELEASED;
        clcd_print("SET TIME (MM:SS)",LINE1(0));
        clcd_print("TIME- ",LINE2(0));
        //sec -> 0 to 59
        clcd_print("*:CLEAR  #:ENTER",LINE4(0)); 
        
    }
    
    
    
    if((key != '*') && (key != '#') && (key != ALL_RELEASED))
    {
        //key=1 2 3 4
        key_count++;
        
        if (key_count<=2)// reading number of sec
        {
            sec=sec*10 + key;
             blink_pos=0;
        }
        else if ((key_count>2) && (key_count<=4)) // reading min
        {
            min=min*10 + key;
             blink_pos=1;
        }
    }
    else if (key=='*')
    {
        if(blink_pos==0)//to clear sec
        {
            sec=0;
            key_count=0;
        }
        else if(blink_pos==1)
        {
            min=0;
            key_count=2;
        }
    }
    else if (key=='#')
    {
        clear_screen();
        operation_flag=TIME_DISPLAY;
        FAN=ON ; // Turn on the fan
        /* Switching ON the Timer2 */
        TMR2ON=ON;
    }
    
    if (wait++ == 15)
    {
        wait=0;
        blink =! blink;
        
        //Printing sec and min on the set time secreen 
        //min
        clcd_putch(min/10 + '0',LINE2(6));
        clcd_putch(min%10 + '0',LINE2(7));
        clcd_putch(':',LINE2(8));
        // sec
        clcd_putch(sec/10 + '0',LINE2(9));
        clcd_putch(sec%10 + '0',LINE2(10));
        
    }
    
    if(blink)
    {
        switch(blink_pos)
        {
            case 0:
                clcd_print("  ",LINE2(9));
                break;
            case 1:
                clcd_print("  ",LINE2(6));
                break;
        }
        
    }
    
}