// src/main.c
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <string.h>
#include <stdbool.h>

#define LED_PORT GPIOB
#define LED_PIN  GPIO3

// --- Timer config (assumes default MSI ~4 MHz after reset).
// We make a 1 kHz tick (PSC = 4000-1), then ARR = 200-1 => 200 ms per update.
#ifdef TIM2_PSC
#undef TIM2_PSC
#define TIM2_PSC   (4000u - 1u)   // 4 MHz / 4000 = 1 kHz
#endif

#ifdef TIM2_ARR
#undef TIM2_ARR
#define TIM2_ARR   (200u  - 1u)   // 200 ticks @1 kHz = 200 ms
#endif

volatile bool blinking = false;

/* UART helpers */
void uart_puts(const char *s) { while (*s) usart_send_blocking(USART2, *s++); }

/* Clocks / GPIO / UART */
void clock_setup(void) {
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_USART2);
    rcc_periph_clock_enable(RCC_TIM2);   // <-- TIM2 clock
}

void gpio_setup(void) {
    /* LED */
    gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN);
    gpio_clear(LED_PORT, LED_PIN);

    /* USART2: PA2=TX (AF7), PA15=RX (AF3) */
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2 | GPIO15);
    gpio_set_af(GPIOA, GPIO_AF7, GPIO2);   // TX
    gpio_set_af(GPIOA, GPIO_AF3, GPIO15);  // RX
}

void usart2_setup(void) {
    usart_set_baudrate(USART2, 115200);
    usart_set_databits(USART2, 8);
    usart_set_parity(USART2, USART_PARITY_NONE);
    usart_set_stopbits(USART2, USART_STOPBITS_1);
    usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);
    usart_set_mode(USART2, USART_MODE_TX_RX);
    usart_enable(USART2);
}

/* TIM2 @ 200 ms updates */
void tim2_setup_200ms(void)
{
    /* Enable clock and hardware-reset the peripheral (L4 way) */
    rcc_periph_clock_enable(RCC_TIM2);
    rcc_periph_reset_pulse(RST_TIM2);

    /* Upcounter with internal clock, update IRQ on overflow */
    timer_set_prescaler(TIM2, TIM2_PSC);
    timer_set_period(TIM2, TIM2_ARR);

    /* Latch PSC/ARR now so first cycle is correct */
    timer_generate_event(TIM2, TIM_EGR_UG);

    /* Enable update interrupt and NVIC line */
    timer_enable_irq(TIM2, TIM_DIER_UIE);
    nvic_enable_irq(NVIC_TIM2_IRQ);
}

void blink_start(void) {
    blinking = true;
    timer_set_counter(TIM2, 0);
    timer_enable_counter(TIM2);
}

void blink_stop(void) {
    blinking = false;
    timer_disable_counter(TIM2);
    timer_clear_flag(TIM2, TIM_SR_UIF);
    gpio_clear(LED_PORT, LED_PIN);            // ensure LED is OFF
}

/* TIM2 ISR: toggle LED on each update */
void tim2_isr(void) {
    if (timer_get_flag(TIM2, TIM_SR_UIF)) {
        timer_clear_flag(TIM2, TIM_SR_UIF);
        if (blinking) gpio_toggle(LED_PORT, LED_PIN);
    }
}

int main(void) {
    clock_setup();
    gpio_setup();
    usart2_setup();
    tim2_setup_200ms();

    uart_puts("STM32L432KC ready. Commands: on | off | blink | stop\r\n> ");

    char buf[16]; int idx = 0;

    for (;;) {
        if (usart_get_flag(USART2, USART_ISR_RXNE)) {
            char c = usart_recv(USART2);

            if (c == '\r' || c == '\n') {
                buf[idx] = 0;
                if (idx > 0) {
                    // to lowercase
                    for (int i = 0; i < idx; ++i)
                        if (buf[i] >= 'A' && buf[i] <= 'Z') buf[i] += 32;

                    if (!strcmp(buf, "on")) {
                        blink_stop();
                        gpio_set(LED_PORT, LED_PIN);
                        uart_puts("\r\nLED ON\r\n> ");
                    } else if (!strcmp(buf, "off")) {
                        blink_stop();
                        gpio_clear(LED_PORT, LED_PIN);
                        uart_puts("\r\nLED OFF\r\n> ");
                    } else if (!strcmp(buf, "blink")) {
                        blink_start();
                        uart_puts("\r\nBLINK 200ms (type 'stop' to end)\r\n> ");
                    } else if (!strcmp(buf, "stop")) {
                        blink_stop();
                        uart_puts("\r\nBLINK STOPPED, LED OFF\r\n> ");
                    } else {
                        uart_puts("\r\nUnknown: on | off | blink | stop\r\n> ");
                    }
                }
                idx = 0;
            } else if (idx < (int)sizeof(buf) - 1) {
                buf[idx++] = c;
                // optional echo: usart_send_blocking(USART2, c);
            }
        }
    }
}
