/*  
    Скрипт считывателя Agylam
    Автор: MIMBOL
*/

// Настройки

#define SECREY_KEY "*IR^DOGG4!YXmw7I09CXXBsTE^dBZxoz" // Секретный ключ устройства
#define DEVICE_ID "6c299457-568e-4009-a3c4-505e4c82e81b" // ID устройства
#define VERSION_HARDWARE "v1.1"
#define TONE_PIN 8 // Пин пищалки
#define RST_PIN 9 // Пин сброса
#define SS_PIN 10 // Пин выбора устройства



// Библиотеки

#include <SPI.h> // Библиотека SPI
#include <MFRC522.h> // Библиотека RFID
#include <mString.h> // Библиотека для работы со строками
#include <MD5.h> // Библиотека для работы с MD5


// "Подключаемся" к железякам

MFRC522 rfid(SS_PIN, RST_PIN); // RFID считыватель


// Разные переменные

bool readUid; // Флаг чтения UID
bool toneActive = true; // Флаг работы пищалки
mString<62> preSign; // Строка для подписи
mString<50> beforeSlicing; // Строка для разбиения
uint16_t PCRandom; // Случайное число для подписи от ПК
uint16_t ARRandom; // Случайное число для подписи от Ардуино


// Вспомогательные функции 

void getHEXuid(char *a, byte uidByte[10], byte size = 0x00){ // Получение HEX UID
    const char *hex = "0123456789ABCDEF";
    for (int i = 0; i < 7; i++) {
        byte lo = uidByte[i] % 16;
        byte hi = uidByte[i] - lo;
      if(i < size){
          *a++ = hex[hi / 16];
          *a++ = hex[lo];
      }else{
          *a++ = hex[0];
          *a++ = hex[0];
      }
    }
    *a = 0;
}

void sendUID(char* UID){ // Отправка UID
    uint16_t ARrandom = random(0,65535); // Случайное число для подписи от Ардуино
    preSign = UID;
    preSign += PCRandom;
    preSign += ARrandom;
    preSign += SECREY_KEY;
    unsigned char* hash=MD5::make_hash(preSign.buf); // Хэш
    char *sign = MD5::make_digest(hash, 16); // Подпись
    free(hash); // Освобождение памяти
    Serial.print("C"); // Отправка ответа
    Serial.print(UID);
    Serial.print(":");
    Serial.print(ARrandom);
    Serial.print(":");
    Serial.println(sign);
}

void smartTone(uint16_t frequency, uint16_t duration) { // Умная пищалка 
  if(toneActive) tone(TONE_PIN, frequency, duration);
}

void readUID() { // Чтение UID
    if (! rfid.PICC_IsNewCardPresent() ) return; // Если нет карты - выход
    if (! rfid.PICC_ReadCardSerial() ) return; // Если не удалось прочитать карту - выход
    char uid[20] = {0};
    getHEXuid(uid, rfid.uid.uidByte, rfid.uid.size); // Получение HEX UID
    sendUID(uid); // Отправка UID
    smartTone(1400, 500); // Пищалка
    readUid = false; // Сброс флага чтения UID
}

void setRandomSeed(){ // Установка случайного сида
  uint32_t seed = 0;
  for (int i = 0; i < 16; i++) {
    seed *= 4;
    seed += analogRead(A0) & 3;
    randomSeed(seed);
  }
}

// Освновные функции 

void setup() { 
    setRandomSeed(); // Настройка случайного сида
    Serial.begin(9600); // Начало работы с Serial
    Serial.setTimeout(50); // Установка таймаута
    SPI.begin(); // Начало работы с SPI
    rfid.PCD_Init(); // Начало работы с RFID
}

void loop() {
    if (Serial.available()) { // Если есть данные в Serial
        beforeSlicing = Serial.readString(); // Строка для разбиения
        char* slices[3]; // Массив строк
        beforeSlicing.split(slices,':'); // Разбиение строки
        mString<10> cmd; // Команда 
        cmd = slices[0]; // Установка команды
        if(cmd == "CARD_READ"){ // Чтение карты
            readUid = true; // Флаг чтения UID
            if(atoi(slices[1]) != 0){ // Если есть PCRandom и он не равен 0
                PCRandom = atoi(slices[1]);
            }else{
                Serial.println("EPCRandomEmptyOrZero"); // Ошибка: PCRandom пуст или равен 0
                return;
            }
            Serial.println("O");
        }else if (cmd == "CARD_CANCEL"){ // Отмена чтения карты
            readUid = false; // Флаг чтения UID
            Serial.println("O");
        }else if (cmd == "TONE_ON"){ // Включение пищалки
            toneActive = true;
            Serial.println("O");
        }else if (cmd == "TONE_OFF"){ // Выключение пищалки
            toneActive = false;
            Serial.println("O");
        }else if (cmd == "GET_ID"){ // Получение ID устройства
            Serial.print("I");
            Serial.println(DEVICE_ID);
        }else if (cmd == "GET_VERSION"){
            Serial.print("V");
            Serial.println(VERSION_HARDWARE);
        }else{
            Serial.println("EUNK_CMD"); // Ошибка: неизвестная команда
        }
    }
    if(readUid) readUID(); // Чтение UID
}