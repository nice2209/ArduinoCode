# UART 통신 실험 데이터 분석 및 딥러닝
# SP/08 - Arduino Uno UART Performance Analysis

"""
## 📚 프로젝트 개요

**실험 목적:**
- Arduino Uno UART 통신의 보드레이트별 성공률 분석
- 소프트웨어 vs 하드웨어 UART 성능 비교
- 딥러닝을 통한 통신 성공률 예측 모델 구축

**실험 데이터:**
- SP/06: 소프트웨어 UART (1200~115200 bps)
- SP/07: 하드웨어 UART (230400~2000000 bps)
- 총 13개 보드레이트 테스트
"""

# ========================================
# 1. 라이브러리 임포트
# ========================================

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import LabelEncoder, StandardScaler
from sklearn.metrics import mean_squared_error, r2_score, classification_report, confusion_matrix
import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import layers
from google.colab import files
import warnings
warnings.filterwarnings('ignore')

# 한글 폰트 설정
plt.rcParams['font.family'] = 'DejaVu Sans'
plt.rcParams['axes.unicode_minus'] = False

print("✅ 라이브러리 임포트 완료")
print(f"TensorFlow 버전: {tf.__version__}")

# ========================================
# 2. 데이터 로드
# ========================================

print("\n" + "="*50)
print("데이터 업로드")
print("="*50)
print("uart_experiment_data.csv 파일을 업로드하세요")

# 파일 업로드
uploaded = files.upload()

# 데이터 읽기
df = pd.read_csv('uart_experiment_data.csv')

print("\n✅ 데이터 로드 완료!")
print(f"데이터 크기: {df.shape}")
print("\n데이터 미리보기:")
print(df.head(10))

# ========================================
# 3. 데이터 탐색 (EDA)
# ========================================

print("\n" + "="*50)
print("데이터 탐색 (EDA)")
print("="*50)

# 기본 통계
print("\n📊 기본 통계:")
print(df.describe())

print("\n📋 데이터 정보:")
print(df.info())

print("\n🔍 결측치 확인:")
print(df.isnull().sum())

# Method별 통계
print("\n📈 Method별 통계:")
print(df.groupby('method')[['baud_rate', 'success_rate']].agg(['mean', 'min', 'max']))

# ========================================
# 4. 데이터 시각화
# ========================================

print("\n" + "="*50)
print("데이터 시각화")
print("="*50)

# Figure 1: 보드레이트 vs 성공률
plt.figure(figsize=(15, 5))

plt.subplot(1, 3, 1)
software_data = df[df['method'] == 'Software']
hardware_data = df[df['method'] == 'Hardware']

plt.plot(software_data['baud_rate'], software_data['success_rate'], 
         'o-', label='Software UART', linewidth=2, markersize=8)
plt.plot(hardware_data['baud_rate'], hardware_data['success_rate'], 
         's-', label='Hardware UART', linewidth=2, markersize=8)
plt.xlabel('Baud Rate (bps)', fontsize=12)
plt.ylabel('Success Rate (%)', fontsize=12)
plt.title('Baud Rate vs Success Rate', fontsize=14, fontweight='bold')
plt.xscale('log')
plt.grid(True, alpha=0.3)
plt.legend()
plt.ylim(-5, 105)

# Figure 2: 비트 폭 vs 성공률
plt.subplot(1, 3, 2)
plt.scatter(df['bit_width_us'], df['success_rate'], 
           c=df['method'].map({'Software': 'blue', 'Hardware': 'red'}),
           s=100, alpha=0.6, edgecolors='black')
plt.xlabel('Bit Width (us)', fontsize=12)
plt.ylabel('Success Rate (%)', fontsize=12)
plt.title('Bit Width vs Success Rate', fontsize=14, fontweight='bold')
plt.xscale('log')
plt.grid(True, alpha=0.3)
plt.legend(['Software', 'Hardware'])

