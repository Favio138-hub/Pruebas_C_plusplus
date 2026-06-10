#include "InteractiveApp.hpp"
#include <iostream>
#include <cstdio>
#include <random>
#include <cmath>
#include <filesystem>
#include <algorithm>
#include <limits>
#include <sstream>

static constexpr double PI = 3.14159265358979323846;

InteractiveApp::InteractiveApp() {
    m_model = DeforestationModel(m_state.params);
}

void InteractiveApp::run() {
    showSplash();
    int opt;
    do {
        showMainMenu();
        opt = readInt("  Seleccione opcion", 0, 9);
        switch (opt) {
            case 1: menuGenerateSynthetic(); break;
            case 2: menuLoadRasters(); break;
            case 3: menuAdjustParams(); break;
            case 4: menuRunAnalysis(); break;
            case 5: menuViewReport(); break;
            case 6: menuExportResults(); break;
            case 7: menuChangeZone(); break;
            case 8: showModelInfo(); break;
            case 9: showAbout(); break;
            case 0: std::cout << "\n  Hasta luego!\n\n"; break;
        }
    } while (opt != 0);
}

// ---------------------------------------------------------------
//  UTILIDADES DE ENTRADA
// ---------------------------------------------------------------

int InteractiveApp::readInt(const std::string& prompt, int min, int max) {
    int val;
    std::string line;
    while (true) {
        std::cout << prompt << " [" << min << "-" << max << "]: ";
        std::getline(std::cin, line);
        std::stringstream ss(line);
        if (ss >> val && val >= min && val <= max) return val;
        std::cout << "  Invalido. Ingrese un numero entre " << min << " y " << max << ".\n";
    }
}

double InteractiveApp::readDouble(const std::string& prompt, double min, double max) {
    double val;
    std::string line;
    while (true) {
        std::cout << prompt << " [" << min << "-" << max << "]: ";
        std::getline(std::cin, line);
        std::stringstream ss(line);
        if (ss >> val && val >= min && val <= max) return val;
        std::cout << "  Invalido. Ingrese un valor entre " << min << " y " << max << ".\n";
    }
}

std::string InteractiveApp::readString(const std::string& prompt) {
    std::cout << prompt << ": ";
    std::string val;
    std::getline(std::cin, val);
    return val;
}

bool InteractiveApp::readBool(const std::string& prompt) {
    std::string val;
    while (true) {
        std::cout << prompt << " (s/n): ";
        std::getline(std::cin, val);
        if (val == "s" || val == "S" || val == "si" || val == "SI") return true;
        if (val == "n" || val == "N" || val == "no" || val == "NO") return false;
        std::cout << "  Responda 's' o 'n'.\n";
    }
}

void InteractiveApp::waitEnter() {
    std::cout << "\n  Presione ENTER para continuar...";
    std::string dummy;
    std::getline(std::cin, dummy);
}

// ---------------------------------------------------------------
//  PANTALLAS
// ---------------------------------------------------------------

void InteractiveApp::showSplash() {
    std::cout << "\n";
    std::cout << "  ==========================================\n";
    std::cout << "   SISTEMA DE MONITOREO DE DEFORESTACION\n";
    std::cout << "   MODELO IRDA v1.0\n";
    std::cout << "   Amazonia Baja Peruana - Zona UTM 18S\n";
    std::cout << "  ==========================================\n";
    std::cout << "   Desarrollado para: Loreto, Peru\n";
    std::cout << "   EPSG:32718 | WGS 84 / UTM zone 18S\n";
    std::cout << "  ==========================================\n\n";
}

