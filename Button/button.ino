// constants won't change. They're used here to set pin numbers:
const int buttonPin = 2;
const int ledPin = 3;

const int BUTTON_PRESSED = HIGH;

// variables will change:
int buttonState = 0;

int mode = 0;  // 0：恆亮、1：慢閃、2：中速閃、3：快閃

int lastReading;
int stableButtonState;

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 30;

unsigned long previousMillis = 0;
bool ledState = true;  // true 代表亮燈

void setup() {
  digitalWrite(ledPin, LOW);
  pinMode(ledPin, OUTPUT);

  pinMode(buttonPin, INPUT);

  // 以實際讀值初始化，避免開機誤判按壓
  lastReading = digitalRead(buttonPin);
  stableButtonState = lastReading;
}

void loop() {
  buttonState = digitalRead(buttonPin);
  unsigned long currentMillis = millis();

  // 按鈕去彈跳
  if (buttonState != lastReading) {
    lastDebounceTime = currentMillis;
  }

  if (currentMillis - lastDebounceTime >= debounceDelay) {
    if (buttonState != stableButtonState) {
      stableButtonState = buttonState;

      // 每次按下只切換一次
      if (stableButtonState == BUTTON_PRESSED) {
        mode = (mode + 1) % 4;

        ledState = true;
        previousMillis = currentMillis;
      }
    }
  }

  lastReading = buttonState;

  if (mode == 0) {
    // 恆亮
    ledState = true;
  } else {
    unsigned long interval;

    if (mode == 1) {
      interval = 1000;  // 慢閃：亮 1 秒、暗 1 秒
    } else if (mode == 2) {
      interval = 500;   // 中速：亮 0.5 秒、暗 0.5 秒
    } else {
      interval = 150;   // 快閃：亮 0.15 秒、暗 0.15 秒
    }

    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      ledState = !ledState;
    }
  }

  // 共陽極：想亮時輸出 LOW，想滅時輸出 HIGH
  digitalWrite(ledPin, ledState ? LOW : HIGH);
}