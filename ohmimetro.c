#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/clocks.h"        
#include "lib/ssd1306.h"
#include "lib/font.h"
#include <math.h>
#include <string.h>

#include "animacao_matriz.pio.h" // Biblioteca PIO para controle de LEDs WS2818B

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define ADC_PIN 28 // GPIO para o voltímetro
#define E24_SIZE   24
#define TOLERANCIA 0.05
#define LED_COUNT 25
#define MATRIZ_PIN 7            // Pino GPIO conectado aos LEDs WS2818B

int R_conhecido = 10000;   // Resistor de 10k ohm
float R_x = 0.0;           // Resistor desconhecido
float ADC_VREF = 3.31;     // Tensão de referência do ADC
int ADC_RESOLUTION = 4095; // Resolução do ADC (12 bits)
PIO pio;
uint sm;



// Matriz para armazenar os desenhos da matriz de LEDs
uint padrao_led[LED_COUNT][3] = {
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
};

// Ordem da matriz de LEDS, útil para poder visualizar na matriz do código e escrever na ordem correta do hardware
int ordem[LED_COUNT] = {0, 1, 2, 3, 4, 9, 8, 7, 6, 5, 10, 11, 12, 13, 14, 19, 18, 17, 16, 15, 20, 21, 22, 23, 24};  


// Rotina para definição da intensidade de cores do led
uint32_t matrix_rgb(unsigned r, unsigned g, unsigned b){
    return (g << 24) | (r << 16) | (b << 8);
}

