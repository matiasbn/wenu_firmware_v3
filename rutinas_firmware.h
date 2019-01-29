#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <time.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <EasyNTPClient.h>
#include <string.h>
#include <Ticker.h>
#include <base64.h>
#include "aes_encryption.h"


//===============================================================
// Variables externas
//===============================================================
extern const char* ssid;
extern const char* password;
extern ESP8266WebServer server;
extern char* accessPassword;
extern char* accessSSID;
extern char* wifiId;
extern char* accessGate;
extern char* clockURL;
extern char* FIRM_V;
extern char* keyCrypto;

//===============================================================
// Variables para Ticker
// askForDataToArduino: Ticker para enviar un # al arduino
// cada 15 segundos al arduino para que este envíe información.
// 
//===============================================================
Ticker askForDataToArduino;

//===============================================================
// Variables para NTP
//===============================================================
WiFiUDP udp;
EasyNTPClient ntpClient(udp, "clock.wenu.cl");

//===============================================================
// Prototipos de funciones 
//===============================================================
void setupAccessPoint(char* _ssid, char* _password); 
void saveCredentials(String _ssid, String _password);


//===============================================================
// Página web para ingresar el SSID y la password
//===============================================================

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>
<form action="configure">
  <center>
      Nombre AP: <input type="text" name="ssid"><br>
      Clave AP: <input type="text" name="password"><br>
      <div style="width: 40px; margin:0 auto">
        <input type="submit" value="Conectar">
      </div>
  </center>
</form> 
</body>
</html>
)=====";


//===============================================================================================
//                    FUNCIONES
//===============================================================================================

//===============================================================
// Rutina que se ejecuta para llamar a la página web para configuración
//===============================================================
void handleRoot() {
 Serial.println("You called root page");
 String s = MAIN_page; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
}

//===============================================================
// Rutina para conexión a nuevo WiFi
// Recibe como parámetros ssid y password.
// Retorna true si es que la conexión fue exitosa.
//===============================================================

bool connectToWifi(const char* _ssid, const char* _password){
  WiFi.begin(_ssid, _password);
  Serial.println("");
  
  //Esperar la conexión
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(_ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  return true;
}

//===============================================================
// Rutina para obtención de SSID y password desde página web
//===============================================================
void handleConfigure() { 
  //Verificar si es que el usuario ingresó ambos parámetros.
  if(server.arg("ssid")=="" || server.arg("password")==""){
     String message = "Ingrese SSID y password";
     server.send(400, "text/plain", message);
     return;
  }
  
  
  // Conectarse a la nueva red de WiFi
  // Ingresa los parámetros ingresados en la página web como 
  // argumentos de connectToWifi, que regresa true si la conexión fue exitosa.
  // Si la conexión fue exitosa, entonces guarda las credenciales.
  if (connectToWifi(server.arg("ssid").c_str(),server.arg("password").c_str())){
    ssid = server.arg("ssid").c_str();
    password = server.arg("password").c_str();    
  }

  //Guardar las credenciales (PENDIENTE)
  // saveCredentials(server.arg("ssid"),server.arg("password"));
  
 
  //Mensaje para mostrarse en página Web una vez que se ingresó el SSID y el password.
  String message = "Conectado a: " + (String)ssid + ", con password: "+ (String)password; 
  server.send(200, "text/plain", message);       //Response to the HTTP request
  
  //Cambiar las credenciales del AP del ESP8266
  // setupAccessPoint(accessSSID,accessPassword);
 
}

//======================================================================
// Rutina para configuración de ESP8266 como Access Point Y Station mode
//======================================================================
void setupAccessPoint(char* _ssid, char* _password) {
  //Separar accessGate por punto para ingresarlo como default gateway
  //MODIFICAR PARA OBTENER GATEWAY DESDE accessGate
  // char* _gateway = strtok(accessGate,".");
  IPAddress gateway(192,168 ,4, 1);
  IPAddress subnet(255, 255, 255, 0);
  
  //Activar el ESP8266 en modo AP y STA
  WiFi.mode(WIFI_STA);
  if(!WiFi.softAPConfig(gateway, gateway, subnet)){
    Serial.println("Configuración de AP falló"); 
  }
  
  //Establecer ssid y password del AP
  boolean result = WiFi.softAP(_ssid, _password);
  if(result == true){
    Serial.println("Ready");
  }
  else{
    Serial.println("Failed!");
  }
}

//====================================================================
// Funcion para el Ticker, que envía un # al Arduino cada 15 segundos
// para que este le envíe información al ESP8266.
//====================================================================
void sendHashTag() {
  Serial.println("#");
}


//=========================================================================
//Función para encriptar y dar formato a la trama recibida desde el Arduino
//=========================================================================
String encryptAndFormat(String _message){
  //Crear el vector de inicialización.
  aes_init();
  // generateIv(iv);
  // runnable();

  // // Convertir String a char.
  char messageToChar[_message.length()-1];
  for(int i=0;i<_message.length();i++){
    messageToChar[i] = _message[i];
  }

  //Encriptar el Mensaje y pasarlo a base64
  String encryptedMessage = encrypt(messageToChar,aes_iv);

  //IV a base64
  String encoded = base64::encode("(char*)aes_iv");
  Serial.println(encoded);

  //Prints
  Serial.println("Mensaje encriptado y en base64");
  Serial.println(encryptedMessage);
  Serial.println("Initizalization vector");
  for(int i =0;i<N_BLOCK;i++){
    Serial.print(aes_iv[i]);
  }  
  Serial.println("");
  Serial.println("Llave privada");
  // String privKey = String(aes_key); 
  // Serial.println(privKey);
  for(int i =0;i<N_BLOCK;i++){
    Serial.print((char)aes_key[i]);
  }  
  
  keyCryptoToHex(keyCrypto);

  Serial.println("");
  // Serial.println("");
  return String("Hola");
}

// //===============================================================
// // Guardar las credenciales en EEPROM
// //===============================================================
// void saveCredentials(String _ssid, String _password) {
//   // EEPROM.begin(512);
//   EEPROM.put(0, _ssid);
//   EEPROM.put(0+sizeof(_ssid), _password);
//   EEPROM.commit();
//   EEPROM.end();
// }

// //===============================================================
// // Obtener SSID y Password desde EEPROM
// //===============================================================
// void loadCredentials() {
//   // EEPROM.begin(512);
//   EEPROM.get(0, ssid);
//   EEPROM.get(0+sizeof(ssid), password);
//   EEPROM.end();
// }
