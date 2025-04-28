# Ohmímetro na BitDogLab

## Objetivo Geral

O objetivo do projeto é construir um ohmímetro utilizando a BitDogLab e um circuito divisor de tensão. Utilizando um resistor conhecido e um resistor desconhecido, o sistema calcula a resistência do desconhecido e exibe as cores correspondentes das três primeiras faixas do resistor tanto na matriz de LEDs quanto no display OLED, além de mostrar o valor numérico da resistência encontrada.

## Descrição Funcional

O sistema realiza leituras do ADC (conectado à GPIO28) a cada milissegundo, acumulando esses valores em uma variável de soma. Após 500 leituras, calcula-se a média das amostras.  
Essa média é usada para encontrar a resistência do resistor desconhecido com a fórmula:

```
R_x = (R_conhecido * media) / (ADC_RESOLUTION - media)
```

onde:  
- `R_conhecido = 10.000 Ω`
- `ADC_RESOLUTION = 4095`

O valor da resistência encontrada é passado para a função `calcular_faixa`, que determina as três faixas de cores do resistor.

O funcionamento da função `calcular_faixa`:
- Normaliza o valor da resistência, realizando divisões sucessivas por 10 enquanto for maior que 100, contando quantas divisões foram feitas (determinando o multiplicador da terceira faixa).
- Em casos menores que 10, realiza multiplicações sucessivas por 10 para normalizar (não foi necessário neste projeto, mas foi implementado).
- Se o valor normalizado for maior que 9.55, é ajustado para 10 e o multiplicador é incrementado.
- Com o valor normalizado, percorre o vetor de valores da base E24 para encontrar o valor mais próximo.
- Extrai os dois dígitos do valor encontrado:
  - Primeiro dígito → cor da primeira faixa
  - Segundo dígito → cor da segunda faixa
- As cores são então exibidas na matriz de LEDs e formatadas em texto para serem exibidas no display OLED.

Ao final, o sistema mostra no display:
- As cores das faixas
- O valor médio lido no ADC
- A resistência calculada

## Pontos Relevantes dos Periféricos e do Código

### Display OLED
Utilizado para mostrar as cores correspondentes de cada faixa do resistor desconhecido, o valor médio lido pelo ADC e o valor da resistência.

### Matriz de LEDs
Utilizada para representar visualmente as cores das três primeiras faixas do resistor desconhecido.

### ADC
Responsável por capturar o valor analógico da tensão no divisor e convertê-lo para um valor digital utilizável no cálculo da resistência.

### Função `calcular_faixa`
Função responsável por determinar, a partir do valor da resistência, quais as três primeiras faixas do resistor e preparar as informações para o display e matriz de LEDs.

### Vetor `e24_2dig`
Armazena os 24 valores "base" da série E24 com dois dígitos significativos, usados para encontrar o valor comercial mais próximo do valor medido.

### Vetor `dig_colors`
Armazena as strings correspondentes às cores dos dígitos (primeira e segunda faixa) de 0 a 9.

### Vetor `mult_colors`
Armazena as strings correspondentes às cores dos multiplicadores (terceira faixa).

### Matriz `dig_colors_rgb`
Guarda os valores RGB associados a cada cor usada para a primeira e segunda faixa.

### Matriz `mult_colors_rgb`
Guarda os valores RGB associados às cores utilizadas na terceira faixa (multiplicador).

### Matriz `padrao_led`
Define o padrão gráfico que será mostrado na matriz de LEDs para representar cada cor de faixa.
