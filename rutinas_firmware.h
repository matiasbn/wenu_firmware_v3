#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <time.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <EasyNTPClient.h>


//===============================================================
// Variables externas
//===============================================================
extern const char* ssid;
extern const char* password;
extern ESP8266WebServer server;
extern char* accessPassword;
extern String wifiId;


// //===============================================================
// // Variables externas
// //===============================================================
WiFiUDP udp;
EasyNTPClient ntpClient(udp, "clock.wenu.cl");


//===============================================================
// Prototipos de funciones 
//===============================================================
void setupAccessPoint(char* ap_name,char* ap_password); 
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
//===============================================================

void handleConnection(){
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
}

//===============================================================
// Rutina para obtención de SSID y password desde página web
//===============================================================
void handleConfigure() { 
  if(server.arg("ssid")=="" || server.arg("password")==""){
     String message = "Ingrese SSID y password";
     server.send(400, "text/plain", message);       //Response to the HTTP request
     return;
  }
  
  //Convertir String a char*
  ssid = server.arg("ssid").c_str();
  password = server.arg("password").c_str();

  //Guardar las credenciales (PENDIENTE)
  // saveCredentials(server.arg("ssid"),server.arg("password"));
  
  //Conectarse a la nueva red de WiFi
  handleConnection();
 
  //Mensaje para mostrarse en página Web una vez que se ingresó el SSID y el password.
  String message = "Conectado a: " + (String)ssid + ", con password: "+ (String)password; 
  server.send(200, "text/plain", message);       //Response to the HTTP request
  
  //Cambiar las credenciales del AP del ESP8266
  /*
   * !!!!!!!CAMBIAR ESTA LINEA CUANDO SE QUIERA PASAR A FIRMWARE BASE!!!!!
   */
  //setupAccessPoint("Wenu-"+wifiId,accessPassword);

  // setupAccessPoint("Wenu-2",accessPassword);
}

//===============================================================
// Rutina para configuración de ESP8266 como Access Point
//===============================================================
void setupAccessPoint(char* ap_name,char* ap_password) {
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.mode(WIFI_STA);
  if(!WiFi.softAPConfig(gateway, gateway, subnet)){
    Serial.println("Configuración de AP falló"); 
  }
  
  boolean result = WiFi.softAP(ap_name, ap_password);
  if(result == true){
    Serial.println("Ready");
  }
  else{
    Serial.println("Failed!");
  }
}

//===============================================================
// Guardar las credenciales en EEPROM
//===============================================================
void saveCredentials(String _ssid, String _password) {
  // EEPROM.begin(512);
  EEPROM.put(0, _ssid);
  EEPROM.put(0+sizeof(_ssid), _password);
  EEPROM.commit();
  EEPROM.end();
}

//===============================================================
// Obtener SSID y Password desde EEPROM
//===============================================================
void loadCredentials() {
  // EEPROM.begin(512);
  EEPROM.get(0, ssid);
  EEPROM.get(0+sizeof(ssid), password);
  EEPROM.end();
}
