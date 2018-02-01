// ---------------------------------------------------------------------------
// CpE 301
// March 6, 2017
// Jeeno & Cyril
// Piano Code
// ---------------------------------------------------------------------------
//
// This code plays a piano thing
//
// ---------------------------------------------------------------------------

#define F_CPU 8000000	// Clock Speed
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BAUD  9600

// To solve for TOP, Clock/[2*Prescaler(TOP+1)] = Key Frequency. Check sound key chart 
#define C3	  3822
#define D3b   3608
#define D3    3405
#define E3b   3214
#define E3    3034
#define F3    2863
#define G3b   2703
#define G3    2551
#define A3b   2408
#define A3    2273
#define B3b   2145
#define B3    2025
#define C4    1911
#define D4b   1804
#define D4    1703
#define E4b   1607
#define E4    1517
#define F4    1432
#define G4b   1351
#define G4    1276
#define A4b   1204
#define A4    1136
#define B4b   1073
#define B4    1012
#define C5    956
#define D5b   902
#define D5    851
#define E5b   804
#define E5    758
#define F5    716
#define G5b   676
#define G5    638
#define A5b   602
#define A5    568
#define B5b   536
#define B5    506
#define C6    478
#define D6b   451
#define D6    426
#define E6b   402
#define E6    379
#define F6    358
#define G6b   338
#define G6    319
#define A6b   301
#define A6    284
#define B6b   268
#define B6    253
#define C7    239

// 7 segment display
#define SEG1  4		// PB4
#define SEG2  5		// PB5
#define SEG4  1		// PC1
#define SEG5  2		// PC2
#define SEG7  3		// PC3
#define SEG10 4		// PC4
#define SEG11 5		// PC5
#define DIG1  5		// PD5
#define DIG2  6		// PD6
#define DIG3  0		// PB0
#define DIG4  7		// PB7

void usart_putchar(unsigned char ch);
void segmentDisp(char x, int y);  // Takes in integers as character data(char x) and position of array(int y) 
void printm(char *str);			 // Prints entire string
void settings8();				// Settings and configurations
void initUART();			   // Initializes UART


char chArray[5];			// This array stores a converted integer value as a character array. Mainly used for segmentDisp();
int note[49] = {C3,D3b,D3,E3b,E3,F3,G3b,G3,A3b,A3,B3b,B3, // Notes that correspond to frequencies for the built in timer.
	C4,D4b,D4,E4b,E4,F4,G4b,G4,A4b,A4,B4b,B4,			 // This array is mainly used for the analog stick component, which will bend our pitch 
	C5,D5b,D5,E5b,E5,F5,G5b,G5,A5b,A5,B5b,B5,			// upwards or downwards based on the ADC.
	C6,D6b,D6,E6b,E6,F6,G6b,G6,A6b,A6,B6b,B6,C7};       
	
unsigned char receivedChar = '\0';                   // This variable is what determines which key is pressed on the keyboard 
int noteIndex,adcVal,num,key,keyTrig,sustainPitch = 0;      // noteIndex is the note array index, adcVal is used for the analogue stick, num is
												   // the current key frequency, sustainPitch is a trigger to sustain the key or not.


