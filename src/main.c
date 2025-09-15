// src/main.c - Optimized version
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <string.h>
#include <stdbool.h>

#define LED_PORT GPIOB
#define LED_PIN  GPIO3

// Timer config - more explicit and configurable
#define TIMER_FREQ_HZ    1000u    // 1 kHz timer frequency
#define BLINK_PERIOD_MS  200u     // 200ms blink period
#define SYSTEM_FREQ_HZ   4000000u // 4 MHz MSI default

#define TIM2_PSC_VAL     ((SYSTEM_FREQ_HZ / TIMER_FREQ_HZ) - 1u)
#define TIM2_ARR_VAL     ((TIMER_FREQ_HZ * BLINK_PERIOD_MS / 1000u) - 1u)

// Command buffer - reduced to 8 bytes (longest command is "blink" = 5 chars)
#define CMD_BUF_SIZE 8

// State management
typedef enum {
    LED_OFF,
    LED_ON,
    LED_BLINKING
} led_state_t;

static volatile led_state_t led_state = LED_OFF;

// --- Optimized UART functions ---
static inline void uart_putchar(char c) {
    usart_send_blocking(USART2, c);
}

static void uart_puts(const char *s) {
    while (*s) uart_putchar(*s++);
}

// --- Hardware setup functions ---
static void clock_setup(void) {
    // Enable all needed clocks in one call
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_USART2);
    rcc_periph_clock_enable(RCC_TIM2);
}

static void gpio_setup(void) {
    // LED setup
    gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN);
    gpio_clear(LED_PORT, LED_PIN);

    // USART2 pins: PA2=TX (AF7), PA15=RX (AF3)
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2 | GPIO15);
    gpio_set_af(GPIOA, GPIO_AF7, GPIO2);   // TX
    gpio_set_af(GPIOA, GPIO_AF3, GPIO15);  // RX
}

static void usart_setup(void) {
    usart_set_baudrate(USART2, 115200);
    usart_set_databits(USART2, 8);
    usart_set_parity(USART2, USART_PARITY_NONE);
    usart_set_stopbits(USART2, USART_STOPBITS_1);
    usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);
    usart_set_mode(USART2, USART_MODE_TX_RX);
    usart_enable(USART2);
}

static void timer_setup(void) {
    // Reset and configure timer
    rcc_periph_reset_pulse(RST_TIM2);
    
    timer_set_prescaler(TIM2, TIM2_PSC_VAL);
    timer_set_period(TIM2, TIM2_ARR_VAL);
    timer_generate_event(TIM2, TIM_EGR_UG);
    
    // Enable interrupt
    timer_enable_irq(TIM2, TIM_DIER_UIE);
    nvic_enable_irq(NVIC_TIM2_IRQ);
}

// --- LED control functions ---
static void led_set_state(led_state_t new_state) {
    led_state = new_state;
    
    switch (new_state) {
        case LED_OFF:
            timer_disable_counter(TIM2);
            timer_clear_flag(TIM2, TIM_SR_UIF);
            gpio_clear(LED_PORT, LED_PIN);
            break;
            
        case LED_ON:
            timer_disable_counter(TIM2);
            timer_clear_flag(TIM2, TIM_SR_UIF);
            gpio_set(LED_PORT, LED_PIN);
            break;
            
        case LED_BLINKING:
            timer_set_counter(TIM2, 0);
            timer_enable_counter(TIM2);
            break;
        default:
            // Should not reach here
            break;
    }
}

// --- Command processing ---
// Use lookup table instead of multiple strcmp calls
typedef struct {
    const char *cmd;
    led_state_t state;
    const char *response;
} command_t;

static const command_t commands[] = {
    {"on",    LED_ON,       "\r\nLED ON\r\n> "},
    {"off",   LED_OFF,      "\r\nLED OFF\r\n> "},
    {"blink", LED_BLINKING, "\r\nBLINK 200ms (type 'stop' to end)\r\n> "},
    {"stop",  LED_OFF,      "\r\nBLINK STOPPED, LED OFF\r\n> "},
    {NULL,    LED_OFF,      NULL} // Sentinel
};

static void process_command(const char *cmd) {
    for (const command_t *c = commands; c->cmd != NULL; c++) {
        if (strcmp(cmd, c->cmd) == 0) {
            led_set_state(c->state);
            uart_puts(c->response);
            return;
        }
    }
    
    // Unknown command
    uart_puts("\r\nUnknown: on | off | blink | stop\r\n> ");
}

// --- Optimized string processing ---
static inline char to_lowercase(char c) {
    return (c >= 'A' && c <= 'Z') ? c + 32 : c;
}

static void to_lowercase_string(char *str, int len) {
    for (int i = 0; i < len; i++) {
        str[i] = to_lowercase(str[i]);
    }
}

// --- Timer ISR ---
void tim2_isr(void) {
    if (timer_get_flag(TIM2, TIM_SR_UIF)) {
        timer_clear_flag(TIM2, TIM_SR_UIF);
        if (led_state == LED_BLINKING) {
            gpio_toggle(LED_PORT, LED_PIN);
        }
    }
}

// --- Main function ---
int main(void) {
    // Hardware initialization
    clock_setup();
    gpio_setup();
    usart_setup();
    timer_setup();

    uart_puts("STM32L432KC ready. Commands: on | off | blink | stop\r\n> ");

    char cmd_buf[CMD_BUF_SIZE];
    int buf_idx = 0;

    while (1) {
        if (usart_get_flag(USART2, USART_ISR_RXNE)) {
            char c = usart_recv(USART2);

            if (c == '\r' || c == '\n') {
                if (buf_idx > 0) {
                    cmd_buf[buf_idx] = '\0';
                    to_lowercase_string(cmd_buf, buf_idx);
                    process_command(cmd_buf);
                }
                buf_idx = 0;
            } else if (buf_idx < CMD_BUF_SIZE - 1) {
                cmd_buf[buf_idx++] = c;
                //echo character back
                uart_putchar(c);
            }
            else {
            // If buffer full, silently ignore additional characters
            }
        }
    }
}