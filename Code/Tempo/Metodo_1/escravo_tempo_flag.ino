/*#######################################################
#							                                          #
#	                  Escravo - ESP-NOW		                #
#					         Mede Tempo - Método 1                #
#	     	                                                #
#	       Wesna Simone Bulla de Araujo - RA:225843	      #
#							                                          #
#########################################################
*/

/*
Esse programa representa o receptor (escravo).

Utilizando o método 1, o escravo mede o tempo através
da função micros() toda vez que recebe um pacote (entra no callback 
de recebimento).

Valores de Taxas usados:
- WIFI_PHY_RATE_1M_L (default)
- WIFI_PHY_RATE_6M
- WIFI_PHY_RATE_12M
- WIFI_PHY_RATE_54M
*/

//Declaração de bibliotecas
#include <esp_now.h>                    //inclui a biblioteca esp_now para o uso do protocolo de comunicação ESP-NOW
#include <WiFi.h>                       //inclui a biblioteca WiFi para configuração da rede sem fio
#include <esp_wifi_internal.h>          //inclui a biblioteca para a configuração das taxas de transmissão do ESP-NOW

#define WIFI_CHANNEL  10                //define canal de comunicação

uint8_t macAddress1Esp32[] = {0x40, 0xF5, 0x20, 0x86, 0x5C, 0xFC };       //define o endereço MAC do outro ESP32 (sem broadcast)
//uint8_t macAddress1Esp32[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};      //define endereço Broadcast

unsigned short vetor_recebido[1000];    //define vetor que irá armazenar o número dos pacotes recebidos
unsigned long tempo[1000];              //define vetor que irá armazenar os valores do tempo obtidos pela função micros()
unsigned long tempo_dif[1000];          //define vetor que irá armazenar a diferença dos tempos entre envios de pacotes

int count = 0;                          //define variável que controla a quantidade de pacotes recebidos
int count_envio = 0;                    //define variável que controla a quantidade de vezes que os pacotes serão enviados
int pacotes = 1000;                     //define quantidade de pacotes
int qtd_envio = 10;                     //define quantidade de vezes que os pacotes serão enviados
int flag_recontagem = 0;                //define flag que sinaliza quando todos os pacote de um dado envio foram enviados
bool printa_uma_vez = 0;                //define variavel que controla os prints do tempo
unsigned long temporizador_inicial = 0; //define variável para armazenar o tempo da última recepção de dados

//define uma estrutura para troca de informações
//Foi definido um vetor de 125 posições de 16 bits cada totalizando 250 bytes
struct DataStruct {  
  unsigned short a[125];         
};

DataStruct dataSend;  //declara a estrutura dataSend
DataStruct dataRecv;  //declara a estrutura dataRecv

//cria uma estrutura esp_now_peer_info_t, que é utilizada para registrar informações sobre um peer (dispositivo) que será adicionado à rede ESPNOW
esp_now_peer_info_t peerInfo;  


// Função Callback de Envio: chamada sempre que um dado é enviado
// mac_addr: é o endereço do dispositivo escravo
// status: é uma variável do próprio protocolo que indica o status do envio, isto é, sucesso ou falha.
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {  

  // Se o envio não foi bem sucedido printa mensagem de aviso
  if (status != ESP_NOW_SEND_SUCCESS) {                                   
      Serial.println("Erro para enviar para o mestre");
  }
}