int main( void )
{
	settings8();

	initUART();
	_delay_ms(10);

	_delay_ms(10);
	printm("\n\n\n\r");
	printm("\n\r	|    ***   ***	  |    ***   ***   ***    |");
	_delay_ms(10);
	printm("\n\r	|    ***   ***	  |    ***   ***   ***    |");
	_delay_ms(10);
	printm("\n\r	|    *w*   *e*	  |    *u*   *i*   *o*    |");
	_delay_ms(10);
	printm("\n\r	|    ***   ***    |    ***   ***   ***    |");
	_delay_ms(10);
	printm("\n\r	|     |     |     |     |     |     |     |       +Pitch    ");
	_delay_ms(10);
	printm("\n\r	|  a  |  s  |  d  |  f  |  j  |  k  |  l  |       -Pitch     ");
	_delay_ms(10);
	printm("\n\r	|     |     |     |     |     |     |     |");
	_delay_ms(10);
	printm("\n\r	|     |     |     |     |     |     |     |");
	printm("\n\n\n\r");


	TCCR1B &= ~_BV(CS11); // turn off the clock

	while (1){
//*****************************************************************************************
//*****************************************************************************************
//when the key is not sustained

				if(sustainPitch == 0){
		switch(receivedChar){
			case 'a': noteIndex = 12+key; break; // the UART interrupt will trigger on a key press from the keyboard
			case 'w': noteIndex = 13+key; break; // the received character will then enter the switch statement
			case 's': noteIndex = 14+key; break; // which will then give us corresponding values to our TOP frequency(Timers freq ICR1B) 
			case 'e': noteIndex = 15+key; break; // key can be used if we want to change octaves with maybe a switch
			case 'd': noteIndex = 16+key; break;
			case 'f': noteIndex = 17+key; break;
			case 'u': noteIndex = 18+key; break;
			case 'j': noteIndex = 19+key; break;
			case 'i': noteIndex = 20+key; break;
			case 'k': noteIndex = 21+key; break;
			case 'o': noteIndex = 22+key; break;
			case 'l': noteIndex = 23+key; break;
			case ';': noteIndex = 24+key; break;
			default: TCCR1B &= ~_BV(CS11);break;
		}
						
		ADCSRA |= (1 << ADSC);					// start conversion
		while ( (ADCSRA&(1<<ADIF)) == 0 );		// wait for conversion to finish
		
		if(ADC > 512){
		adcVal = note[noteIndex]*.5*((512.0-ADC)/512.0);
		num = 8000000/(2*8*(note[noteIndex]+adcVal+1));
		ICR1 = note[noteIndex]+adcVal;
		}
		else{
		adcVal =note[noteIndex]*((ADC-512.0)/512.0);
		num =  8000000/(2*8*(note[noteIndex]-adcVal*1.0));
		ICR1 = note[noteIndex]-adcVal;
		}

												// setting ICR1 will produce an external sound off the speakers
		
		
	for(int i = 0; i<17; i++){					// works as a delay.. this is so we can get the key displayed on the segment with single press
	sprintf(chArray,"%d",num);
	if(num < 1000)								// if the number is less than 1000, we can figure out whether it is a 3 digit or not
	chArray[3] = 'n';							// if it is a 3 digit, the 4rth Digit on the segment display will not show anything		
	for(int i=0; i<=4; i++)						// num is the current key frequency, which will get converted into a character array
	segmentDisp(chArray[i],i);					// so that it is displayed on the segment display
												// if it is a 3 figure value, the last element of the array will be an 'n' 
}												// these values get passed in the segmentDisp(); 4 times because there are 4 digits on the segdisplay

		sustainPitch = 3;						// this mode wont sustain the key pitch, so once it is played the speaker will play no sound
		ICR1 = 0;								

		}
		
//*****************************************************************************************	
//*****************************************************************************************	
//when the key is sustained	

		if(sustainPitch == 1){
		switch(receivedChar){
			case 'a': noteIndex = 12+key; break;
			case 'w': noteIndex = 13+key; break;
			case 's': noteIndex = 14+key; break;
			case 'e': noteIndex = 15+key; break;
			case 'd': noteIndex = 16+key; break;
			case 'f': noteIndex = 17+key; break;
			case 'u': noteIndex = 18+key; break;
			case 'j': noteIndex = 19+key; break;
			case 'i': noteIndex = 20+key; break;
			case 'k': noteIndex = 21+key; break;
			case 'o': noteIndex = 22+key; break;
			case 'l': noteIndex = 23+key; break;
			case ';': noteIndex = 24+key; break;
			default:break;
			
		}
						
		ADCSRA |= (1 << ADSC);					// start conversion
		while ( (ADCSRA&(1<<ADIF)) == 0 );		// wait for conversion to finish
		
		if(ADC > 512){
		adcVal = note[noteIndex]*.5*((512.0-ADC)/512.0);
		num = 8000000/(2*8*(note[noteIndex]+adcVal+1));
		ICR1 = note[noteIndex]+adcVal;
		}
		else{
		adcVal =note[noteIndex]*((ADC-512.0)/512.0);
		num =  8000000/(2*8*(note[noteIndex]-adcVal*1.0));
		ICR1 = note[noteIndex]-adcVal;
		}

   		sprintf(chArray,"%d",num);
		if(num < 1000){
			chArray[3] = 'n';
	chArray[4] = 'n';		
		}
		for(int i=0; i<=4; i++)
			segmentDisp(chArray[i],i);


		}
//*****************************************************************************************
//*****************************************************************************************
//basically mode 3, which is like an empty while loop

	}


	return 0;
}

