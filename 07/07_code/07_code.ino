/*
 * SP/07 - 고속 한계점 탐색
 * Board: Arduino Uno
 * 
 * 하드웨어 UART를 사용한 고속 통신 테스트
 * 루프백: D1(TX) ──── D0(RX)
 * 
 * Baud Rates: 230400, 460800, 921600, 1000000, 2000000
 */

// 테스트할 고속 보드레이트
long baudRates[] = {230400, 460800, 921600, 1000000, 2000000};
int numRates = 5;

// 테스트 문자
char testChars[] = {'a', 'A', '0', '9', '!'};
int numTestChars = 5;

// 통계 구조체
struct BaudStats {
  long baudRate;
  float bitWidth;
  int totalTests;
  int successCount;
  int errorCount;
  float successRate;
  bool hardwareSupported;
};

BaudStats stats[5];
int currentRateIndex = 0;

void setup() {
  // 초기 설정 (9600 bps로 시작)
  Serial.begin(9600);
  
  Serial.println("########################################");
  Serial.println("#  SP/07 - High-Speed Limit Test      #");
  Serial.println("########################################");
  Serial.println("Method: Hardware UART (D0/D1)");
  Serial.println("Loopback: D1(TX) ──── D0(RX)");
  Serial.println();
  Serial.println("IMPORTANT: Connect jumper wire!");
  Serial.println("  D1 ──── D0");
  Serial.println();
  Serial.println("Testing Baud Rates:");
  for (int i = 0; i < numRates; i++) {
    Serial.print("  ");
    Serial.print(baudRates[i]);
    Serial.print(" bps (bit width: ");
    Serial.print(1000000.0 / baudRates[i], 2);
    Serial.println(" us)");
  }
  Serial.println("########################################");
  Serial.println();
  Serial.println("Starting in 5 seconds...");
  Serial.println("(Prepare oscilloscope for high-speed capture)");
  
  delay(5000);
  
  Serial.println("\nStarting tests...\n");
  delay(1000);
}

