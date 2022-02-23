/**
 * 5 semestre - Eng. da Computação - Insper
 * Rafael Corsi - rafael.corsi@insper.edu.br
 *
 * Projeto 0 para a placa SAME70-XPLD
 *
 * Objetivo :
 *  - Introduzir ASF e HAL
 *  - Configuracao de clock
 *  - Configuracao pino In/Out
 *
 * Material :
 *  - Kit: ATMEL SAME70-XPLD - ARM CORTEX M7
 */

// Botão 1 - Pin Number 9 PD28
// Botão 2 - Pin Number 3 PC31
// Botão 3 - Pin Number 4 PA19

// LED 1 - PIN number 7 PA0
// LED 2 - PIN number 8 PC30
// LED 3 - PIN number 6 PB2

/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include "asf.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/

#define LED1_PIO PIOA           
#define LED1_PIO_ID ID_PIOA    
#define LED1_PIO_IDX 0       
#define LED1_PIO_IDX_MASK (1u << LED1_PIO_IDX)  

#define LED2_PIO PIOC
#define LED2_PIO_ID ID_PIOC
#define LED2_PIO_IDX 30
#define LED2_PIO_IDX_MASK (1u << LED2_PIO_IDX)

#define LED3_PIO PIOB
#define LED3_PIO_ID ID_PIOB
#define LED3_PIO_IDX 2
#define LED3_PIO_IDX_MASK (1u << LED3_PIO_IDX)

// Configuracoes do botao
#define BUT1_PIO PIOD
#define BUT1_PIO_ID ID_PIOD
#define BUT1_PIO_IDX 28
#define BUT1_PIO_IDX_MASK (1 << BUT1_PIO_IDX)

#define BUT2_PIO PIOC
#define BUT2_PIO_ID ID_PIOC
#define BUT2_PIO_IDX 31
#define BUT2_PIO_IDX_MASK (1 << BUT2_PIO_IDX)

#define BUT3_PIO PIOA
#define BUT3_PIO_ID ID_PIOA
#define BUT3_PIO_IDX 19
#define BUT3_PIO_IDX_MASK (1 << BUT3_PIO_IDX)


/************************************************************************/
/* constants                                                            */
/************************************************************************/

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/

/************************************************************************/
/* prototypes                                                           */
/************************************************************************/

void init(void);

/************************************************************************/
/* interrupcoes                                                         */
/************************************************************************/

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

// Função de inicialização do uC
void init(void)
{
	// Initialize the board clock
	sysclk_init();

	// Desativa WatchDog Timer
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	// Ativa o PIO na qual os LEDs foram conectados
	pmc_enable_periph_clk(LED1_PIO_ID);
	pmc_enable_periph_clk(LED2_PIO_ID);
	pmc_enable_periph_clk(LED3_PIO_ID);
	
	// Inicializa PIOs dos botoes
	pmc_enable_periph_clk(BUT1_PIO_ID);
	pmc_enable_periph_clk(BUT2_PIO_ID);
	pmc_enable_periph_clk(BUT3_PIO_ID);

	//Inicializa PC8 como saída
	pio_set_output(LED1_PIO, LED1_PIO_IDX_MASK, 0, 0, 0);
	pio_set_output(LED2_PIO, LED2_PIO_IDX_MASK, 0, 0, 0);
	pio_set_output(LED3_PIO, LED3_PIO_IDX_MASK, 0, 0, 0);
	
	pio_set_input(BUT1_PIO, BUT1_PIO_IDX_MASK, PIO_DEFAULT);
	pio_set_input(BUT2_PIO, BUT2_PIO_IDX_MASK, PIO_DEFAULT);
	pio_set_input(BUT3_PIO, BUT3_PIO_IDX_MASK, PIO_DEFAULT);
	
	pio_pull_up(BUT1_PIO, BUT1_PIO_IDX_MASK, 1);
	pio_pull_up(BUT2_PIO, BUT2_PIO_IDX_MASK, 1);
	pio_pull_up(BUT3_PIO, BUT3_PIO_IDX_MASK, 1);


}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/

// Funcao principal chamada na inicalizacao do uC.
int main(void)
{
  init();

  // super loop
  // aplicacoes embarcadas não devem sair do while(1).
  while (1)
  {
	if(!pio_get(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK)) {
		for (int c = 0; c < 5; c++) {
		// coloca 0 no pino do LED
		pio_clear(LED1_PIO, LED1_PIO_IDX_MASK);
		delay_ms(200);
			
		pio_set(LED1_PIO, LED1_PIO_IDX_MASK);
		delay_ms(200);
		}
	} else {
		pio_set(LED1_PIO, LED1_PIO_IDX_MASK);
	}
	  
	if(!pio_get(BUT2_PIO, PIO_INPUT, BUT2_PIO_IDX_MASK)) {
		for (int c = 0; c < 5; c++) {
			// coloca 0 no pino do LED
			pio_clear(LED2_PIO, LED2_PIO_IDX_MASK);
			delay_ms(200);
			  	  
			pio_set(LED2_PIO, LED2_PIO_IDX_MASK);
			delay_ms(200);
		}
		} else {
		pio_set(LED2_PIO, LED2_PIO_IDX_MASK);
	}
	
	if(!pio_get(BUT3_PIO, PIO_INPUT, BUT3_PIO_IDX_MASK)) {
		for (int c = 0; c < 5; c++) {
			// coloca 0 no pino do LED
			pio_clear(LED3_PIO, LED3_PIO_IDX_MASK);
			delay_ms(200);
				
			pio_set(LED3_PIO, LED3_PIO_IDX_MASK);
			delay_ms(200);
		}
		} else {
		pio_set(LED3_PIO, LED3_PIO_IDX_MASK);
	}
			
			
  }
  return 0;
}