void InteractiveApp::showMainMenu() {
    std::cout << "\n";
    std::cout << "  =============== MENU PRINCIPAL ===============\n";
    printf("  Ubicacion: %s\n", m_state.locationName.c_str());
    printf("  UTM: Zona %d%c | Este: %.0f | Norte: %.0f\n",
           m_state.utmZone, m_state.utmHemisphere,
           m_state.utmEasting, m_state.utmNorthing);
    printf("  Resolucion: %.0fm | Periodo: %d-%d\n",
           m_state.cellSize, m_state.yearT1, m_state.yearT2);
    printf("  Datos cargados: %s | Analisis: %s\n",
           m_state.dataLoaded ? "SI" : "NO",
           m_state.analysisRun ? "EJECUTADO" : "PENDIENTE");
    std::cout << "  ==============================================\n";
    std::cout << "   1. Generar datos sinteticos (Loreto, Z18S)\n";
    std::cout << "   2. Cargar rasters NDVI (.asc)\n";
    std::cout << "   3. Ajustar parametros del modelo IRDA\n";
    std::cout << "   4. Ejecutar analisis de deforestacion\n";
    std::cout << "   5. Ver reporte y estadisticas\n";
    std::cout << "   6. Exportar resultados a archivos\n";
    std::cout << "   7. Cambiar zona UTM / ubicacion\n";
    std::cout << "   8. Acerca del modelo IRDA\n";
    std::cout << "   9. Acerca de este programa\n";
    std::cout << "   0. Salir\n";
    std::cout << "  ==============================================\n";
}

void InteractiveApp::showAbout() {
    std::cout << "\n";
    std::cout << "  ==========================================\n";
    std::cout << "   SISTEMA DE MONITOREO DE DEFORESTACION\n";
    std::cout << "   MODELO IRDA v1.0\n";
    std::cout << "  ==========================================\n";
    std::cout << "   Desarrollado para analisis de\n";
    std::cout << "   deforestacion en la Amazonia Baja\n";
    std::cout << "   Peruana, departamento de Loreto.\n\n";
    std::cout << "   Zona UTM: 18S (EPSG:32718)\n";
    std::cout << "   Lenguaje: C++17\n";
    std::cout << "   Formato raster: ASCII Grid (.asc)\n\n";
    std::cout << "   Compatible con QGIS, ArcGIS, GRASS GIS\n";
    std::cout << "  ==========================================\n";
    waitEnter();
}

void InteractiveApp::showModelInfo() {
    std::cout << "\n";
    std::cout << "  ============ MODELO IRDA ============\n";
    std::cout << "   Indice de Riesgo de Deforestacion\n";
    std::cout << "   en Amazonia Baja\n";
    std::cout << "  =====================================\n\n";
    std::cout << "  IRDA = w1*DNDVI + w2*VHI + w3*SDP\n\n";
    printf("  Pesos actuales:\n");
    printf("    w1 (Delta NDVI):  %.2f\n", m_state.params.weightDeltaNDVI);
    printf("    w2 (VHI):         %.2f\n", m_state.params.weightVHI);
    printf("    w3 (SDP):         %.2f\n", m_state.params.weightSDP);
    printf("    w4 (NRF):         %.2f\n", m_state.params.weightNRF);
    std::cout << "\n";
    std::cout << "  Componentes:\n";
    std::cout << "  DNDVI  = Diferencia NDVI entre periodos\n";
    std::cout << "           (+ = perdida de vegetacion)\n";
    std::cout << "  VHI    = Tasa anual de declive vegetal\n";
    std::cout << "  SDP    = Presion espacial de vecinos\n";
    std::cout << "           (efecto de frontera)\n\n";
    std::cout << "  Clasificacion:\n";
    std::cout << "    0.00-0.15  Sin deforestacion\n";
    std::cout << "    0.15-0.30  Riesgo BAJO\n";
    std::cout << "    0.30-0.50  Riesgo MODERADO\n";
    std::cout << "    0.50-0.70  Riesgo ALTO\n";
    std::cout << "    0.70-1.00  Riesgo MUY ALTO (deforestacion)\n";
    waitEnter();
}

