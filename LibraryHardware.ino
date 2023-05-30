#include <SPI.h>
#include <MFRC522.h>
#include <mString.h>
#include <microDS3231.h>
#include <MD5.h>

#define RST_PIN 9
#define SS_PIN 10
#define SECREY_KEY "kVz%CpP$X5NtyDE#XRV7Mg8gJfR5R6Ms"
#define DEVICE_ID "YLSRXZ646SQH23VX"
#define TIMEZONE 5
#define TIMECORRECT -2

MFRC522 rfid(SS_PIN, RST_PIN);
MicroDS3231 rtc;

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(50);
  SPI.begin();
  rfid.PCD_Init();
  if (!rtc.begin()) {
    Serial.println("TIME_MODULE_ERR");
    while (true) {}
  }
  Serial.println("LOAD_SUCCESS");
}

bool readUid;
mString<60> preSign;
void loop() {
  if (Serial.available()) {
    String cmd = Serial.readString();
    if (cmd == "CARD_READ") {
      readUid = true;
      Serial.println("OK");
    } else if (cmd == "PING") {
      Serial.println("PONG");
      tone(8, 1200, 500);
      delay(500);
      tone(8, 1400, 500);
      delay(500);
      tone(8, 1200, 500);
    } else if (cmd == "CARD_CANCEL") {
      readUid = false;
      Serial.println("OK");
    } else if (cmd == "GET_SIGN") {
      preSign = SECREY_KEY;
      preSign += String(rtc.getUnix(TIMEZONE)+(TIMECORRECT));
      unsigned char* hash=MD5::make_hash(preSign.buf);
      char *sign = MD5::make_digest(hash, 16);
      free(hash);
      Serial.println(sign);
      Serial.println("OK");
    } else {
      Serial.println("ERR_UNK_CMD");
    }
  }
  if (readUid) readUID();
}

void readUID() {
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    char buf[20] = {0};
    getHEXuid(buf, rfid.uid.uidByte, rfid.uid.size);
    Serial.print("CARD_");
    Serial.println(buf);
    tone(8, 1400, 500);
    readUid = false;
  }
}

void getHEXuid(char *a, byte uidByte[10], byte size = 0x00){
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

