#include "nfc_module.h"
#include "sd_module.h"
#define JOY_Y  35
#define JOY_SW 32

#define PN532_IRQ   (21)  
#define PN532_RESET (5) 

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);
uint8_t uidLeido[7];
uint8_t uidLongitud = 0;
String uidString = "";

void setupNFC() {
    Wire.begin(25, 26);          // SDA=25, SCL=26 (pines por defecto de I2C en ESP32)
    nfc.begin();               
    
    uint32_t version = nfc.getFirmwareVersion();
    if (!version) {
        Serial.println("Error: PN532 no encontrado. Revisa conexiones o modo I2C.");
                // Detener ejecución
    }
    Serial.print("PN532 firmware: 0x");
    Serial.println(version, HEX);
    
    nfc.SAMConfig();             // Configurar modo normal (seguridad pasiva)
    Serial.println("NFC listo.");
}

bool capturarUID() {
    uidString = "";
    uint8_t success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uidLeido, &uidLongitud, 100);
    
    if (success) {
        for (uint8_t i = 0; i < uidLongitud; i++) {
            uidString += String(uidLeido[i], HEX);
            uidString.toUpperCase();
        }
        return true;
    }
    return false;
}

String obtenerUltimoUID() {
    return uidString;
}

bool clonarTarjeta() {
    uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }; // Clave estándar de fábrica
    uint8_t uidDestino[7];
    uint8_t uidLenDestino;

    // 1. Esperamos hasta 5 segundos a que el usuario acerque la tarjeta "virgen"
    if (!nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uidDestino, &uidLenDestino, 5000)) {
        return false; // Se acabó el tiempo o no detectó tarjeta
    }

    // 2. Autenticar el Sector 0 (Bloque 0) de la nueva tarjeta con la clave por defecto
    if (!nfc.mifareclassic_AuthenticateBlock(uidDestino, uidLenDestino, 0, 0, keya)) {
        return false; // Falló la autenticación
    }

    // 3. Preparar los 16 bytes del Bloque 0 que vamos a inyectar
    uint8_t bloque0[16] = {0};
    
    // Copiar los 4 bytes del UID que leímos originalmente
    for(int i = 0; i < 4; i++) {
        bloque0[i] = uidLeido[i];
    }
    
    bloque0[4] = uidLeido[0] ^ uidLeido[1] ^ uidLeido[2] ^ uidLeido[3];
    
    bloque0[5] = 0x08; 
    bloque0[6] = 0x04; 
    bloque0[7] = 0x00; 

    return nfc.mifareclassic_WriteDataBlock(0, bloque0);
}

void flujoCapturaRFID(bool &dentroDeOpcion) {
    bool enFlujoNFC = true;
    String opcionesNFC[4] = {"1. Seguir capt.", "2. Clonar ID", "3. Guardar ID", "4. Regresar"};

    while (enFlujoNFC) {
        display.clearBuffer();
        display.setFont(u8g2_font_6x12_tr);
        display.drawStr(10, 20, "¿Capturar tarjeta?");
        display.drawStr(10, 40, "Acerca la tarjeta...");
        display.drawStr(10, 60, "(Click para cancelar)");
        display.sendBuffer();

        bool leida = false;
        while (!leida) {
            leida = capturarUID();
            
            // Si el usuario se arrepiente y presiona click, sale al menú principal
            if (digitalRead(JOY_SW) == LOW) {
                delay(300);
                dentroDeOpcion = false; 
                return;
            }
        }

        // --- 2. TARJETA CAPTURADA: MOSTRAR LAS 4 OPCIONES ---
        int indexNFC = 0;
        bool accionSeleccionada = false;

        while (!accionSeleccionada) {
            int valorY = analogRead(JOY_Y);
            if (valorY < 1000) { if (indexNFC > 0) indexNFC--; delay(200); }
            if (valorY > 3000) { if (indexNFC < 3) indexNFC++; delay(200); }

            if (digitalRead(JOY_SW) == LOW) {
                accionSeleccionada = true;
                delay(300);
            }

            display.clearBuffer();
            display.setFont(u8g2_font_6x12_tr);
            display.setCursor(0, 10);
            display.print("ID: " + obtenerUltimoUID());
            display.drawLine(0, 14, 128, 14);

            for (int i = 0; i < 4; i++) {
                int yPos = 26 + (i * 12);
                if (i == indexNFC) display.drawStr(0, yPos, ">");
                display.setCursor(10, yPos);
                display.print(opcionesNFC[i]);
            }
            display.sendBuffer();
        }

        // --- 3. EJECUTAR LA OPCIÓN ELEGIDA ---
        display.clearBuffer();
        
        if (indexNFC == 0) {
            // 1. Seguir capturando
            continue; 
            
        } else if (indexNFC == 1) {
            // 2. Clonar ID
            clonarTarjeta();
            display.drawStr(20, 30, "Funcion Clonar...");
            display.sendBuffer();
            delay(1500);
            
        }  else if (indexNFC == 2) {
            // 3. Guardar ID
            guardarTarjeta(obtenerUltimoUID()); // <-- AGREGA ESTA LÍNEA
            display.drawStr(20, 30, "ID Guardado en SD!");
            display.sendBuffer();
            delay(1500);
            
        } else if (indexNFC == 3) {
            // 4. Regresar a menu principal
            enFlujoNFC = false;      // Rompe este ciclo
            dentroDeOpcion = false;  // Le avisa al main que ya no estamos en el submenú
        }
    }
}