void testBaudRate(long baudRate, int testIndex) {
  // ==== (1) per-character 로그용 배열 추가 ====
  const int maxTests = 15;  // 5글자 x 3회 = 15
  char sentLog[maxTests];
  char recvLog[maxTests];
  bool gotDataLog[maxTests];
  bool okLog[maxTests];
  int logIndex = 0;
  // =========================================

  // 결과 출력용 9600 bps
  Serial.end();
  delay(100);
  Serial.begin(9600);
  delay(100);
  
  Serial.println("========================================");
  Serial.print("TEST #");
  Serial.print(testIndex + 1);
  Serial.print(": ");
  Serial.print(baudRate);
  Serial.println(" bps");
  Serial.println("========================================");
  
  float bitWidth = 1000000.0 / baudRate;
  
  Serial.print("Bit Width (Theory): ");
  Serial.print(bitWidth, 2);
  Serial.println(" us");
  Serial.println();
  
  // 테스트 보드레이트로 변경
  Serial.end();
  delay(200);
  Serial.begin(baudRate);
  delay(200);
  
  // 버퍼 비우기
  while (Serial.available() > 0) {
    Serial.read();
  }
  
  int testSuccess = 0;
  int testErrors = 0;
  int noDataCount = 0;
  
  // 각 문자를 3번씩 테스트 (총 15회)
  for (int repeat = 0; repeat < 3; repeat++) {
    for (int i = 0; i < numTestChars; i++) {
      char testChar = testChars[i];
      
      // 전송
      Serial.write(testChar);
      Serial.flush();  // 전송 완료 대기
      
      // 수신 대기
      unsigned long timeout = millis() + 100;
      while (Serial.available() == 0 && millis() < timeout) {
        delayMicroseconds(10);
      }

      // 기본 로그 값 세팅
      if (logIndex < maxTests) {
        sentLog[logIndex] = testChar;
        recvLog[logIndex] = 0;
        gotDataLog[logIndex] = false;
        okLog[logIndex] = false;
      }
      
      // 수신 확인
      if (Serial.available() > 0) {
        char received = Serial.read();
        if (received == testChar) {
          testSuccess++;
          if (logIndex < maxTests) {
            recvLog[logIndex] = received;
            gotDataLog[logIndex] = true;
            okLog[logIndex] = true;
          }
        } else {
          testErrors++;
          if (logIndex < maxTests) {
            recvLog[logIndex] = received;
            gotDataLog[logIndex] = true;
            okLog[logIndex] = false;
          }
        }
      } else {
        testErrors++;
        noDataCount++;
        // 이미 gotDataLog=false, okLog=false 로 세팅됨
      }

      if (logIndex < maxTests) logIndex++;
      delay(10);  // 다음 문자 전 대기
    }
  }
  
  // 통계 저장
  stats[testIndex].baudRate = baudRate;
  stats[testIndex].bitWidth = bitWidth;
  stats[testIndex].totalTests = testSuccess + testErrors;
  stats[testIndex].successCount = testSuccess;
  stats[testIndex].errorCount = testErrors;
  stats[testIndex].successRate = (testSuccess * 100.0) / (testSuccess + testErrors);
  stats[testIndex].hardwareSupported = (testSuccess > 0);
  
  // 결과 출력을 위해 9600으로 변경
  Serial.end();
  delay(100);
  Serial.begin(9600);
  delay(100);
  
  // ---- 결과 요약 출력 ----
  Serial.println("----------------------------------------");
  Serial.println("Results:");
  Serial.print("  Total Tests: ");
  Serial.println(testSuccess + testErrors);
  Serial.print("  Success: ");
  Serial.print(testSuccess);
  Serial.print(" (");
  Serial.print((testSuccess * 100.0) / (testSuccess + testErrors), 1);
  Serial.println("%)");
  Serial.print("  Errors: ");
  Serial.print(testErrors);
  Serial.print(" (");
  Serial.print((testErrors * 100.0) / (testSuccess + testErrors), 1);
  Serial.println("%)");
  Serial.print("  No Data: ");
  Serial.println(noDataCount);

  // ---- 여기서 문자별 로그 출력 ----
  Serial.println();
  Serial.println("Per-character log:");
  for (int i = 0; i < logIndex; i++) {
    Serial.print("  #");
    if (i + 1 < 10) Serial.print("0");
    Serial.print(i + 1);
    Serial.print("  Sent='");
    Serial.print(sentLog[i]);
    Serial.print("'  ");

    if (!gotDataLog[i]) {
      Serial.println("Received=<NO DATA>  -> ✗");
    } else {
      Serial.print("Received='");
      Serial.print(recvLog[i]);
      Serial.print("'  -> ");
      Serial.println(okLog[i] ? "✓" : "✗");
    }
  }
  Serial.println();

  // 평가
  if (testSuccess == 15) {
    Serial.println("  Status: ✓ EXCELLENT - Fully supported");
  } else if (testSuccess >= 12) {
    Serial.println("  Status: ⚠ GOOD - Minor errors");
  } else if (testSuccess >= 8) {
    Serial.println("  Status: ⚠ MARGINAL - Significant errors");
  } else if (testSuccess > 0) {
    Serial.println("  Status: ✗ POOR - Mostly failing");
  } else {
    Serial.println("  Status: ✗ FAILED - Not supported");
  }
  
  Serial.println("========================================");
  Serial.println();
  
  delay(2000);
}

