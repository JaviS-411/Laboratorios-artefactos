#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire, -1);

const int LM35 = A0;
int lectura;
float temperatura;

void setup() {
  Serial.begin(9600);
  display.begin(0x3C, true);
}

void loop() {
  lectura = analogRead(LM35);
  temperatura = (lectura * 5.0 / 1023.0) * 100;

  Serial.println(temperatura);

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temp: ");

  display.setTextSize(2);
  display.setCursor(0, 20);
  display.print(temperatura);
  display.print(" C");

  display.display();
  delay(1000);
}
