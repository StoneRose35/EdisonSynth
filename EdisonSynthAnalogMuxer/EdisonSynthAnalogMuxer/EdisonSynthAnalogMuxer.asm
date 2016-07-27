/*
 * EdisonSynthAnalogMuxer.asm
 *
 *  Created: 15.12.2015 20:42:04
 *   Author: fuerh_000
 */ 

 /* CPU is at 2MHz
 SCL frequency should be 62500 Hz

 portd0-2 is an output which should be connected to addres of the external multiplexers (CD4051!?)
 the twi slave address is 42
 */

 #include <m16def.inc>
 .equ N_CHANNELS = 0x40
.org 0x000
jmp reset


// interrupt for TWI
.org TWIaddr
jmp TWIInterrupt 

.org OVF0addr
jmp Counter0Interrupt


// starting point of the application
 .org 0x038
 reset:


 // init stack pointer
  ldi r16, high(RAMEND)
 out SPH, r16
 ldi r16, low(RAMEND)
 out SPL, r16

 // set the bankActive variable
 //lds r16,bankActive
 ori r16,0x01
 sts bankActive,r16


 // setup twi
 //*******************33
 ldi r16,(1<<TWEA)|(1<<TWEN)|(1<<TWIE)
 out TWCR,r16


 // address is 42
 ldi r16,0x54
 out TWAR,r16

 // set bit rate register
 ldi r16,0x02
 out TWBR,r16


 // setup adc's
 // ***********************3
 ldi r16,(1<<REFS0)|(1<<ADLAR)
 out ADMUX,r16

 ldi r16,(1<<ADEN)|(0<<ADSC)|(1<<ADPS1)|(1<<ADPS0)
 out ADCSRA,r16



 // set portd to out
 ldi r16,0b01100111
 out DDRD,r16


 // setup counter 1 for fast 8-bit pwm mode generating a square wave to feed the charge pumpt for the lcd contrast voltag
 ldi r16,0b10000001
 out TCCR1A,r16
 ldi r16,0b00001001
 out TCCR1B,r16
 ldi r16,0x80
 ldi r17,0x00
 out OCR1AH,r17
 out OCR1AL,r16


 // enable interrupts
 sei

 // THIS IS THE MAIN ROUTINE
 main:
 
 ldi r19,0x00
 rjmp ADCReadOutLoop

 rjmp main

 ADCReadOutLoop:
 // set multiplexer channel ,which are the tree lsb's of the channel counter r19
 in r17,ADMUX
 andi r17,0b11100000
 mov r16,r19
 andi r16,0x07
 or r17,r16
 out ADMUX,r17
 
 // set the external multiplexer channel, which are the uppermost three bits
 mov r16,r19
 lsr r16
 lsr r16
 lsr r16
 out portd,r16


 // wait loop between external muxer set and adc begin
ldi r17,0x50
cont5:
dec r17
brne cont5


 // start a adc conversion
 in r18,ADCSRA
 ori r18,(1<<ADSC)
 out ADCSRA,r18
 
 // wait until result is there
 waitForAdc:
 in r18,ADCSRA
 sbrs r18,ADIF
 rjmp waitForAdc

 // check which bank should be updated
 lds r16,bankActive
 sbrs r16,0
 rjmp updateBankB

 updateBankA:
 in r16,ADCH
 ldi XH,HIGH(controllerValuesA)
 ldi XL,LOW(controllerValuesA)
 add XL,r19
 ldi r17,0x00
 adc XH,r17
 st X,r16
 rjmp cont1

 updateBankB:
 in r16,ADCH
 ldi XH,HIGH(controllerValuesB)
 ldi XL,LOW(controllerValuesB)
 add XL,r19
 ldi r17,0x00
 adc XH,r17
 st X,r16

 cont1:
  ori r18,(1<<ADIF)
 out ADCSRA,r18
 inc r19
 cpi r19,N_CHANNELS
 brne ADCReadOutLoop
 // 64 values read
 ldi r19,0x00 
 lds r16,bankActive
 sbrs r16,0
 rjmp setAupdating
 ori r16,0x02 // set bank a ready
 andi r16,0b11111110 // set bank b "updating"
 sts bankActive,r16
 rjmp ADCReadOutLoop