void printFinalSummary() {
  Serial.println("\n\n");
  Serial.println("########################################");
  Serial.println("#      HIGH-SPEED TEST SUMMARY        #");
  Serial.println("########################################");
  Serial.println();
  Serial.println("Baud Rate  | Bit Width | Tests | Success | Errors | Rate   | Status");
  Serial.println("-----------|-----------|-------|---------|--------|--------|--------");
  
  for (int i = 0; i < numRates; i++) {
    // Baud Rate
    if (baudRates[i] < 1000000) Serial.print(" ");
    Serial.print(baudRates[i]);
    Serial.print(" | ");
    
    // Bit Width
    float bitWidth = 1000000.0 / baudRates[i];
    if (bitWidth >= 10) Serial.print(" ");
    if (bitWidth >= 100) Serial.print(" ");
    Serial.print(bitWidth, 2);
    Serial.print(" us | ");
    
    // Tests
    if (stats[i].totalTests < 10) Serial.print(" ");
    Serial.print(stats[i].totalTests);
    Serial.print("    | ");
    
    // Success
    if (stats[i].successCount < 10) Serial.print(" ");
    Serial.print(stats[i].successCount);
    Serial.print("      | ");
    
    // Errors
    if (stats[i].errorCount < 10) Serial.print(" ");
    Serial.print(stats[i].errorCount);
    Serial.print("     | ");
    
    // Rate
    if (stats[i].successRate < 10) Serial.print("  ");
    else if (stats[i].successRate < 100) Serial.print(" ");
    Serial.print(stats[i].successRate, 1);
    Serial.print("% | ");
    
    // Status
    if (stats[i].successRate == 100.0) {
      Serial.println("✓ OK");
    } else if (stats[i].successRate >= 80.0) {
      Serial.println("⚠ WARN");
    } else {
      Serial.println("✗ FAIL");
    }
  }
  
  Serial.println("########################################");
  Serial.println();
  
  // 최대 안정 속도 찾기
  long maxStableBaud = 0;
  for (int i = 0; i < numRates; i++) {
    if (stats[i].successRate == 100.0) {
      maxStableBaud = stats[i].baudRate;
    }
  }
  
  Serial.print("Maximum Stable Baud Rate: ");
  if (maxStableBaud > 0) {
    Serial.print(maxStableBaud);
    Serial.println(" bps");
  } else {
    Serial.println("< 230400 bps");
  }
  Serial.println();
  
  // CSV 데이터
  Serial.println("CSV Format (for data analysis):");
  Serial.println("baud_rate,bit_width_us,total_tests,success,errors,success_rate,supported");
  
  for (int i = 0; i < numRates; i++) {
    Serial.print(stats[i].baudRate);
    Serial.print(",");
    Serial.print(stats[i].bitWidth, 2);
    Serial.print(",");
    Serial.print(stats[i].totalTests);
    Serial.print(",");
    Serial.print(stats[i].successCount);
    Serial.print(",");
    Serial.print(stats[i].errorCount);
    Serial.print(",");
    Serial.print(stats[i].successRate, 2);
    Serial.print(",");
    Serial.println(stats[i].hardwareSupported ? "YES" : "NO");
  }
  
  Serial.println("\n########################################");
  Serial.println("Test Complete!");
  Serial.println("Check oscilloscope for waveform quality");
  Serial.println("########################################");
}

void loop() {
  if (currentRateIndex < numRates) {
    testBaudRate(baudRates[currentRateIndex], currentRateIndex);
    currentRateIndex++;
  } else {
    // 모든 테스트 완료
    printFinalSummary();
    
    // 무한 대기
    while (true) {
      delay(1000);
    }
  }
}

