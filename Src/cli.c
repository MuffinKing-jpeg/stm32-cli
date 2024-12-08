#include "cli.h"
#include "cmd_list.h"
#include "usart.h"
#include "toggle_pin.h"
#include <string.h>

CCMRAM char command_buffer[RX_BUFFER_SIZE] = {0};
uint16_t buffer_index = 0;
uint8_t rx_data = '\0';

uint32_t calculate_hash(const char *str)
{
    uint32_t hash = HASH_PRIME;
    int c;

    while ((c = *str++))
    {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

#ifndef USE_PRECALCULATED_HASH

void populate_cmd_hash()
{
    for (uint8_t i = 0; i < QTY_CMD; i++)
    {
        command_table[i].hash = calculate_hash(command_table[i].command_name);
    }
}

#endif

void start_rx()
{
    HAL_UART_Receive_DMA(&CLI_UART, &rx_data, 1);
}

void process_command()
{

    if (buffer_index < RX_BUFFER_SIZE -1 )
    {
        command_buffer[buffer_index] = rx_data;

        switch (rx_data)
        {
        case '\n':
        case '\r':
            parse_command();
            break;

        case '\b':
        case 0x7F:
            clear_last_input();
            break;
        case 0x03:
            clear_input();
            break;
        default:
            if (buffer_index < RX_BUFFER_SIZE - 1)
            {
                HAL_UART_Transmit_DMA(&CLI_UART, &rx_data, 1);
                buffer_index++;
            }
            start_rx();
            break;
        }
    }
    else
    {
        command_buffer[buffer_index] = '\0';
    }
}

void parse_command()
{
    HAL_UART_Transmit(&CLI_UART, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY);
    command_buffer[buffer_index] = '\0'; //Replacing newline with null 

    uint8_t is_command_found = 0;
    uint32_t cmd = 0;
    Tokens args = {0};
    tokenize(command_buffer, args, &cmd);

    for (uint8_t i = 0; i < QTY_CMD; i++)
    {
        if (command_table[i].hash == cmd)
        {
            command_table[i].handler(args);
            is_command_found = 1;
        }
        
    }
    if (is_command_found == 0)
    {
        rejected_cmd();
    }
    clear_buffer();
}

void tokenize(const char *input, Tokens args, uint32_t *cmd_hash)
{
    int arg_count = 0;

    char input_copy[RX_BUFFER_SIZE];
    strncpy(input_copy, input, sizeof(input_copy));
    input_copy[sizeof(input_copy) - 1] = '\0';

    // Tokenize the command
    char *token = strtok(input_copy, " ");
    if (token != NULL)
    {
        *cmd_hash = calculate_hash(token);
        // Tokenize arguments
        while ((token = strtok(NULL, " ")) != NULL && arg_count < MAX_TOKENS)
        {
            strncpy(args[arg_count], token, MAX_TOKEN_LENGTH);
            args[arg_count][MAX_TOKEN_LENGTH - 1] = '\0';
            arg_count++;
        }
    }
}

void rejected_cmd()
{
    const char unknown_seq[] = "Unknown command\r\n";
    HAL_UART_Transmit(&CLI_UART, (uint8_t *)unknown_seq, sizeof(unknown_seq), HAL_MAX_DELAY);
    clear_buffer();
}

void cmd_clear(Tokens args){
    (void)args;
    clear_input();
}

void clear_input()
{
    if (buffer_index > 0)
    {
        const char clear_seq[] = "\033[1J\033[H"; // Some legacy ASCII shit.
        HAL_UART_Transmit(&CLI_UART, (uint8_t *)clear_seq, sizeof(clear_seq), HAL_MAX_DELAY);
    }
    clear_buffer();
}

void clear_last_input()
{
    if (buffer_index > 0)
    {
        command_buffer[buffer_index] = 0;
        buffer_index--; // Remove last character
        command_buffer[buffer_index] = 0;
        char backspace_seq[] = "\b \b";
        HAL_UART_Transmit(&CLI_UART, (uint8_t *)backspace_seq, strlen(backspace_seq), HAL_MAX_DELAY);
    }
    start_rx();
}

void clear_buffer()
{
    buffer_index = 0;
    memset(command_buffer, 0, RX_BUFFER_SIZE);
    start_rx();
}
