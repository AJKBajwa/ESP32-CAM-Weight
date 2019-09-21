#include "HX711.h"

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 3;

HX711 scale;
String weight;
void setup() {
  Serial.begin(57600);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  // Initialization
  scale.set_scale();
  scale.tare();
  Serial.println("Place a known weight on the scale and enter the weight value in grams.");
  Serial.print("Enter weight value: ");
  while (!Serial.available())
  {
    if (Serial.available())
    {
      weight = Serial.readString();
      delay(500);
      Serial.println(weight + " g.");
      break;
    }
  }
}

void loop()
{
  if (scale.is_ready())
  {
    long reading = scale.get_units(10);
    double factor = 0;
    for (int i = 0; i <= 10; i++)
    {
      Serial.print("HX711 reading: ");
      Serial.println(reading);
      Serial.print("Factor to enter into the set_scale: ");
      if (i == 0)
      {
        factor =  double(reading) / double(weight.toFloat());
      }
      else
      {
        factor =  (factor + double(reading) / double(weight.toFloat())) / 2;
      }
      Serial.println(factor);
      delay(1000);
    }
  }
  else
  {
    Serial.println("HX711 not found.");
  }

  while (1)
  {}
}
