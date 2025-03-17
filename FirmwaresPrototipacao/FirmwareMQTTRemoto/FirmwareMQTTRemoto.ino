/**
 *
 * Código Projeto TFG - Botijão de Gás Inteligente - MQTT via broker na nuvem
 * Autor: Yerro Cândido Ferreira
 * Instituição: UNIFEI
 *
**/

#include <ESP8266WiFi.h> // biblioteca responsável pelo funcionamento do módulo ESP8266
#include <PubSubClient.h> // bibiloteca responsável por realizar a conexão MQTT à um broker
#include <HX711.h> // biblioteca responsável pelo modulo HX711

#define LOADCELL_DOUT_PIN D7 //pinos que receberam os dados do modulo HX711
#define LOADCELL_SCK_PIN D6 //pinos que receberam os dados do modulo HX711

float dado = 0; // variavel que receberá o dado atual lido na balança
float pesototal = 27000;  // peso total do botijao cheio (27 kg ou 27000g)
float per = 0; // variavel que receberá a porcentagem atual do dado comparado com o peso total
HX711 scale; //Definição da variável do tipo referente ao módulo HX711, que amplifica e normaliza os dados obtidos pelas células de carga

// Definindo as credenciais da rede Wifi e da conexão MQTT
const char* ssid = ""; //Nome da rede Wifi
const char* password = ""; //Senha da rede Wifi
const char* mqtt_broker = "test.mosquitto.org"; //Endereço do mensageiro MQTT
const char* mqtt_username = ""; //Credenciais do mensageiro, caso haja
const char* mqtt_password= ""; //Credenciais do mensageiro, caso haja
const int mqtt_port = 1883; // Porta utilizado pelo protocolo MQTT, 1883 é a padrão

bool mqttStatus = 0; // variavel que dita se está tudo correto com a conexão MQTT
bool connectMQTT(); // Definição da função de conexão ao mensageiro

WiFiClient balancaClient; //Definição da variável do tipo cliente da rede sem fio
PubSubClient client(balancaClient); //Definição da variável do tipo cliente do método publish-subscribe, que necessita de um cliente de rede sem fio

void conectarWifi() { // Esta função realiza a conexão à rede wifi cujos dados foram informados nas variaveis
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi conectado");
  Serial.println("Endereco IP: " + WiFi.localIP().toString());
}

void setup() {
  Serial.begin(115200);

  Serial.println("Inicializando a balanca.");

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN); // realiza a inicialização da balança com base nos pinos de conexão ao HX711 informados anteriormente

  Serial.println("Leitura inicial antes da calibracao:");
  Serial.print("Leitura unica: \t\t");
  Serial.println(scale.read());	

  Serial.print("Media de 20 leituras pre-calibracao: \t\t");
  Serial.println(scale.read_average(20)); 

  scale.set_scale(-9.5); // Calibrando a balança com o valor de calibração já obtido anteriormente
  scale.tare(); // Tarando a balanca (zerando)

  Serial.println("Apos calibracao:");

  Serial.print("Leitura unica: \t\t");
  Serial.println(scale.read());          

  Serial.print("Media de 20 leituras: \t\t");
  Serial.println(scale.read_average(20));   

  // Configurando o ESP8266 para o modo de acesso
  Serial.print("Configurando o modo de acesso.....");  

  conectarWifi(); // Chama a função de conexão à rede sem fio

  mqttStatus =  connectMQTT(); // Obtém um resultado positivo ou negativo da conexão ao mensageiro, corpo da função se encontra mais abaixo
}

// Função que realiza a conexão MQTT com o broker
bool connectMQTT() {
  byte tentativa = 0;
  client.setServer(mqtt_broker, mqtt_port); // define o servidor/broker e a porta utilizada

  do {
    String client_id = "Balanca TFG Yerro - "; // define a identificação da balança no broker
    client_id += String(WiFi.macAddress()); // define a identificação da balança no broker

    // caso a conexão falhe, realiza mais 4 tentativas
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) { 
      Serial.println("Exito na conexão:");
      Serial.printf("Cliente %s conectado ao broker\n", client_id.c_str());
    } else {
      Serial.print("Falha ao conectar: ");
      Serial.print(client.state());
      Serial.println();
      Serial.print("Tentativa: ");
      Serial.println(tentativa);
      delay(2000);
    }
    tentativa++;
  } while (!client.connected() && tentativa < 5);

  if (tentativa < 5) {    
    return 1;
  }else{
    Serial.println("Não conectado");    
    return 0;
  }
}

void loop() {
  dado = scale.get_units(10);  // realiza a obtenção de uma media de 10 valores atuais da balança
  per = ((dado/pesototal)*100); // realiza o calculo da porcentagem
  
  // caso esteja tudo certo com a conexão, realiza a publicação do dado e da porcentagem atual, nos tópicos "tfgyerro/dados" e "tfgyerro/porcento"
  if (mqttStatus){ 
        client.publish("tfgyerro/dados", String(dado).c_str());
        client.publish("tfgyerro/porcento", String(per).c_str());
        Serial.println("LEITURA ENVIADA");
  
        if(per < 60){ // analisa se a porcentagem está abaixo de 60% ou não, informando se esta na reserva ou não
          client.publish("tfgyerro/reserva", "CHEGOU NA RESERVA"); // caso esteja na reserva, envia uma mensagem no tópico "tfgyerro/reserva"
        }
  }
  client.loop(); // entra em loop, para não haver necessidade de uma nova conexão 
  yield(); // aguarda os processos serem realizados
  scale.power_down(); //coloca a balanca para dormir (economia de energia)
  delay(10000); //tempo entre um envio/leitura e outro
  scale.power_up(); //acorda a balanca
}
