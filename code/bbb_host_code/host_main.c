#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>

#include "prussdrv.h"
#include "pruss_intc_mapping.h"
#include "common/mio.h"
#include "../bbb_common/pb_msg.h"
#include "../bbb_common/msg_queue.h"

#define PERROR() printf("[!] %s:%u\n", __FUNCTION__, __LINE__)

/* doc: spruh73c, table 2.2 */
#define CM_WKUP_MIO_ADDR 0x44e00400
#define CM_WKUP_MIO_SIZE 0x1000

/* doc: spruh73c, table 8.28 */
#define CM_WKUP_ADC_TSC_CLKCTRL 0xbc


#define PRU_NUM 0


/* host pru shared memory */
static void zero_words(size_t n)
{
	mio_handle_t mio;
	size_t i;

	if (mio_open(&mio, 0x80001000, 0x1000))
	{
		printf("unable to zero_words\n");
		return ;
	}

	for (i = 0; i != n; ++i)
		mio_write_uint32(&mio, i * sizeof(uint32_t), 0);

	mio_close(&mio);
}


static int cm_wkup_enable_adc_tsc(void)
{
	mio_handle_t mio;

	if (mio_open(&mio, CM_WKUP_MIO_ADDR, CM_WKUP_MIO_SIZE)) return -1;
	mio_write_uint32(&mio, CM_WKUP_ADC_TSC_CLKCTRL, 2);
	mio_close(&mio);

	return 0;
}


static int
read_header( struct header *hp )
{
	static const size_t sharedram_offset = 2048;
	volatile uint32_t* p;

	prussdrv_map_prumem(PRUSS0_SHARED_DATARAM, (void**)&p);

   memcpy( hp, &p[sharedram_offset], sizeof( struct header ) );

	return 0;
}


static int
read_channel_data( struct header *hp, msg *m )
{
	static const size_t sharedram_offset = 2048 * 4;
	size_t data_offset = sizeof( struct header ) + ( ( hp->seqno - 1 ) % 2 ) * MSG_DATA_SIZE;
	volatile char *p;

	prussdrv_map_prumem( PRUSS0_SHARED_DATARAM, (void**)&p );

   m->mtype = MG_TYPE;
   memcpy( &m->data, &p[sharedram_offset + data_offset], MSG_DATA_SIZE );

	return 0;
}


static volatile unsigned int is_sigint = 0;


static void on_sigint(int x)
{
	is_sigint = 1;
}


/* doc: spruh73c, table 2.2 */
#define CM_PER_MIO_ADDR 0x44e00000
#define CM_PER_MIO_SIZE (128 * 1024)

/* doc: spruh73c, table 2.2 */
#define CM_WKUP_MIO_ADDR 0x44e00400
#define CM_WKUP_MIO_SIZE 0x1000

/* doc: spruh73c, table 8.28 */
#define CM_PER_EPWMSS0_CLKCTRL 0xd4
#define CM_PER_EPWMSS1_CLKCTRL 0xcc
#define CM_PER_EPWMSS2_CLKCTRL 0xd8

/* doc: spruh73c, table 8.28 */
#define CM_WKUP_ADC_TSC_CLKCTRL 0xbc

/* doc: spruh73c, table 2.2 */
#define CM_WKUP_MIO_ADDR 0x44e00400
#define CM_WKUP_MIO_SIZE 0x1000

/* doc: spruh73c, table 8-88 */
#define CM_CLKSEL_DPLL_CORE 0x68

#define CM_CLKDCOLDO_DPLL_PER 0x7c
#define CM_DIV_M4_DPLL_CORE 0x80
#define CM_CLKMODE_DPLL_PER 0x8c

#define CM_CLKSEL_DPLL_PERIPH 0x9c
#define CM_DIV_M2_DPLL_MPU 0xa8


/* tsc adc */

/* doc: spruh73c, table 2.2 */
#define ADC_MIO_ADDR 0x44e0d000
#define ADC_MIO_SIZE (8 * 1024)

