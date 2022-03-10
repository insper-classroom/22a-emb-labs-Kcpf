#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

// LED
#define LED_PIO PIOA
#define LED_PIO_ID ID_PIOA
#define LED_IDX 0
#define LED_IDX_MASK (1 << LED_IDX)

// Botao
#define BUT_PIO PIOA
#define BUT_PIO_ID ID_PIOA
#define BUT_IDX  11
#define BUT_IDX_MASK (1 << BUT_IDX)

#define BUT1_PIO PIOD
#define BUT1_PIO_ID	ID_PIOD
#define BUT1_PIO_IDX 28
#define BUT1_IDX_MASK (1 << BUT1_PIO_IDX)

#define BUT2_PIO PIOC
#define BUT2_PIO_ID	ID_PIOC
#define BUT2_PIO_IDX 31
#define BUT2_IDX_MASK (1 << BUT2_PIO_IDX)

#define BUT3_PIO PIOA
#define BUT3_PIO_ID	ID_PIOA
#define BUT3_PIO_IDX 19
#define BUT3_IDX_MASK (1 << BUT3_PIO_IDX)

int frequency = 100;
char str[128];
int counter;
volatile int but_flag = 0;
volatile int but_decrease_flag = 0;
volatile int change_freq_flag = 0;
volatile int start_counter = 0;

void pisca_led(int t){

	for (int i=0;i<30;i++){
		pio_clear(LED_PIO, LED_IDX_MASK);											

		delay_ms(t);
		pio_set(LED_PIO, LED_IDX_MASK);
		delay_ms(t);
		
		if(i == 0) {
			gfx_mono_draw_string("s", i + 5, 16, &sysfont);
		}
		if (i == 5) {
			gfx_mono_draw_string("a", i + 5, 16, &sysfont);
		}
		
		if (i == 10) {
			gfx_mono_draw_string("m", i + 5, 16, &sysfont);
		}
		
		if (i == 15) {
			gfx_mono_draw_string("p", i + 5, 16, &sysfont);
		}
		
		if (i == 20) {
			gfx_mono_draw_string("a", i + 5, 16, &sysfont);
		}
		
		if (i == 25) {
			gfx_mono_draw_string("s", i + 5, 16, &sysfont);
		}
		
		if (i == 26) {
			gfx_mono_draw_string("!", i + 5, 16, &sysfont);
		}
	}
	
	gfx_mono_draw_string("    ", 0, 16, &sysfont);
}

void configure_button(Pio *p_pio, const uint32_t ul_mask, uint32_t ul_id, void (*p_handler) (uint32_t, uint32_t), int only_rise) {
	pmc_enable_periph_clk(ul_id);

	// Configura PIO para lidar com o pino do botão como entrada
	// com pull-up
	pio_configure(p_pio, PIO_INPUT, ul_mask, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(p_pio, ul_mask, 60);

	// Configura interrupção no pino referente ao botao e associa
	// função de callback caso uma interrupção for gerada
	// a função de callback é a: but_callback()
	pio_handler_set(p_pio,
		ul_id,
		ul_mask,
		only_rise ? PIO_IT_RISE_EDGE : PIO_IT_EDGE,
		p_handler
	);

	// Ativa interrupção e limpa primeira IRQ gerada na ativacao
	pio_enable_interrupt(p_pio, ul_mask);
	pio_get_interrupt_status(p_pio);
	
	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(ul_id);
	NVIC_SetPriority(ul_id, 4); // Prioridade 4
}

void but_callback(void) {
	but_flag = 1;
}

void but1_callback(void) {
	start_counter = (pio_get(BUT1_PIO, PIO_INPUT, BUT1_IDX_MASK)) ? 0 : 1;
	change_freq_flag = 1;
}

void but3_callback(void) {
	but_decrease_flag = 1;
}

void draw(int x, int y) {
	gfx_mono_draw_string("           ", 0, 16, &sysfont);
	sprintf(str, "%d", frequency);
	gfx_mono_draw_string(str, x, y, &sysfont);
}

void decrease(int *freq) {
	*freq -= 100;
}

void increase(int *freq) {
	*freq += 100;
}


void io_init(void) {

	// Configura led
	pmc_enable_periph_clk(LED_PIO_ID);
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEFAULT);
	
	
	// Configura botoes
	configure_button(BUT_PIO, BUT_IDX_MASK, BUT_PIO_ID, but_callback, 1);
	configure_button(BUT1_PIO, BUT1_IDX_MASK, BUT1_PIO_ID, but1_callback, 0);
	configure_button(BUT3_PIO, BUT3_IDX_MASK, BUT3_PIO_ID, but3_callback, 1);
}

int main (void) {
	board_init();
	sysclk_init();
	delay_init();
	
	// Desativa watchdog
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	// configura botao com interrupcao
	io_init();
	
  // Init OLED
	gfx_mono_ssd1306_init();
	draw(50, 16);

  /* Insert application code here, after the board has been2 initialized. */
	while(1) {
		
		if (but_flag) {
			 pisca_led(10000/frequency);
			 but_flag = 0;
		}
		
		if (but_decrease_flag) {
			decrease(&frequency);
			draw(50, 16);
			but_decrease_flag = 0;
		}
		
		if (change_freq_flag) {
			if(start_counter) {
				counter++;
			} else {
				if (counter > 10000000) {
					decrease(&frequency);
				}
				else {
					increase(&frequency);
				}
				
				counter = 0;
				change_freq_flag = 0;
				draw(50, 16);
			}
		}
		
	}
}
