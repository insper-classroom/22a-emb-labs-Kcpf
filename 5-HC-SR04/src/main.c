#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

typedef struct  {
	uint32_t year;
	uint32_t month;
	uint32_t day;
	uint32_t week;
	uint32_t hour;
	uint32_t minute;
	uint32_t second;
} calendar;

volatile uint32_t counter_rtt = 0;
volatile char start = 1;
volatile char stop = 0;
volatile char error_flag = 0;
char str[128];

#define TRIGGER_PIO PIOA
#define TRIGGER_PIO_ID ID_PIOA
#define TRIGGER_PIO_IDX 6
#define TRIGGER_PIO_IDX_MASK (1u << TRIGGER_PIO_IDX)

#define ECHO_PIO PIOD
#define ECHO_PIO_ID ID_PIOD
#define ECHO_PIO_IDX 30
#define ECHO_PIO_IDX_MASK (1u << ECHO_PIO_IDX)

void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource) {

	uint16_t pllPreScale = (int) (((float) 32768) / freqPrescale);
	
	rtt_sel_source(RTT, false);
	rtt_init(RTT, pllPreScale);
	
	if (rttIRQSource & RTT_MR_ALMIEN) {
		uint32_t ul_previous_time;
		ul_previous_time = rtt_read_timer_value(RTT);
		while (ul_previous_time == rtt_read_timer_value(RTT));
		rtt_write_alarm_time(RTT, IrqNPulses+ul_previous_time);
	}

	/* config NVIC */
	NVIC_DisableIRQ(RTT_IRQn);
	NVIC_ClearPendingIRQ(RTT_IRQn);
	NVIC_SetPriority(RTT_IRQn, 4);
	NVIC_EnableIRQ(RTT_IRQn);

	/* Enable RTT interrupt */
	if (rttIRQSource & (RTT_MR_RTTINCIEN | RTT_MR_ALMIEN))
	rtt_enable_interrupt(RTT, rttIRQSource);
	else
	rtt_disable_interrupt(RTT, RTT_MR_RTTINCIEN | RTT_MR_ALMIEN);
	
}

void RTT_Handler(void) {
	uint32_t ul_status;

	/* Get RTT status - ACK */
	ul_status = rtt_get_status(RTT);

	/* IRQ due to Alarm */
	if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS) {
		error_flag = 1;
	}
	
	/* IRQ due to Time has changed */
	if ((ul_status & RTT_SR_RTTINC) == RTT_SR_RTTINC) {
		// BLINK Led
	}

}

void signalCallback(void) {
	if(pio_get(ECHO_PIO, PIO_INPUT, ECHO_PIO_IDX_MASK)){
		RTT_init(32000, 753, RTT_MR_ALMIEN);
	} else {
		counter_rtt = rtt_read_timer_value(RTT);
		stop = 1;
	}
}


int main (void)
{
	board_init();
	sysclk_init();
	
	pmc_enable_periph_clk(TRIGGER_PIO_ID);
	pmc_enable_periph_clk(ECHO_PIO_ID);
	
	pio_set_output(TRIGGER_PIO, TRIGGER_PIO_IDX_MASK, 0, 0, 0);
	pio_set_input(ECHO_PIO, ECHO_PIO_IDX_MASK, 0);
	
	pio_handler_set(
		ECHO_PIO,
		ECHO_PIO_ID,
		ECHO_PIO_IDX_MASK,
		PIO_IT_EDGE,
		signalCallback
	);
	
	pio_enable_interrupt(ECHO_PIO, ECHO_PIO_IDX_MASK);
	NVIC_EnableIRQ(ECHO_PIO_ID);
	NVIC_SetPriority(ECHO_PIO_ID, 4);
	
	gfx_mono_ssd1306_init();
	
	double distance = 0;
	double time = 0;
	

  /* Insert application code here, after the board has been initialized. */
	while(1) {
		if (start) {
 			pio_set(TRIGGER_PIO, TRIGGER_PIO_IDX_MASK);
			delay_us(10);
			pio_clear(TRIGGER_PIO, TRIGGER_PIO_IDX_MASK);
			
			start = 0;
		}
		
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
		
		if (stop) {
			time = (double) counter_rtt / 32000;
			distance = time * 170;
			
			gfx_mono_draw_string("           ", 0, 0, &sysfont);
			sprintf(str, "%.2lf cm", distance * 100);
			gfx_mono_draw_string(str, 0,0, &sysfont);
			
			stop = 0;
			start = 1;
		}
		
		if (error_flag) {
			gfx_mono_draw_string("           ", 0, 0, &sysfont);
			sprintf(str, "Mau Contato");
			gfx_mono_draw_string(str, 0,0, &sysfont);
			
			error_flag = 0;
			start = 1;
		}
	}
}
