/**
 *
 * Código Projeto TFG - Botijão de Gás Inteligente - HTTP Server
 * Autor: Yerro Cândido Ferreira
 * Instituição: UNIFEI
 *
**/

#include <ESP8266WiFi.h> // biblioteca responsável pelo funcionamento do módulo ESP8266
#include <ESPAsyncWebServer.h> // biblioteca responsável pela criação do servidor HTTP
#include <HX711.h> // biblioteca responsável pelo modulo HX711

#define LOADCELL_DOUT_PIN D7 //pinos que receberam os dados do modulo HX711
#define LOADCELL_SCK_PIN D6 //pinos que receberam os dados do modulo HX711

// Definindo as credenciais da rede Wifi
const char* ssid = ""; //Nome da rede Wifi
const char* password = ""; //Senha da rede Wifi
float dado = 0; // variavel que receberá o dado atual lido na balança
float pesototal = 27000; // peso total do botijao cheio (27 kg ou 27000g)
float per = 0; // variavel que receberá a porcentagem atual do dado comparado com o peso total
String reserva; // indicará se o botijão esta ou não na reserva
HX711 scale; //Definição da variável do tipo referente ao módulo HX711, que amplifica e normaliza os dados obtidos pelas células de carga

// Criando um objeto da biblioteca assíncrona utilizando a porta 80
AsyncWebServer server(80);

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

// Função para lidar com rotas não existentes
void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Rota Incorreta");
}

void setup() {

  Serial.begin(115200);

  Serial.println();

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

  // Determina as credenciais da rede WiFi gerada pelo ESP8266
  conectarWiFi();

  //criarAPWiFi();

  // Criação da rota que exibe o valor atual lido pela balanca
  server.on("/dados", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(dado).c_str());
  });
  // Criação da rota que exibe a porcentagem calculada atualmente
   server.on("/porcento", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(per).c_str());
  });
  // Criação da rota que exibe 1 caso o botijão esteja na reserva e 0 caso não
   server.on("/reserva", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(reserva).c_str());
  });
  
  // define a função para lidar com rotas não existentes
  server.onNotFound(notFound);

  // Começa o servidor
  server.begin();
}
 
void loop(){ // Deixa a leitura da balança em loop a cada 10 segundos

  dado = scale.get_units(10); // realiza a obtenção de uma media de 10 valores da balança
  per = ((dado/pesototal)*100); // realiza o calculo da porcentagem

  if(per < 60){ // analisa se a porcentagem está abaixo de 60% ou não, informando se esta na reserva ou não
  reserva = "1";
  }else{
  reserva = "0";
  }
  scale.power_down(); //coloca a balanca para dormir (economia de energia)
  delay(10000); //tempo entre um envio/leitura e outro
  scale.power_up(); //acorda a balanca
  
}