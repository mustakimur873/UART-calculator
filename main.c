// @file main.c
// @brief Main source code for a UART-based simple floating-point calculator program.

#include "TM4C123GH6PM.h"
#include "SysTick_Delay.h"
#include "UART0.h"
#include <ctype.h> 
#include <string.h>
#include <stdbool.h>
#include <stdio.h>  // for sprintf()

// Helper function to convert string to float
float string_to_float(const char *str) {
    float result = 0.0f;
    float fraction = 0.0f;
    bool is_negative = false;
    bool decimal_point_seen = false;
    float divisor = 10.0f;

    if (*str == '-') {
        is_negative = true;
        str++;
    }

    while (*str) {
        if (*str >= '0' && *str <= '9') {
            if (!decimal_point_seen) {
                result = result * 10.0f + (*str - '0');
            } else {
                fraction += (*str - '0') / divisor;
                divisor *= 10.0f;
            }
        } else if (*str == '.') {
            decimal_point_seen = true;
        }
        str++;
    }

    result += fraction;
    return is_negative ? -result : result;
}

// Helper function to output float to UART0 (with 3 decimal places)
void UART0_Output_Float(float num) {
    char buffer[32];
    if (num < 0) {
        num = -num;
        UART0_Output_Character('-');
    }
   
    int int_part = (int)num;
    int decimal_part = (int)((num - int_part + 0.0005f) * 1000); // rounding up correctly

    if (decimal_part >= 1000) { // handle rounding overflow
        int_part += 1;
        decimal_part = 0;
    }

    sprintf(buffer, "%d.%03d", int_part, decimal_part);
    UART0_Output_String(buffer);
}


int main(void)
{
    SysTick_Delay_Init();
    UART0_Init();

    while (1)
    {
        UART0_Output_String("Enter an operation (+, -, *, /, <<, >>): ");
        char input_string[10] = {0};
        UART0_Input_String(input_string, sizeof(input_string));

        bool is_shift_left = false;
        bool is_shift_right = false;
        char operator = 0;

        if (strcmp(input_string, "<<") == 0) {
            is_shift_left = true;
        }
        else if (strcmp(input_string, ">>") == 0) {
            is_shift_right = true;
        }
        else {
            operator = input_string[0];
        }

        if (operator == '+' || operator == '-' || operator == '*' || operator == '/' || is_shift_left || is_shift_right)
        {
            UART0_Output_Newline();
            UART0_Output_String("Enter the first number: ");
            char num1_str[20] = {0};
            UART0_Input_String(num1_str, sizeof(num1_str));
            float num1_f = string_to_float(num1_str);

            UART0_Output_Newline();
            UART0_Output_String("Enter the second number: ");
            char num2_str[20] = {0};
            UART0_Input_String(num2_str, sizeof(num2_str));
            float num2_f = string_to_float(num2_str);

            float result_f = 0.0f;
            bool valid = true;

            if (is_shift_left || is_shift_right) {
                // Force cast to integers
                int num1 = (int)num1_f;
                int num2 = (int)num2_f;

                if (num2 < 0) {
                    UART0_Output_Newline();
                    UART0_Output_String("Error: Shift amount must be non-negative.");
                    valid = false;
                } else if (is_shift_left) {
                    result_f = (float)(num1 << num2);
                } else if (is_shift_right) {
                    result_f = (float)(num1 >> num2);
                }
            }
            else {
                if (operator == '+') {
                    result_f = num1_f + num2_f;
                }
                else if (operator == '-') {
                    result_f = num1_f - num2_f;
                }
                else if (operator == '*') {
                    result_f = num1_f * num2_f;
                }
                else if (operator == '/') {
                    if (num2_f == 0.0f) {
                        UART0_Output_Newline();
                        UART0_Output_String("Error: Divide by zero.");
                        valid = false;
                    } else {
                        result_f = num1_f / num2_f;
                    }
                }
            }

            if (valid) {
                UART0_Output_Newline();
                UART0_Output_String("Result: ");
                UART0_Output_Float(result_f);


            }
        }
        else
        {
            UART0_Output_Newline();
            UART0_Output_String("Invalid Operation. Try again.");
        }

        UART0_Output_Newline();
        SysTick_Delay1ms(500);
    }
}