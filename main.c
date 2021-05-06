/*
 * File:   main.c
 * Author: edgar
 *
 * Created on 4 de agosto de 2019, 11:27 PM
 */


#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>

volatile unsigned char adcval;
volatile unsigned char contador;
unsigned char tecla, i, error_brillo;
unsigned int brillo;

char cadena_brillo[4] = "";


void EnviaTextoUART(const char *texto);
unsigned char RecibeByte(void);
void TransmiteByte(unsigned char data);
void EnviaVoltajeUART();


int main(void) 
{
	contador = 0;
	
	//Se configuran los puertos de entrada y salida
	DDRA = 0;
	DDRB |=(1 << PB3);
	DDRD |= (1 << PD1) | (1 << PD4) | (1 << PD5) | (1 << PD7);
	
	//Se configura el ADC
	ADMUX = 0x60;
	ADCSRA = 0x8F;
	
	//Se configura el UART
	UBRRH = 0;
	UBRRL = 12;
	UCSRA = (1 << U2X);
	UCSRB = (1<<RXEN) | (1<<TXEN);
	
	//Se configuran los PWM
	TCCR0 = (1 << COM01) | (1 << COM00) | (1 << WGM01) | (1 << WGM00) | (1 << CS02) | (1 << CS00);
	TIMSK |= (1<<TOIE0);
	
	OCR0 = 0x00;
	
	TCCR1A = (1 << COM1A1) | (1 << COM1A0)  | (1 << WGM10);
	TCCR1B = (1 << CS12) | (1 << CS10) | (1 << WGM12);
	
	OCR1AL = 0x00;
	
	TCCR2 = (1 << WGM21) | (1 << WGM20) | (1 << COM21) | (1 << COM20) | (1 << CS22) | (1 << CS21) | (1 << CS20);
	OCR2 = 0x00;
	
	sei();
	
	EnviaTextoUART("ESIME Zacatenco\r\n");
	EnviaTextoUART("Alumno: Edgar Salceda\r\n");
	EnviaTextoUART("Fecha de entrega: 7 de Agosto de 2019\r\n\r\n");
	
	while(1)
	{
		EnviaTextoUART("*** Menu Principal ***\r\n\r\n");
		EnviaTextoUART("1. Ver Voltaje en potenciometro\r\n");
		EnviaTextoUART("2. Cambiar brillo color rojo\r\n");
		EnviaTextoUART("3. Cambiar brillo color verde\r\n");
		EnviaTextoUART("4. Cambiar brillo color azul\r\n\r\n");
		tecla = RecibeByte();
		
		if(tecla == '1')
		{
			EnviaVoltajeUART();
		}
		else if(tecla > 0x31U && tecla < 0x35U)
		{
			EnviaTextoUART("Ingrese el valor a 3 digitos 0 a 255\r\n\r\n");
			error_brillo = 0;
			for(i = 0; i < 3; i++)
			{
				cadena_brillo[i] = RecibeByte();
				if(cadena_brillo[i] < 0x30U || cadena_brillo[i] > 0x39)
				{
					error_brillo = 1;
					break;
				}
			}
			
			if(error_brillo)
				EnviaTextoUART("\r\nError. Tecla no valida.\r\n\r\n");
			else
			{
				EnviaTextoUART("Recibido: ");
				EnviaTextoUART(cadena_brillo);
				EnviaTextoUART("\r\n");
				
				brillo = atoi(cadena_brillo);
				
				if(brillo > 255)
					EnviaTextoUART("\r\nValor no valido.\r\n\r\n");
				else
				{
					if(tecla == '2')
						OCR0 = (unsigned char) brillo;
					else if(tecla == '3')
						OCR1A = (unsigned char) brillo;
					else
						OCR2 = (unsigned char) brillo;
					EnviaTextoUART("\r\nBrillo cambiado.\r\n\r\n");
				}
			}
				
		}
		else
			EnviaTextoUART("\r\nError. Opcion no valida.\r\n\r\n");
		
	}
	return 0;
}

void EnviaTextoUART(const char *texto)
{
	if(texto == NULL)
		return;
	while(*texto)
	{
		TransmiteByte(*texto++);
	}
}

unsigned char RecibeByte(void)
{
	while(!(UCSRA & (1<<RXC)) );
	return UDR;
}

void TransmiteByte(unsigned char data)
{
	while(!(UCSRA & (1<<UDRE)) );
	UDR =data;
}

void EnviaVoltajeUART()
{
	unsigned int voltaje;
	char cadena[50], aux1, aux2, aux3;
	voltaje = (((unsigned int)adcval) * 197) / 100;	
	
	aux1 = voltaje % 10;
	voltaje /= 10;
	aux2 = voltaje % 10;
	aux3 = voltaje / 10;
	
	sprintf(cadena, "\r\nVoltaje: %d.%d%d\r\n\r\n", aux3, aux2, aux1);
	EnviaTextoUART(cadena);
}

ISR(ADC_vect)
{
	adcval = ADCH;
}

ISR(TIMER0_OVF_vect)
{
	ADCSRA |= (1 << ADSC);
}