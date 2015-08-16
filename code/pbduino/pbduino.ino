#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <TimerThree.h>

/* Author: github2k15
   Timer3 library is required for this to work

Resources:
 1. Timer3 from http://playground.arduino.cc/Code/Timer1  
 2. http://apcmag.com/arduino-projects-digital-audio-recorder.htm/
 3. Arduino ATMEGA2560 datasheet
    http://www.atmel.com/Images/Atmel-2549-8-bit-AVR-Microcontroller-ATmega640-1280-1281-2560-2561_datasheet.pdf
	 [from https://www.arduino.cc/en/Main/ArduinoBoardMega2560]
*/


/* Network related definitions */

IPAddress dest_ip(192, 168, 10, 147);
unsigned int dest_port = 8888; 

#define cbi(flag, bit)			( flag &= ~_BV(bit) )
#define sbi(flag, bit)			( flag |= _BV(bit) )

byte our_mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress our_ip(192, 168, 10, 177);
unsigned int our_port = 8888; 

#ifdef TIMER5
const int SAMP_CLOCKS = 640;      // 160Mhz/25kHz = 640
#else // TIMER3
const int SAMP_DELAY_US = 110;
#endif

EthernetUDP Udp;


/* Sampling related definitions */

typedef unsigned char Sample;
//#define ADMUX_INIT_ONCE
#define NUM_CHANNELS          (4)
#define NUM_SAMPLES           (240)
#define SAMPLE_BYTES          (sizeof(Sample))

struct __attribute__((__packed__)) buffer_h {
	unsigned short int sample_seq;
	unsigned char num_channels;
	unsigned char num_samples;
	unsigned short int send_timestamp;
	Sample data[0];
};


/* Buffering related definitions */

#define NUM_BUFFERS              (4)
#define BUFFER_SIZE                  (sizeof(struct buffer_h) + SAMPLE_BYTES*NUM_SAMPLES*NUM_CHANNELS)
volatile byte buffer[NUM_BUFFERS][BUFFER_SIZE];

#define NEXT_BUFFER(n)           ((n+1) % NUM_BUFFERS)
#define PREV_BUFFER(n)           ((n-1) % NUM_BUFFERS)

volatile unsigned int active_buffer = 0;
struct buffer_h *active_buffer_ptr;
unsigned int samples_collected = 0;
Sample *sample_ptr = 0;
static unsigned short int sample_seq = 0;

// set to 1 in the timer code to push a packet out from loop()
volatile static byte push_pkt = 0;
volatile static byte cont_sampling = 1;


/* Misc definitions */

static int led=HIGH;
static unsigned long int count=0;



/* setup_channels() */
void setup_channels() { /* do nothing */ }


#ifdef TIMER5
inline void setup_interrupts()
{
	noInterrupts();
	TCCR5A = 0;
	TCCR5B = 0;

	TCNT5 = SAMP_CLOCKS;
	TCCR5B |= (1 << CS10 );
	TIMSK5 |= (1<<TOIE5);
	interrupts();
}
#endif


/* setup() */
void setup()
{
	setup_channels();

	Serial.begin(115200);
	while (!Serial) {
		; // wait for serial port to connect. Needed for Leonardo only
	}

	delay(1);
	Serial.println( "Serial interface ready" );

	Ethernet.begin(our_mac, our_ip);
	Udp.begin(our_port);

	delay(1);
	Serial.println( "Ethernet ready" );

#ifdef ADMUX_INIT_ONCE
	// We are doing this everytime we switch channels
	cbi(ADCSRA, ADPS2);
	sbi(ADCSRA,ADPS1);
	sbi(ADCSRA,ADPS0);

	ADMUX = 0x65;
#endif

	pinMode( 13, OUTPUT );

	active_buffer = 0;
	samples_collected = 0;
	active_buffer_ptr = (struct buffer_h *) buffer[active_buffer];
	sample_ptr = active_buffer_ptr->data;

	delay(1);
   cont_sampling = 1;
#ifdef TIMER5
	setup_interrupts();
#else
	Timer3.initialize(1);
	Timer3.attachInterrupt(do_sampling);
	Timer3.setPeriod(SAMP_DELAY_US);
#endif

}



#ifdef TIMER5
ISR(TIMER5_OVF_vect)
{
	TCNT5 = SAMP_CLOCKS;        // preload timer
	TCCR5B |= (1 << CS10 );

	if (--count==0) {
		led = ~led;
		count=1000;
	}
}
#else
void do_sampling()
{
   Sample sam;
	unsigned char i;

   if (!cont_sampling) {
      return;
   }

	for (i=0; i<NUM_CHANNELS; i++) {
		// Switch input channel
		// REFS[1:0] = 0x40 -> AVCC ref
		// ADLAR = 0x20 -> left shifted values (readin g ADCH is sufficient)
#ifndef ADMUX_INIT_ONCE
		// set prescaling
		cbi(ADCSRA, ADPS2);
		sbi(ADCSRA, ADPS1);
		sbi(ADCSRA, ADPS0);

		ADMUX = 0x60 | i;
#endif

		// start sampling
		sbi(ADCSRA, ADSC);
		while(bit_is_set(ADCSRA, ADSC)); // wait until sampling is complete

      sam = ADCH;
		*sample_ptr = sam;
		sample_ptr++;
	}

	samples_collected++; 

	if (samples_collected==NUM_SAMPLES) {
//      Timer3.stop();
      cont_sampling = 0;
//		active_buffer = NEXT_BUFFER(active_buffer);
//		active_buffer_ptr = (struct buffer_h *) buffer[active_buffer];
//		sample_ptr = active_buffer_ptr->data;
//		samples_collected = 0;

		led = led ^ 1;
		push_pkt = 1;
	}
}
#endif


void loop()
{
	struct buffer_h *send_buffer_ptr;
	digitalWrite( 13, led );


	if (push_pkt) {
		send_buffer_ptr = (struct buffer_h *) buffer[active_buffer];

		active_buffer = NEXT_BUFFER(active_buffer);
		active_buffer_ptr = (struct buffer_h *) buffer[active_buffer];

      cont_sampling = 1;
		push_pkt = 0;

		sample_ptr = active_buffer_ptr->data;
		samples_collected = 0;

		send_buffer_ptr->sample_seq = sample_seq++;
		send_buffer_ptr->num_channels = NUM_CHANNELS;
		send_buffer_ptr->num_samples = NUM_SAMPLES;
		send_buffer_ptr->send_timestamp = micros() & 0xffff;

		Udp.beginPacket(dest_ip, dest_port);
		Udp.write((char *)send_buffer_ptr, BUFFER_SIZE);
		Udp.endPacket();
	}

}

