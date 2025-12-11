/*
 * SP/07 - High-Speed UART Loopback Test
 * Board: Arduino Uno (ATmega328P 16MHz)
 *
 * 하드웨어 UART 루프백:
 *   D1(TX) ──── D0(RX)
 *
 * 테스트 속도:
 *   230400, 460800, 921600, 1000000, 2000000 bps
 *
 * 시리얼 모니터:
 *   9600 bps
 */

const long baudRates[] = {230400, 460800, 921600, 1000000, 2000000};
const int numRates = 5;

const char testChars[] = {'a', 'A', '0', '9', '!'};
const int numTestChars = 5;
const int repeatsPerChar = 3;       // 각 문자 3번씩 → 총 15회

// 최소 통계만 저장해서 RAM 절약
struct BaudStats {
  long baud;
  uint8_t totalTests;   // 보통 15
  uint8_t success;      // 성공 횟수
};

BaudStats stats[numRates];
int currentRateIndex = 0;

void setup() {
  Serial.begin(9600);
  delay(200);

  Serial.println("########################################");
  Serial.println("#  SP/07 - High-Speed UART Loopback   #");
  Serial.println("########################################");
  Serial.println("Board : Arduino Uno (ATmega328P 16MHz)");
  Serial.println("Method: Hardware UART (D0/D1)");
  Serial.println("Loopback: D1(TX) ──── D0(RX)");
  Serial.println();
  Serial.println("IMPORTANT: Connect jumper wire!");
  Serial.println("  D1 (TX) ──── D0 (RX)");
  Serial.println();
  Serial.println("Serial Monitor: 9600 baud");
  Serial.println();
  Serial.println("Testing Baud Rates:");
  for (int i = 0; i < numRates; i++) {
    Serial.print("  ");
    Serial.print(baudRates[i]);
    Serial.print(" bps  (bit width ≈ ");
    Serial.print(1000000.0 / baudRates[i], 2);
    Serial.println(" us)");
  }
  Serial.println("########################################");
  Serial.println("Starting in 5 seconds...");
  Serial.println("(Prepare oscilloscope if needed)");
  Serial.println();

  delay(5000);
}

void runBaudTest(int index) {
  const long baud = baudRates[index];
  const uint8_t maxTests = numTestChars * repeatsPerChar; // 15

  // per-character 로그 (지역 변수 → 스택에 잠깐만 있음)
  char sentLog[maxTests];
  char recvLog[maxTests];
  bool gotLog[maxTests];
  bool okLog[maxTests];
  uint8_t logIndex = 0;

  // 안내 출력 (9600)
  Serial.println("========================================");
  Serial.print("TEST #");
  Serial.print(index + 1);
  Serial.print(" : ");
  Serial.print(baud);
  Serial.println(" bps");
  Serial.println("========================================");
  Serial.print("Theoretical bit width: ");
  Serial.print(1000000.0 / baud, 2);
  Serial.println(" us");
  Serial.println();

  // ---- 테스트용 보드레이트로 전환 ----
  Serial.flush();
  Serial.end();
  delay(150);               // UART 종료 안정화
  Serial.begin(baud);
  delay(250);               // 새 Baud로 안정화

  // RX 버퍼 깨끗하게 비우기
  for (int i = 0; i < 40; i++) {
    while (Serial.available()) Serial.read();
    delayMicroseconds(200);
  }

  uint8_t success = 0;
  uint8_t errors = 0;
  uint8_t noData = 0;

  // ------- 실제 송/수신 테스트 -------
  for (int r = 0; r < repeatsPerChar; r++) {
    for (int i = 0; i < numTestChars; i++) {
      char tx = testChars[i];

      if (logIndex < maxTests) {
        sentLog[logIndex] = tx;
        recvLog[logIndex] = 0;
        gotLog[logIndex]  = false;
        okLog[logIndex]   = false;
      }

      Serial.write(tx);
      Serial.flush();

      unsigned long timeout = millis() + 100; // 100ms 대기
      while (Serial.available() == 0 && millis() < timeout) {
        delayMicroseconds(10);
      }

      if (Serial.available() > 0) {
        char rx = Serial.read();
        if (rx == tx) {
          success++;
          if (logIndex < maxTests) {
            recvLog[logIndex] = rx;
            gotLog[logIndex] = true;
            okLog[logIndex] = true;
          }
        } else {
          errors++;
          if (logIndex < maxTests) {
            recvLog[logIndex] = rx;
            gotLog[logIndex] = true;
            okLog[logIndex] = false;
          }
        }
      } else {
        errors++;
        noData++;
        // gotLog=false, okLog=false 그대로 유지
      }

      if (logIndex < maxTests) logIndex++;
      delay(10);
    }
  }

  // ---- 통계 저장 ----
  stats[index].baud       = baud;
  stats[index].totalTests = success + errors;
  stats[index].success    = success;

  // ---- 결과 출력을 위해 다시 9600으로 전환 ----
  Serial.flush();
  Serial.end();
  delay(150);
  Serial.begin(9600);
  delay(250);

  // ---- 요약 출력 ----
  Serial.println("----------------------------------------");
  Serial.print("Baud        : ");
  Serial.print(baud);
  Serial.println(" bps");
  Serial.print("Total Tests : ");
  Serial.println((int)(success + errors));
  Serial.print("Success     : ");
  Serial.print((int)success);
  Serial.print(" (");
  Serial.print((success * 100.0) / (success + errors), 1);
  Serial.println(" %)");
  Serial.print("Errors      : ");
  Serial.print((int)errors);
  Serial.print(" (");
  Serial.print((errors * 100.0) / (success + errors), 1);
  Serial.println(" %)");
  Serial.print("No Data     : ");
  Serial.println((int)noData);
  Serial.println();

  // ---- 문자별 로그 출력 ----
  Serial.println("Per-character log:");
  for (uint8_t i = 0; i < logIndex; i++) {
    Serial.print("  #");
    if (i + 1 < 10) Serial.print("0");
    Serial.print(i + 1);
    Serial.print("  Sent='");
    Serial.print(sentLog[i]);
    Serial.print("'  ");

    if (!gotLog[i]) {
      Serial.println("Received=<NO DATA>  -> ✗");
    } else {
      Serial.print("Received='");
      Serial.print(recvLog[i]);
      Serial.print("'  -> ");
      Serial.println(okLog[i] ? "✓" : "✗");
    }
  }
  Serial.println();

  // 상태 평가
  if (success == stats[index].totalTests) {
    Serial.println("Status: ✓ EXCELLENT - Fully supported");
  } else if (success >= stats[index].totalTests * 0.8) {
    Serial.println("Status: ⚠ GOOD - Minor errors");
  } else if (success > 0) {
    Serial.println("Status: ✗ POOR - Mostly failing");
  } else {
    Serial.println("Status: ✗ FAILED - No reliable RX");
  }

  Serial.println("========================================");
  Serial.println();
  delay(2000);
}

