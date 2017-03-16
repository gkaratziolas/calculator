#define __MAIN_C__

#include <stdio.h>
#include <stdint.h>

#define SEG_BASE_ADDR 0x40000000
#define PAD_BASE_ADDR 0x50000000

volatile uint32_t* display_buffer  = (volatile uint32_t*) SEG_BASE_ADDR;
volatile uint32_t* keypad_buffer = (volatile uint32_t*) PAD_BASE_ADDR;

#define OP_PLUS     8
#define OP_MINUS    9
#define OP_TIMES    10
#define OP_DIVIDE   11
#define OP_EQUALS   4
#define OP_C        5
#define OP_DP       6
#define OP_SQUARED  0


#define MAX_DIGITS 7
#define NO_DP 10

typedef struct read_name {
   float number;
   int function;
} read_type;

float power10(int y);
void intToStr(int x, char str[]);
read_type read_pad(void);
void float_to_display( float f);
void string_to_display( uint32_t* number_string, uint32_t DP_position );
uint8_t char_to_display_code( uint8_t c );
float calc(float num1, float num2, int op);
void clear_string( uint32_t* str, uint32_t length );

//power function, eg 2^3 = 8
float power10(int y) { // if the power function is only used for powers of 10 this could be optimised and x could be an int
	float out = 1;
	for(;y>0;y--)
		out *= 10;
	for(;y<0;y++)
		out /= 10;
	return out;
}

// read_type read_pad()
// {
// 	uint32_t digits=0;
// 	uint32_t i;
// 	uint32_t DP=10;
// 	uint32_t key_pressed;
// 	read_type result = { .number = 0, .function = 0};

// 	uint32_t str_float[9];

// 	while(1)
// 	{
// 		key_pressed = *keypad_buffer;
// 		if ( key_pressed != 0 )
// 		{
// 			if ( (key_pressed>15) && (digits<MAX_DIGITS+1) )
// 			{
// 				digits++;
// 				for ( i=8-digit; i<MAX_DIGITS+1; i++ )
// 				{
// 					str_float[0] = str_float[1]; // shift digits across
// 				str_float[1] = str_float[2];
// 				str_float[2] = str_float[3];
// 				str_float[3] = str_float[4];
// 				str_float[4] = str_float[5];
// 				str_float[5] = str_float[6];
// 				str_float[6] = str_float[7];
// 				str_float[7] = str_float[8];

// 				if(DP == 10){
// 					// all of the temp-16 can probably be assigned to a variable
// 					result.number = result.number * 10 + (temp-16);
// 				}
// 				else{
// 					result.number += (temp-16)* power10((DP-i));
// 					str_float[9] = str_float[9] - 1; // shift the dp when new numbers entered
// 				}
				
// 				str_float[8] = temp + 32;
// 			}
// 			else if ( key_pressed == OP_DP )
// 			{

// 			}
// 			else if ( key_pressed == OP_C )
// 			{

// 			}
// 			else{
// 				result.function = key_pressed;
// 				return result;	
// 			}
// 		}
// }

read_type read_pad()
{
	int i=0;
	int DP=10;
	int temp;
	int clear_all = 0;
	read_type result = { .number = 0, .function = 0};
	
	uint32_t str_float[10];
	clear_string(str_float, (uint32_t) 9);
//	str_float[8] = ' ';
	
	str_float[9] = 10;
	while(1)
	{
		temp = *keypad_buffer;
		if(temp != 0)
		{
			if((temp>15)&&(i<9)) { // number
				i++;
				str_float[0] = str_float[1]; // shift digits across
				str_float[1] = str_float[2];
				str_float[2] = str_float[3];
				str_float[3] = str_float[4];
				str_float[4] = str_float[5];
				str_float[5] = str_float[6];
				str_float[6] = str_float[7];
				str_float[7] = str_float[8];

				if(DP == 10){
					// all of the temp-16 can probably be assigned to a variable
					result.number = result.number * 10 + (temp-16);
				}
				else{
					result.number += (temp-16)* power10((DP-i));
					str_float[9] = str_float[9] - 1; // shift the dp when new numbers entered
				}
				
				str_float[8] = temp + 32;
				clear_all = 0;
			}
			else if(temp==6) { // decimal point
				DP = i;
				str_float[9] = 8;
				clear_all = 0;
			}
			else if(temp==5) { // clear // may need to change if. what if typing num2 and then press ce
				result.number = 0;
				i = 0;
				DP = 10;
				str_float[0] = ' '; // need to clear display
				str_float[1] = ' ';
				str_float[2] = ' ';
				str_float[3] = ' ';
				str_float[4] = ' ';
				str_float[5] = ' ';
				str_float[6] = ' ';
				str_float[7] = ' ';
				str_float[8] = ' ';
				str_float[9] = 10;

				if(clear_all){
					result.function = temp;
					return result;
				}
				clear_all = 1;

			}	
			else {// operation
				result.function = temp;
				return result;
			}
			string_to_display(str_float, str_float[9]);
		}
	}
}