void InteractiveApp::showCurrentConfig() {
    printf("\n  Configuracion actual:\n");
    printf("  Ubicacion: %s\n", m_state.locationName.c_str());
    printf("  UTM: Zona %d%c | Este: %.0f | Norte: %.0f\n",
           m_state.utmZone, m_state.utmHemisphere,
           m_state.utmEasting, m_state.utmNorthing);
    printf("  Resolucion: %.0fm\n", m_state.cellSize);
    printf("  Periodo: %d - %d\n", m_state.yearT1, m_state.yearT2);
    printf("  Datos: %s\n", m_state.dataLoaded ? "Cargados" : "Vacio");
    printf("  Analisis: %s\n", m_state.analysisRun ? "Completado" : "Pendiente");
}

// ---------------------------------------------------------------
//  1. GENERAR DATOS SINTETICOS
// ---------------------------------------------------------------

struct SatelliteScene {
    RasterGrid red, nir, swir, blue;
};

class AmazonGenerator {
public:
    static constexpr double PI = 3.14159265358979323846;

    AmazonGenerator(int seed) : m_rng(seed), m_ndviDist(0.65, 0.12), m_noiseDist(0.0, 0.02) {}

    SatelliteScene generate(int rows, int cols, double xll, double yll,
                            double cellSize, int year, bool deforest) {
        SatelliteScene s;
        double nd = -9999.0;
        s.red  = RasterGrid(rows, cols, xll, yll, cellSize, nd);
        s.nir  = RasterGrid(rows, cols, xll, yll, cellSize, nd);
        s.swir = RasterGrid(rows, cols, xll, yll, cellSize, nd);
        s.blue = RasterGrid(rows, cols, xll, yll, cellSize, nd);

        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                double ndviVal = generatePixelNDVI(r, c, rows, cols, year, deforest);
                s.red.setValue(r, c,  std::max(0.0, 0.15 * (1.0 - ndviVal) + m_noiseDist(m_rng)));
                s.nir.setValue(r, c,  std::max(0.0, 0.40 * (1.0 + ndviVal) + m_noiseDist(m_rng)));
                s.swir.setValue(r, c, std::max(0.0, 0.20 * (1.0 - ndviVal * 0.5) + m_noiseDist(m_rng)));
                s.blue.setValue(r, c, std::max(0.0, 0.05 * (1.0 - ndviVal * 0.3) + m_noiseDist(m_rng)));
            }
        }
        return s;
    }

private:
    std::mt19937 m_rng;
    std::normal_distribution<double> m_ndviDist;
    std::normal_distribution<double> m_noiseDist;
    std::uniform_real_distribution<double> m_riverDist{0.0, 1.0};

    double generatePixelNDVI(int r, int c, int rows, int cols, int year, bool deforest) {
        double cx = double(c) / cols;
        double cy = double(r) / rows;

        double ndvi = 0.78;
        ndvi += 0.06 * std::sin(cx * 3.0 * PI + 1.2) * std::cos(cy * 2.5 * PI + 0.8);
        ndvi += 0.04 * std::sin(cx * 9.0 * PI + cy * 7.0 * PI);

        double distRiver = std::abs(std::sin(cx * 5.0 * PI + 0.5) * 0.4
                                  + std::cos(cy * 4.0 * PI) * 0.3 - 0.15);
        if (distRiver < 0.04) ndvi = 0.08 + m_riverDist(m_rng) * 0.08;

        double vega = std::sin(cx * 5.0 * PI) * std::sin(cy * 3.5 * PI);
        if (vega > 0.65) ndvi = 0.52 + m_riverDist(m_rng) * 0.12;

        double agri = std::exp(-distRiver * 12.0) * (0.4 + 0.4 * std::sin(cx * 2.5 * PI));
        if (agri > 0.35 && ndvi > 0.4) ndvi = 0.30 + m_riverDist(m_rng) * 0.20;

        if (deforest) {
            int nPatches = 8 + int(5.0 * (year - 2015) / 10.0);
            for (int p = 0; p < nPatches; ++p) {
                double pcx = 0.5 + 0.4 * std::sin(p * 2.13 + year * 0.07);
                double pcy = 0.5 + 0.4 * std::cos(p * 1.73 + year * 0.05);
                double dx = (cx - pcx) / (0.06 + 0.02 * std::sin(p * 0.7));
                double dy = (cy - pcy) / (0.05 + 0.02 * std::cos(p * 0.9));
                double d2 = dx * dx + dy * dy;
                double ang = std::atan2(dy, dx);
                double fb = 1.0 + 0.6 * std::cos(2.5 * ang + p * 0.7);
                if (d2 / fb < 1.0) {
                    double intensity = 1.0 - std::sqrt(d2 / fb);
                    ndvi = ndvi * (1.0 - intensity * 0.95)
                         + (0.08 + m_riverDist(m_rng) * 0.12) * intensity * 0.95;
                }
            }
        }

        ndvi += m_ndviDist(m_rng) - 0.65 + m_noiseDist(m_rng);
        return std::clamp(ndvi, 0.0, 1.0);
    }
};

