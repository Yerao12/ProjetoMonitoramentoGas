/**
 *
 * Código Projeto TFG - Botijão de Gás Inteligente - MQTT via broker local no ESP8266
 * Autor: Yerro Cândido Ferreira
 * Instituição: UNIFEI
 *
**/
#include <ESP8266WiFi.h> // biblioteca responsável pelo funcionamento do módulo ESP8266
#include <MQTT.h> // biblioteca responsável pelas conexões MQTT
#include <uMQTTBroker.h> // biblioteca responsável pela criação do broker local no módulo ESP8266
#include <HX711.h> // biblioteca responsável pelo modulo HX711

#define LOADCELL_DOUT_PIN D7 //pinos que receberam os dados do modulo HX711
#define LOADCELL_SCK_PIN D6 //pinos que receberam os dados do modulo HX711

float dado = 0; // variavel que receberá o dado atual lido na balança
float pesototal = 27000;  // peso total do botijao cheio (27 kg ou 27000g)
float per = 0; // variavel que receberá a porcentagem atual do dado comparado com o peso total
HX711 scale; //Definição da variável do tipo referente ao módulo HX711, que amplifica e normaliza os dados obtidos pelas células de carga

// Definindo as credenciais da rede Wifi 
const char* ssid = ""; //Nome da rede Wifi
const char* password = ""; //Senha da rede Wifi

//Definição da variável do tipo referente ao BrokerMQTT local, realizado pelo ESP8266
uMQTTBroker BrokerESP;

// Esta função realiza a conexão à rede wifi cujos dados foram informados nas variaveis
void conectarWiFi(){
  Serial.println("Connecting to "+(String)ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  
  Serial.println("WiFi conectado");
  Serial.println("Endereco IP: " + WiFi.localIP().toString());
}

// Alternativamente, há a possilibilidade de criação de uma rede local pelo modulo ESP8266
//void criarAPWiFi(){
//  WiFi.softAP(ssid, password);
//  Serial.println("AP criado");
//  Serial.println("Endereco IP: " + WiFi.softAPIP().toString());
//}

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
  
  // Conectar à rede WiFi
  conectarWiFi();

  // Ou inicializar a rede WiFi criada pelo ESP8266
  //criarWiFiAP();

  // Dá início ao broker MQTT
  Serial.println("Inicializando broker MQTT.");
  BrokerESP.init();
  
}

void loop() {
  dado = scale.get_units(10);  // realiza a obtenção de uma media de 10 valores atuais da balança
  per = ((dado/pesototal)*100); // realiza o calculo da porcentagem

  BrokerESP.publish("dados", String(dado).c_str()); // realiza o envio do valor lido pela balança para o tópico "dados"
  BrokerESP.publish("porcento", String(per).c_str()); // realiza o envio da porcentagem calculada para o tópico "porcento"
  Serial.println("LEITURA ENVIADA");

  if(per < 60){ // analisa se a porcentagem está abaixo de 60% ou não, informando se esta na reserva ou não
    BrokerESP.publish("reserva", "CHEGOU NA RESERVA"); // caso esteja abaixo de 60%, é enviado uma mensagem para o tópico "reserva"
  }

  yield(); // aguarda os processos serem realizados
  scale.power_down(); //coloca a balanca para dormir (economia de energia)
  delay(10000); //tempo entre um envio/leitura e outro
  scale.power_up(); //acorda a balanca

}
