/*
 * Improved 38400bps Loopback Test
 * TX/RX 검증 분리 + 수신 노이즈 검출 추가
 */

const long TEST_BAUD = 38400;
const char TEST_CHAR = 'X';
const int NUM_TESTS = 10;

int success = 0, wrongChar = 0, noData = 0, unexpectedData = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("=== 38400bps TX/RX Integrity Test ===");
  Serial.println("Connect:  D1(TX) ─── D0(RX)");
  Serial.println("Monitor: 9600 baud");
  delay(1500);

  Serial.println("\n[Switching to 38400bps]\n");
  Serial.end();
  delay(100);
  Serial.begin(TEST_BAUD);
  delay(100);

  while (Serial.available()) Serial.read(); // 초기 수신 버퍼 정리

  for (int i=0; i<NUM_TESTS; i++) {

    Serial.write(TEST_CHAR);  // 송신
    Serial.flush();

    unsigned long timeout = millis()+100;
    while (Serial.available()==0 && millis()<timeout);

    if (Serial.available()>0){
      char r = Serial.read();

      if(r == TEST_CHAR)
        success++;
      else
        wrongChar++;  // 수신됐지만 내용 다름 (RX 검증 FAIL)

    } else {
      noData++; // 응답 없음 (TX 또는 RX 문제)
    }

    // 🔎 수신버퍼에 찌꺼기 남아있으면 "예상 외 입력"으로 카운트
    while (Serial.available()) {
      Serial.read();
      unexpectedData++;
    }

    delay(10);
  }

  // 결과출력용 다시 9600bps로 전환
  Serial.end(); delay(100);
  Serial.begin(9600); delay(100);

  Serial.println("\n===== RESULT =====");
  Serial.print("Sent/Expected : '"); Serial.print(TEST_CHAR); Serial.println("'");
  Serial.print("Correct RX    : "); Serial.println(success);
  Serial.print("Wrong Char RX : "); Serial.println(wrongChar);
  Serial.print("No Response   : "); Serial.println(noData);
  Serial.print("Unexpected RX : "); Serial.println(unexpectedData);
  Serial.println("==================");
}
void loop(){}