//########################################
//#  SP/07 - High-Speed Limit Test      #
//########################################
//Method: Hardware UART (D0/D1)
//Loopback: D1(TX) ──── D0(RX)
//
//IMPORTANT: Connect jumper wire!
//  D1 ──── D0
//
//Testing Baud Rates:
//  230400 bps (bit width: 4.34 us)
//  460800 bps (bit width: 2.17 us)
//  921600 bps (bit width: 1.09 us)
//  1000000 bps (bit width: 1.00 us)
//  2000000 bps (bit width: 0.50 us)
//########################################
//
//Starting in 5 seconds...
//(Prepare oscilloscope for high-speed capture)
//
//Starting tests...
//
//========================================
//TEST #1: 230400 bps
//========================================
//Bit Width (Theory): 4.34 us
//
//----------------------------------------
//Results:
//  Total Tests: 15
//  Success: 15 (100.0%)
//  Errors: 0 (0.0%)
//  No Data: 0
//  Status: ✓ EXCELLENT - Fully supported
//========================================
//
//========================================
//TEST #2: 460800 bps
//========================================
//Bit Width (Theory): 2.17 us
//
//----------------------------------------
//Results:
//  Total Tests: 15
//  Success: 15 (100.0%)
//  Errors: 0 (0.0%)
//  No Data: 0
//  Status: ✓ EXCELLENT - Fully supported
//========================================
//
//========================================
//TEST #3: 921600 bps
//========================================
//Bit Width (Theory): 1.09 us
//
//----------------------------------------
//Results:
//  Total Tests: 15
//  Success: 15 (100.0%)
//  Errors: 0 (0.0%)
//  No Data: 0
//  Status: ✓ EXCELLENT - Fully supported
//========================================
//
//========================================
//TEST #4: 1000000 bps
//========================================
//Bit Width (Theory): 1.00 us
//
//----------------------------------------
//Results:
//  Total Tests: 15
//  Success: 15 (100.0%)
//  Errors: 0 (0.0%)
//  No Data: 0
//  Status: ✓ EXCELLENT - Fully supported
//========================================
//
//========================================
//TEST #5: 2000000 bps
//========================================
//Bit Width (Theory): 0.50 us
//
//----------------------------------------
//Results:
//  Total Tests: 15
//  Success: 15 (100.0%)
//  Errors: 0 (0.0%)
//  No Data: 0
//  Status: ✓ EXCELLENT - Fully supported
//========================================
//
//
//
//
//########################################
//#      HIGH-SPEED TEST SUMMARY        #
//########################################
//
//Baud Rate  | Bit Width | Tests | Success | Errors | Rate   | Status
//-----------|-----------|-------|---------|--------|--------|--------
// 230400 | 4.34 us | 15    | 15      |  0     | 100.0% | ✓ OK
// 460800 | 2.17 us | 15    | 15      |  0     | 100.0% | ✓ OK
// 921600 | 1.09 us | 15    | 15      |  0     | 100.0% | ✓ OK
//1000000 | 1.00 us | 15    | 15      |  0     | 100.0% | ✓ OK
//2000000 | 0.50 us | 15    | 15      |  0     | 100.0% | ✓ OK
//########################################
//
//Maximum Stable Baud Rate: 2000000 bps
//
//CSV Format (for data analysis):
//baud_rate,bit_width_us,total_tests,success,errors,success_rate,supported
//230400,4.34,15,15,0,100.00,YES
//460800,2.17,15,15,0,100.00,YES
//921600,1.09,15,15,0,100.00,YES
//1000000,1.00,15,15,0,100.00,YES
//2000000,0.50,15,15,0,100.00,YES
//
//########################################
//Test Complete!
//Check oscilloscope for waveform quality
//########################################
//



