#include "rutinas_firmware.h"

//------------Parametros de configuracion modificados por script-------------------//
//  
// El script del calibrador modificará estos parámetros para ajustarlos a las credenciales del dispositivo
// que se va a calibrar.
// wifiId: id del dispositivo.
// keyMAC: llave privada para el HMAC.
// serverURL:time es el servidor a donde apuntan los Wenu.
// keyCrypto: llave privada de AES.


// String wifiId = "WIFI_ID";
char* wifiId = "7";
char* keyMAC = "FIRM_HMAC";
char* serverURL = "URL_SERVER";
char* keyCrypto = "FIRM_CRYPTO";

//------------Parametros de configuracion fijos-------------------//
//  Estos parámetros se ajustan de forma manual, dependiendo de lo siguiente:
//  FIRM_V: ?
//  accessSSID: es el nombre que tendrá el ESP8266 cuando esté en modo AP. wifiId se determina cuando
//  el script trae el id de la base de datos para el dispositivo en particular.
//  Se obtiene de una concatenación de Wenu- y el id del dispositivo.
//  accessPassword: es la contraseña que tendrá el AP del ESP8266 una vez que se configure cuales son las 
//  credenciales del router del cliente.
//  accessGate: dirección del default gateway del AP del ESP8266.
//  clockURL: servidor SNTP para sincronizar la hora, de forma que las tramas enviadas desde el ESP8266 tienen la hora
//  en que fueron obtenidas y no la hora de llegada al servidor.


char* FIRM_V = "3.0";
char* accessSSID=strcat("Wenu-",wifiId);;
char* accessPassword = "28421759";
char* accessGate = "192.168.4.1";
char* clockURL = "clock.wenu.cl";


//---------------------------------------------------------------
// SSID y Password del router del cliente. 
// ESP8266WebServer server(80) define el puerto del servidor web.
//---------------------------------------------------------------

const char* ssid = "";
const char* password = "";
ESP8266WebServer server(80); //Server on port 80

//----------------------------------------------------------------------------
// Variables para comunicación serial.
// incomingByte se utiliza para leer lo que el Arduino manda por puerto serie.
// counterDataFrame se utiliza para contar la cantidad de tramas que han llegado
// por puerto Serial.
//----------------------------------------------------------------------------
char incomingByte;
char* message;
int counterDataFrame=0;




//==============================================================
//                  SETUP
//==============================================================
void setup(void){
  Serial.begin(115200);
  
  //Configurar el ESP como AP con las credenciales dependiendo de su ID.
  //true para encender modo AP.
  setupAccessPoint(accessSSID,"");

  //Rutinas de manejo de peticiones HTTP.
  //Las funciones están presentes en rutinas_firmware.h
  server.on("/", handleRoot);

  //Configurar el ESP para cuando el usuario ingrese SSID y pass
  server.on("/configure", handleConfigure);


  server.begin();
  Serial.println("HTTP server started");

  //Iniciar EEPROM (PENDIENTE)
  // EEPROM.begin(512);
  // loadCredentials();
  // Serial.println("SSID: "+ ssid);
  // Serial.println("Password" + password);

  //Ticker que envia
  askForDataToArduino.attach(3,startCommunication);

}
//==============================================================
//                     LOOP
//==============================================================
void loop(void){
  server.handleClient();
  
  //Funciones para NTP
  //Serial.println(ntpClient.getUnixTime());

  if (Serial.available()>0 && Serial.read()=="*"){
    while(Serial.available()>0) {
  


      // Contar la cantidad de tramas mediante el separador $
      if(Serial.read()=="$"){
        counterDataFrame++;
      }

      //Cuando llega la tercera trama, romper el ciclo
      if(counterDataFrame==2){
        break;
      
      //Imprimir en puerto serie
      Serial.print("I received: ");
      Serial.println(incomingByte);
      }
    }
  }
}
