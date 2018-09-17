#include "HX711.h" 
//avaliando o teste em que o celular moto z play pesa em média 165g
/*
 * teste 1 :
 * com o valor de 231.15 ele me devolve -98 em média, para o motoz play obs: parafusos continuam não muito presos
 */
 
  #define HX711_DATA 14
  #define HX711_CLK  12
  HX711 scale(HX711_DATA, HX711_CLK, 128);
  ADC_MODE(ADC_VCC);
  float calibration_factor;
  float weight;
  
  void setup(){
  Serial.begin(115200);
  scale.set_scale(-124.42);
  scale.tare();
  scale.read();
}
void loop() {
//  Serial.println("5s para posicionar o celular");
//  for(int i = 5; i >= 0; i--){
//    Serial.println(i);
//    delay(1000);
//    }
//  Serial.println("lendo....");
//  delay(1000);
//para descobrir o peso preciso pegar esse valor que ira printar e dividir pelo peso conhecido (precisa mais precisao)
//  Serial.println("\n\n");
  
  Serial.println(scale.get_units());
//  Serial.println("\n\n");
//-------------------------------------------

     //testando se está lendo corretamente
    // Serial.println(scale.get_units(10));
}