//v2
//########################################
//#  SP/07 - High-Speed Limit Test      #
//########################################
//Method: Hardware UART (D0/D1)
//Loopback: D1(TX) ──── D0(RX)
//
//IMPORTANT: Connect jumper wire!
//  D1 ──── D0
//
//Testing Baud Rates:
//  230400 bps (bit width: 4.34 us)
//  460800 bps (bit width: 2.17 us)
//  921600 bps (bit width: 1.09 us)
//  1000000 bps (bit width: 1.00 us)
//  2000000 bps (bit width: 0.50 us)
//########################################
//
//Starting in 5 seconds...
//(Prepare oscilloscope for high-speed capture)
//
//Starting tests...
//
//========================================
//TEST #1: 230400 bps
//========================================
//Bit Width (Theory): 4.34 us
//
//----------------------------------------
//Results:
//  Total Tests: 15
//  Success: 15 (100.0%)
//  Errors: 0 (0.0%)
//  No Data: 0
//
//Per-character log:
//  #01  Sent='a'  Received='a'  -> ✓
//  #02  Sent='A'  Received='A'  -> ✓
//  #03  Sent='0'  Received='0'  -> ✓
//  #04  Sent='9'  Received='9'  -> ✓
//  #05  Sent='!'  Received='!'  -> ✓
//  #06  Sent='a'  Received='a'  -> ✓
//  #07  Sent='A'  Received='A'  -> ✓
//  #08  Sent='0'  Received='0'  -> ✓
//  #09  Sent='9'  Received='9'  -> ✓
//  #10  Sent='!'  Received='!'  -> ✓
//  #11  Sent='a'  Received='a'  -> ✓
//  #12  Sent='A'  Received='A'  -> ✓
//  #13  Sent='0'  Received='0'  -> ✓
//  #14  Sent='9'  Received='9'  -> ✓
//  #15  Sent='!'  Received='!'  -> ✓
//
//  Status: ✓ EXCELLENT - Fully supported
//========================================
//
//========================================
//TEST #2: 460800 bps
//========================================
//Bit Width (Theory): 2.17 us
//
//----------------------------------------
//Results:
//  Total Tests: 15
//  Success: 15 (100.0%)
//  Errors: 0 (0.0%)
//  No Data: 0
//
//Per-character log:
//  #01  Sent='a'  Received='a'  -> ✓
//  #02  Sent='A'  Received='A'  -> ✓
//  #03  Sent='0'  Received='0'  -> ✓
//  #04  Sent='9'  Received='9'  -> ✓
//  #05  Sent='!'  Received='!'  -> ✓
//  #06  Sent='a'  Received='a'  -> ✓
//  #07  Sent='A'  Received='A'  -> ✓
//  #08  Sent='0'  Received='0'  -> ✓
//  #09  Sent='9'  Received='9'  -> ✓
//  #10  Sent='!'  Received='!'  -> ✓
//  #11  Sent='a'  Received='a'  -> ✓
//  #12  Sent='A'  Received='A'  -> ✓
//  #13  Sent='0'  Received='0'  -> ✓
//  #14  Sent='9'  Received='9'  -> ✓
//  #15  Sent='!'  Received='!'  -> ✓
//
//  Status: ✓ EXCELLENT - Fully supported
//========================================
//
//========================================
//TEST #3: 921600 bps
//========================================
//Bit Width (Theory): 1.09 us
//
//----------------------------------------
//Results:
//  Total Tests: 15
//  Success: 15 (100.0%)
//  Errors: 0 (0.0%)
//  No Data: 0
//
//Per-character log:
//  #01  Sent='a'  Received='a'  -> ✓
//  #02  Sent='A'  Received='A'  -> ✓
//  #03  Sent='0'  Received='0'  -> ✓
//  #04  Sent='9'  Received='9'  -> ✓
//  #05  Sent='!'  Received='!'  -> ✓
//  #06  Sent='a'  Received='a'  -> ✓
//  #07  Sent='A'  Received='A'  -> ✓
//  #08  Sent='0'  Received='0'  -> ✓
//  #09  Sent='9'  Received='9'  -> ✓
//  #10  Sent='!'  Received='!'  -> ✓
//  #11  Sent='a'  Received='a'  -> ✓
//  #12  Sent='A'  Received='A'  -> ✓
//  #13  Sent='0'  Received='0'  -> ✓
//  #14  Sent='9'  Received='9'  -> ✓
//  #15  Sent='!'  Received='!'  -> ✓
//
//  Status: ✓ EXCELLENT - Fully supported
//========================================
//
//========================================
//TEST #4: 1000000 bps
//========================================
//Bit Width (Theory): 1.00 us
//
//----------------------------------------
//Results:
//  Total Tests: 15
//  Success: 15 (100.0%)
//  Errors: 0 (0.0%)
//  No Data: 0
//
//Per-character log:
//  #01  Sent='a'  Received='a'  -> ✓
//  #02  Sent='A'  Received='A'  -> ✓
//  #03  Sent='0'  Received='0'  -> ✓
//  #04  Sent='9'  Received='9'  -> ✓
//  #05  Sent='!'  Received='!'  -> ✓
//  #06  Sent='a'  Received='a'  -> ✓
//  #07  Sent='A'  Received='A'  -> ✓
//  #08  Sent='0'  Received='0'  -> ✓
//  #09  Sent='9'  Received='9'  -> ✓
//  #10  Sent='!'  Received='!'  -> ✓
//  #11  Sent='a'  Received='a'  -> ✓
//  #12  Sent='A'  Received='A'  -> ✓
//  #13  Sent='0'  Received='0'  -> ✓
//  #14  Sent='9'  Received='9'  -> ✓
//  #15  Sent='!'  Received='!'  -> ✓
//
//  Status: ✓ EXCELLENT - Fully supported
//========================================
//
//========================================
//TEST #5: 2000000 bps
//========================================
//Bit Width (Theory): 0.50 us
//
//----------------------------------------
//Results:
//  Total Tests: 15
//  Success: 15 (100.0%)
//  Errors: 0 (0.0%)
//  No Data: 0
//
//Per-character log:
//  #01  Sent='a'  Received='a'  -> ✓
//  #02  Sent='A'  Received='A'  -> ✓
//  #03  Sent='0'  Received='0'  -> ✓
//  #04  Sent='9'  Received='9'  -> ✓
//  #05  Sent='!'  Received='!'  -> ✓
//  #06  Sent='a'  Received='a'  -> ✓
//  #07  Sent='A'  Received='A'  -> ✓
//  #08  Sent='0'  Received='0'  -> ✓
//  #09  Sent='9'  Received='9'  -> ✓
//  #10  Sent='!'  Received='!'  -> ✓
//  #11  Sent='a'  Received='a'  -> ✓
//  #12  Sent='A'  Received='A'  -> ✓
//  #13  Sent='0'  Received='0'  -> ✓
//  #14  Sent='9'  Received='9'  -> ✓
//  #15  Sent='!'  Received='!'  -> ✓
//
//  Status: ✓ EXCELLENT - Fully supported
//========================================
//
//
//
//
//########################################
//#      HIGH-SPEED TEST SUMMARY        #
//########################################
//
//Baud Rate  | Bit Width | Tests | Success | Errors | Rate   | Status
//-----------|-----------|-------|---------|--------|--------|--------
// 230400 | 4.34 us | 15    | 15      |  0     | 100.0% | ✓ OK
// 460800 | 2.17 us | 15    | 15      |  0     | 100.0% | ✓ OK
// 921600 | 1.09 us | 15    | 15      |  0     | 100.0% | ✓ OK
//1000000 | 1.00 us | 15    | 15      |  0     | 100.0% | ✓ OK
//2000000 | 0.50 us | 15    | 15      |  0     | 100.0% | ✓ OK
//########################################
//
//Maximum Stable Baud Rate: 2000000 bps
//
//CSV Format (for data analysis):
//baud_rate,bit_width_us,total_tests,success,errors,success_rate,supported
//230400,4.34,15,15,0,100.00,YES
//460800,2.17,15,15,0,100.00,YES
//921600,1.09,15,15,0,100.00,YES
//1000000,1.00,15,15,0,100.00,YES
//2000000,0.50,15,15,0,100.00,YES
//
//########################################
//Test Complete!
//Check oscilloscope for waveform quality
//########################################