setAupdating:
 ori r16,0x05 // set bank b ready (4) and a updating (1)
  sts bankActive,r16
 rjmp ADCReadOutLoop

 // interrupt handler for TWI events
 TWIInterrupt:
 push r16
 push r17
 push r18 
 push r19
 in r16,SREG
 push r16 

 // check status register
 in r16,TWSR
 andi r16,0xF8

 cpi r16,0xA8 // address+r has been received, start transmission 
 brne checkDataByteTransmitted


 //  address plus read has been received, start sending first byte
 
 // loading 0 into TWI counter
 ldi r19,0x00
 sts twiCounter,r19

 // check which bank is active
 sendValues: // twi counter is in r19
 lds r16,bankActive
 sbrs r16,0
 rjmp sendValues_sendBankA
 // send bank B
 sendValues_sendBankB:
  ldi XH,HIGH(controllerValuesB)
 ldi XL,LOW(controllerValuesB)
 add XL,r19
 ldi r16,0x00
 adc XH,r16
 ld r17,X
 out TWDR,r17 // put value from memory to twi data register
 in r16,TWSR
 ori r16,(1 << TWIE)|(1<<TWEA)
 out TWSR,r16
 rjmp   sendValues_incCounter

 // send bank A
 sendValues_sendBankA:
 ldi XH,HIGH(controllerValuesA)
 ldi XL,LOW(controllerValuesA)
 add XL,r19
 ldi r16,0x00
 adc XH,r16
 ld r17,X
 out TWDR,r17 // put value from memory to twi data register
 in r16,TWSR
 ori r16,(1 << TWIE)|(1<<TWEA)
 out TWSR,r16
//rjmp  sendValues_incCounter

 sendValues_incCounter:
 inc r19
 sts twiCounter,r19
 rjmp returnFromTwiInt

 // check for case "$B8", data byte transmitted and ACK received
 checkDataByteTransmitted:
 cpi r16,0xB8
 brne errorcase

// data has been transmitted and ACK has been received
lds r19,twiCounter
cpi r19,N_CHANNELS
brne sendValues // send values if last hasn't been sent yet
ldi r19,0x00
rcall switchOnDebugLed
sts twiCounter,r19
rjmp returnFromTwiInt

errorcase:
rcall switchOnDebugLed

 returnFromTwiInt:
 // clear interrupt flag
 in r16,TWCR
 ori r16,(1<<TWINT)
 out TWCR,r16


 pop r16
 out SREG,r16
 pop r19
 pop r18
 pop r17
 pop r16
 reti


 switchOnDebugLed:
 push r16
 in r16,PORTD
 ori r16,0b01000000
 out PORTD,r16
 ldi r16,0x05
 out TCCR0,r16
 in r16,TIMSK
 ori r16,(1<<TOIE0)
 out TIMSK,r16

 pop r16
 ret

 // switches off the debug led
 Counter0Interrupt:
 push r16
 in r16,SREG
 push r16


 in r16,PORTD
 andi r16,0b10111111
 out PORTD,r16
 ldi r16,0x00
 out TCCR0,r16 // stop the clock

 pop r16
 out SREG,r16
 pop r16
 reti

 // program variables
 .dseg
 .org SRAM_START
 controllerValuesA:
 .byte 64
 controllerValuesB:
 .byte 64
 bankActive: 
 // bank control variable, BIT 0: bank active (1 is bank A, 0 is bank B)
 // BIT 1: Bank A ready (1: yes!!, 0: no)
 // BIT 2: Bank B ready (1: yes!!, 0: no)
 .byte 1
 twiCounter:
 .byte 1