void InteractiveApp::menuGenerateSynthetic() {
    std::cout << "\n  ===== GENERAR DATOS SINTETICOS =====\n";
    std::cout << "  Escenario: Loreto, Zona UTM 18S\n\n";

    int rows  = readInt("  Filas", 50, 2000);
    int cols  = readInt("  Columnas", 50, 2000);
    double cs = readDouble("  Resolucion (m)", 5, 500);

    m_state.yearT1 = readInt("  Ano inicial (e.g. 2015)", 1984, 2025);
    m_state.yearT2 = readInt("  Ano final (e.g. 2025)", m_state.yearT1 + 1, 2025);
    m_state.cellSize = cs;

    bool withDef = readBool("  Incluir parches de deforestacion");

    int seed = 42;

    std::cout << "\n  Generando datos...\n";
    printf("  Escena: %dx%d pix | Resolucion: %.0fm\n", rows, cols, cs);
    printf("  Periodo: %d - %d\n", m_state.yearT1, m_state.yearT2);
    double areaHa = (rows * cols * cs * cs) / 10000.0;
    printf("  Area total: %.2f ha (%.2f km2)\n", areaHa, areaHa / 100.0);

    AmazonGenerator gen(seed);
    SatelliteScene scT1 = gen.generate(rows, cols, m_state.utmEasting, m_state.utmNorthing,
                                        cs, m_state.yearT1, false);
    SatelliteScene scT2 = gen.generate(rows, cols, m_state.utmEasting, m_state.utmNorthing,
                                        cs, m_state.yearT2, withDef);

    std::cout << "\n  Calculando NDVI...\n";
    m_state.ndviT1 = SpectralIndices::computeNDVI(scT1.nir, scT1.red);
    m_state.ndviT2 = SpectralIndices::computeNDVI(scT2.nir, scT2.red);

    printf("  NDVI %d - media: %.4f | min: %.4f | max: %.4f\n",
           m_state.yearT1, m_state.ndviT1.getMean(),
           m_state.ndviT1.getMin(), m_state.ndviT1.getMax());
    printf("  NDVI %d - media: %.4f | min: %.4f | max: %.4f\n",
           m_state.yearT2, m_state.ndviT2.getMean(),
           m_state.ndviT2.getMin(), m_state.ndviT2.getMax());

    if (readBool("\n  Guardar rasters NDVI en disco")) {
        namespace fs = std::filesystem;
        if (!fs::exists(m_state.lastOutputDir)) fs::create_directories(m_state.lastOutputDir);
        m_state.ndviT1.save(m_state.lastOutputDir + "/ndvi_" + std::to_string(m_state.yearT1) + ".asc");
        m_state.ndviT2.save(m_state.lastOutputDir + "/ndvi_" + std::to_string(m_state.yearT2) + ".asc");
        std::cout << "  Rasters guardados en: " << m_state.lastOutputDir << "/\n";
    }

    m_state.dataLoaded = true;
    m_state.analysisRun = false;
    std::cout << "\n  Datos generados exitosamente!\n";
}

