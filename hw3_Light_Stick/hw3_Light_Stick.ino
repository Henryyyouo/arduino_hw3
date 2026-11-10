// ===== 接腳設定 =====
const byte R_PIN = 9;
const byte G_PIN = 11;
const byte B_PIN = 10;
const byte BUTTON_PIN = 2;

// 共陽極 RGB 設 true；共陰極設 false
const bool COMMON_ANODE = true;

// 按下按鈕時為 LOW
// 若你的模組按下是 HIGH，改成 HIGH
const byte BUTTON_PRESSED = LOW;

// ===== 按鈕時間設定 =====
const unsigned long DEBOUNCE_MS = 30;
const unsigned long LONG_PRESS_MS = 1000;

// ===== 顏色設定 =====
// 順序：紅、橙、黃、綠、藍、紫、粉
const byte colors[][3] = {
  {255,   0,   0},
  {255,  80,   0},
  {255, 255,   0},
  {  0, 255,   0},
  {  0,   0, 255},
  {150,   0, 255},
  {255,  30, 100}
};

const byte COLOR_COUNT = sizeof(colors) / sizeof(colors[0]);
byte colorIndex = 0;

// ===== 模式設定 =====
enum LightMode {
  CONSTANT_MODE,
  BLINK_MODE,
  BREATH_MODE,
  HEARTBEAT_MODE
};

const byte MODE_COUNT = 4;
byte mode = CONSTANT_MODE;
unsigned long modeStartTime = 0;

// ===== 按鈕狀態 =====
byte lastReading;
byte stableButtonState;

unsigned long lastDebounceTime = 0;
unsigned long pressStartTime = 0;

bool pressActive = false;
bool longPressHandled = false;

void setup() {
  // 先設定關燈電位
  digitalWrite(R_PIN, COMMON_ANODE ? HIGH : LOW);
  digitalWrite(G_PIN, COMMON_ANODE ? HIGH : LOW);
  digitalWrite(B_PIN, COMMON_ANODE ? HIGH : LOW);

  pinMode(R_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.begin(9600);

  lastReading = digitalRead(BUTTON_PIN);
  stableButtonState = lastReading;

  modeStartTime = millis();

  Serial.println("Light stick ready!");
  printMode();
}

void loop() {
  unsigned long now = millis();

  checkButton(now);
  updateLight(now);
}

// ===== 偵測按鈕：去彈跳、短按、長按 =====
void checkButton(unsigned long now) {
  byte reading = digitalRead(BUTTON_PIN);

  // 原始訊號變化時，重新計算去彈跳時間
  if (reading != lastReading) {
    lastDebounceTime = now;
    lastReading = reading;
  }

  // 訊號穩定後，才確認按鈕狀態
  if (now - lastDebounceTime >= DEBOUNCE_MS) {
    if (reading != stableButtonState) {
      stableButtonState = reading;

      if (stableButtonState == BUTTON_PRESSED) {
        // 剛按下
        pressStartTime = now;
        pressActive = true;
        longPressHandled = false;
      } else {
        // 剛放開
        if (pressActive && !longPressHandled) {
          if (now - pressStartTime >= LONG_PRESS_MS) {
            nextMode(now);
          } else {
            nextColor();
          }
        }

        pressActive = false;
      }
    }
  }

  // 持續按住達 1 秒：只切換一次模式
  if (pressActive &&
      stableButtonState == BUTTON_PRESSED &&
      reading == BUTTON_PRESSED &&
      !longPressHandled &&
      now - pressStartTime >= LONG_PRESS_MS) {

    nextMode(now);
    longPressHandled = true;
  }
}

// ===== 短按：切換顏色 =====
void nextColor() {
  colorIndex = (colorIndex + 1) % COLOR_COUNT;

  Serial.print("Color: ");
  Serial.println(colorIndex + 1);
}

// ===== 長按：切換模式 =====
void nextMode(unsigned long now) {
  mode = (mode + 1) % MODE_COUNT;
  modeStartTime = now;

  printMode();
}

void printMode() {
  Serial.print("Mode: ");

  switch (mode) {
    case CONSTANT_MODE:
      Serial.println("Constant");
      break;

    case BLINK_MODE:
      Serial.println("Blink");
      break;

    case BREATH_MODE:
      Serial.println("Breathing");
      break;

    case HEARTBEAT_MODE:
      Serial.println("Heartbeat");
      break;
  }
}

// ===== 根據模式更新燈光 =====
void updateLight(unsigned long now) {
  unsigned long elapsed = now - modeStartTime;
  byte brightness = 255;

  switch (mode) {
    case CONSTANT_MODE:
      // 恆亮
      brightness = 255;
      break;

    case BLINK_MODE:
      // 亮 300 毫秒、暗 300 毫秒
      brightness = (elapsed % 600 < 300) ? 255 : 0;
      break;

    case BREATH_MODE: {
      // 一次呼吸共 2 秒：
      // 前 1 秒漸亮，後 1 秒漸暗
      unsigned long phase = elapsed % 2000;

      if (phase < 1000) {
        brightness = phase * 255UL / 1000;
      } else {
        brightness = (2000 - phase) * 255UL / 1000;
      }
      break;
    }

    case HEARTBEAT_MODE: {
      // 每 1 秒出現兩次閃光：
      // 怦怦……怦怦……
      unsigned long phase = elapsed % 1000;

      if (phase < 100) {
        brightness = 255;
      } else if (phase >= 220 && phase < 360) {
        brightness = 255;
      } else {
        brightness = 0;
      }
      break;
    }
  }

  showColor(brightness);
}

// ===== 將目前顏色乘上亮度 =====
void showColor(byte brightness) {
  byte red =
    (unsigned long)colors[colorIndex][0] * brightness / 255;

  byte green =
    (unsigned long)colors[colorIndex][1] * brightness / 255;

  byte blue =
    (unsigned long)colors[colorIndex][2] * brightness / 255;

  writeRGB(red, green, blue);
}

// ===== 輸出 RGB，處理共陽極反相 =====
void writeRGB(byte red, byte green, byte blue) {
  if (COMMON_ANODE) {
    analogWrite(R_PIN, 255 - red);
    analogWrite(G_PIN, 255 - green);
    analogWrite(B_PIN, 255 - blue);
  } else {
    analogWrite(R_PIN, red);
    analogWrite(G_PIN, green);
    analogWrite(B_PIN, blue);
  }
}