/* doc: spruh73c, table 12.4 */
#define ADC_REG_SYSCONFIG 0x10
#define ADC_REG_IRQENABLE_SET 0x2c
#define ADC_REG_IRQWAKEUP 0x34
#define ADC_REG_DMAENABLE_SET 0x38
#define ADC_REG_CTRL 0x40
#define ADC_REG_ADC_CLKDIV 0x4c
#define ADC_REG_STEPENABLE 0x54
#define ADC_REG_TS_CHARGE_STEPCONFIG 0x5c
#define ADC_REG_TS_CHARGE_DELAY 0x60
#define ADC_REG_STEP1CONFIG 0x64
#define ADC_REG_STEP2CONFIG 0x6C
#define ADC_REG_STEP3CONFIG 0x74
#define ADC_REG_STEP4CONFIG 0x7C
#define ADC_REG_FIFO0COUNT 0xe4
#define ADC_REG_FIFO0DATA 0x100

static int adc_setup(void)
{
	mio_handle_t mio;

	/* enable adc_tsc clocking */
	if (cm_wkup_enable_adc_tsc())
	{
		PERROR();
		return -1;
	}

	if (mio_open(&mio, ADC_MIO_ADDR, ADC_MIO_SIZE))
	{
		PERROR();
		return -1;
	}

	/* disable the module, enable step registers writing */
	mio_write_uint32(&mio, ADC_REG_CTRL, 1 << 2);

	/* setup adc */
	mio_write_uint32(&mio, ADC_REG_SYSCONFIG, 1 << 2);
	mio_write_uint32(&mio, ADC_REG_IRQENABLE_SET, 0);
	mio_write_uint32(&mio, ADC_REG_IRQWAKEUP, 0);
	mio_write_uint32(&mio, ADC_REG_DMAENABLE_SET, 0);
	mio_write_uint32(&mio, ADC_REG_ADC_CLKDIV, 0);
	mio_write_uint32(&mio, ADC_REG_TS_CHARGE_DELAY, 1);
	// sb original was 1 << 19
	//mio_write_uint32(&mio, ADC_REG_TS_CHARGE_STEPCONFIG, 1 << 19);
	mio_write_uint32(&mio, ADC_REG_TS_CHARGE_STEPCONFIG, (0x100) << 19);

	/* enable channel 1 */
	mio_write_uint32(&mio, ADC_REG_STEPENABLE,
			( 1 << 4 ) | ( 1 << 3 ) | ( 1 << 2 ) |
			(1 << 1) | (1 << 0) );

	//#define AVG		(0x100)
#define AVG		(0x000)

	/* channel 1, continuous mode, averaging = 2 */
	mio_write_uint32(&mio, ADC_REG_STEP1CONFIG, AVG | 1 );

	/* channel 2, continuous mode, averaging = 2 */

	mio_write_uint32(&mio, ADC_REG_STEP2CONFIG, AVG | 1 | (1 << 19));
	mio_write_uint32(&mio, ADC_REG_STEP3CONFIG, AVG | 1 | (0x2 << 19));
	mio_write_uint32(&mio, ADC_REG_STEP4CONFIG, AVG | 1 | (0x3 << 19));

	/* enable the module */
	mio_write_uint32(&mio, ADC_REG_CTRL, 1);
	mio_close(&mio);

	return 0;
}



/* main */

int
main( int argc, char *argv[] )
{
   // Buffer written into
   struct header hdr;
   msg m;

   int qid;

	tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

	adc_setup();
	prussdrv_init();
   qid = mg_open( "/tmp/pb_msgqueue" );

	if ( prussdrv_open( PRU_EVTOUT_0 ) ) {
		printf( "prussdrv_open open failed\n" );
		return -1;
	}

	prussdrv_pruintc_init( &pruss_intc_initdata );

	// zero_words(n);

	// Write data and execution code on PRU0
	prussdrv_load_datafile( PRU_NUM, "./data.bin" );
	prussdrv_exec_program_at( PRU_NUM, "./text.bin", START_ADDR );

	signal( SIGINT, on_sigint );

	while (is_sigint == 0) {
      prussdrv_pru_wait_event( PRU_EVTOUT_0 );
      prussdrv_pru_clear_event( PRU_EVTOUT_0, PRU0_ARM_DONE_INTERRUPT );

      read_header( &hdr );

      read_channel_data( &hdr, &m );

      mg_send( qid, &m );
	}

	// Disable pru and close memory mapping
	prussdrv_pru_disable(PRU_NUM);
	prussdrv_exit();

   // Release message queue
   mg_release( qid );

	return 0;
}


