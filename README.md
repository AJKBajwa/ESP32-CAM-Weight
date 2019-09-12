# ESP32-CAM-Weight
Use ESP32 Cam to upload images to firebase when the weight is above a set level
## Setting up the ESP CAM
### Pre-requisites
For setting up the ESP CAM you need the following hardware:
  1. ESP CAM
  2. FTDI or Arduino Uno
  3. Jumper Cables (Male - Female and Female - Female)

### Pin-out
If you have an FTDI, connect your ESP Cam to the FTDI as follows: [ESP CAM to FTDI Wiring](ESP32-CAM-wiring-FTDI1.png).

If you have an Arduino uno, connect your ESP Cam to the Arduino as follows : [ESP CAM to Arduino Wiring](ESP32-CAM-wiring-Arduino.png).

## Uploading the code
### Pre-requisites
To start programming the ESP-CAM you need to install Arduino IDE on your computer. Click this [link](https://www.arduino.cc/en/Main/Software) to download the arduino IDE.

After downloading you need to install the ESP boards. 
Click on **File->Preferences** and add the line below in the Additional Boards Manager URLs:

```
,https://dl.espressif.com/dl/package_esp32_index.json
```

Now click on **Tools->Boards->Boards Manager**.
A new window will pop up. In the search section type **esp32**. Click on the option and install. 
You are ready to program the ESP CAM now. 

## Verifying the setup. 
To verify that everything is working smoothly. Open the file **CameraWebServer.ino** given in the respository.
Change the ssid and password to those of your Wifi router.

Click on **Tools** and make sure all the settings are as follows:
  1. Board : ESP32 Wrover Module
  2. Upload Speed: 115200
  3. Flash Frequency: 40 MHz
  4. Flash Mode: QIO
  5. Partition Scheme: Huge APP (3MB No OTA)
  6. Core Debug Level: None
  7. Port: (Choose the port here that your FTDI or Arduino Uno is connected to).
  8. Programmer: AVR ISP
  
  (**Note: If you are using Arduino Uno, make sure to remove the chip from the board before programming.**
 
 After the settings are complete click on upload. 
 The IDE will start compiling. 
 After the compiling has been completed it will change to uploading, at this moment press the reset button on the ESP-CAM once.
 The following information or something similar will be displayed at the bottom. [Uploading Code](Uploading-Code.PNG). 
 
 Wait for the upload to complete.
 Remove the USB cable.
 Remove the jumper connecting IO0 to GND. 
 Connect the USB cable again and open the serial monitor. 
 It will give you an IP address.
 Open that IP address in the browser, scroll down and click on start streaming. 
 Congratulations!!. Your ESP CAM is working. 
 
 
Now comes the hard part. 
Install esp-idf from this [link](https://docs.espressif.com/projects/esp-idf/en/stable/get-started/index.html#get-started-get-esp-idf).
