#include <RCSwitch.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

#define TRANSMIT_PIN 12
#define BUZZER_PIN 11
RCSwitch Transmitter = RCSwitch();
LiquidCrystal_I2C lcd(0x27, 16, 2);

RCSwitch Receiver = RCSwitch();


const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {10,9,8,7};
byte colPins[COLS] = {6,5,4,3};
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

unsigned long lastTime = 0;
String currentTask = "None";
bool inputMode = false;
byte tableInput = 0;

void setup() {
  lcd.init();
  lcd.backlight();

  Transmitter.enableTransmit(TRANSMIT_PIN);
  Transmitter.setProtocol(1);
  Transmitter.setPulseLength(350);
  Transmitter.setRepeatTransmit(10);
  Receiver.enableReceive(digitalPinToInterrupt(2));

  pinMode(BUZZER_PIN, OUTPUT);

  showCurrentTask();
}

void showCurrentTask() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Current Task:");
  lcd.setCursor(0,1);
  lcd.print(currentTask);
}

void receiveTasks() {
  if (Receiver.available()) {
    unsigned long now = millis();

    if (now - lastTime > 300) {
      unsigned int packet = Receiver.getReceivedValue();

      byte typeId = (packet >> 8) & 0b11;
      if (typeId == 0b10) {
        byte tableId = (packet >> 2) & 0b111111;
        byte taskId  = packet & 0b00000011;
        currentTask = "T" + String(taskId) + " -> Table " + String(tableId);
      }
      else if (packet == 0b1100000000) {
        currentTask = "Robot stuck!";
        tone(BUZZER_PIN, 1000, 500); // 1000Гц, 500 мс
      }

      lastTime = now;
    }
    Receiver.resetAvailable();
  }
}


void loop() {
  receiveTasks();

  char key = keypad.getKey();

  if (!inputMode) { // обычный режим — кнопка A для начала ввода
    
    if (key == 'A') {
      inputMode = true;
      tableInput = 0;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Enter TableID:");
      lcd.setCursor(0,1);
      lcd.print(tableInput);
    }
  }
  else { // режим ввода номера стола

    if (key >= '0' && key <= '9') {
      byte digit = key - '0';
      byte newValue = tableInput * 10 + digit;
      if (newValue <= 63) {
        tableInput = newValue;
        lcd.setCursor(0,1);
        lcd.print("                ");
        lcd.setCursor(0,1);
        lcd.print(tableInput);
      }
    }
    else if (key == '#') { // подтверждение
      byte taskId = 1;
      byte packet = 0b01 << 8 | (tableInput << 2) | taskId;
      Transmitter.send(packet, 10);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Task sent!");
      delay(1000);
      inputMode = false;
      showCurrentTask();
    }
    else if (key == '*') { // сброс ввода
      tableInput = 0;
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print(tableInput);
    }
    else if (key == 'B') { // возврат в режим показа статуса
      tableInput = 0;
      inputMode = false;
      showCurrentTask();
    }
  }

  delay(50);
}
