#include "rutinas_firmware.h"

//------------Parametros de configuracion modificados por script-------------------//
//  
// El script del calibrador modificará estos parámetros para ajustarlos a las credenciales del dispositivo
// que se va a calibrar.
// wifiId: id del dispositivo.
// keyMAC: llave privada para el HMAC.
// serverURL:time es el servidor a donde apuntan los Wenu.
// keyCrypto: llave privada de AES.

//CAMBIO CON FIRMWARE
// String wifiId = "WIFI_ID";
char* wifiId = "1451";
//CAMBIO CON FIRMWARE
// char* keyMAC = "FIRM_HMAC";
char* keyMAC = "tJhJool";
char* serverURL = "URL_SERVER";
//CAMBIO CON FIRMWARE
// char* keyCrypto = "FIRM_CRYPTO";
char* keyCrypto = "Ei7rWfj0ffQZiXUu";

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
//  DATAFRAME_AMOUNT: indica la cantidad de signos $ que debe tener la trama recibida
//  por el ESP8266. Es 1 para el caso de monofásico y 3 para el caso de trifásico.


char* FIRM_V = "3.0";
char* accessSSID=strcat("Wenu-",wifiId);;
char* accessPassword = "28421759";
char* accessGate = "192.168.4.1";
char* clockURL = "clock.wenu.cl";
int DATAFRAME_AMOUNT=3;



//---------------------------------------------------------------
// SSID y Password del router del cliente. 
// ESP8266WebServer server(80) define el puerto del servidor web.
//---------------------------------------------------------------

const char* ssid = "";
const char* password = "";
ESP8266WebServer server(80); //Server on port 80

//----------------------------------------------------------------------------
// Variables para comunicación serial.
// incomingMessage es el mensaje que se obtiene desde el Arduino. Se comienza a guardar
// cuando llega un * y se detiene cuando llegan tres $ para el caso de polifásico
// y un $ para el caso de monofásico.
// discard se utiliza para descartar o no el paquete dependiendo de las verificaciones
// hechas en loop.
// formattedMessage es el mensaje encriptado y con formato adecuado para ser tratado
// en backend.
//----------------------------------------------------------------------------
int counterDataFrame=0;
String incomingMessage;
bool discard = false;
String formattedMessage;


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

  //Setear la cantidad de DATAFRAME_AMOUNT si es que es monofásico

  //CAMBIO CON FIRMWARE
  // if(FIRM_V=="1.0"){
  //   DATAFRAME_AMOUNT = 1;

  // }

  server.begin();
  Serial.println("HTTP server started");

  //Iniciar EEPROM (PENDIENTE)
  // EEPROM.begin(512);
  // loadCredentials();
  // Serial.println("SSID: "+ ssid);
  // Serial.println("Password" + password);

  //Ticker que envia
  // askForDataToArduino.attach(15,sendHashTag);

}


//==============================================================
//                     LOOP
//==============================================================
void loop(void){
  server.handleClient();
  
  //Funciones para NTP
  //Serial.println(ntpClient.getUnixTime());


  //Las funciones de encriptación y envió deben ocurrir dentro de este if,
  //ya que si no, estas se ejecutarán con cada vuelta del loop 
  if (Serial.available()>0){
    
    //Recibir todo el mensaje hasta que se encuentra con '\0'
    incomingMessage = Serial.readStringUntil('\0');

    //Contar la cantidad de símbolos $
    for(int i =0; i<incomingMessage.length();i++){
      if(incomingMessage[i]=='$'){
        counterDataFrame++;
      }
    }

    //Descartar si es que no llegaron los tres $ 
    // o si no comienza con un *.
    if(counterDataFrame!=DATAFRAME_AMOUNT || incomingMessage[0]!='*'){
      discard = true;
    }

    //Si no se debe descartar la trama, se encripta y se da formato.
    //Si se descarta
    if(!discard){
      formattedMessage = encryptAndFormat(incomingMessage);
      // encryptAndFormat(incomingMessage);
      Serial.println("El mensaje con formato es: "+formattedMessage);
    }
  }
  
  //Volver al estado inicial para la próxima trama
  counterDataFrame=0;
  discard=false;
}