# Figure 3: Method별 성공률 분포
plt.subplot(1, 3, 3)
df.boxplot(column='success_rate', by='method', ax=plt.gca())
plt.xlabel('Method', fontsize=12)
plt.ylabel('Success Rate (%)', fontsize=12)
plt.title('Success Rate Distribution by Method', fontsize=14, fontweight='bold')
plt.suptitle('')

plt.tight_layout()
plt.savefig('uart_analysis_overview.png', dpi=300, bbox_inches='tight')
plt.show()

print("✅ 시각화 완료 (uart_analysis_overview.png 저장)")

# Figure 4: 히트맵
plt.figure(figsize=(10, 6))
pivot_table = df.pivot_table(values='success_rate', 
                              index='speed_category', 
                              columns='method', 
                              aggfunc='mean')
sns.heatmap(pivot_table, annot=True, fmt='.1f', cmap='RdYlGn', 
            vmin=0, vmax=100, cbar_kws={'label': 'Success Rate (%)'})
plt.title('Success Rate Heatmap: Speed Category vs Method', 
          fontsize=14, fontweight='bold')
plt.xlabel('Method', fontsize=12)
plt.ylabel('Speed Category', fontsize=12)
plt.tight_layout()
plt.savefig('uart_heatmap.png', dpi=300, bbox_inches='tight')
plt.show()

print("✅ 히트맵 완료 (uart_heatmap.png 저장)")

# ========================================
# 5. 데이터 전처리
# ========================================

print("\n" + "="*50)
print("데이터 전처리")
print("="*50)

# 특성 선택
features = ['baud_rate', 'bit_width_us', 'method']
target = 'success_rate'

# Method를 숫자로 인코딩
le = LabelEncoder()
df['method_encoded'] = le.fit_transform(df['method'])

# 특성과 타겟 분리
X = df[['baud_rate', 'bit_width_us', 'method_encoded']].values
y = df[target].values

# 데이터 분할 (80% 학습, 20% 테스트)
X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, random_state=42
)

# 특성 스케일링
scaler = StandardScaler()
X_train_scaled = scaler.fit_transform(X_train)
X_test_scaled = scaler.transform(X_test)

print(f"✅ 학습 데이터: {X_train_scaled.shape}")
print(f"✅ 테스트 데이터: {X_test_scaled.shape}")
print(f"✅ Method 인코딩: {dict(zip(le.classes_, le.transform(le.classes_)))}")

# ========================================
# 6. 모델 1: 회귀 모델 (성공률 예측)
# ========================================

print("\n" + "="*50)
print("모델 1: 성공률 예측 (회귀)")
print("="*50)

# 모델 구축
regression_model = keras.Sequential([
    layers.Dense(64, activation='relu', input_shape=(3,)),
    layers.Dropout(0.2),
    layers.Dense(32, activation='relu'),
    layers.Dropout(0.2),
    layers.Dense(16, activation='relu'),
    layers.Dense(1)
])

regression_model.compile(
    optimizer=keras.optimizers.Adam(learning_rate=0.001),
    loss='mse',
    metrics=['mae']
)

print("\n📊 모델 구조:")
regression_model.summary()

# 모델 학습
print("\n🚀 모델 학습 시작...")
history = regression_model.fit(
    X_train_scaled, y_train,
    validation_split=0.2,
    epochs=200,
    batch_size=4,
    verbose=0
)

# 학습 결과 시각화
plt.figure(figsize=(12, 4))

plt.subplot(1, 2, 1)
plt.plot(history.history['loss'], label='Training Loss')
plt.plot(history.history['val_loss'], label='Validation Loss')
plt.xlabel('Epoch')
plt.ylabel('Loss (MSE)')
plt.title('Model Loss During Training')
plt.legend()
plt.grid(True, alpha=0.3)

