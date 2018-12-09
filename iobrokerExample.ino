//Подключаем библиотеки
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h> //https://github.com/knolleary/pubsubclient
#include <Bounce2.h> //https://github.com/thomasfredericks/Bounce2
//Настройки сети
byte mac[] = {  0xAB, 0xBC, 0xCD, 0xDE, 0xEF, 0x31 };
byte ip[] = { 192, 168, 1, 31 }; //IP-адрес платы arduino
byte mqttserver[] = { 192, 168, 1, 7 }; //IP-адрес сервера ioBroker
//#define mqtt_server "m14.cloudmqtt.com" // mqtt брокер
#define mqtt_login "revixooe"
#define mqtt_password "nqqDyjgQpPmY"
#define mqtt_port  1883 //10241 // порт подключения к mqtt, у cloudmqtt порт отличный от 1883
EthernetClient ethClient;
void callback(char* topic, byte* payload, unsigned int length);
PubSubClient client(mqttserver, mqtt_port, callback, ethClient);
//Глобальные переменные
#define LED_bathroom 7
#define LED_koridor 6
#define LED_temnaya 5
#define LED_free 4

#define ButtonBanthromPIN A0
#define ButtonKoridorPIN A1
#define ButtonTemnayaPIN A2
#define ButtonfreePIN A3 //Пока свободная кнопка для управления пином. Включил сразу, чтобы потом не перепрошивать
#define ButtonDoorBellPIN A4 //Квартиная кнопка звонка

int ledBathroomState;
int ledKoridorState;
int ledTemnayaState;
int ledFreeState;

