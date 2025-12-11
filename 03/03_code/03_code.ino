/*
 * SP/03 - 2채널 동시 관찰 (동기화 버전)
 * Board: Arduino Uno
 * Oscilloscope: 
 *   CH1 (Yellow) → D8 (Software TX)
 *   CH2 (Blue)   → D1 (Hardware TX)
 * Baud Rate: 9600 bps
 * 
 * 주의: Serial Monitor를 열지 마세요! (파형 관찰 중)
 */

#define TX_BIT 0  // D8 = PORTB0
#define BAUD_RATE 9600
#define BIT_DELAY (1000000 / BAUD_RATE)

#define TX_HIGH() (PORTB |= (1 << TX_BIT))
#define TX_LOW()  (PORTB &= ~(1 << TX_BIT))

void setup() {
  // D8(Software TX) 출력 설정
  DDRB |= (1 << TX_BIT);
  TX_HIGH();
  
  // Hardware UART 초기화
  Serial.begin(9600);
  
  delay(2000);
}

void sendByteBitBang(char c) {
  // Start bit
  TX_LOW();
  delayMicroseconds(BIT_DELAY);
  
  // Data bits (LSB first)
  for (int i = 0; i < 8; i++) {
    if ((c >> i) & 0x01) {
      TX_HIGH();
    } else {
      TX_LOW();
    }
    delayMicroseconds(BIT_DELAY);
  }
  
  // Stop bit
  TX_HIGH();
  delayMicroseconds(BIT_DELAY);
}

void loop() {
  // ★ 핵심: 두 신호를 거의 동시에 전송
  
  // 1. D8으로 'a' 전송 (Software UART)
  sendByteBitBang('a');
  
  // 2. D1으로 'a' 전송 (Hardware UART)
  Serial.write('a');
  
  // 오실로스코프 Single 모드로 잡기 위한 긴 대기
  delay(2000);  // 2초 간격
}
