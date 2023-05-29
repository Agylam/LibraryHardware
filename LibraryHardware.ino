#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9 
#define SS_PIN 10

MFRC522 rfid(SS_PIN, RST_PIN);

void setup(){
  Serial.begin(9600);
  Serial.setTimeout(50);
  SPI.begin();
  rfid.PCD_Init();
}

bool readUid;

void loop () {
  if (Serial.available()) {
    String cmd = Serial.readString();
    if(cmd == "CARD_READ"){
        readUid = true;
        Serial.println("OK");
    }else if(cmd == "PING"){
        Serial.println("PONG");
        tone(8,1200,500);
        delay(500);
        tone(8,1400,500);
        delay(500);
        tone(8,1200,500);
    }else if(cmd == "CARD_CANCEL"){
      readUid = false;
      Serial.println("OK");
    }else{
        Serial.println("ERR_UNK_CMD");
    }

  }
  if (readUid) readUID();
}

void readUID(){
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    uint32_t ID;
    for (byte i = 0; i < 4; i++) {
      ID <<= 8;
      ID |= rfid.uid.uidByte[i];
    }
    Serial.print("CARD_");
    Serial.println(ID, HEX);
    tone(8,1400,500);
    readUid = false;
  }
}