unsigned long buttonPressBathroom;
// Instantiate a Bounce object
//Bounce debouncer = Bounce();
Bounce debouncer1 = Bounce();
Bounce debouncer2 = Bounce();
Bounce debouncer3 = Bounce();
Bounce debouncer4 = Bounce();
Bounce debouncer5 = Bounce();
int BathroomState;
unsigned int send_interval = 10; //интервал отправки показаний на сервер по-умолчанию 10 секунд
unsigned long last_time = 0; //текущее время для таймера
char buff[20];

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println ("");
  Serial.println ("-------");
  Serial.println ("New callback of MQTT-broker");
  //преобразуем тему(topic) и значение (payload) в строку
  payload[length] = '\0';
  String strTopic = String(topic);
  String strPayload = String((char*)payload);
  //Исследуем что "прилетело" от сервера по подписке:
  //Изменение интервала опроса
  if (strTopic == "ArduinoMega/send_interval") {
    int tmp = strPayload.toInt();
    if (tmp == 0) {
      send_interval = 10;
    } else {
      send_interval = strPayload.toInt();
    }
  }
  //Управление светодиодом
  if (strTopic == "Lighting/Bathroom") {
    if (strPayload == "off" || strPayload == "0" || strPayload == "false") digitalWrite(LED_bathroom, LOW);
    if (strPayload == "on" || strPayload == "1" || strPayload == "true") digitalWrite(LED_bathroom, HIGH);
  }

  if (strTopic == "Lighting/Koridor") {
    if (strPayload == "off" || strPayload == "0" || strPayload == "false") digitalWrite(LED_koridor, LOW);
    if (strPayload == "on" || strPayload == "1" || strPayload == "true") digitalWrite(LED_koridor, HIGH);
  }

  if (strTopic == "Lighting/Temnaya") {
    if (strPayload == "off" || strPayload == "0" || strPayload == "false") digitalWrite(LED_temnaya, LOW);
    if (strPayload == "on" || strPayload == "1" || strPayload == "true") digitalWrite(LED_temnaya, HIGH);
  }

  if (strTopic == "Lighting/free") {
    if (strPayload == "off" || strPayload == "0" || strPayload == "false") digitalWrite(LED_free, LOW);
    if (strPayload == "on" || strPayload == "1" || strPayload == "true") digitalWrite(LED_free, HIGH);
  }


  Serial.print (strTopic);
  Serial.print (" ");
  Serial.println (strPayload);
  Serial.println ("-------");
  Serial.println ("");
}
void setup() {
  Serial.begin(9600);
  Serial.println("Start...");


  // Setup the button
  pinMode(ButtonBanthromPIN, INPUT);
  // Активируем встроенный резистор для кнопки
  digitalWrite(ButtonBanthromPIN, HIGH);


  // Setup the button
  pinMode(ButtonKoridorPIN, INPUT);
  // Активируем встроенный резистор для кнопки
  digitalWrite(ButtonKoridorPIN, HIGH);

   // Setup the button
  pinMode(ButtonTemnayaPIN, INPUT);
  // Активируем встроенный резистор для кнопки
  digitalWrite(ButtonTemnayaPIN, HIGH);

   // Setup the button
  pinMode(ButtonfreePIN, INPUT);
  // Активируем встроенный резистор для кнопки
  digitalWrite(ButtonfreePIN, HIGH);

     // Setup the button
  pinMode(ButtonDoorBellPIN, INPUT);
  // Активируем встроенный резистор для кнопки
  digitalWrite(ButtonDoorBellPIN, HIGH);

  // После настройки кнопки настраиваем антидребезг.
  debouncer1.attach(ButtonBanthromPIN);
  debouncer1.interval(5);

  debouncer2.attach(ButtonKoridorPIN);
  debouncer2.interval(5);

  debouncer3.attach(ButtonTemnayaPIN);
  debouncer3.interval(5);

  debouncer4.attach(ButtonfreePIN);
  debouncer4.interval(5);

  debouncer5.attach(ButtonDoorBellPIN);
  debouncer5.interval(5);


  //Инициализируем порты ввода-вывода, прописываем начальные значения
  pinMode(LED_bathroom, OUTPUT);
  digitalWrite(LED_bathroom, ledBathroomState);

  pinMode(LED_koridor, OUTPUT);
  digitalWrite(LED_koridor, ledKoridorState);

  pinMode(LED_temnaya, OUTPUT);
  digitalWrite(LED_temnaya, ledTemnayaState);

  pinMode(LED_free, OUTPUT);
  digitalWrite(LED_free, ledFreeState);
  //стартуем сетевое подключение
  Ethernet.begin(mac, ip);
  Serial.print("IP: ");
  Serial.println(Ethernet.localIP());
 

}
void loop() {

  // Update the debouncer and get the changed state
  boolean changedButtonBathroom = debouncer1.update();

  if ( changedButtonBathroom  ) {
    // Получаем обновлённые значения состояния кнопки
    int value = debouncer1.read();
    if ( value == HIGH) {
      ledBathroomState = !ledBathroomState;
      String temp_ledBathroomState =  String(ledBathroomState); //Строка для хранения состояния Led пина. Нужна для конвертирования в массив и отправки по mqtt.
      char BathroomStateMqtt[50]; //Массимв для отправки по mqtt
      temp_ledBathroomState.toCharArray(BathroomStateMqtt, temp_ledBathroomState.length() + 1); //Конвертируем  в массив для публикации по mqtt
      digitalWrite(LED_bathroom, ledBathroomState );
      Serial.println("Состояние кнопки ванной изменилось");
      client.publish("Lighting/Bathroom", BathroomStateMqtt);
    }
  }

  // Update the debouncer and get the changed state
  boolean changedTKoridorBathroom  = debouncer2.update();
//Кнопки отправляем сразу(без интервалов)
  if ( changedTKoridorBathroom ) {
    // Получаем обновлённые значения состояния кнопки
    int value2 = debouncer2.read();
    if ( value2 == HIGH) {
      ledKoridorState = !ledKoridorState;
      String temp_ledKoridorState =  String(ledKoridorState); //Строка для хранения состояния Led пина. Нужна для конвертирования в массив и отправки по mqtt.
      char KoridorStateMqtt[50]; //Массив для отправки по mqtt
      temp_ledKoridorState.toCharArray(KoridorStateMqtt, temp_ledKoridorState.length() + 1); //Конвертируем  в массив для публикации по mqtt
      digitalWrite(LED_koridor, ledKoridorState );
      Serial.println("Состояние кнопки коридора изменилось");
      client.publish("Lighting/Koridor", KoridorStateMqtt);
    }
  }
  boolean changedTemnayaBathroom  = debouncer3.update();
  if ( changedTemnayaBathroom ) {
    // Получаем обновлённые значения состояния кнопки
    int value3 = debouncer3.read();
    if ( value3 == HIGH) {
      ledTemnayaState = !ledTemnayaState;
      String temp_ledTemnayaState =  String(ledTemnayaState); //Строка для хранения состояния Led пина. Нужна для конвертирования в массив и отправки по mqtt.
      char TemnayaStateMqtt[50]; //Массив для отправки по mqtt
      temp_ledTemnayaState.toCharArray(TemnayaStateMqtt, temp_ledTemnayaState.length() + 1); //Конвертируем  в массив для публикации по mqtt
      digitalWrite(LED_temnaya, ledTemnayaState );
      Serial.println("Состояние кнопки тёмной изменилось");
      client.publish("Lighting/Temnaya", TemnayaStateMqtt);
    }
  }

    boolean changedFreeBathroom  = debouncer4.update();
  if ( changedFreeBathroom ) {
    // Получаем обновлённые значения состояния кнопки
    int value4 = debouncer4.read();
    if ( value4 == HIGH) {
      ledFreeState = !ledFreeState;
      String temp_ledFreeState =  String(ledFreeState); //Строка для хранения состояния Led пина. Нужна для конвертирования в массив и отправки по mqtt.
      char FreeStateMqtt[50]; //Массив для отправки по mqtt
      temp_ledFreeState.toCharArray(FreeStateMqtt, temp_ledFreeState.length() + 1); //Конвертируем  в массив для публикации по mqtt
      digitalWrite(LED_free, ledFreeState );
      Serial.println("Состояние свободной кнопки изменилось");
      client.publish("Lighting/free", FreeStateMqtt);
    }
  }

  boolean changedDoorBellButton  = debouncer5.update();
  if ( changedDoorBellButton ) {
    // Получаем обновлённые значения состояния кнопки
    int value5 = debouncer5.read();
    if ( value5 == HIGH) {   
      char DoorBellStateMqtt[50]; //Массив для отправки по mqtt
      String temp_DoorBellState = "1";
      temp_DoorBellState.toCharArray(DoorBellStateMqtt, temp_DoorBellState.length() + 1); //Конвертируем  в массив для публикации по mqtt
      Serial.println("Состояние свободной кнопки изменилось");
      client.publish("Koridor/DoorBell", DoorBellStateMqtt);
    }
  }

  //Если соединение MQTT неактивно, то пытаемся установить его и опубликовать/подписаться
  if (!client.connected()) {
    Serial.print("Connect to MQTT-broker...  ");
    //Подключаемся и публикуемся/подписываемся
    if (client.connect("ArduinoMega")) {
      Serial.println("success");
      //Значение с датчиков
      //Подписываемся на интервал опроса
      client.subscribe("ArduinoMega/send_interval");
      //Подписываемся на переменную управления светодиодом
      client.subscribe("Lighting/#");
    } else {
      //Если не подключились, ждем 10 секунд и пытаемся снова
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 10 seconds");
      delay (10000);
    }
    //Если соединение активно, то отправляем данные на сервер с заданным интервалом времени(для датчиков)
  } else {
    if (millis() > (last_time + send_interval * 1000)) {
      last_time = millis();

    }
  }
  //Проверка входящих соединений по подписке
   if (client.connect("example1")) { client.loop();}
}