void printSummary() {
  Serial.println();
  Serial.println("########################################");
  Serial.println("#        HIGH-SPEED TEST SUMMARY       #");
  Serial.println("########################################");
  Serial.println();
  Serial.println("Baud Rate | BitWidth | Tests | Success | Errors | Rate   | Status");
  Serial.println("----------|----------|-------|---------|--------|--------|--------");

  long maxStableBaud = 0;

  for (int i = 0; i < numRates; i++) {
    long baud = stats[i].baud;
    uint8_t total = stats[i].totalTests;
    uint8_t success = stats[i].success;
    uint8_t errors = total - success;
    float bitWidth = 1000000.0 / baud;
    float rate = (total > 0) ? (success * 100.0 / total) : 0.0;

    // Baud 출력 정렬
    if (baud < 1000000) Serial.print(" ");
    Serial.print(baud);
    Serial.print(" | ");

    // Bit width
    if (bitWidth < 10) Serial.print(" ");
    Serial.print(bitWidth, 2);
    Serial.print("us | ");

    // Tests
    if (total < 10) Serial.print(" ");
    Serial.print((int)total);
    Serial.print("     | ");

    // Success
    if (success < 10) Serial.print(" ");
    Serial.print((int)success);
    Serial.print("       | ");

    // Errors
    if (errors < 10) Serial.print(" ");
    Serial.print((int)errors);
    Serial.print("      | ");

    // Rate
    if (rate < 10) Serial.print("  ");
    else if (rate < 100) Serial.print(" ");
    Serial.print(rate, 1);
    Serial.print("% | ");

    if (rate == 100.0) {
      Serial.println("✓ OK");
      maxStableBaud = baud; // 마지막 100% 속도
    } else if (rate >= 80.0) {
      Serial.println("⚠ WARN");
    } else {
      Serial.println("✗ FAIL");
    }
  }

  Serial.println("########################################");
  Serial.print("Maximum Stable Baud (100%): ");
  if (maxStableBaud > 0) {
    Serial.print(maxStableBaud);
    Serial.println(" bps");
  } else {
    Serial.println("< 230400 bps");
  }
  Serial.println("########################################");
}

void loop() {
  if (currentRateIndex < numRates) {
    runBaudTest(currentRateIndex);
    currentRateIndex++;
  } else {
    printSummary();
    // 끝나면 그대로 대기
    while (true) {
      delay(1000);
    }
  }
}
