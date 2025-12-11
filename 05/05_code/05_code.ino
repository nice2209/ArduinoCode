/*
 * SP/05 - 문자 반복 검증
 * Board: Arduino Uno
 * Connection: D8 ──── D0 (Loopback)
 * 
 * 다양한 문자를 순환하며 송수신 테스트
 * 각 문자의 비트 패턴 분석
 * 
 * Baud Rate: 9600 bps
 */

#define TX_BIT 0  // D8 = PORTB0
#define BAUD_RATE 9600
#define BIT_DELAY (1000000 / BAUD_RATE)

#define TX_HIGH() (PORTB |= (1 << TX_BIT))
#define TX_LOW()  (PORTB &= ~(1 << TX_BIT))

// 테스트할 문자 배열
char testChars[] = {'a', 'A', 'Z', '0', '9', '!', '@', ' ', '\n'};
int numChars = 9;
int charIndex = 0;

unsigned long totalTests = 0;
unsigned long successCount = 0;
unsigned long errorCount = 0;

// 각 문자별 통계
struct CharStats {
  char character;
  int tested;
  int success;
  int errors;
};

CharStats stats[9];

void setup() {
  // D8(Software TX) 출력 설정
  DDRB |= (1 << TX_BIT);
  TX_HIGH();
  
  // Hardware UART 초기화
  Serial.begin(9600);
  
  // 통계 초기화
  for (int i = 0; i < numChars; i++) {
    stats[i].character = testChars[i];
    stats[i].tested = 0;
    stats[i].success = 0;
    stats[i].errors = 0;
  }
  
  // 시작 메시지
  Serial.println("========================================");
  Serial.println("   SP/05 - Character Variation Test");
  Serial.println("========================================");
  Serial.println("Connection: D8 ──── D0 (Loopback)");
  Serial.println("Baud Rate: 9600 bps");
  Serial.println();
  Serial.println("Test Characters:");
  Serial.print("  ");
  for (int i = 0; i < numChars; i++) {
    if (testChars[i] == ' ') {
      Serial.print("[SPACE] ");
    } else if (testChars[i] == '\n') {
      Serial.print("[LF] ");
    } else {
      Serial.print("'");
      Serial.print(testChars[i]);
      Serial.print("' ");
    }
  }
  Serial.println();
  Serial.println("----------------------------------------");
  Serial.println();
  
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

void printBinary(char c) {
  for (int i = 7; i >= 0; i--) {
    Serial.print((c >> i) & 0x01);
  }
}

void printCharName(char c) {
  if (c == ' ') {
    Serial.print("[SPACE]");
  } else if (c == '\n') {
    Serial.print("[LF]   ");
  } else {
    Serial.print("'");
    Serial.print(c);
    Serial.print("'    ");
  }
}

void printDetailedStats() {
  Serial.println();
  Serial.println("========================================");
  Serial.println("        CHARACTER STATISTICS");
  Serial.println("========================================");
  Serial.println("Char | ASCII | Binary   | Tested | OK | Fail | Rate");
  Serial.println("-----|-------|----------|--------|----|----- |------");
  
  for (int i = 0; i < numChars; i++) {
    // Character
    printCharName(stats[i].character);
    Serial.print(" | ");
    
    // ASCII (hex)
    if ((int)stats[i].character < 16) Serial.print("0");
    Serial.print((int)stats[i].character, HEX);
    Serial.print("    | ");
    
    // Binary
    printBinary(stats[i].character);
    Serial.print(" | ");
    
    // Tested
    if (stats[i].tested < 10) Serial.print(" ");
    Serial.print(stats[i].tested);
    Serial.print("     | ");
    
    // Success
    if (stats[i].success < 10) Serial.print(" ");
    Serial.print(stats[i].success);
    Serial.print(" | ");
    
    // Errors
    if (stats[i].errors < 10) Serial.print(" ");
    Serial.print(stats[i].errors);
    Serial.print("   | ");
    
    // Success Rate
    if (stats[i].tested > 0) {
      float rate = (stats[i].success * 100.0) / stats[i].tested;
      if (rate < 100) Serial.print(" ");
      Serial.print(rate, 1);
      Serial.println("%");
    } else {
      Serial.println("  -  ");
    }
  }
  
  Serial.println("========================================");
  Serial.print("TOTAL: ");
  Serial.print(totalTests);
  Serial.print(" tests | Success: ");
  Serial.print(successCount);
  Serial.print(" (");
  Serial.print((successCount * 100.0) / totalTests, 1);
  Serial.print("%) | Errors: ");
  Serial.print(errorCount);
  Serial.print(" (");
  Serial.print((errorCount * 100.0) / totalTests, 1);
  Serial.println("%)");
  Serial.println("========================================");
  Serial.println();
}

void loop() {
  totalTests++;
  char testChar = testChars[charIndex];
  stats[charIndex].tested++;
  
  // 테스트 시작
  Serial.print("[Test #");
  Serial.print(totalTests);
  Serial.print("] Char: ");
  printCharName(testChar);
  Serial.print(" (0x");
  if ((int)testChar < 16) Serial.print("0");
  Serial.print((int)testChar, HEX);
  Serial.print(", ");
  printBinary(testChar);
  Serial.print(") → ");
  
  // 전송
  sendByteBitBang(testChar);
  
  // 수신 대기
  delay(50);
  
  // 수신 확인
  if (Serial.available() > 0) {
    char received = Serial.read();
    
    if (received == testChar) {
      Serial.println("✓ OK");
      successCount++;
      stats[charIndex].success++;
    } else {
      Serial.print("✗ ERROR! Received: '");
      Serial.print(received);
      Serial.print("' (0x");
      Serial.print((int)received, HEX);
      Serial.println(")");
      errorCount++;
      stats[charIndex].errors++;
    }
  } else {
    Serial.println("✗ NO DATA!");
    errorCount++;
    stats[charIndex].errors++;
  }
  
  // 다음 문자로 이동
  charIndex = (charIndex + 1) % numChars;
  
  // 한 사이클 완료 시 통계 출력
  if (charIndex == 0) {
    printDetailedStats();
  }
  
  delay(800);
}

//========================================
//   SP/05 - Character Variation Test
//========================================
//Connection: D8 ──── D0 (Loopback)
//Baud Rate: 9600 bps
//
//Test Characters:
//  'a' 'A' 'Z' '0' '9' '!' '@' [SPACE] [LF] 
//----------------------------------------
//
//[Test #1] Char: 'a'     (0x61, 01100001) → ✓ OK
//[Test #2] Char: 'A'     (0x41, 01000001) → ✓ OK
//[Test #3] Char: 'Z'     (0x5A, 01011010) → ✓ OK
//[Test #4] Char: '0'     (0x30, 00110000) → ✓ OK
//[Test #5] Char: '9'     (0x39, 00111001) → ✓ OK
//[Test #6] Char: '!'     (0x21, 00100001) → ✓ OK
//[Test #7] Char: '@'     (0x40, 01000000) → ✓ OK
//[Test #8] Char: [SPACE] (0x20, 00100000) → ✓ OK
//[Test #9] Char: [LF]    (0x0A, 00001010) → ✓ OK
//
//========================================
//        CHARACTER STATISTICS
//========================================
//Char | ASCII | Binary   | Tested | OK | Fail | Rate
//-----|-------|----------|--------|----|----- |------
//'a'     | 61    | 01100001 |  1     |  1 |  0   | 100.0%
//'A'     | 41    | 01000001 |  1     |  1 |  0   | 100.0%
//'Z'     | 5A    | 01011010 |  1     |  1 |  0   | 100.0%
//'0'     | 30    | 00110000 |  1     |  1 |  0   | 100.0%
//'9'     | 39    | 00111001 |  1     |  1 |  0   | 100.0%
//'!'     | 21    | 00100001 |  1     |  1 |  0   | 100.0%
//'@'     | 40    | 01000000 |  1     |  1 |  0   | 100.0%
//[SPACE] | 20    | 00100000 |  1     |  1 |  0   | 100.0%
//[LF]    | 0A    | 00001010 |  1     |  1 |  0   | 100.0%
//========================================
//TOTAL: 9 tests | Success: 9 (100.0%) | Errors: 0 (0.0%)
//========================================
//
//[Test #10] Char: 'a'     (0x61, 01100001) → ✓ OK
//[Test #11] Char: 'A'     (0x41, 01000001) → ✓ OK
//[Test #12] Char: 'Z'     (0x5A, 01011010) → ✓ OK
//[Test #13] Char: '0'     (0x30, 00110000) → ✓ OK
//[Test #14] Char: '9'     (0x39, 00111001) → ✓ OK
//[Test #15] Char: '!'     (0x21, 00100001) → ✓ OK
//[Test #16] Char: '@'     (0x40, 01000000) → ✓ OK
//[Test #17] Char: [SPACE] (0x20, 00100000) → ✓ OK
//[Test #18] Char: [LF]    (0x0A, 00001010) → ✓ OK
//
//========================================
//        CHARACTER STATISTICS
//========================================
//Char | ASCII | Binary   | Tested | OK | Fail | Rate
//-----|-------|----------|--------|----|----- |------
//'a'     | 61    | 01100001 |  2     |  2 |  0   | 100.0%
//'A'     | 41    | 01000001 |  2     |  2 |  0   | 100.0%
//'Z'     | 5A    | 01011010 |  2     |  2 |  0   | 100.0%
//'0'     | 30    | 00110000 |  2     |  2 |  0   | 100.0%
//'9'     | 39    | 00111001 |  2     |  2 |  0   | 100.0%
//'!'     | 21    | 00100001 |  2     |  2 |  0   | 100.0%
//'@'     | 40    | 01000000 |  2     |  2 |  0   | 100.0%
//[SPACE] | 20    | 00100000 |  2     |  2 |  0   | 100.0%
//[LF]    | 0A    | 00001010 |  2     |  2 |  0   | 100.0%
//========================================
//TOTAL: 18 tests | Success: 18 (100.0%) | Errors: 0 (0.0%)
//========================================
//
//[Test #19] Char: 'a'     (0x61, 01100001) → ✓ OK
//[Test #20] Char: 'A'     (0x41, 01000001) → ✓ OK
//[Test #21] Char: 'Z'     (0x5A, 01011010) → ✓ OK
//[Test #22] Char: '0'     (0x30, 00110000) → ✓ OK
//[Test #23] Char: '9'     (0x39, 00111001) → ✓ OK
//[Test #24] Char: '!'     (0x21, 00100001) → ✓ OK
//[Test #25] Char: '@'     (0x40, 01000000) → ✓ OK
//[Test #26] Char: [SPACE] (0x20, 00100000) → ✓ OK
//[Test #27] Char: [LF]    (0x0A, 00001010) → ✓ OK
//
//========================================
//        CHARACTER STATISTICS
//========================================
//Char | ASCII | Binary   | Tested | OK | Fail | Rate
//-----|-------|----------|--------|----|----- |------
//'a'     | 61    | 01100001 |  3     |  3 |  0   | 100.0%
//'A'     | 41    | 01000001 |  3     |  3 |  0   | 100.0%
//'Z'     | 5A    | 01011010 |  3     |  3 |  0   | 100.0%
//'0'     | 30    | 00110000 |  3     |  3 |  0   | 100.0%
//'9'     | 39    | 00111001 |  3     |  3 |  0   | 100.0%
//'!'     | 21    | 00100001 |  3     |  3 |  0   | 100.0%
//'@'     | 40    | 01000000 |  3     |  3 |  0   | 100.0%
//[SPACE] | 20    | 00100000 |  3     |  3 |  0   | 100.0%
//[LF]    | 0A    | 00001010 |  3     |  3 |  0   | 100.0%
//========================================
//TOTAL: 27 tests | Success: 27 (100.0%) | Errors: 0 (0.0%)
//========================================