plt.subplot(1, 2, 2)
plt.plot(history.history['mae'], label='Training MAE')
plt.plot(history.history['val_mae'], label='Validation MAE')
plt.xlabel('Epoch')
plt.ylabel('MAE')
plt.title('Model MAE During Training')
plt.legend()
plt.grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig('regression_training.png', dpi=300, bbox_inches='tight')
plt.show()

# 모델 평가
y_pred = regression_model.predict(X_test_scaled, verbose=0).flatten()

mse = mean_squared_error(y_test, y_pred)
rmse = np.sqrt(mse)
r2 = r2_score(y_test, y_pred)

print("\n📈 회귀 모델 성능:")
print(f"  MSE: {mse:.2f}")
print(f"  RMSE: {rmse:.2f}")
print(f"  R² Score: {r2:.4f}")

# 예측 vs 실제
plt.figure(figsize=(8, 6))
plt.scatter(y_test, y_pred, s=100, alpha=0.6, edgecolors='black')
plt.plot([0, 100], [0, 100], 'r--', linewidth=2, label='Perfect Prediction')
plt.xlabel('Actual Success Rate (%)', fontsize=12)
plt.ylabel('Predicted Success Rate (%)', fontsize=12)
plt.title('Regression Model: Predicted vs Actual', fontsize=14, fontweight='bold')
plt.legend()
plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig('regression_prediction.png', dpi=300, bbox_inches='tight')
plt.show()

print("✅ 회귀 모델 학습 완료")

# ========================================
# 7. 모델 2: 분류 모델 (성공/실패 예측)
# ========================================

print("\n" + "="*50)
print("모델 2: 통신 성공/실패 분류")
print("="*50)

# 이진 분류를 위한 타겟 생성 (90% 이상: 성공)
y_binary = (y >= 90).astype(int)
y_train_binary = (y_train >= 90).astype(int)
y_test_binary = (y_test >= 90).astype(int)

print(f"✅ 성공(1): {np.sum(y_binary == 1)}개")
print(f"✅ 실패(0): {np.sum(y_binary == 0)}개")

# 분류 모델 구축
classification_model = keras.Sequential([
    layers.Dense(32, activation='relu', input_shape=(3,)),
    layers.Dropout(0.3),
    layers.Dense(16, activation='relu'),
    layers.Dropout(0.3),
    layers.Dense(8, activation='relu'),
    layers.Dense(1, activation='sigmoid')
])

classification_model.compile(
    optimizer=keras.optimizers.Adam(learning_rate=0.001),
    loss='binary_crossentropy',
    metrics=['accuracy']
)

print("\n📊 분류 모델 구조:")
classification_model.summary()

# 모델 학습
print("\n🚀 분류 모델 학습 시작...")
history_class = classification_model.fit(
    X_train_scaled, y_train_binary,
    validation_split=0.2,
    epochs=150,
    batch_size=4,
    verbose=0
)

# 학습 결과 시각화
plt.figure(figsize=(12, 4))

plt.subplot(1, 2, 1)
plt.plot(history_class.history['loss'], label='Training Loss')
plt.plot(history_class.history['val_loss'], label='Validation Loss')
plt.xlabel('Epoch')
plt.ylabel('Loss')
plt.title('Classification Model Loss')
plt.legend()
plt.grid(True, alpha=0.3)

plt.subplot(1, 2, 2)
plt.plot(history_class.history['accuracy'], label='Training Accuracy')
plt.plot(history_class.history['val_accuracy'], label='Validation Accuracy')
plt.xlabel('Epoch')
plt.ylabel('Accuracy')
plt.title('Classification Model Accuracy')
plt.legend()
plt.grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig('classification_training.png', dpi=300, bbox_inches='tight')
plt.show()

# 모델 평가
y_pred_proba = classification_model.predict(X_test_scaled, verbose=0)
y_pred_binary = (y_pred_proba > 0.5).astype(int).flatten()

print("\n📈 분류 모델 성능:")
print(classification_report(y_test_binary, y_pred_binary, 
                           target_names=['Fail', 'Success']))

