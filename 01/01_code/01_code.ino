/*
 * SP/01 - 임의 문자 파형 관찰
 * Board: Arduino Uno
 * Oscilloscope: MSO2012B CH1 → D8
 * Baud Rate: 9600 bps
 * Character: 'a' (0x61, 0b01100001)
 */

#define TX_PIN 8
#define BAUD_RATE 9600
#define BIT_DELAY (1000000 / BAUD_RATE) // 약 104µs

unsigned long testCount = 0;

void setup() {
  pinMode(TX_PIN, OUTPUT);
  digitalWrite(TX_PIN, HIGH); // UART idle state는 HIGH
  
  Serial.begin(9600);
  Serial.println("=== SP/01 UART Waveform Test ===");
  Serial.println("Oscilloscope: Connect CH1 to D8");
  Serial.print("Baud Rate: ");
  Serial.println(BAUD_RATE);
  Serial.print("Bit Width: ");
  Serial.print(BIT_DELAY);
  Serial.println(" us");
  Serial.println("Sending 'a' (0x61)...\n");
  
  delay(2000); // 오실로스코프 준비 시간
}

void sendByte(char c) {
  // Start bit (LOW)
  digitalWrite(TX_PIN, LOW);
  delayMicroseconds(BIT_DELAY);
  
  // Data bits 8개 (LSB first)
  // 'a' = 0x61 = 0b01100001
  // 순서: 1-0-0-0-0-1-1-0
  for (int i = 0; i < 8; i++) {
    digitalWrite(TX_PIN, (c >> i) & 0x01);
    delayMicroseconds(BIT_DELAY);
  }
  
  // Stop bit (HIGH)
  digitalWrite(TX_PIN, HIGH);
  delayMicroseconds(BIT_DELAY);
}

void loop() {
  testCount++;
  
  Serial.print("Test #");
  Serial.print(testCount);
  Serial.println(": Sending 'a'");
  
  sendByte('a');
  
  delay(500); // 0.5초 간격으로 반복 (관찰하기 좋은 속도)
}

/*
 * ========================================
 * 하드웨어 연결 가이드 (SP/01)
 * ========================================
 *
 * [필요한 장비]
 * - Arduino Uno 보드
 * - 오실로스코프 (MSO2012B 또는 유사 모델)
 * - 점퍼선 또는 프로브
 * - USB 케이블 (Arduino ↔ PC)
 *
 * [연결 방법]
 * 1. Arduino Uno를 USB로 PC에 연결
 * 2. 오실로스코프 CH1(Yellow) → Arduino D8 핀
 * 3. 오실로스코프 GND → Arduino GND 핀
 *
 * [오실로스코프 설정]
 * - CH1: 5V/div
 * - Time Base: 200µs/div (또는 500µs/div)
 * - Trigger: CH1, Edge, Rising, 2.5V
 * - Trigger Mode: Normal 또는 Single
 *
 * [관찰할 내용]
 * - Start Bit: LOW (0V)
 * - Data Bits: 'a' = 0x61 = 0b01100001
 *   LSB first이므로: 1-0-0-0-0-1-1-0 순서
 * - Stop Bit: HIGH (5V)
 * - 각 비트 폭: 약 104µs (9600 bps)
 *
 * [측정 항목]
 * - Measure → Width → 비트 하나의 시간 폭
 * - 이론값: 1/9600 = 104.17µs
 * - 오차율 계산: ((실측값 - 이론값) / 이론값) × 100%
 *
 * [주의사항]
 * ⚠️ Serial Monitor를 열어도 됩니다 (상태 확인용)
 * ⚠️ D8 핀의 신호만 관찰합니다
 * ⚠️ 오실로스코프 프로브는 10X로 설정
 */
