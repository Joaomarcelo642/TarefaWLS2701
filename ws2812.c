

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

#define IS_RGBW false
#define NUM_PIXELS 25
#define WS2812_PIN 7
#define LED_RGB_PIN 13

// Pinos dos botões A e B
#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6


// Variável global para armazenar o número atual
int numero_atual = 0;

// Buffer para armazenar quais LEDs estão ligados matriz 5x5
bool led_buffer[NUM_PIXELS];

// Map dos números (0-9) em formato de 5x5
bool numeros[10][NUM_PIXELS] = {
    {1, 1, 1, 1, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 1, 1, 1, 1
    }, // 0

    {0, 1, 1, 1, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 1, 1, 0, 0,
    0, 0, 1, 0, 0
    }, // 1

    {1, 1, 1, 1, 1,
    1, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
    0, 0, 0, 0, 1,
    1, 1, 1, 1, 1}, // 2

    {1, 1, 1, 1, 1,
    0, 0, 0, 0, 1,
    1, 1, 1, 1, 1,
    0, 0, 0, 0, 1,
    1, 1, 1, 1, 1}, // 3

    {1, 0, 0, 0, 0,
    0, 0, 0, 0, 1,
    1, 1, 1, 1, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1}, // 4
    
    {1, 1, 1, 1, 1,
    0, 0, 0, 0, 1,
    1, 1, 1, 1, 1,
    1, 0, 0, 0, 0,
    1, 1, 1, 1, 1
    }, // 5

    {1, 1, 1, 1, 1,
    1, 0, 0, 0, 1,
    1, 1, 1, 1, 1,
    1, 0, 0, 0, 0,
    1, 1, 1, 1, 1}, // 6

    {0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 1, 1, 1, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 1, 1}, // 7

    {1, 1, 1, 1, 1,
    1, 0, 0, 0, 1,
    1, 1, 1, 1, 1,
    1, 0, 0, 0, 1,
    1, 1, 1, 1, 1}, // 8

    {1, 1, 1, 1, 1,
    0, 0, 0, 0, 1,
    1, 1, 1, 1, 1,
    1, 0, 0, 0, 1,
    1, 1, 1, 1, 1}  // 9
    
};

static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 20) | ((uint32_t)(g) << 0) | (uint32_t)(0);
}

void set_number(int numero)
{
    // Limpa a matriz antes de exibir o novo número
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        led_buffer[i] = 0;
    }

    // Exibe o número correspondente na matriz 5x5
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        led_buffer[i] = numeros[numero][i];
    }

    // Atualiza os LEDs com os valores do buffer
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        if (led_buffer[i])
        {
            put_pixel(urgb_u32(50, 50, 50));  // LED aceso
        }
        else
        {
            put_pixel(0);  // LED apagado
        }
    }
}

void piscar_led_rgb()
{
    // Pisca o LED RGB 5 vezes por segundo
    for (int i = 0; i < 5; i++)
    {
        gpio_put(LED_RGB_PIN, 1); // Liga o LED
        sleep_ms(100);  // 100 ms aceso
        gpio_put(LED_RGB_PIN, 0); // Desliga o LED
        sleep_ms(100);  // 100 ms apagado
    }
}

int main()
{
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    // Configura os pinos dos botões A e B
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);

    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);

    // Inicializa o pino do LED RGB
    gpio_init(LED_RGB_PIN);
    gpio_set_dir(LED_RGB_PIN, GPIO_OUT);

    piscar_led_rgb();  // Pisca o LED RGB para indicar que o programa está rodando

    while (1)
    {
        // Leitura do estado dos botões
        if (!gpio_get(BUTTON_A_PIN))  // Botão A pressionado
        {
            numero_atual = (numero_atual + 1) % 10;  // Incrementa e faz loop de 0 a 9
            set_number(numero_atual);
            sleep_ms(200);  // Debounce para evitar múltiplos incrementos
        }

        if (!gpio_get(BUTTON_B_PIN))  // Botão B pressionado
        {
            numero_atual = (numero_atual - 1 + 10) % 10;  // Decrementa e faz loop de 9 a 0
            set_number(numero_atual);
            sleep_ms(200);  // Debounce para evitar múltiplos decrementos
        }

        sleep_ms(100);  // Delay para evitar uso excessivo de CPU
    }

    return 0;
}