void settings8(){
	TCCR1A |= _BV(COM1B1);				 //Clear OC1A/OC1B on compare match
	TCCR1B |= _BV(WGM13) |  _BV(CS11);	//mode 8, prescaler 8, PWM, Phase and Frequency Correct (TOP value is ICR1)


	DDRB  |= ( 1 << PORTB2) | (0 << PORTB6); // make Port B an output
	PORTB |= (1 << PORTB6);
	
	DDRC |= (0 << PORTC0);
	DIDR0 = 0x1;						// disable digital input on ADC0 pin

									  // ADC code
	ADMUX = 0x0;	                 // Reference = Aref, ADC0 (PC.0) used as analog input
							    	// data is right-justified
	ADCSRA = 0x87;				   // enable ADC, system clock used for A/D conversion
	ADCSRB = 0x0;				  // free running mode

	DDRD  |= (0 << PORTD3)|(0 << PORTD2)|(1 << PORTD7);	   // set pd2 and pd3 as input
	PORTD |= (1 << PORTD3)|(1 << PORTD2)|(0 << PORTD7);   // port2 and 3 pull up enabled pd7 is an LED initialy turned off

	OCR1B = 1;											// set volume off initially

	EICRA |= (1<<ISC01)|(1<<ISC10);
	EIMSK |= (1<<INT0);
	
													// 7seg code
	DDRB |= (1<<PORTB0)|(1<<PORTB4)|(1<<PORTB5)|(1<<PORTB7);
	PORTB |= (1<<PORTB0)|(1<<PORTB7);
	
	DDRC |= (1<<PORTC1)|(1<<PORTC2)|(1<<PORTC3)|(1<<PORTC4)|(1<<PORTC5);
												
	DDRD |= (1<<PORTD5)|(1<<PORTD6);
	PORTD|= (1<<PORTD5)|(1<<PORTD6);

	
	
	noteIndex = 12;				// Initialize the noteIndex and frequency to C4 
	ICR1 = C4;
}


void initUART(){
	unsigned int baudrate;

	
	baudrate = ((F_CPU/16)/BAUD) - 1;					   // Set baud rate:  UBRR = [F_CPU/(16*BAUD)] -1
	UBRR0H = (unsigned char) (baudrate >> 8);
	UBRR0L = (unsigned char) baudrate;

	UCSR0B |= (1 << RXEN0) | (1 << TXEN0);		       // Enable receiver and transmitter
	UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);		  // Set data frame: 8 data bits, 1 stop bit, no parity
	UCSR0B |= (1 << RXCIE0);						 // Enable receiver interrupt
	sei();											// Enable global interrupts
}

												
ISR (USART_RX_vect){							// UART receiver interrupt handler


	receivedChar = UDR0;				     // Read data from the RX buffer
	TCCR1B |= _BV(CS11);
	
	if(sustainPitch == 3)
	sustainPitch = 0;

	 if(receivedChar == 32){           // 32 is the ASCII code for a space
		 if(sustainPitch == 1 || sustainPitch == 3) // when the user hits the space bar they sustain the pitch or not
		 sustainPitch = 0;
		 else
		 sustainPitch = 1;				

		 PORTD ^= (1 << PORTD7);// LED indication whether the key is being sustained
	 }
	 if(receivedChar == '+')  // To increase the pitch, the plus key is pressed
	 keyTrig = 1;            // To decrease the pitch, the minus key is pressed
	 else if(receivedChar == '-')
	 keyTrig = 2;
	 

	 if(keyTrig==1&&key==0)				// key is basically increasing the index that holds the key by an octave
	 key = 12;						   // keyTrig is determined from the + or - key
	 else if(keyTrig==2&&key==12)
	 key = 0;

}


//Rotary encoder -- directional determination for volume changing using both interrupts
ISR(INT0_vect ){
					 
	int vol = OCR1B;  // to make sure lowest volume is ocr1b = 0
					 // On a the falling edge of input A if we are going CW both
					//  input A and B of the encoder match, if we go CCW A&B do not match..					
	if(bit_is_clear(PINB, PORTB6)&&bit_is_clear(PIND, PIND2))
	OCR1B += 1;
	else{
		OCR1B -= 1;	
		if(vol < 1)			//make sure volume never goes below 0
		OCR1B = 0;
	}
}

