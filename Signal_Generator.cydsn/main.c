/* ========================================
 * PC controlled signal generator
 *
 * Author: Angad Gill (angadsinghgill@gmail.com)
 * 
 * ========================================
*/
#include "project.h"
#include "stdio.h"
#define RX_LEN 20
#define ERR_MSG ": invalid input_str. Type 'help' to see available input_strs\r\n"
#define CMD_LEN 4
#define OPTION_LEN 4
#define NUM_LEN 10

const char help_message[] = 
    "\r\nHi! This is a PC controlled signal generator.\r\n"
    "input_strs:\r\n"      
    "  'help' to see this help message.\r\n"
    "  'led' to control the LED.\r\n"
    "  'pwm' to control PWMs.\r\n"
    ;

void extract_first_word(char * input_str, char * first_word, const uint word_len){
    unsigned int i;
    for(i=0; i<strlen(input_str) && i<word_len; i++){
        if(input_str[i]==' ') break;
        else first_word[i] = input_str[i];
    }
}

void send_help_msg(){
    UART_SpiUartPutArray(help_message, strlen(help_message));
}

void send_crlf(){
    UART_SpiUartPutArray("\r\n", 2);
}

void send_error_msg(char * input_str){
    UART_SpiUartPutArray(input_str, strlen(input_str));
    UART_SpiUartPutArray(ERR_MSG, strlen(ERR_MSG));   
}

void send_uint(uint value){
    char value_str[NUM_LEN];
    sprintf(value_str, "%d", value);
    UART_SpiUartPutArray(value_str, strlen(value_str));
    send_crlf();
}

void execute_input_str(char * input_str){
    char cmd[CMD_LEN] = "";
    extract_first_word(input_str, cmd, CMD_LEN);

    /* help */
    if(strcmp(input_str, "help")==0) send_help_msg();
    
    /* pwm */
    else if(strcmp(cmd, "pwm")==0){
        const char cmd_format[] =  
        "pwm format: pwm <id> <option> [property] [number]\r\n"
        "ids:\r\n"
        "  1: port 1.5\r\n"
        "  2: port 0.4\r\n"
        "option:\r\n"
        "  on\r\n"
        "  off\r\n"
        "  set\r\n"
        "  get\r\n"
        "property:\r\n"
        "  p: period\r\n"
        "  c: compare\r\n"
        ;
        char pwm_id = input_str[4];
        char option[OPTION_LEN] = "";
        extract_first_word(input_str+6, option, OPTION_LEN);
        char property = input_str[10];
        char number_str[NUM_LEN] = "";
        extract_first_word(input_str+12, number_str, NUM_LEN);
        uint number = 0;
        sscanf(number_str, "%u", &number);
        
        switch(pwm_id){
            case '1':
                if(strcmp(option, "on")==0){
                    uint enabled = PWM_1_ReadControlRegister()>>7 && 0x01;  // mask Enable bit
                    if(enabled); else if(PWM_1_initVar) PWM_1_Wakeup(); else PWM_1_Start();
                }
                else if(strcmp(option, "off")==0){
                    PWM_1_Sleep();
                }
                else if(strcmp(option, "get")==0){
                    if(property=='p') send_uint(PWM_1_ReadPeriod());
                    else if(property=='c') send_uint(PWM_1_ReadCompare());
                    else if(property=='r') send_uint(PWM_1_ReadControlRegister());
                    else UART_SpiUartPutArray(cmd_format, strlen(cmd_format));
                }
                else if(strcmp(option, "set")==0){
                    if(property=='p'){
                        PWM_1_WritePeriod(number);
                        send_uint(PWM_1_ReadPeriod());
                    }
                    else if(property=='c'){
                        PWM_1_WriteCompare(number);
                        send_uint(PWM_1_ReadCompare());
                    }
                    else UART_SpiUartPutArray(cmd_format, strlen(cmd_format));
                }
                else UART_SpiUartPutArray(cmd_format, strlen(cmd_format));
                break;
            case '2':
                if(strcmp(option, "on")==0){
                    uint enabled = PWM_2_ReadControlRegister()>>7 && 0x01;  // mask Enable bit
                    if(enabled); else if(PWM_2_initVar) PWM_2_Wakeup(); else PWM_2_Start();
                }
                else if(strcmp(option, "off")==0){
                    PWM_2_Sleep();
                }
                else if(strcmp(option, "get")==0){
                    if(property=='p') send_uint(PWM_2_ReadPeriod());
                    else if(property=='c') send_uint(PWM_2_ReadCompare());
                    else if(property=='r') send_uint(PWM_2_ReadControlRegister());
                    else UART_SpiUartPutArray(cmd_format, strlen(cmd_format));
                }
                else if(strcmp(option, "set")==0){
                    if(property=='p'){
                        PWM_2_WritePeriod(number);
                        send_uint(PWM_2_ReadPeriod());
                    }
                    else if(property=='c'){
                        PWM_2_WriteCompare(number);
                        send_uint(PWM_2_ReadCompare());
                    }
                    else UART_SpiUartPutArray(cmd_format, strlen(cmd_format));
                }
                else UART_SpiUartPutArray(cmd_format, strlen(cmd_format));
                break;
            default:
                UART_SpiUartPutArray(cmd_format, strlen(cmd_format));
        }
        
    }    
    
    /* led */
    else if(strcmp(cmd, "led")==0){
        const char cmd_format[] =  
        "led format: pwm <option>\r\n"
        "port 1.6\r\n"
        "option:\r\n"
        "  on\r\n"
        "  off\r\n"
        ;
        char option[OPTION_LEN] = "";
        extract_first_word(input_str+4, option, OPTION_LEN);
        
        if(strcmp(option, "on")==0) LED_Write(1u);
        else if(strcmp(option, "off")==0) LED_Write(0u);
        else UART_SpiUartPutArray(cmd_format, strlen(cmd_format));
        }

    /* no match */
    else send_error_msg(input_str);
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
                        execute_input_str(rx_str);
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