# Confusion Matrix
cm = confusion_matrix(y_test_binary, y_pred_binary)
plt.figure(figsize=(6, 5))
sns.heatmap(cm, annot=True, fmt='d', cmap='Blues', 
            xticklabels=['Fail', 'Success'],
            yticklabels=['Fail', 'Success'])
plt.xlabel('Predicted', fontsize=12)
plt.ylabel('Actual', fontsize=12)
plt.title('Confusion Matrix', fontsize=14, fontweight='bold')
plt.tight_layout()
plt.savefig('confusion_matrix.png', dpi=300, bbox_inches='tight')
plt.show()

print("✅ 분류 모델 학습 완료")

# ========================================
# 8. 실용 예제: 보드레이트 추천 시스템
# ========================================

print("\n" + "="*50)
print("보드레이트 추천 시스템")
print("="*50)

def recommend_baudrate(required_success_rate, method='Hardware'):
    """
    요구 성공률에 따라 최적 보드레이트 추천
    """
    method_encoded = le.transform([method])[0]
    
    # 테스트할 보드레이트 범위
    test_bauds = [1200, 2400, 4800, 9600, 19200, 38400, 57600, 
                  115200, 230400, 460800, 921600, 1000000, 2000000]
    
    recommendations = []
    
    for baud in test_bauds:
        bit_width = 1000000.0 / baud
        features = np.array([[baud, bit_width, method_encoded]])
        features_scaled = scaler.transform(features)
        
        predicted_rate = regression_model.predict(features_scaled, verbose=0)[0][0]
        
        if predicted_rate >= required_success_rate:
            recommendations.append({
                'baud_rate': baud,
                'predicted_success': predicted_rate,
                'bit_width_us': bit_width
            })
    
    return recommendations

# 예제 1: 95% 이상 성공률 필요
print("\n예제 1: 95% 이상 성공률이 필요한 경우 (Hardware)")
recs = recommend_baudrate(95, 'Hardware')
print(f"추천 보드레이트: {len(recs)}개")
for rec in recs[:3]:  # 상위 3개만 출력
    print(f"  - {rec['baud_rate']:7d} bps: 예상 성공률 {rec['predicted_success']:.1f}%")

# 예제 2: Software UART로 90% 이상
print("\n예제 2: 90% 이상 성공률이 필요한 경우 (Software)")
recs = recommend_baudrate(90, 'Software')
print(f"추천 보드레이트: {len(recs)}개")
for rec in recs[:3]:
    print(f"  - {rec['baud_rate']:7d} bps: 예상 성공률 {rec['predicted_success']:.1f}%")

# ========================================
# 9. 종합 분석 및 결론
# ========================================

print("\n" + "="*50)
print("종합 분석 및 결론")
print("="*50)

# 전체 데이터 시각화
fig, axes = plt.subplots(2, 2, figsize=(15, 10))

# 1. Method별 성공률
axes[0, 0].bar(['Software', 'Hardware'], 
               [df[df['method']=='Software']['success_rate'].mean(),
                df[df['method']=='Hardware']['success_rate'].mean()],
               color=['blue', 'red'], alpha=0.7, edgecolor='black')
axes[0, 0].set_ylabel('Average Success Rate (%)', fontsize=11)
axes[0, 0].set_title('Average Success Rate by Method', fontweight='bold')
axes[0, 0].grid(True, alpha=0.3, axis='y')
axes[0, 0].set_ylim(0, 100)

# 2. 보드레이트 분포
axes[0, 1].hist([software_data['baud_rate'], hardware_data['baud_rate']], 
                bins=10, label=['Software', 'Hardware'], 
                color=['blue', 'red'], alpha=0.6, edgecolor='black')