// Rotina para desenhar o padrão de LED
void display_desenho(){
    uint32_t valor_led;

    for (int i = 0; i < LED_COUNT; i++){
        valor_led = matrix_rgb(padrao_led[ordem[24-i]][0], padrao_led[ordem[24-i]][1], padrao_led[ordem[24-i]][2]);
        
        // Atualiza o LED
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}


// Dois dígitos significativos * 10 do E24:
static const int e24_2dig[E24_SIZE] = {
    10,11,12,13,15,16,18,20,22,24,27,30,
    33,36,39,43,47,51,56,62,68,75,82,91
};

static const char *dig_colors[10] = {
    "preto","marrom","vermelho","laranja",
    "amarelo","verde","azul","violeta",
    "cinza","branco"
};

static const char *mult_colors[] = {
    "prata","ouro","preto","marrom","vermelho","laranja",
    "amarelo","verde","azul","violeta","cinza","branco"
};

static const int dig_colors_rgb[10][3] = {
    {0, 0, 0}, // preto (apagado)
    {35, 10, 4}, // marrom (não sai como esperado)
    {51, 0, 0}, // vermelho 
    {51, 10, 0}, // laranja
    {51, 51, 0}, // amarelo
    {0, 51, 0}, // verde
    {0, 0, 51}, // azul
    {25, 6, 55}, // violeta
    {15, 10, 5}, // cinza
    {51, 51, 51} // branco
};

static const int mult_colors_rgb[12][3] = {
    {38, 28, 18}, // prata
    {51, 23, 0}, // ouro
    {0, 0, 0}, // preto (apagado)
    {35, 10, 4}, // marrom (não sai como esperado)
    {51, 0, 0}, // vermelho 
    {51, 10, 0}, // laranja
    {51, 51, 0}, // amarelo
    {0, 51, 0}, // verde
    {0, 0, 51}, // azul
    {25, 6, 55}, // violeta
    {15, 10, 5}, // cinza
    {51, 51, 51} // branco
};

void calcular_faixa(double resistencia, char cor1[], char cor2[], char cor3[])
{
    int pot = 0;
    double normalizado = resistencia;
    // Faz divisão por 10 até que o número esteja entre 1 e 100
    while (normalizado >= 100.0) {
        normalizado /= 10.0;
        pot++;
    }

    // Caso esteja entre 0 e 10, precisamos que a potência seja negativa
    while (normalizado < 10.0 && normalizado > 0.0) {
        normalizado *= 10.0;
        pot--;
    }

    // Se a resistência estiver acima de 95.5 ele está mais próximo de 100
    // e, portanto, deve ser arredondado para 10.0
    if (normalizado > 95.5) {
        normalizado = 10.0;
        pot++;
    }

    int prox = e24_2dig[0];
    // Verifica a base mais próxima
    for (int i = 1; i < E24_SIZE; i++){
        if(fabs(normalizado - e24_2dig[i]) < fabs(normalizado - prox)){
            prox = e24_2dig[i];
        }
    }


    // Verifica os digitos da faixa
    int digito1 = prox / 10;
    int digito2 = prox % 10;

    // Coloca as cores da primeira faixa na matriz
    for (int i = 0; i <= 20; i += 5){
        padrao_led[i][0] = dig_colors_rgb[digito1][0];
        padrao_led[i][1] = dig_colors_rgb[digito1][1];
        padrao_led[i][2] = dig_colors_rgb[digito1][2];
    }

    // Coloca as cores da segunda faixa na matriz
    for (int i = 1; i <= 21; i += 5){
        padrao_led[i][0] = dig_colors_rgb[digito2][0];
        padrao_led[i][1] = dig_colors_rgb[digito2][1];
        padrao_led[i][2] = dig_colors_rgb[digito2][2];
    }

    // Coloca as cores da terceira faixa na matriz
    for (int i = 2; i <= 22; i += 5){
        padrao_led[i][0] = mult_colors_rgb[pot + 2][0];
        padrao_led[i][1] = mult_colors_rgb[pot + 2][1];
        padrao_led[i][2] = mult_colors_rgb[pot + 2][2];
    }

    display_desenho(); // Atualiza o padrão de LED

    // Passa os dígitos para a string
    strcpy(cor1, dig_colors[digito1]);
    strcpy(cor2, dig_colors[digito2]);
    strcpy(cor3, mult_colors[pot+2]);
}


int main(){
    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA);                                        // Pull up the data line
    gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
    ssd1306_t ssd;                                                // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd);                                         // Configura o display
    ssd1306_send_data(&ssd);                                      // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);


    // Configuração do PIO
    pio = pio0; 
    uint offset = pio_add_program(pio, &animacao_matriz_program);
    sm = pio_claim_unused_sm(pio, true);
    animacao_matriz_program_init(pio, sm, offset, MATRIZ_PIN);


    adc_init();
    adc_gpio_init(ADC_PIN); // GPIO 28 como entrada analógica

    float tensao;
    char str_x[5]; // Buffer para armazenar a string
    char str_y[5]; // Buffer para armazenar a string

    char faixa1[20] = "", faixa2[20] = "", faixa3[20] = "";
    char cor1[20] = "", cor2[20] = "", cor3[20] = "";

    bool cor = true;
    while (true)
    {
        adc_select_input(2); // Seleciona o ADC para eixo X. O pino 28 como entrada analógica

        float soma = 0.0f;
        for (int i = 0; i < 500; i++)
        {
            soma += adc_read();
            sleep_ms(1);
        }
        float media = soma / 500.0f;

        // Fórmula simplificada: R_x = R_conhecido * ADC_encontrado /(ADC_RESOLUTION - adc_encontrado)
        R_x = (R_conhecido * media) / (ADC_RESOLUTION - media);

        sprintf(str_x, "%1.0f", media); // Converte o inteiro em string
        sprintf(str_y, "%1.0f", R_x);   // Converte o float em string


        calcular_faixa(R_x, cor1, cor2, cor3);

        sprintf(faixa1, "Fx1: %s", cor1);
        sprintf(faixa2, "Fx2: %s", cor2);
        sprintf(faixa3, "Fx3: %s", cor3);

        // cor = !cor;
        //  Atualiza o conteúdo do display com animações
        ssd1306_fill(&ssd, !cor);                     // Limpa o display
        ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor); // Desenha um retângulo
        ssd1306_draw_string(&ssd, faixa1, 8, 6); // Desenha uma string
        ssd1306_line(&ssd, 3, 15, 123, 15, cor);      // Desenha uma linha
        ssd1306_draw_string(&ssd, faixa2, 8, 18);  // Desenha uma string
        ssd1306_line(&ssd, 3, 27, 123, 27, cor);      // Desenha uma linha
        ssd1306_draw_string(&ssd, faixa3, 8, 30);  // Desenha uma string
        ssd1306_line(&ssd, 3, 39, 123, 39, cor);           // Desenha uma linha
        ssd1306_draw_string(&ssd, "ADC", 13, 42);          // Desenha uma string
        ssd1306_draw_string(&ssd, "Resisten.", 50, 42);    // Desenha uma string
        ssd1306_line(&ssd, 44, 39, 44, 61, cor);           // Desenha uma linha vertical
        ssd1306_draw_string(&ssd, str_x, 8, 52);           // Desenha uma string
        ssd1306_draw_string(&ssd, str_y, 59, 52);          // Desenha uma string
        ssd1306_send_data(&ssd);                           // Atualiza o display
        sleep_ms(700);
    }
}