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

volatile int counter_tc = 0;
volatile char start = 1;
volatile char stop = 0;
volatile char oled_flag = 0;
char str[128];

#define TRIGGER_PIO PIOA
#define TRIGGER_PIO_ID ID_PIOA
#define TRIGGER_PIO_IDX 6
#define TRIGGER_PIO_IDX_MASK (1u << TRIGGER_PIO_IDX)

#define ECHO_PIO PIOD
#define ECHO_PIO_ID ID_PIOD
#define ECHO_PIO_IDX 30
#define ECHO_PIO_IDX_MASK (1u << ECHO_PIO_IDX)

void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq){
	uint32_t ul_div;
	uint32_t ul_tcclks;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();

	/* Configura o PMC */
	pmc_enable_periph_clk(ID_TC);

	/** Configura o TC para operar em  freq hz e interrupçcão no RC compare */
	tc_find_mck_divisor(freq, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
	tc_init(TC, TC_CHANNEL, ul_tcclks | TC_CMR_CPCTRG);
	tc_write_rc(TC, TC_CHANNEL, (ul_sysclk / ul_div) / freq);

	/* Configura NVIC*/
	NVIC_SetPriority(ID_TC, 4);
	NVIC_EnableIRQ((IRQn_Type) ID_TC);
	tc_enable_interrupt(TC, TC_CHANNEL, TC_IER_CPCS);
}

void TC3_Handler(void) {
	/**
	* Devemos indicar ao TC que a interrupção foi satisfeita.
	* Isso é realizado pela leitura do status do periférico
	**/
	volatile uint32_t status = tc_get_status(TC1, 0);

	/** Muda o estado do LED (pisca) **/
	oled_flag = 1; 
}


void signalCallback(void) {
	if(pio_get(ECHO_PIO, PIO_INPUT, ECHO_PIO_IDX_MASK)){
		tc_start(TC0, 0);
	} else {
		tc_stop(TC0, 0);
		counter_tc = tc_read_cv(TC0, 0);
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
	
	TC_init(TC0, ID_TC0, 0, 42);
	
	TC_init(TC1, ID_TC3, 0, 4);
	tc_start(TC1, 0);
	
	double distance = 0;
	double time = 0;
	

  /* Insert application code here, after the board has been initialized. */
	while(1) {
		//pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
		
		if (start) {
			pio_set(TRIGGER_PIO, TRIGGER_PIO_IDX_MASK);
			delay_us(10);
			pio_clear(TRIGGER_PIO, TRIGGER_PIO_IDX_MASK);
			
			start = 0;
		}
		
		if (stop) {
			time = (double) counter_tc / 32000.0;
			distance = time * 170;
			
			stop = 0;
			start = 1;
		}
		
		if (oled_flag) {
			gfx_mono_draw_string("           ", 0, 0, &sysfont);
			sprintf(str, "%lf", distance);
			gfx_mono_draw_string(str, 0,0, &sysfont);
			
			oled_flag = 0;
		}
		
		
		
	}
}
