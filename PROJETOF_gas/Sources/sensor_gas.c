
/***********************************************************************
 * @file           : sensor_gas.c
 * @author         : Maryana Moraes, Maely Lira, Ronyer Lopes
 * @brief          : Main program body
 ***********************************************************************/

#include "sensor_gas.h"
#include "stm32f1xx.h"

static void RCC_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN
                 |  RCC_APB2ENR_IOPBEN
                 |  RCC_APB2ENR_AFIOEN
                 |  RCC_APB2ENR_USART1EN;

    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
}

static void GPIO_Init(void)
{

    /* PA0 — entrada flutuante (DOUT MQ-2): MODE=00 CNF=01 → 0x4 */
    GPIOA->CRL &= ~(0xF << 0);
    GPIOA->CRL |=  (0x4 << 0);

    /* PA2 — saída push-pull 2MHz (LED verde): MODE=10 CNF=00 → 0x2 */
    GPIOA->CRL &= ~(0xF << 8);
    GPIOA->CRL |=  (0x2 << 8);

    /* PA6 — saída push-pull 2MHz (LED vermelho): MODE=10 CNF=00 → 0x2 */
    GPIOA->CRL &= ~(0xF << 24);
    GPIOA->CRL |=  (0x2 << 24);

    /* PA9  — TX USART1: saída AF push-pull 50MHz: MODE=11 CNF=10 → 0xB */
    GPIOA->CRH &= ~(0xF << 4);
    GPIOA->CRH |=  (0xB << 4);

    /* PA10 — RX USART1: entrada flutuante: MODE=00 CNF=01 → 0x4 */
    GPIOA->CRH &= ~(0xF << 8);
    GPIOA->CRH |=  (0x4 << 8);

    /* PB1 — saída push-pull 2MHz (Buzzer): MODE=10 CNF=00 → 0x2 */
    GPIOB->CRL &= ~(0xF << 4);
    GPIOB->CRL |=  (0x2 << 4);

    /* Estado inicial */
    GPIOA->BSRR = (1 << (2  + 16));  /* LED verde off    */
    GPIOA->BSRR = (1 << (6  + 16));  /* LED vermelho off */
    GPIOB->BSRR = (1 << (1  + 16));  /* Buzzer off       */
}

//  USART1 — 9600 baud, 8N1, só TX                                   
//  BRR = fclk / baud = 8.000.000 / 9600 = 833                       
static void UART_Init(void)
{
    USART1->BRR  = 833;
    USART1->CR1  = USART_CR1_TE     
                 | USART_CR1_UE;    
}
 
static void uart_putc(char c)
{
    while (!(USART1->SR & USART_SR_TXE)) {} 
    USART1->DR = (uint8_t)c;
}

static void uart_puts(const char *s)
{
    while (*s) uart_putc(*s++);
}

//  TIM2 — contador livre, 1 tick = 1ms                               
//  PSC = 7999 → 8MHz / 8000 = 1000 Hz                               
static void Timer_Init(void)
{
    TIM2->CR1  = 0;
    TIM2->PSC  = 7999;
    TIM2->ARR  = 0xFFFFFFFF;
    TIM2->CNT  = 0;
    TIM2->EGR  = TIM_EGR_UG;   
    TIM2->SR   = 0;
    TIM2->CR1 |= TIM_CR1_CEN;
}

static uint32_t millis(void) { return TIM2->CNT; }
static uint8_t  elapsed(uint32_t start, uint32_t ms)
                            { return (millis() - start) >= ms; }

//  Helpers                                                            
static inline void led_verde_on(void)     { GPIOA->BSRR = (1 << 2);         }
static inline void led_verde_off(void)    { GPIOA->BSRR = (1 << (2 + 16));  }
static inline void led_vermelho_on(void)  { GPIOA->BSRR = (1 << 6);         }
static inline void led_vermelho_off(void) { GPIOA->BSRR = (1 << (6 + 16));  }
static inline void buzzer_on(void)        { GPIOB->BSRR = (1 << 1);         }
static inline void buzzer_off(void)       { GPIOB->BSRR = (1 << (1 + 16));  }
static inline uint8_t gas_present(void)   { return !(GPIOA->IDR & (1 << 0));}
                                              
void sensor_gas_run(void)
{
    RCC_Init();
    GPIO_Init();
    Timer_Init();
    UART_Init();

    uart_puts("\r\n== Sistema de deteccao de gas iniciando ==\r\n");

    // --- Boot: pisca LED vermelho 3x para sinalizar que o sistema está pronto ---
    uint8_t  boot_done  = 0;
    uint8_t  boot_count = 0;
    uint8_t  boot_led   = 0;
    uint32_t t_boot     = millis();

    // --- Variáveis do loop principal --- 
    uint32_t t_sensor     = millis();
    uint32_t t_led        = millis();
    uint32_t t_log        = millis();
    uint8_t  led_state    = 0;
    uint8_t  gas_detected = 0;
    uint8_t  sample_idx   = 0;
    uint8_t  gas_count    = 0;

    while (1)
    {
        uint32_t now = millis();

        //  Sequência de boot: pisca LED vermelho 3x    
        if (!boot_done)
        {
            if (elapsed(t_boot, 300))
            {
                t_boot = now;
                if (boot_led == 0) {
                    led_vermelho_on();
                    boot_led = 1;
                } else {
                    led_vermelho_off();
                    boot_led = 0;
                    boot_count++;
                    if (boot_count >= 3) {
                        boot_done = 1;
                        led_verde_on();   /* liga LED verde ao terminar boot */
                        uart_puts("Sistema pronto.\r\n");
                    }
                }
            }
            continue;
        }

        //  Leitura do sensor a cada 10ms               
        if (elapsed(t_sensor, 10))
        {
            t_sensor = now;

            if (gas_present())
                gas_count++;

            sample_idx++;

            if (sample_idx >= 3)
            {
                uint8_t prev_detected = gas_detected;
                gas_detected = (gas_count >= 2) ? 1 : 0;
                sample_idx   = 0;
                gas_count    = 0;
 
                if (gas_detected && !prev_detected) {
                    uart_puts("[ALERTA] Gas detectado!\r\n");
                    led_verde_off();
                    buzzer_on();
                }

                if (!gas_detected && prev_detected) {
                    uart_puts("[OK] Ambiente limpo.\r\n");
                    led_vermelho_off();
                    led_verde_on();
                    buzzer_off();
                    led_state = 0;
                }
            }
        }

        //  Pisca LED vermelho a 80ms quando há gás                  
        if (gas_detected && elapsed(t_led, 80))
        {
            t_led     = now;
            led_state = !led_state;
            if (led_state) led_vermelho_on();
            else           led_vermelho_off();
        }

        //  Log periódico a cada 2s                                  
        if (elapsed(t_log, 2000))
        {
            t_log = now;
            if (gas_detected) uart_puts("[STATUS] Gas presente.\r\n");
            else              uart_puts("[STATUS] Sem gas.\r\n");
        }
    }
}