// ---------------------------------------------------------------
//  2. CARGAR RASTERS
// ---------------------------------------------------------------

void InteractiveApp::menuLoadRasters() {
    std::cout << "\n  ===== CARGAR RASTERS NDVI =====\n";
    std::cout << "  Formato: ASCII Grid (.asc)\n\n";

    std::string path;
    while (true) {
        path = readString("  Ruta del raster NDVI tiempo 1");
        if (std::filesystem::exists(path)) break;
        std::cout << "  Archivo no encontrado.\n";
    }

    RasterGrid r1;
    if (!r1.load(path)) {
        std::cout << "  Error al cargar raster.\n";
        waitEnter();
        return;
    }
    m_state.ndviT1 = r1;
    m_state.yearT1 = readInt("  Ano del raster T1", 1984, 2025);

    while (true) {
        path = readString("  Ruta del raster NDVI tiempo 2");
        if (std::filesystem::exists(path)) break;
        std::cout << "  Archivo no encontrado.\n";
    }

    RasterGrid r2;
    if (!r2.load(path)) {
        std::cout << "  Error al cargar raster.\n";
        waitEnter();
        return;
    }

    if (r1.getRows() != r2.getRows() || r1.getCols() != r2.getCols()) {
        std::cout << "  Error: Los rasters no tienen las mismas dimensiones.\n";
        waitEnter();
        return;
    }

    m_state.ndviT2 = r2;
    m_state.yearT2 = readInt("  Ano del raster T2", m_state.yearT1 + 1, 2025);
    m_state.cellSize = r1.getCellSize();
    m_state.dataLoaded = true;
    m_state.analysisRun = false;

    printf("\n  Rasters cargados: %dx%d pixeles\n", r1.getRows(), r1.getCols());
    printf("  Resolucion: %.0fm\n", r1.getCellSize());
    printf("  Periodo: %d - %d\n", m_state.yearT1, m_state.yearT2);
}

// ---------------------------------------------------------------
//  3. AJUSTAR PARAMETROS
// ---------------------------------------------------------------

void InteractiveApp::menuAdjustParams() {
    std::cout << "\n  ===== AJUSTAR PARAMETROS IRDA =====\n";
    printf("  Pesos actuales:\n");
    printf("    w1 (Delta NDVI):  %.2f\n", m_state.params.weightDeltaNDVI);
    printf("    w2 (VHI):         %.2f\n", m_state.params.weightVHI);
    printf("    w3 (SDP):         %.2f\n", m_state.params.weightSDP);
    printf("    w4 (NRF):         %.2f\n", m_state.params.weightNRF);
    printf("  Nota: w1+w2+w3+w4 debe sumar 1.0\n\n");

    m_state.params.weightDeltaNDVI = readDouble("  w1 (Delta NDVI)", 0.0, 1.0);
    m_state.params.weightVHI      = readDouble("  w2 (VHI)", 0.0, 1.0);
    m_state.params.weightSDP      = readDouble("  w3 (SDP)", 0.0, 1.0);
    m_state.params.weightNRF     = readDouble("  w4 (NRF)", 0.0, 1.0);

    double sum = m_state.params.weightDeltaNDVI + m_state.params.weightVHI
               + m_state.params.weightSDP + m_state.params.weightNRF;
    if (std::abs(sum - 1.0) > 0.01) {
        printf("  Los pesos suman %.2f, normalizando...\n", sum);
        m_state.params.weightDeltaNDVI /= sum;
        m_state.params.weightVHI      /= sum;
        m_state.params.weightSDP      /= sum;
        m_state.params.weightNRF     /= sum;
    }

    m_model.setParameters(m_state.params);
    if (m_state.analysisRun) m_state.analysisRun = false;

    printf("\n  Nuevos pesos:\n");
    printf("    w1: %.2f | w2: %.2f | w3: %.2f | w4: %.2f\n",
           m_state.params.weightDeltaNDVI, m_state.params.weightVHI,
           m_state.params.weightSDP, m_state.params.weightNRF);
    std::cout << "  Parametros actualizados.\n";
}

