/********************************************************
*							                                          
*	                 Escravo - ESP-NOW		                
*	             Verifica Estabilidade - Método 1         
*	     	                                                
*	       Wesna Simone Bulla de Araujo - RA:225843	      
*							                                          
*********************************************************
*/

/*
Esse programa representa o receptor (escravo).

Utilizando o método 1, o escravo pode verificar a quantidade
de perdas, se existe inversão de pacote durante a transmissão
e se o conteúdo do pacote é alterado.

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
//uint8_t macAddress1Esp32[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

unsigned short conteudo[125];           //define vetor que irá armazenar o conteudo do pacote recebido
unsigned short vetor_recebido[10000];   //define vetor que irá armazenar o número dos pacotes recebidos

int count = 0;                          //define variável que controla a quantidade de pacotes recebidos
int perdas = 0;                         //define variável que irá armazenar a quantidade de perda de pacotes
int perdas_conteudo = 0;                //define variável que irá armazenar a quantidade de perda do conteudo do pacote
int count_envio = 0;                    //define variável que controla a quantidade de vezes que os pacotes serão enviados
int ordem = 0;                          //define variável que irá armazenar a quantidade de inversao na ordem de envio do pacote
int pacotes = 10000;                    //define quantidade de pacotes
int tam_pacote = 125;                   //define tamanho do pacote
int qtd_envio = 1000;                   //define quantidade de vezes que os pacotes serão enviados
bool printa_uma_vez = 0;                //define variavel que controla os prints do tempo

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
          printa_uma_vez = 0;                       //permite que o script printe o próximo conjunto de pacotes
          vetor_recebido[0] = dataRecv.a[0];        //adiciona o primeiro pacote recebido no vetor_recebido
          count = 1;                                //reinicia a contagem da quantidade de pacotes esperados.
                                                    //Note que como um valor já fou adicionado o contador foi atualizado para 1.
    }    
  }
    return;
  }

  //parte principal do callback de recebimento --> o armazenamento do dado
  memcpy(&dataRecv, incomingData, sizeof(dataRecv));  //copia os dados recebidos para a estrutura de dados

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

  esp_wifi_set_channel(WIFI_CHANNEL,WIFI_SECOND_CHAN_NONE);                        //configura o canal de transmissão  
  esp_wifi_internal_set_fix_rate(ESP_IF_WIFI_STA, true,   WIFI_PHY_RATE_54M);      //muda a taxa de transmissão (alterar aqui os valores das taxas)

  WiFi.disconnect();                                                               //desabilita wifi
  
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

//Função que confere a ordem de chegada dos pacotes
void confere_ordem_pacote(){
    for(int c=1;c<pacotes;c++){
      if(vetor_recebido[c] < vetor_recebido[c-1]){                  //se o valor atual do vetor for menor que o valor anterior
        ordem = ordem + 1;                                          //quer dizer que houve inversão de pacotes. O contador é incrementado
      }
    }
    Serial.println(ordem);                                         //printa a quantidade das inversãos obtidos para a transmissão total
    ordem = 0;                                                     //zera contador
}

//Função que confere a quantidade de perdas de pacotes
//Essa função foi montada considerando 4 possíveis envios de pacote, por exemplo 
//de acorco com a sequência original 0 1 2 3 4 5
//(1) repetição de pacote: 0 0 2 3 4 5 (perda do pacote 1)
//(2) repetição de pacote: 0 0 1 2 3 4 (perda do pacote 5)
//(3) perda do pacote zero: 1 2 3 4 5 6
//(4) perda de um pacote qualquer: 0 1 3 4 5 6 (perda do pacote 2)
void confere_perda_pacote(){

    int dif = 0;                                                        //define variavel para armazenar a diferença entre o pacote desejado e o pacote que foi recebido
    int flag_repeticao = 0;                                             //sinaliza se o envio anterior teve pacote repetido ou não
    
    for(int c=0;c<(pacotes-1);c++){
      if(c == 0){                                                       //(3) se c for igual a zero espera-se que o pacote recebido tbm seja o zero
        if(vetor_recebido[0] != 0){                                     //se não for, quer dizer que houve perdas
          perdas = perdas + 1;                                          //incrementa contador
        }
      }
      else{                                                             //se c não for zero
        if((vetor_recebido[c-1]+1) != (vetor_recebido[c])){             //(2) verifica se o valor do pacote anterior somado com um é diferente do valor atual
          if(vetor_recebido[c-1] == vetor_recebido[c]){                 //se sim, verifica se o valor anterior do pacote é igual ao atual
                perdas = perdas + 1;                                    //se for, é porque houve repetição de envio de pacote. O contador é incrementado
                flag_repeticao = 1;                                     //sinaliza que houve repetição de pacote
          }
          else{                                                         //se não
            if(flag_repeticao == 1){                                    //verifica que houve repetição anteriormente
                flag_repeticao = 0;                                     //se sim, zera a flag
                if((vetor_recebido[c-1]+2) != vetor_recebido[c]){       //(1) verifica se o valor anterior somado com 2 é diferente do valor atual
                    dif = vetor_recebido[c] - (vetor_recebido[c-1]+2);  //se sim calcula a quantidade de perdas obtidas
                    perdas = perdas + dif;                              //e soma com as perdas anteriores
                }
            }
            else{                                                       //(4) se não houve repetição anterior, calcula a quantidade de perdas obtidas
              dif = vetor_recebido[c] - (vetor_recebido[c-1]+1);        //
              perdas = perdas + dif;                                    //
            }
          }
        }
      }
    }
    Serial.print(perdas);                                               
    Serial.print(" ");
    perdas = 0;                                                         //zera variavel perdas
}


//Função que confere o conteudo do pacote
void confere_conteudo_pacote(){

  //vetor comparacao
  for(int c=0; c<(tam_pacote-1); c++){
      conteudo[c] = c+1;
  }
  for(int c=0;c<(tam_pacote-1);c++){
      if(dataRecv.a[c+1] != conteudo[c]){                 //se o conteudo recebido for diferente do valor esperado
        perdas_conteudo = perdas_conteudo + 1;            //incrementa contador de perdas
      }
  }
  Serial.print(perdas_conteudo);
  Serial.print(" ");
  perdas_conteudo = 0;
}

//Main
//Recomenda-se chamar uma função por vez para faciliatar a visualização pela serial.
void loop() {

  if(count_envio < qtd_envio){              //se quantidade de envios for menor que a quantidade desejada
    if (printa_uma_vez == 0){               //verifica se é possível fazer um novo printe
      //confere_conteudo_pacote();          // 
      if(count == pacotes){                 //se sim, verifica se já atingiu a quantidade de pacotes para poder printar
          printa_uma_vez = 1;               //tendo atingido, bloqueia o printe de novos valores por um tempo
          confere_perda_pacote();           //
          //confere_ordem_pacote();         //
          count_envio = count_envio + 1;    //incrementa a quantidade de envio esperado
      }
    }
  }
} 