//Função Callback Recebe: chamada sempre que um dado é recebido
//mac_addr: é o endereço do dispositivo escravo
//incomingData: é o dado que será recebido
//len: é o tamanho do dado
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {

  //verifica se algum dado já foi printado --> controle para printar na serial
  if((printa_uma_vez == 1)){

    //se sim, verifica se a quantidade de vezes enviada já atingiu o valor desejado
    if((count_envio < qtd_envio)){

        //se não, copia os dados recebidos para a estrutura de dados (lembrando que o programa envia mais do que o desejado
        //para dar uma folga caso haja perdas)
        memcpy(&dataRecv, incomingData, sizeof(dataRecv));

        //verifica se o novo valor recebido é menor que o último valor armazenado no vetor_recebido (que contem o número exato de pacotes desejados).
        //aqui considera-se que não existe inversão na ordem dos pacotes (o que foi analisado via testes de estabilidade).
        //se for, quer dizer que a contagem recomeçou
        if(dataRecv.a[0] < vetor_recebido[count-1]){
          printa_uma_vez = 0;                     //permite que o script printe o próximo conjunto de pacotes
          flag_recontagem = 1;                    //permite que o valor recebido seja armazenado no vetor_recebido
          count = 0;                              //reinicia a contagem da quantidade de pacotes esperados
        }    
    }
    //se a flag_contagem for nula quer dizer que ainda existe pacotes para serem recebidos (pacotes não mais desejados)
    //então o novo dado não é permitido ser armazenado no vetor_recebido 
    if(flag_recontagem == 0){
          return;
    }
    //mantem flag_recontagem nula
    flag_recontagem = 0;
  }

  //parte principal do callback de recebimento --> o armazenamento do dado
  memcpy(&dataRecv, incomingData, sizeof(dataRecv));  //copia os dados recebidos para a estrutura de dados

  //mede tempo entre recebimento dos pacotes (1000 pacotes)
  temporizador_inicial = micros();

  //se count for zero ou for o limite de pacotes
  //ambos os vetores de tempo recebem o mesmo valor na posição zero (não há comparação para a primeira medição)
  //count é reiniciado
  if ((count == 0) || (count == pacotes)){
    tempo[0] = temporizador_inicial;
    tempo_dif[0] = tempo[0];
    count = 0;
  }
  //caso contrário somente o vetor de tempo recebe os valores obtidos pelo micros()
  else{
    tempo[count] = temporizador_inicial; 
  }

  //o valor recebido (número do pacote apenas) é armazenado no vetor_recebido
  vetor_recebido[count] = dataRecv.a[0];

  //incrementa variavel que está sincronizada com o número do pacote recebido
  count = count + 1;
 }


//Configuração do protocolo ESP-NOW e da mudança de taxa
void setup() {
  Serial.begin(115200);           //configura taxa para transmissão serial. Taxa default: 115200bps

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
  
  if (esp_now_init() != ESP_OK) {                                                  //inicializa o ESP-NOW e verifica se há erros
    Serial.println("Error initializing ESP-NOW");                                  //se houver erros
    delay(2500);                                                                   //espera por 2,5 segundos
    ESP.restart();                                                                 //reinicia o dispositivo
  }
  esp_now_register_send_cb(OnDataSent);  //registra a função de callback que é chamada quando os dados são enviados
  esp_now_register_recv_cb(OnDataRecv);  //registra a função de callback que é chamada quando os dados são recebidos
  
  memcpy(peerInfo.peer_addr, macAddress1Esp32, 6);                                //copia o endereço MAC do outro ESP32 para a estrutura peerInfo
  peerInfo.channel = WIFI_CHANNEL;                                                //define o canal de comunicação como 0 na estrutura peerInfo
  peerInfo.encrypt = false;                                                       //define a encriptação como desativada na estrutura peerInfo

  // Tenta adicionar o outro ESP32 à lista de pares de comunicação ESP-NOW e verifica se foi bem sucedido
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {                                    //caso não seja possível adicionar o outro ESP32
    Serial.println("Failed to add peer");                                         //exibe mensagem de falha no display
    delay(2500);
    ESP.restart();                                                                //e reinicia o dispositivo
  }
}


//Função que calcula a diferença entre os tempos de cada recebimento de pacote
void tempo_transmissao(){
  for (int c=1;c<pacotes;c++){    
    tempo_dif[c] = tempo[c] - tempo[c-1];
  }
}


//Main
void loop() {

  if(count_envio < qtd_envio){                  //se quantidade de envios for menor que a quantidade desejada
    if (printa_uma_vez == 0){                   //verifica se é possível fazer um novo printe
      if(count == pacotes){                     //se sim, verifica se já atingiu a quantidade de pacotes para poder printar
          printa_uma_vez = 1;                   //tendo atingido, bloqueia o printe de novos valores por um tempo
          Serial.print("Tempo Inicial: ");      //printa os valores de tempo obtidos durante cada recebimento
          for(int c = 0; c<pacotes; c++){       //
              Serial.print(tempo[c]);           //
              Serial.print(" ");                //
          }
          Serial.println("");                   //
          tempo_transmissao();                  //
          Serial.print("Tempo entre pacotes: ");//printa a diferença de tempo entre os recebimentos de pacote      
          for(int c = 0; c<pacotes; c++){       //
              Serial.print(tempo_dif[c]);       //
              Serial.print(" ");                //
          }
          Serial.println("");                   //
          count_envio = count_envio + 1;        //incrementa a quantidade de envio esperado
      }
    }
  }
} 

