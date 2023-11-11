/*#######################################################
#							                                          #
#	                  Mestre - ESP-NOW                    #
#                 Mede Tempo - Método 4		              #
#							                                          #
#	     	                                                #
#	       Wesna Simone Bulla de Araujo - RA:225843	      #
#							                                          #
#########################################################
*/

/*
Esse programa representa o transmissor (mestre).

Utilizando o método 4, o mestre só envia o próximo pacote se
receber uma confirmação do escravo e se o valor do pacote enviado
foi recebido corretamente. Caso o escravo tenha recebido um pacote
não esperado ele sinaliza para o mestre que tenta enviar novamente 
o pacote.

Os valores do pacote são incrementados somente após o programa
entrar na função callback de envio, sinalizando que algum dado
foi enviado.

Valores de Taxas usados:
- WIFI_PHY_RATE_1M_L (default)
- WIFI_PHY_RATE_6M
- WIFI_PHY_RATE_12M
- WIFI_PHY_RATE_54M
*/


//Declaração de bibliotecas
#include <stdint.h>
#include <esp_now.h>                  //inclui a biblioteca esp_now para o uso do protocolo de comunicação ESP-NOW
#include <WiFi.h>                     //inclui a biblioteca WiFi para configuração da rede sem fio
#include <esp_wifi_internal.h>        //inclui a biblioteca para a configuração das taxas de transmissão do ESP-NOW

#define WIFI_CHANNEL  10              //define canal de comunicação

int c = 0;                            //define variavel que indica o número do pacote
int flag_confirmacao = 1;             //define variavel que recebe a confirmação do escravo
int pacotes = 2000;                   //indica quantidade de pacotes a serem enviados. Note que foi colocado um número maior de pacotes, 
                                      //para considerar possíveis perdas

uint8_t macAddress2Esp32[] = {0xA4, 0xE5, 0x7C, 0x6D, 0xFB, 0xD8};       //define o endereço MAC do outro ESP32 (sem broadcast)
//uint8_t macAddress2Esp32[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};     //define endereço Broadcast


//define uma estrutura para troca de informações
//Foi definido um vetor de 125 posições de 16 bits cada totalizando 250 bytes
struct DataStruct { 
  unsigned short a[125];
};

DataStruct dataSend;  //declara a estrutura dataSend
DataStruct dataRecv;  //declara a estrutura dataRecv


esp_now_peer_info_t peerInfo;  //cria uma estrutura esp_now_peer_info_t, que é utilizada para registrar informações
                               //sobre um peer (dispositivo) que será adicionado à rede ESPNOW



// Função Callback de Envio: chamada sempre que um dado é enviado
// mac_addr: é o endereço do dispositivo escravo
// status: é uma variável do próprio protocolo que indica o status do envio, isto é, sucesso ou falha.
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {  // Função chamada quando os dados são enviados
  
  // Se o envio não foi bem sucedido printa mensagem de aviso
  if (status != ESP_NOW_SEND_SUCCESS) {                                  // Se o envio foi bem sucedido, ...
    Serial.println("Erro para enviar para o escravo");
  }
  c = c + 1;            //incrementa número do pacote
}


//Função Callback Recebe: chamada sempre que um dado é recebido
//mac_addr: é o endereço do dispositivo escravo
//incomingData: é o dado que será recebido
//len: é o tamanho do dado
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
    memcpy(&dataRecv, incomingData, sizeof(dataRecv));    //copia os dados recebidos para a estrutura de dados

    //verifica se a sinalização enviada pelo escravo é positiva
    if(dataRecv.a[0] == 1){
        flag_confirmacao = 1;                             //se sim, sinaliza para o mestre
    }
}


//Configuração do protocolo ESP-NOW e da mudança de taxa
void setup() {
  Serial.begin(115200);  //configura taxa para transmissão serial. Taxa default: 115200bps

  //configuração wifi para alterar a taxa de transmissão
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  esp_wifi_stop();
  esp_wifi_deinit();

  //configuração para alterar a taxa de transmissão
  wifi_init_config_t my_config = WIFI_INIT_CONFIG_DEFAULT();                      
  my_config.ampdu_tx_enable = 0;                                                  
  esp_wifi_init(&my_config);                                                      
  esp_wifi_start();

  esp_wifi_set_channel(WIFI_CHANNEL,WIFI_SECOND_CHAN_NONE);                         //configura o canal de transmissão
  esp_wifi_internal_set_fix_rate(ESP_IF_WIFI_STA, true,   WIFI_PHY_RATE_54M);       //muda a taxa de transmissão (alterar aqui os valores das taxas)

  WiFi.disconnect();                                                                //desabilita wifi

  if (esp_now_init() != ESP_OK) {                                                   //inicializa o ESP-NOW e verifica se há erros
      Serial.println("Error initializing ESP-NOW");                                 //se houver erros
      delay(2500);                                                                  //espera por 2,5 segundos
      ESP.restart();                                                                //reinicia o dispositivo
  }

  esp_now_register_send_cb(OnDataSent);             //registra a função de callback que é chamada quando os dados são enviados
  esp_now_register_recv_cb(OnDataRecv);             //registra a função de callback que é chamada quando os dados são recebidos
  
  memcpy(peerInfo.peer_addr, macAddress2Esp32, 6);  //copia o endereço MAC do escravo para a estrutura peerInfo
  peerInfo.channel = WIFI_CHANNEL;                  //define o canal de comunicação como 10 na estrutura peerInfo
  peerInfo.encrypt = false;                         //define a encriptação como desativada na estrutura peerInfo

  // Tenta adicionar o outro ESP32 à lista de pares de comunicação ESP-NOW e verifica se foi bem sucedido
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {      //caso não seja possível adicionar o outro ESP32, 
    Serial.println("Failed to add peer");           //exibe mensagem de falha no display
    delay(2500);
    ESP.restart();                                  //e reinicia o dispositivo
  }  

    //Monta um pacote de 250 bytes
    dataSend.a[0] = 0;        
    for(c=1; c<125; c++){
      dataSend.a[c] = c;
    }

    c = 0;                                          //atribui o valor zero para o primeiro pacote
}

//Main
void loop() {   

    c = 0;                                        //inicializa a variavel responsável pelo número do pacote
    flag_confirmacao = 1;                         //habilita flag_confirmacao para permtir um primeiro envio
    while(c<= pacotes){                           //envia 1000 pacotes de 250 bytes
        if((flag_confirmacao == 1)){              //se o dado chegou até o escravo
          if(dataRecv.a[1] != 2){                 //confere se o dado chegou corretamente
            c = dataRecv.a[1];                    //se não, atribui a variavel c o valor do pacote enviado pelo escravo
          }
          dataSend.a[0] = c;                      //a estrutura de envio na posição 0 recebe o valor do número do pacote
          esp_now_send(macAddress2Esp32, (uint8_t *)&dataSend, sizeof(dataSend));   //o dado é enviado
          flag_confirmacao = 0;                   //zera a flag de confirmacao, que sera novamente setada pela confirmação do escravo
        }
        else{
          flag_confirmacao = 1;                   //se o escravo não sinalizar, tenta-se um novo envio
        }
    }
}
