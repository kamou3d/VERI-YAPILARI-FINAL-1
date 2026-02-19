#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

// Desteklenen Veri Tipleri
typedef enum {
    DTYPE_FLOAT32,
    DTYPE_FLOAT16, // Simülasyon amaçlı (uint16_t olarak saklanır)
    DTYPE_INT8     // Quantized
} DataType;

// Union: Aynı bellek adresini farklı pointer türleri ile yönetmek için
typedef union {
    float* as_f32;
    uint16_t* as_f16;
    int8_t* as_int8;
    void* raw;
} DataPointer;

// Tensor Yapısı: TinyML için dinamik veri taşıyıcısı
typedef struct {
    DataPointer data;   // Verinin tutulduğu yer (Union sayesinde esnek)
    int size;           // Toplam eleman sayısı
    DataType type;      // Şu an hangi tipte veri tutuyor?
    
    // Quantization Meta-verileri (Sadece INT8 modunda anlamlı)
    float scale;
    int zero_point;
} Tensor;

// Tensor oluşturma fonksiyonu (Varsayılan Float32)
Tensor* create_tensor_f32(int size, float* initial_values) {
    Tensor* t = (Tensor*)malloc(sizeof(Tensor));
    t->size = size;
    t->type = DTYPE_FLOAT32;
    t->scale = 1.0f;
    t->zero_point = 0;

    // Bellek ayırma (32-bit float için)
    t->data.as_f32 = (float*)malloc(size * sizeof(float));

    if (initial_values != NULL) {
        for (int i = 0; i < size; i++) {
            t->data.as_f32[i] = initial_values[i];
        }
    }
    return t;
}

// Quantization Fonksiyonu: Float32 -> Int8 dönüşümü
// Formül: Real_Value = (Int8_Value - Zero_Point) * Scale
// Ters Formül: Int8_Value = (Real_Value / Scale) + Zero_Point
void quantize_tensor(Tensor* t) {
    if (t->type != DTYPE_FLOAT32) {
        printf("HATA: Sadece Float32 tensorler quantize edilebilir.\n");
        return;
    }

    printf("\n[Sistem] Quantization islemi baslatiliyor (32-bit -> 8-bit)...\n");

    // 1. Min ve Max değerleri bul
    float min_val = t->data.as_f32[0];
    float max_val = t->data.as_f32[0];
    for (int i = 1; i < t->size; i++) {
        if (t->data.as_f32[i] < min_val) min_val = t->data.as_f32[i];
        if (t->data.as_f32[i] > max_val) max_val = t->data.as_f32[i];
    }

    // 2. Scale ve Zero Point hesapla
    // Int8 aralığı: -128 ile 127 (255 adım)
    float scale = (max_val - min_val) / 255.0f;
    int zero_point = (int)(-min_val / scale) - 128;

    // 3. Yeni bellek alanı ayır (Int8 için)
    int8_t* new_data = (int8_t*)malloc(t->size * sizeof(int8_t));

    // 4. Verileri dönüştür ve sıkıştır
    for (int i = 0; i < t->size; i++) {
        // Matematiksel dönüşüm
        float scaled = (t->data.as_f32[i] / scale) + zero_point;
        
        // Clipping (Sınırlandırma -128 ile 127 arasına)
        if (scaled > 127) scaled = 127;
        if (scaled < -128) scaled = -128;
        
        new_data[i] = (int8_t)round(scaled);
    }

    // 5. Eski (büyük) belleği temizle
    free(t->data.as_f32);

    // 6. Tensörü güncelle
    t->data.as_int8 = new_data; // Union sayesinde int8 pointer'ı atıyoruz
    t->type = DTYPE_INT8;
    t->scale = scale;
    t->zero_point = zero_point;

    printf("[Sistem] Quantization tamamlandi. Bellek tasarrufu: %%75\n");
}

// Yazdırma Fonksiyonu
void print_tensor(Tensor* t) {
    printf("Tensor (Tip: %s, Boyut: %d): [ ", 
           (t->type == DTYPE_FLOAT32) ? "Float32" : "Int8 (Quantized)", t->size);
    
    for (int i = 0; i < t->size; i++) {
        if (t->type == DTYPE_FLOAT32) {
            printf("%.4f ", t->data.as_f32[i]);
        } else if (t->type == DTYPE_INT8) {
            printf("%d ", t->data.as_int8[i]);
        }
    }
    printf("]\n");
}

int main() {
    // Örnek ağırlıklar (Weights)
    float weights[] = { -1.2f, 0.5f, 2.3f, -0.8f, 1.5f };
    int size = 5;

    // 1. Adım: Float Tensör Oluştur
    Tensor* my_tensor = create_tensor_f32(size, weights);
    print_tensor(my_tensor);

    // Bellek kullanımını simüle et
    printf("Float32 Bellek Kullanimi: %lu bytes\n", size * sizeof(float));

    // 2. Adım: Quantization Uygula
    // Bu işlem belleği 20 byte'tan 5 byte'a düşürecek
    quantize_tensor(my_tensor);
    print_tensor(my_tensor);

    printf("Int8 Bellek Kullanimi: %lu bytes\n", size * sizeof(int8_t));
    
    // De-quantization (Doğrulama - yaklaşık değerlere geri dönüş)
    printf("\n[Sistem] Int8 verisinden geri okuma (De-quantization simülasyonu):\n");
    for(int i=0; i<size; i++) {
        float recovered = (my_tensor->data.as_int8[i] - my_tensor->zero_point) * my_tensor->scale;
        printf("Orjinal: %.4f -> Kurtarilan: %.4f\n", weights[i], recovered);
    }

    // Temizlik
    free(my_tensor->data.raw);
    free(my_tensor);

    return 0;
}