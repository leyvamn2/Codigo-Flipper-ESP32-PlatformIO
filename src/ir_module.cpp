#include "ir_module.h"
#include "sd_module.h" // Incluimos el módulo SD para guardar los códigos

IRrecv irrecv(IR_RX_PIN);
IRsend irsend(IR_TX_PIN);
decode_results results;

// Pines del Joystick (Asegúrate de que coincidan con tu esquemático)
#define JOY_Y  35
#define JOY_SW 32

void setupIR() {
    irrecv.enableIRIn();
    irsend.begin();
}

bool recibirCodigoIR(String &resultadoHex, uint32_t &codigoCrudo) {
    if (irrecv.decode(&results)) {
        resultadoHex = resultToHexidecimal(&results);
        codigoCrudo = (uint32_t)results.value;
        irrecv.resume();
        return true;
    }
    return false;
}

void enviarCodigoIR(uint32_t codigo) {
    irsend.sendNEC(codigo, 32); 
}

// Nueva función para manejar el submenú de opciones
void mostrarOpcionesIR(String codigoHex, uint32_t codigoCrudo, bool &dentroDeOpcion) {
    bool enOpcionesIR = true;
    int indexOpcion = 0;
    String opcionesIR[3] = {"1. Replicar", "2. Guardar", "3. Regresar"};

    while (enOpcionesIR) {
        int valorY = analogRead(JOY_Y);
        
        // Navegación del menú
        if (valorY < 1000) { if (indexOpcion > 0) indexOpcion--; delay(200); }
        if (valorY > 3000) { if (indexOpcion < 2) indexOpcion++; delay(200); }

        // Dibujar el menú
        display.clearBuffer();
        display.setFont(u8g2_font_6x12_tr);
        display.setCursor(0, 10);
        display.print("Cod: " + codigoHex);
        display.drawLine(0, 14, 128, 14);

        for (int i = 0; i < 3; i++) {
            int yPos = 26 + (i * 12);
            if (i == indexOpcion) display.drawStr(0, yPos, ">");
            display.setCursor(10, yPos);
            display.print(opcionesIR[i]);
        }
        display.sendBuffer();

        // Selección de opción
        if (digitalRead(JOY_SW) == LOW) {
            delay(300); // Antirrebote
            display.clearBuffer();

            if (indexOpcion == 0) {
                // Replicar Señal
                display.drawStr(10, 30, "Enviando senal...");
                display.sendBuffer();
                enviarCodigoIR(codigoCrudo);
                delay(1000);
            } else if (indexOpcion == 1) {
                // Guardar Señal
                guardarIR("IR: " + codigoHex); // Usamos la misma función de SD
                display.drawStr(10, 30, "Guardado en SD!");
                display.sendBuffer();
                delay(1000);
            } else if (indexOpcion == 2) {
                // Regresar
                enOpcionesIR = false;
            }
        }
    }
}

void flujoInfrarrojo(bool &dentroDeOpcion) {
    bool enFlujoIR = true;
    String codigoLeidoHex = "";
    uint32_t codigoCrudo = 0;

    while (enFlujoIR) {
        display.clearBuffer();
        display.setFont(u8g2_font_6x12_tr);
        display.drawStr(10, 20, "Modo Infrarrojo");
        display.drawStr(10, 40, "Esperando senal...");
        display.drawStr(10, 62, "Pulsa Joystick = Salir");
        display.sendBuffer();

        // Esperamos a recibir un código o a que el usuario presione el botón
        while (!recibirCodigoIR(codigoLeidoHex, codigoCrudo)) {
            if (digitalRead(JOY_SW) == LOW) {
                delay(300);
                dentroDeOpcion = false;
                enFlujoIR = false;
                return;
            }
        }

        // Si recibimos un código, mostramos el menú de opciones
        if (codigoLeidoHex != "") {
            mostrarOpcionesIR(codigoLeidoHex, codigoCrudo, dentroDeOpcion);
            // Al salir del menú de opciones, reiniciamos las variables para capturar otra vez
            codigoLeidoHex = "";
            codigoCrudo = 0;
        }
    }
}