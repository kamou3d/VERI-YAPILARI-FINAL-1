# TinyML Primitive Tensor Library in C

Bu proje, bellek kısıtlı gömülü sistemler (Arduino, ESP32 vb.) üzerinde Yapay Zeka modellerini çalıştırmak için tasarlanmış **hafif sıklet (lightweight) bir Tensör kütüphanesidir.**

##  Amaç
Standart 32-bit Float dizileri mikrodenetleyici RAM'ini hızla doldurur. Bu proje, **Dynamic Quantization** tekniklerini kullanarak model boyutlarını 4 katına kadar küçültmeyi ve C dilinin `union` yapısı ile bellek yönetimini optimize etmeyi amaçlar.

##  Kullanılan Teknolojiler ve Yöntemler
- **Dil:** C (Standart C99)
- **Yaklaşım:** Agentic Coding (Gemini 3 Pro destekli)
- **Teknikler:**
  - `Struct` ve `Union` ile dinamik tip yönetimi.
  - **Quantization:** Float32 -> Int8 dönüşümü.
  - Pointer Aritmetiği.

##  Özellikler
1. **Dinamik Tip Değişimi:** Aynı veri yapısı Float32, Float16 (simüle) veya Int8 tutabilir.
2. **Quantization Motoru:** Otomatik Min/Max hesaplama ve ölçekleme (scaling).
3. **Bellek Tasarrufu:** 32-bit veriyi 8-bit'e indirgeyerek %75 RAM tasarrufu sağlar.

##  Kurulum ve Çalıştırma

Kodu derlemek için GCC derleyicisi yeterlidir:

```bash
gcc main.c -o tensor_app -lm
./tensor_app
