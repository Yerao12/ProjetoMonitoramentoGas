/**
 *
 * Código Projeto TFG - Botijão de Gás Inteligente - Código para Calibração
 * Autor: Yerro Cândido Ferreira
 * Instituição: UNIFEI
 *
**/

#include "HX711.h"

#define LOADCELL_DOUT_PIN D7
#define LOADCELL_SCK_PIN D6

HX711 scale;

void setup() {
  Serial.begin(115200);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
}

void loop() {

  if (scale.is_ready()) {
    scale.set_scale();    
    Serial.println("Tarando... tire qualquer peso da balanca.");
    delay(5000);
    scale.tare(); // balanca sendo zerada
    Serial.println("Tara realizada...");
    Serial.print("Coloque um peso conhecido na balanca.");
    delay(5000);
    long leitura = scale.get_units(10);
    Serial.print("Resultado: ");
    Serial.println(leitura);
  } 
  else {
    Serial.println("Modulo HX711 nao foi encontrado.");
  }
  delay(1000);
}

//fator de calibracao sera o resultado de (leitura)/(pesoconhecido)