uint8_t char_to_display_code( uint8_t c )
{
    switch(c){
        case ' ':
            return 0;
        case '0':
            // zero is stored as 10
            // this allows display to default to being blank on a reset
            return 10;
        case 'E':
            return 11;
        case 'r':
            return 12;
        case 'o':
            return 13;
        case 'U':
            return 14;
        case '-':
            return 15;
        default:
            // non-zero numbers are coded as themselves
            // char "8" is coded as uint32_t 8
            return c-48;
    }
}

void string_to_display( uint32_t* number_string, uint32_t DP_position )
{
	uint32_t i;

	for( i=0; i<9; i++ )
	{
		if ( i == DP_position ){
			// code in DP position has 16 added to it to code for the decimal point
			display_buffer[i] = char_to_display_code( number_string[i] ) + 16;
		} else {
			// other codes are directly looked up
			display_buffer[i] = char_to_display_code( number_string[i] );
		}
	}
}


// reverses a string 'str' of length 'len'
void reverse( char *str, int len )
{
    int i=0, j=len-1, temp;
    while (i<j)
    {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++; j--;
    }
}

uint32_t right_align_string( uint32_t* str, uint32_t DP)
{
	while ( str[8] == ' ' ||  str[8] == '0' )
	{
		if ( DP+2 == NO_DP ){
			return NO_DP;
		}
		// shift string left
		// str[0] remains the same (sign bit)
		str[8] = str[7];
		str[7] = str[6];
		str[6] = str[5];
		str[5] = str[4];
		str[4] = str[3];
		str[3] = str[2];
		str[2] = str[1];
		str[1] = ' ';
		DP++;
	}
	return DP;
}


void clear_string( uint32_t* str, uint32_t length )
{
	uint32_t i;
	for ( i=0; i<length; i++)
	{
		str[i] = ' ';
	}
}

void error_string( uint32_t* str, uint32_t error_code)
{
	str[0] = 'E';
	str[1] = 'r';
	str[2] = 'r';
	str[3] = 'o';
	str[4] = 'r';
	str[5] = ' ';
	str[6] = ' ';
	str[7] = ' ';
	// int error code converted to relevant char
	str[8] = error_code + 48;
}

void int_to_string(uint32_t* str, uint32_t display_number)
{
	uint32_t i, digit;

    for ( i=MAX_DIGITS; i>0; i-- )
    {
    	digit  = display_number % 10;
    	str[i] = digit + '0';
    	display_number = display_number / 10;
    }
}

uint32_t float_to_string( float f, uint32_t* str )
{
	uint32_t i;
	uint32_t DP;
	uint32_t y;
    
    // clear the number string
	clear_string(str, 9);

    // set sign char
	if ( f < 0 ){
		str[0] = '-';
		f = -f;
	} else {
        str[0] = ' ';
	}

    // check the input for special cases
    // zero
    if ( f == 0 ){
    	str[8] = '0';
    	return NO_DP;
    }
    // too large to display
    else if ( f > 9999999 ) {
    	error_string( str, 1 );
    	return NO_DP;
    }
    // too small to display
    else if ( f < 0.000001 ) {
    	error_string( str, 2 );
    	return NO_DP;
    } 


    // float can be displayed
    else{
        if ( f < 1 ){
        	DP = 1;
        }
        else{
	    	for ( i=1; i<8; i++ )
	    	{
	    		if ( f < power10(i) ){
	    			DP = i;
	    			break;
	    		}
	    	}
	    }
    	f = f * power10(MAX_DIGITS-DP);
    	y = (uint32_t) (f+0.5);
    	int_to_string(str, y);


   		DP = right_align_string(str, DP);
   		if ( DP == 8 ){
   			DP = NO_DP;
   		}
    	return DP;
    }
}


float calc(float num1, float num2, int op){
	switch(op){
		case OP_PLUS:
			return num1 + num2;
		case OP_MINUS:
			return num1 - num2;
		case OP_TIMES:
			return num1 * num2;
		case OP_DIVIDE:
			return num1 / num2;
		default:
			return 0;
	}
}

int main(void) {
	
	float num1;
	float num2;
	int op;
	uint32_t DP;
	read_type x;
	uint32_t buf[9];

	// float_to_display(x);
	uint32_t mode = 0;
	while(1)
	{
        if ( mode == 0 ){
			x = read_pad();
			op   = x.function;
			if ( op == OP_C ){
				mode = 0;
				num1 = 0;
			}
			else{
				num1 = x.number;
				mode = 1;
			}

			// need a check if op is OP SQUARED and return the answer
			// if (op == OP_SQUARED){
			//	   num1 = num1 * num1;
			// 	   float_to_display(num1);
			// }
        }
        else{
			x = read_pad();
			num2 = x.number;
			num1 = calc(num1, num2, op);
			op = x.function;
			if ( op == OP_EQUALS ){
				mode = 0;
				DP = float_to_string(num1, buf);
				string_to_display(buf, DP);
			}
			else if ( op == OP_C ){
				mode = 0;
				num1 = 0;
				num2 = 0;
			}
			else{
				DP = float_to_string(num1, buf);
				string_to_display(buf, DP);
			}
		}
	}
}

