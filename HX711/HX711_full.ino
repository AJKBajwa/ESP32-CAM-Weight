/**

   HX711 library for Arduino - example file
   https://github.com/bogde/HX711

   MIT License
   (c) 2018 Bogdan Necula

**/
#include "HX711.h"


// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 3;

#define ESP_INT 6

// Global weight variable
double new_weight = 0;
double old_weight = 0;

HX711 scale;

void setup() {
  Serial.begin(115200);
  pinMode(ESP_INT, OUTPUT);
  digitalWrite(ESP_INT, LOW);
  Serial.println("Initializing the scale");

  // Initialize library with data output pin, clock input pin and gain factor.
  // Channel selection is made by passing the appropriate gain:
  // - With a gain factor of 64 or 128, channel A is selected
  // - With a gain factor of 32, channel B is selected
  // By omitting the gain factor parameter, the library
  // default "128" (Channel A) is used here.
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  Serial.println("Before setting up the scale:");
  Serial.print("read: \t\t");
  Serial.println(scale.read());			// print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));  	// print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));		// print the average of 5 readings from the ADC minus the tare weight (not set yet)

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);	// print the average of 5 readings from the ADC minus tare weight (not set) divided
  // by the SCALE parameter (not set yet)

  scale.set_scale(-91.56);                      // this value is obtained by calibrating the scale with known weights; Enter the value obtained from calibrate.ino here.
  scale.tare();				        // reset the scale to 0

  Serial.println("After setting up the scale:");

  Serial.print("read: \t\t");
  Serial.println(scale.read());                 // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));       // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));		// print the average of 5 readings from the ADC minus the tare weight, set with tare()

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);        // print the average of 5 readings from the ADC minus tare weight, divided by the SCALE parameter set with set_scale
 
  Serial.println("Readings:");
}

void loop() {

  new_weight = scale.get_units(10);
  Serial.print("\t| average reading:\t");
  Serial.println(new_weight);
  if (abs(new_weight - old_weight) >= 10)
  {
    Serial.println("Taking picture: Change in weight detected.");
    digitalWrite(ESP_INT, HIGH);
    delay(200);
    digitalWrite(ESP_INT, LOW);
  }
  old_weight = new_weight;
  scale.power_down();			        // put the ADC in sleep mode
  delay(5000);
  scale.power_up();
}
