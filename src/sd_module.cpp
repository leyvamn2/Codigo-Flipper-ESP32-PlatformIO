#include "sd_module.h"

bool setupSD() {
    if (!SD.begin(SD_CS)) return false;
    return true;
}

void guardarTarjeta(String uid) {
    File file = SD.open("/tarjetas.txt", FILE_APPEND);
    if (file) {
        file.println(uid);
        file.close();
    }
}

String leerListaTarjetas() {
    File file = SD.open("/tarjetas.txt");
    if (!file) return "No hay datos";
    String lista = "";
    while (file.available()) {
        lista += (char)file.read();
    }
    file.close();
    return lista;
}

void borrarTodasLasTarjetas() {
    SD.remove("/tarjetas.txt");
}