// ---------------------------------------------------------------
//  4. EJECUTAR ANALISIS
// ---------------------------------------------------------------

void InteractiveApp::menuRunAnalysis() {
    if (!m_state.dataLoaded) {
        std::cout << "\n  No hay datos cargados. Genere o cargue rasters primero.\n";
        waitEnter();
        return;
    }

    std::cout << "\n  ===== EJECUTANDO ANALISIS IRDA =====\n";
    double interval = m_state.yearT2 - m_state.yearT1;
    printf("  Periodo: %d - %d (%.0f anos)\n", m_state.yearT1, m_state.yearT2, interval);

    m_model.setParameters(m_state.params);
    m_state.lastResult = m_model.analyze(m_state.ndviT1, m_state.ndviT2, interval);
    m_state.analysisRun = true;

    std::cout << "\n  Analisis completado!\n";
}

// ---------------------------------------------------------------
//  5. VER REPORTE
// ---------------------------------------------------------------

void InteractiveApp::menuViewReport() {
    if (!m_state.analysisRun) {
        std::cout << "\n  Primero ejecute el analisis (opcion 4).\n";
        waitEnter();
        return;
    }
    printReport(m_state.lastResult, m_state.cellSize, m_state.yearT1, m_state.yearT2);
    waitEnter();
}

void InteractiveApp::printReport(const DeforestationResult& result, double cellSize,
                                  int yearT1, int yearT2) {
    std::cout << "\n";
    std::cout << "  ==========================================\n";
    printf("   REPORTE DE DEFORESTACION\n");
    printf("   %s\n", m_state.locationName.c_str());
    printf("   Periodo: %d - %d\n", yearT1, yearT2);
    printf("   Zona UTM: %d%c\n", m_state.utmZone, m_state.utmHemisphere);
    std::cout << "  ==========================================\n\n";

    double cellAreaHa = (cellSize * cellSize) / 10000.0;

    long long grandTotal = 0;
    for (int l = 0; l <= 4; ++l) {
        auto it = result.pixelCountByClass.find(l);
        if (it != result.pixelCountByClass.end()) grandTotal += it->second;
    }
    if (grandTotal == 0) grandTotal = 1;

    printf("%-38s %15s %15s %12s\n", "CLASIFICACION DE RIESGO:", "", "", "");
    printf("  -----------------------------------------\n");
    printf("  %-35s %15s %15s %12s\n", "Categoria", "Pixeles", "Area (ha)", "%");
    printf("  -----------------------------------------\n");

    long long totalPix = 0;
    for (int l = 0; l <= 4; ++l) {
        auto it = result.pixelCountByClass.find(l);
        long long cnt = (it != result.pixelCountByClass.end()) ? it->second : 0;
        totalPix += cnt;
        double ha = cnt * cellAreaHa;
        double pct = 100.0 * cnt / grandTotal;
        printf("  %-35s %15lld %15.2f %11.2f%%\n",
               DeforestationModel::riskLevelToString(static_cast<RiskLevel>(l)).c_str(),
               cnt, ha, pct);
    }

    printf("  -----------------------------------------\n");
    double totalHa = totalPix * cellAreaHa;
    printf("  %-35s %15lld %15.2f %12s\n", "AREA TOTAL ANALIZADA", totalPix, totalHa, "100%");
    printf("  %-35s %15s %15.2f %11.2f%%\n",
           "AREA DEFORESTADA (ALTO+MUY ALTO)", "",
           result.totalDeforestedAreaHa, result.percentAreaDeforested);
    printf("  -----------------------------------------\n\n");

    printf("  ESTADISTICAS DEL MODELO:\n");
    printf("    IRDA promedio:  %.4f\n", result.irdaScore.getMean());
    printf("    IRDA maximo:    %.4f\n", result.irdaScore.getMax());
    printf("    IRDA std dev:   %.4f\n", result.irdaScore.getStdDev());
    printf("    Delta NDVI avg: %.4f\n", result.deltaNDVI.getMean());
    printf("    VHI promedio:   %.4f\n", result.vhi.getMean());
    printf("    SDP promedio:   %.4f\n", result.sdp.getMean());

    printf("\n  PESOS DEL MODELO:\n");
    printf("    w1 (Delta NDVI):  %.2f\n", m_state.params.weightDeltaNDVI);
    printf("    w2 (VHI):         %.2f\n", m_state.params.weightVHI);
    printf("    w3 (SDP):         %.2f\n", m_state.params.weightSDP);
    printf("    w4 (NRF):         %.2f\n", m_state.params.weightNRF);
}

