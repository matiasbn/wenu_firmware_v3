#include <Arduino.h>
#line 1 "/Users/matiasbarrios/Desktop/wenu-esp8266firmware-2/wenu-esp8266firmware-2.ino"
#line 1 "/Users/matiasbarrios/Desktop/wenu-esp8266firmware-2/wenu-esp8266firmware-2.ino"
#include "rutinas_firmware.h"

//------------Parametros de configuracion modificados por script-------------------//
/* 
 *  El script del calibrador modificará estos parámetros para ajustarlos a las credenciales del dispositivo
 *  que se va a calibrar.
 *  wifiId: id del dispositivo.
 *  keyMAC: llave privada para el HMAC.
 *  serverURL:time es el servidor a donde apuntan los Wenu.
 *  keyCrypto: llave privada de AES.
 * 
 * HAY QUE MODIFICARLOS ANTES DE CONVERTIR EN FIRMWARE BASE
*/
// String wifiId = "WIFI_ID";
char* wifiId = "7";
char* keyMAC = "FIRM_HMAC";
char* serverURL = "URL_SERVER";
char* keyCrypto = "FIRM_CRYPTO";

//------------Parametros de configuracion fijos-------------------//
/* 
 *  Estos parámetros se ajustan de forma manual, dependiendo de lo siguiente:
 *  FIRM_V: ?
 *  accessSSID: es el nombre que tendrá el ESP8266 cuando esté en modo AP. wifiId se determina cuando
 *  el script trae el id de la base de datos para el dispositivo en particular.
 *  Se obtiene de una concatenación de Wenu- y el id del dispositivo.
 *  accessPassword: es la contraseña que tendrá el AP del ESP8266 una vez que se configure cuales son las 
 *  credenciales del router del cliente.
 *  accessGate: dirección del default gateway del AP del ESP8266.
 *  clockURL: servidor SNTP para sincronizar la hora, de forma que las tramas enviadas desde el ESP8266 tienen la hora
 *  en que fueron obtenidas y no la hora de llegada al servidor.
*/

char* FIRM_V = "3.0";
char* accessSSID=strcat("Wenu-",wifiId);;
char* accessPassword = "28421759";
char* accessGate = "192.168.4.1";
char* clockURL = "clock.wenu.cl";


// //---------------------------------------------------------------
// SSID y Password del router del cliente. 
// ESP8266WebServer server(80) define el puerto del servidor web.
//---------------------------------------------------------------

const char* ssid = "";
const char* password = "";
ESP8266WebServer server(80); //Server on port 80

//==============================================================
//                  SETUP
//==============================================================
void setup(void){
  Serial.begin(115200);
  //CAMBIAR ESTA LINEA ANTES DE UTILIZAR COMO FIRMWARE BASE!
  setupAccessPoint(accessSSID,"");
  // setupAccessPoint("Wenu-2","");

  //Rutinas de manejo de peticiones HTTP.
  //Las funciones están presentes en rutinas_firmware.h
  server.on("/", handleRoot);
  server.on("/configure", handleConfigure);
  server.begin();
  Serial.println("HTTP server started");

  //Iniciar EEPROM (PENDIENTE)
  // EEPROM.begin(512);
  // loadCredentials();
  // Serial.println("SSID: "+ ssid);
  // Serial.println("Password" + password);

  //Iniciar timeClient para ntp
}
//==============================================================
//                     LOOP
//==============================================================
void loop(void){
  server.handleClient();
  //Funciones para NTP
  Serial.println(ntpClient.getUnixTime());
  delay(2000);
}

