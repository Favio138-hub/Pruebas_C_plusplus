#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

// Función de activación básica (Sigmide) para las neuronas
double funcionActivacion(double x) {
    return 1.0 / (1.0 + std::exp(-x));
}

int main() {
    // Configurar una semilla aleatoria para simular datos
    std::srand(std::time(0));

    std::cout << "================================================" << std::endl;
    std::cout << "   SIMULACION DE ANALISIS NEURONAL EN C++      " << std::endl;
    std::cout << "================================================" << std::endl;

    // Simular 4 entradas sensoriales (ej. Visión, Audio, Tacto, Memoria)
    std::vector<double> entradas = {0.85, 0.34, 0.12, 0.95};
    
    std::cout << "\n[1] Capturando Estimulos Sensoriales (Entradas):" << std::endl;
    for(size_t i = 0; i < entradas.size(); ++i) {
        std::cout << " -> Canal Neuronal " << i + 1 << ": " << entradas[i] << std::endl;
    }

    // Simular Pesos Sinápticos (Conexiones entre neuronas) creados al azar
    std::cout << "\n[2] Procesando a traves de la Capa Oculta (Sinapsis):" << std::endl;
    double sumaCapa = 0.0;
    for(size_t i = 0; i < entradas.size(); ++i) {
        double pesoAleatorio = (std::rand() % 100) / 100.0;
        std::cout << " * Conexion " << i + 1 << " - Peso Sinaptico: " << pesoAleatorio << std::endl;
        sumaCapa += entradas[i] * pesoAleatorio;
    }

    // Calcular la respuesta final de la red neuronal
    double resultadoNeuronal = funcionActivacion(sumaCapa);

    std::cout << "\n[3] Resultado del Analisis Neuronal:" << std::endl;
    std::cout << " >> Potencial de Accion Acumulado: " << sumaCapa << std::endl;
    std::cout << " >> Tasa de Disparo Final (Activacion): " << resultadoNeuronal * 100 << "%" << std::endl;

    if (resultadoNeuronal > 0.75) {
        std::cout << "\n[ESTADO]: Estimulo Alto detectado. Respuesta cerebral ACTIVADA." << std::endl;
    } else {
        std::cout << "\n[ESTADO]: Estimulo Bajo. Neuronas en estado de reposo." << std::endl;
    }

    std::cout << "================================================" << std::endl;
    return 0;
}
