/* ========================================
 * PC controlled signal generator
 *
 * Author: Angad Gill (angadsinghgill@gmail.com)
 * 
 * ========================================
*/
#include "project.h"
#define RX_LEN 20
#define ERR_MSG ": invalid command. Type 'help' to see available commands\r\n"

const char help_message[] = 
    "\r\nHi! This is a PC controlled signal generator.\r\n"
    "Options:\r\n"                         
    "\t'led on' to turn on the LED.\r\n"
    "\t'led off' to turn off the LED.\r\n"
    "\t'pwm on' to turn on the PWM.\r\n"
    "\t'pwm off' to turn off the PWM.\r\n"
    "\t'help' to see this help message.\r\n";


void execute_command(char * command){
    if(strcmp(command, "help")==0){
        UART_SpiUartPutArray(help_message, strlen(help_message));
    }        
    else if(strcmp(command, "pwm on")==0){
        PWM_1_Start();
    }
    else if(strcmp(command, "pwm off")==0){
        PWM_1_Stop();
    } 
    else if(strcmp(command, "led on")==0){
        LED_Write(1u);
    } 
    else if(strcmp(command, "led off")==0){
        LED_Write(0u);
    } 
    else{
        UART_SpiUartPutArray(command, strlen(command));
        UART_SpiUartPutArray(ERR_MSG, strlen(ERR_MSG));
    }
}


int main(void)
{
    UART_Start();
    uint32 rx_data;
    char rx_str[RX_LEN];
    int rx_str_idx = 0;
    
    UART_SpiUartPutArray(help_message, strlen(help_message));
    UART_SpiUartWriteTxData('>');
    
    for(;;)
    {
        if (UART_SpiUartGetRxBufferSize()!=0){
            while(UART_SpiUartGetRxBufferSize() != 0){
                rx_data = UART_SpiUartReadRxData();
                if(rx_data=='\n'){
                    if(rx_str_idx > 1){
                        /* rx_str is more than just "\r" */
                        rx_str[rx_str_idx-1] = '\0';  // set trailing "\r" to null
                        execute_command(rx_str);
                    }
                    rx_str_idx = 0;
                    memset(rx_str, '\0', RX_LEN);
                    UART_SpiUartWriteTxData('>');
                }
                else{
                    rx_str[rx_str_idx]=rx_data;
                    rx_str_idx++;
                }
            }
        }
    }
}
