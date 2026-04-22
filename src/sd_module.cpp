#include "sd_module.h"

// Le decimos explícitamente a la S3 cuáles son los pines SPI de tu diagrama
#define SPI_SCK  18
#define SPI_MISO 19
#define SPI_MOSI 23

bool setupSD() {
    // Inicializamos el bus SPI con tus pines
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SD_CS); 
    
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

//Módulo IR
void guardarIR(String codigo) {
    // Abre (o crea si no existe) el archivo controles.txt y añade el texto al final
    File file = SD.open("/controles.txt", FILE_APPEND);
    if (file) {
        file.println(codigo);
        file.close();
    }
}