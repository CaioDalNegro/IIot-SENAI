#include "ESP8266WiFi.h"
#include <PubSubClient.h>

// Prototipos de metodos que serao definidos mais adiante
bool connectMQTT();

// estabelece conexao com o broker e retorna situacao
// Recebe todas as publicacoes do topico informado no metodo e retorna
void callback(char* topic, byte* payload, unsigned int length);

// Parametros de conexao WiFi
const char* ssid = "linksys"; // REDE
const char* password = ""; // SENHA
const char* mqtt_broker = "test.mosquitto.org"; // host do broker
const char* topic = "MeuTopico/Teste_topico"; // topico para subscricao e publicacao
const char* mqtt_username = "";
const char* mqtt_password = "";


const int mqtt_port = 1883; // Porta
bool mqttStatus = 0; // Variavel de status de conexao MQTT

// objetos
WiFiClient espClient; // objeto responsavel pela conexao WiFi
PubSubClient client(espClient); // Objeto responsavel pela conexao com broker mosquitto

// Definindo o Setup
void setup(void) {
    Serial.begin(9600); // inicia conexao com monitor serial
    Serial.println("Iniciando Conexao...\n"); // Aviso inicial de conexao
    WiFi.begin(ssid, password); // Inicia conexao com rede WiFi

    // Inicia loop de tentativas de conexao ao servico WiFi
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println(".");
    }
    Serial.print("WiFi conectado!\n\t");
    Serial.print(WiFi.localIP()); // Envia IP atraves da UART
    mqttStatus = connectMQTT(); // Chama conexao MQTT com broker e retorna status
}

// Definicao do looping do ESP
void loop() {
    static long long polling = 0; // define intervalo de pooling
    if (mqttStatus) { // Verifica se houve conexao

        client.loop();

        if (millis() > polling+5000) { // a cada periodo de 5 segundos publica info
        polling = millis();
        client.publish(topic, "{teste123,113007042022}");
    }
  }
}

bool connectMQTT() {
  byte tentativa = 0;
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);

  // Implementacao do metodo de conexao com o broker
  do {
    // Define o ID do cliente (a propria placa ESP)
    String client_id = "ESP-"; // Que usa o prefixo ESP-
    client_id += String(WiFi.macAddress()); // Conectando com seu respectivo MAC address

    // Tenta estabelecer a conexao com o broker
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
        // Conexao estabelecida com sucesso, informa os dados do cliente (a placa)
        Serial.println("Exito na conexao!");
        Serial.printf("Cliente %s conectado ao broker\n: ", client_id.c_str()); 
    } else {
      //informa falha na conexão e aguarda 2 segundos para nova tentativa
        Serial.print("Falha ao conectar: ");
        Serial.println(client.state());
        Serial.println();
        Serial.println("Tentativa: ");
        Serial.println(tentativa);
        delay(2000);
    } 
    tentativa++; // Incrementa numero de tentativas
  }while (!client.connected() && tentativa < 5); // Limita numero de tentativas
  if (tentativa < 5) {
      // Publicacao realizada com sucesso
      client.publish(topic, "teste123,1130072402202"); // uma mensagens é publicadas
      client.subscribe(topic); // se inscreve no broker para receber mensagens
      return 1; //retorna 1 confirmado sucesso na conexão
  } else {
      // caso contrario avisa falha e retorna 0
      Serial.println("Nao conectado");
      return 0; // informa falha na conexao
  }
}

// Este metodo quando o client identifica nova mensagem no broker
void callback(char* topic, byte* payload, unsigned int length) {
    // char* topic identifica o topico registrado
    //Byte *payload conjunto de bytes que foram publicados
    //int lenght é o tamanho do vetor de bytes do pyload
    Serial.print("Mensagem chegou no topico: "); 
    Serial.println(topic);
    Serial.print("Mensagem: "); 
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
      Serial.println();
      Serial.println("-------------------------------");
}