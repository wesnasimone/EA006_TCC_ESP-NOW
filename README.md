# Um Estudo sobre o ESP-NOW 🧠📡💻

O presente trabalho de fim de curso foi dedicado ao estudo do protocolo sem fio
ESP-NOW presente nos microcontroladores ESP32 no contexto de interfaces cérebro-
computador (BCI). Esse estudo de caso visa analisar uma possível substituição da interface
bluetooth presente nas placas Cyton desenvolvidas pela OpenBCI pelo protocolo ESP-
NOW de modo a garantir taxas de amostragens maiores e compatíveis com o conversor
analógico digital ADS1299, que é responsável pela coleta dos sinais de eletroencefalografia.
Tal substituição pode ser vantajosa, pois pode impactar positivamente no desempenho de
BCIs. Através de experimentos considerando sinalizações de envio e recebimento de pacotes, endereços com e sem broadcast e analises de perdas de pacotes durante a transmissão,
foi possível observar que o protocolo atinge valores superiores a interface bluetooth tendo
poucas perdas no modo sem broadcast. No entanto, as transmissões são bastante instá-veis, embora ocorram com pouca frequência, o que pode levar a substituição da interface
bluetooth não ser tão vantajosa.

Esse repositório foi organizado em três pastas:
- [Code](https://github.com/wesnasimone/EA006_TCC_ESP-NOW/tree/main/Code): contém todos os códigos desenvolvidos durante os experimentos. Aqui é possível entender melhor algumas funções do protocolo ESP-NOW, bem como o modo para se alterar a taxa de amostragem padrão do protocolo para valores maiores (algo pouco explorado e documentado);
- [Dados](https://github.com/wesnasimone/EA006_TCC_ESP-NOW/tree/main/Dados): contém os resultados obtidos através da execução de cada código construido, tanto em relação as taxas de transmissão, quanto em relação as perdas durante os envios de pacotes.
- [Plotes](https://github.com/wesnasimone/EA006_TCC_ESP-NOW/tree/main/Plotes): contém os códigos em python utilizados para plotar os gráficos dos resultados.
- [Diagramas](https://github.com/wesnasimone/EA006_TCC_ESP-NOW/tree/main/Diagramas): contém os esquemáticos de funcionamento dos códigos para cada metodologia adotada.

Além disso, é importante mencionar que o desenvolvimento do projeto foi feito por meio da IDE do Arduino versão 2.0.3 e o pacote ESP32 instalado na IDE foi a versão 1.0.6.

---
### Principais Referências
[1] https://blog.eletrogate.com/conhecendo-o-esp32-usando-arduino-ide-2/

[2] https://randomnerdtutorials.com/esp-now-esp32-arduino-ide/

[3] https://blog.eletrogate.com/esp-now-comunicacao-sem-fio-entre-esp32s/

[4] https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_now.html#

[5] https://github.com/thomasfla/Linux-ESPNOW/tree/master/ESP32-Test

