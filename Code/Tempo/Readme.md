## Métodos para verificar o tempo entre transmissões do protocolo ESP-NOW ⏰

- [Método 1](https://github.com/wesnasimone/EA006_TCC_ESP-NOW/tree/main/Code/Tempo/Metodo_1): Utiliza uma flag de confirmação do próprio mestre para sinalizar que um envio foi feito e que o próximo pode ser realizado;
- [Método 2](https://github.com/wesnasimone/EA006_TCC_ESP-NOW/tree/main/Code/Tempo/Metodo_2): Utiliza uma flag de confirmação do próprio mestre para sinalizar que um envio foi feito e um verificador para garantir que o próximo envio só seja feito se o pacote atual é igual ou maior que o pacote anterior mais um;
- [Método 3](https://github.com/wesnasimone/EA006_TCC_ESP-NOW/tree/main/Code/Tempo/Metodo_3): Utiliza o conceito de handshake, de modo que o escravo só envia o próximo pacote ao receber uma confirmação do escravo de que um pacote foi recebido.
- [Método 4](https://github.com/wesnasimone/EA006_TCC_ESP-NOW/tree/main/Code/Tempo/Metodo_4): Utiliza o conceito de handshake, de modo que o escravo só envia o próximo pacote ao receber uma confirmação do escravo de que um pacote foi recebido e do conteúdo recebido. Se o conteúdo não corresponder ao valor esperado pelo escravo o mestre é avisado e uma nova transmissão com o valor esperado é feita.

O tempo de envio entre pacotes é feita através da função micros() e calculando a diferença entre o tempo de envio do pacote atual com o tempo de envio do pacote anterior.