axes[0, 1].set_xlabel('Baud Rate (bps)', fontsize=11)
axes[0, 1].set_ylabel('Count', fontsize=11)
axes[0, 1].set_title('Baud Rate Distribution', fontweight='bold')
axes[0, 1].legend()
axes[0, 1].set_xscale('log')

# 3. 속도 카테고리별 성공률
category_order = ['Low', 'Medium', 'High', 'VeryHigh', 'Ultra']
category_data = df.groupby('speed_category')['success_rate'].mean().reindex(category_order)
axes[1, 0].plot(category_order, category_data, 'o-', linewidth=2, markersize=10)
axes[1, 0].set_xlabel('Speed Category', fontsize=11)
axes[1, 0].set_ylabel('Average Success Rate (%)', fontsize=11)
axes[1, 0].set_title('Success Rate by Speed Category', fontweight='bold')
axes[1, 0].grid(True, alpha=0.3)
axes[1, 0].set_ylim(0, 100)

# 4. 에러율
df['error_rate'] = 100 - df['success_rate']
axes[1, 1].scatter(df['baud_rate'], df['error_rate'], 
                   c=df['method'].map({'Software': 'blue', 'Hardware': 'red'}),
                   s=150, alpha=0.6, edgecolors='black')
axes[1, 1].set_xlabel('Baud Rate (bps)', fontsize=11)
axes[1, 1].set_ylabel('Error Rate (%)', fontsize=11)
axes[1, 1].set_title('Error Rate vs Baud Rate', fontweight='bold')
axes[1, 1].set_xscale('log')
axes[1, 1].grid(True, alpha=0.3)
axes[1, 1].legend(['Software', 'Hardware'])

plt.tight_layout()
plt.savefig('comprehensive_analysis.png', dpi=300, bbox_inches='tight')
plt.show()

print("\n" + "="*70)
print("📊 핵심 발견 사항 (Key Findings)")
print("="*70)

print("\n1️⃣ 소프트웨어 UART (Software):")
print(f"   - 최대 안정 속도: 19200 bps (100% 성공)")
print(f"   - 평균 성공률: {df[df['method']=='Software']['success_rate'].mean():.1f}%")
print(f"   - 한계점: 38400 bps부터 오류 발생 시작")

print("\n2️⃣ 하드웨어 UART (Hardware):")
print(f"   - 최대 테스트 속도: 2000000 bps (100% 성공!)")
print(f"   - 평균 성공률: {df[df['method']=='Hardware']['success_rate'].mean():.1f}%")
print(f"   - 소프트웨어 대비 약 {2000000/19200:.0f}배 빠름")

print("\n3️⃣ 비트 폭과 성공률의 관계:")
print(f"   - 비트 폭 > 50µs: 거의 100% 성공")
print(f"   - 비트 폭 < 20µs: 소프트웨어 UART 한계")
print(f"   - 비트 폭 < 1µs: 하드웨어 UART만 가능")

print("\n4️⃣ 딥러닝 모델 성능:")
print(f"   - 회귀 모델 R² Score: {r2:.4f}")
print(f"   - 분류 모델 정확도: {np.mean(y_test_binary == y_pred_binary)*100:.1f}%")

print("\n5️⃣ 실용적 권장사항:")
print("   - 안정적 통신 필요: 9600 bps (양쪽 모두 100%)")
print("   - 중속 통신: 19200 bps (소프트웨어), 230400 bps (하드웨어)")
print("   - 고속 통신: 하드웨어 UART 필수 (921600+ bps)")

print("\n" + "="*70)
print("✅ 분석 완료! 모든 그래프와 결과가 저장되었습니다.")
print("="*70)

# ========================================
# 10. 모델 저장
# ========================================

print("\n💾 모델 저장 중...")
regression_model.save('uart_regression_model.h5')
classification_model.save('uart_classification_model.h5')
print("✅ 모델 저장 완료!")
print("   - uart_regression_model.h5")
print("   - uart_classification_model.h5")

print("\n🎉 모든 분석이 완료되었습니다! 🎉")