// ---------------------------------------------------------------
//  6. EXPORTAR RESULTADOS
// ---------------------------------------------------------------

void InteractiveApp::menuExportResults() {
    if (!m_state.analysisRun) {
        std::cout << "\n  Primero ejecute el analisis (opcion 4).\n";
        waitEnter();
        return;
    }

    std::string dir = readString("  Directorio de salida");
    if (dir.empty()) dir = m_state.lastOutputDir;

    m_state.lastOutputDir = dir;
    m_model.exportResults(m_state.lastResult, dir);

    // Also export NDVI rasters if not already saved
    namespace fs = std::filesystem;
    if (!fs::exists(dir + "/ndvi_" + std::to_string(m_state.yearT1) + ".asc")) {
        m_state.ndviT1.save(dir + "/ndvi_" + std::to_string(m_state.yearT1) + ".asc");
        m_state.ndviT2.save(dir + "/ndvi_" + std::to_string(m_state.yearT2) + ".asc");
        std::cout << "  - ndvi_" << m_state.yearT1 << ".asc\n";
        std::cout << "  - ndvi_" << m_state.yearT2 << ".asc\n";
    }
}

// ---------------------------------------------------------------
//  7. CAMBIAR ZONA UTM
// ---------------------------------------------------------------

void InteractiveApp::menuChangeZone() {
    std::cout << "\n  ===== CONFIGURACION UTM =====\n";
    printf("  Actual: Zona %d%c | Este: %.0f | Norte: %.0f\n",
           m_state.utmZone, m_state.utmHemisphere,
           m_state.utmEasting, m_state.utmNorthing);

    if (readBool("  Usar preset Loreto (Zona 18S)")) {
        m_state.utmZone = 18;
        m_state.utmHemisphere = 'S';
        m_state.utmEasting = 450000.0;
        m_state.utmNorthing = 9548000.0;
        m_state.locationName = "Loreto, Amazonia Baja Peruana";
        std::cout << "  Preset Loreto Zona 18S cargado.\n";
    } else {
        m_state.utmZone = readInt("  Zona UTM", 1, 60);
        std::string hem = readString("  Hemisferio (N/S)");
        m_state.utmHemisphere = (hem == "N" || hem == "n") ? 'N' : 'S';
        m_state.utmEasting = readDouble("  Coordenada Este (Easting)", 100000, 999999);
        m_state.utmNorthing = readDouble("  Coordenada Norte (Northing)", 0, 10000000);
        m_state.locationName = readString("  Nombre de ubicacion");
    }

    printf("\n  Nueva configuracion:\n");
    printf("  Zona UTM: %d%c (EPSG:32%d%d)\n",
           m_state.utmZone, m_state.utmHemisphere,
           m_state.utmHemisphere == 'S' ? 7 : 6, m_state.utmZone);
    printf("  Este: %.0f | Norte: %.0f\n", m_state.utmEasting, m_state.utmNorthing);
    printf("  Ubicacion: %s\n", m_state.locationName.c_str());
}