void segmentDisp(char x, int y){ // displays value of segment all the time
							
	switch(x){				  
		case '0':			 
				PORTB |= (1<<SEG1)|(1<<SEG2);						// for one digit key segment there are 7 parts 
				PORTC |= (1<<SEG4)|(1<<SEG7)|(1<<SEG10)|(1<<SEG11);// to activate on or off
				PORTC &= ~(1<<SEG5);							  // so by these 7 parts you turn on and off different parts of the digit
		break;
		case '1':
				PORTB &= ~(1<<SEG1)&~(1<<SEG2);
				PORTC |= (1<<SEG4)|(1<<SEG7);
				PORTC &= ~(1<<SEG5)&~(1<<SEG10)&~(1<<SEG11);
		break;
		case '2':
				PORTB |= (1<<SEG1)|(1<<SEG2);
				PORTC |= (1<<SEG5)|(1<<SEG7)|(1<<SEG11);
				PORTC &= ~(1<<SEG4)&~(1<<SEG10);
		break;
		case '3':
				PORTB |= (1<<SEG2);
				PORTB &= ~(1<<SEG1);
				PORTC |= (1<<SEG4)|(1<<SEG5)|(1<<SEG7)|(1<<SEG11);
				PORTC &= ~(1<<SEG10);
		break;
		case '4':
				PORTB &= ~(1<<SEG1)&~(1<<SEG2);
				PORTC |= (1<<SEG4)|(1<<SEG5)|(1<<SEG7)|(1<<SEG10);
				PORTC &= ~(1<<SEG11);
		break;
		case '5':
				PORTB |= (1<<SEG2);
				PORTB &= ~(1<<SEG1);
				PORTC |= (1<<SEG4)|(1<<SEG5)|(1<<SEG10)|(1<<SEG11);
				PORTC &= ~(1<<SEG7);
		break;
		case '6':
				PORTB |= (1<<SEG1)|(1<<SEG2);
				PORTC |= (1<<SEG4)|(1<<SEG5)|(1<<SEG10)|(1<<SEG11);
				PORTC &= ~(1<<SEG7);
		break;
		case '7':
				PORTB &= ~(1<<SEG1)&~(1<<SEG2);
				PORTC |= (1<<SEG4)|(1<<SEG7)|(1<<SEG11);
				PORTC &= ~(1<<SEG5)&~(1<<SEG10);
		break;
		case '8':
		PORTB |= (1<<SEG2)|(1<<SEG1);;
		PORTC |= (1<<SEG4)|(1<<SEG5)|(1<<SEG7)|(1<<SEG10)|(1<<SEG11);
		break;
		case '9':
				PORTB |= (1<<SEG2);
				PORTB &= ~(1<<SEG1);
				PORTC |= (1<<SEG4)|(1<<SEG5)|(1<<SEG7)|(1<<SEG10)|(1<<SEG11);
		break;
		case 'n':			// Blank 
		PORTB &= ~(1<<SEG1)&~(1<<SEG2);
		PORTC &= ~(1<<SEG4)&~(1<<SEG5)&~(1<<SEG7)&~(1<<SEG10)&~(1<<SEG11);

		
		default: break;
	}	
						   // to decide which 4 digit sections on the segment is displayed
						  // that certain pin is grounded to activate
	switch(y){			 // the segment is basically activating all 4 of these segments one at a time but super fast	
		case 0:	PORTD &= ~(1<<DIG1); _delay_ms(1); PORTD |= (1<<DIG1); break;
		case 1:	PORTD &= ~(1<<DIG2); _delay_ms(1); PORTD |= (1<<DIG2); break;
		case 2:	PORTB &= ~(1<<DIG3); _delay_ms(1); PORTB |= (1<<DIG3); break;
		case 3: PORTB &= ~(1<<DIG4); _delay_ms(1); PORTB |= (1<<DIG4); break;
		default: break;
	}
	
}

void printm(char *str){ 		// print the string

	int i = 0;

	/* loop to transmit string character by character until NULL is reached */
	while(str[i] != '\0')
	{
		usart_putchar(str[i]);
		i++;
	}
}
void usart_putchar(unsigned char ch){ 	// transmit and put a character to serial terminal
	/* wait until UDR is empty to transmit */
	while(!(UCSR0A &(1<<UDRE0)));
	UDR0 = ch;
}