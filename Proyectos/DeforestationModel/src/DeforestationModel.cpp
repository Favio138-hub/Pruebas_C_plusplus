#include "DeforestationModel.hpp"
#include "SpectralIndices.hpp"
#include <iostream>
#include <cstdio>
#include <filesystem>
#include <algorithm>
#include <vector>

DeforestationModel::DeforestationModel(const IRDAParams& params)
    : m_params(params) {}

void DeforestationModel::setParameters(const IRDAParams& params) {
    m_params = params;
}

IRDAParams DeforestationModel::getParameters() const {
    return m_params;
}

DeforestationResult DeforestationModel::analyze(const RasterGrid& ndviT1,
                                                  const RasterGrid& ndviT2,
                                                  double timeIntervalYears) {
    if (ndviT1.getRows() != ndviT2.getRows() || ndviT1.getCols() != ndviT2.getCols()) {
        throw std::invalid_argument("Los rasters NDVI deben tener las mismas dimensiones");
    }

    DeforestationResult result;

    std::cout << "\n==========================================\n";
    std::cout << "  MODELO IRDA - INDICE DE RIESGO DE\n";
    std::cout << "  DEFORESTACION EN AMAZONIA BAJA\n";
    std::cout << "==========================================\n\n";

    // Paso 1: Delta NDVI (diferencia temporal)
    std::cout << "[1/4] Calculando Delta NDVI..." << std::endl;
    result.deltaNDVI = computeDeltaNDVI(ndviT1, ndviT2);
    std::cout << "       Delta NDVI min: " << result.deltaNDVI.getMin()
              << " max: " << result.deltaNDVI.getMax() << std::endl;

    // Paso 2: VHI - Vegetation Health Index (tasa de declive)
    std::cout << "[2/4] Calculando VHI (Vegetation Health Index)..." << std::endl;
    result.vhi = computeVHI(result.deltaNDVI, timeIntervalYears);
    std::cout << "       VHI min: " << result.vhi.getMin()
              << " max: " << result.vhi.getMax() << std::endl;

    // Paso 3: SDP - Spatial Deforestation Pressure
    std::cout << "[3/4] Calculando SDP (Spatial Deforestation Pressure)..." << std::endl;
    result.sdp = computeSDP(result.deltaNDVI);
    std::cout << "       SDP min: " << result.sdp.getMin()
              << " max: " << result.sdp.getMax() << std::endl;

    // Paso 4: IRDA Score compuesto
    std::cout << "[4/4] Computando IRDA Score final..." << std::endl;
    result.irdaScore = computeIRDAScore(result.deltaNDVI, result.vhi, result.sdp);
    std::cout << "       IRDA min: " << result.irdaScore.getMin()
              << " max: " << result.irdaScore.getMax() << std::endl;

    // Clasificacion de riesgo
    std::cout << "\nClasificando niveles de riesgo..." << std::endl;
    int rows = result.irdaScore.getRows();
    int cols = result.irdaScore.getCols();
    double noData = result.irdaScore.getNoDataValue();

    result.riskClassification = RasterGrid(rows, cols,
        result.irdaScore.getXLLCorner(), result.irdaScore.getYLLCorner(),
        result.irdaScore.getCellSize(), noData);

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            double score = result.irdaScore.getValue(r, c);
            if (result.irdaScore.isNoData(score)) {
                result.riskClassification.setValue(r, c, noData);
                continue;
            }

            RiskLevel level = classifyRisk(score);
            result.riskClassification.setValue(r, c, static_cast<double>(level));
            result.pixelCountByClass[static_cast<int>(level)]++;
        }
    }

    // Calculo de areas
    double cellSize = result.irdaScore.getCellSize();
    double cellAreaHa = (cellSize * cellSize) / 10000.0; // m2 a hectareas

    long long totalValidPixels = 0;
    long long deforestedPixels = 0;

    for (auto& [classId, count] : result.pixelCountByClass) {
        totalValidPixels += count;
        if (classId >= static_cast<int>(RiskLevel::HIGH)) {
            deforestedPixels += count;
        }
    }

    result.totalDeforestedAreaHa = deforestedPixels * cellAreaHa;
    result.percentAreaDeforested = totalValidPixels > 0 ?
        (100.0 * deforestedPixels / totalValidPixels) : 0.0;

    return result;
}

RasterGrid DeforestationModel::computeDeltaNDVI(const RasterGrid& ndviT1,
                                                  const RasterGrid& ndviT2) const {
    return SpectralIndices::temporalDifference(ndviT1, ndviT2);
}

RasterGrid DeforestationModel::computeVHI(const RasterGrid& deltaNDVI,
                                            double timeIntervalYears) const {
    int rows = deltaNDVI.getRows();
    int cols = deltaNDVI.getCols();
    double noData = deltaNDVI.getNoDataValue();

    RasterGrid vhi(rows, cols,
                    deltaNDVI.getXLLCorner(), deltaNDVI.getYLLCorner(),
                    deltaNDVI.getCellSize(), noData);

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            double delta = deltaNDVI.getValue(r, c);
            if (deltaNDVI.isNoData(delta)) {
                vhi.setValue(r, c, noData);
            } else {
                // VHI = tasa anual de cambio de NDVI
                // Valores positivos = perdida de vegetacion acelerada
                double vhiVal = delta / timeIntervalYears;
                vhi.setValue(r, c, std::max(0.0, vhiVal));
            }
        }
    }
    return vhi;
}

RasterGrid DeforestationModel::computeSDP(const RasterGrid& deltaNDVI, int windowSize) const {
    int rows = deltaNDVI.getRows();
    int cols = deltaNDVI.getCols();
    double noData = deltaNDVI.getNoDataValue();

    RasterGrid sdp(rows, cols,
                    deltaNDVI.getXLLCorner(), deltaNDVI.getYLLCorner(),
                    deltaNDVI.getCellSize(), noData);

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            double centerVal = deltaNDVI.getValue(r, c);
            if (deltaNDVI.isNoData(centerVal)) {
                sdp.setValue(r, c, noData);
                continue;
            }

            auto [windowMean, count] = deltaNDVI.getWindowMean(r, c, windowSize);
            if (deltaNDVI.isNoData(windowMean)) {
                sdp.setValue(r, c, noData);
            } else {
                // SDP: presion espacial de deforestacion
                // Si vecinos tienen mayor perdida que el pixel central => presion alta
                double sdpVal = windowMean - centerVal;
                sdp.setValue(r, c, std::max(0.0, sdpVal));
            }
        }
    }
    return sdp;
}

RasterGrid DeforestationModel::computeIRDAScore(const RasterGrid& deltaNDVI,
                                                  const RasterGrid& vhi,
                                                  const RasterGrid& sdp) const {
    int rows = deltaNDVI.getRows();
    int cols = deltaNDVI.getCols();
    double noData = deltaNDVI.getNoDataValue();
    double w1 = m_params.weightDeltaNDVI;
    double w2 = m_params.weightVHI;
    double w3 = m_params.weightSDP;

    std::vector<double> rawScores;
    rawScores.reserve(rows * cols);

    RasterGrid irda(rows, cols,
                     deltaNDVI.getXLLCorner(), deltaNDVI.getYLLCorner(),
                     deltaNDVI.getCellSize(), noData);

    double maxScore = 0.0;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            double dNDVI = deltaNDVI.getValue(r, c);
            double vhiVal = vhi.getValue(r, c);
            double sdpVal = sdp.getValue(r, c);

            if (deltaNDVI.isNoData(dNDVI) || vhi.isNoData(vhiVal) || sdp.isNoData(sdpVal)) {
                irda.setValue(r, c, noData);
                continue;
            }

            double score = w1 * dNDVI + w2 * vhiVal + w3 * sdpVal;
            rawScores.push_back(score);
            irda.setValue(r, c, 0.0);
            if (score > maxScore) maxScore = score;
        }
    }

    if (maxScore < 1e-10) maxScore = 1.0;

    size_t idx = 0;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (!irda.isNoData(r, c)) {
                double normalized = std::clamp(rawScores[idx] / maxScore, 0.0, 1.0);
                irda.setValue(r, c, normalized);
                ++idx;
            }
        }
    }

    return irda;
}

RiskLevel DeforestationModel::classifyRisk(double irdaScore) {
    if (irdaScore < 0.15) return RiskLevel::NO_DEFORESTATION;
    if (irdaScore < 0.30) return RiskLevel::LOW;
    if (irdaScore < 0.50) return RiskLevel::MODERATE;
    if (irdaScore < 0.70) return RiskLevel::HIGH;
    return RiskLevel::VERY_HIGH;
}

std::string DeforestationModel::riskLevelToString(RiskLevel level) {
    switch (level) {
        case RiskLevel::NO_DEFORESTATION: return "SIN DEFORESTACION";
        case RiskLevel::LOW:              return "RIESGO BAJO";
        case RiskLevel::MODERATE:         return "RIESGO MODERADO";
        case RiskLevel::HIGH:             return "RIESGO ALTO";
        case RiskLevel::VERY_HIGH:        return "RIESGO MUY ALTO (DEFORESTACION)";
        default:                          return "DESCONOCIDO";
    }
}

void DeforestationModel::exportResults(const DeforestationResult& result,
                                        const std::string& outputDir) const {
    namespace fs = std::filesystem;

    if (!fs::exists(outputDir)) {
        fs::create_directories(outputDir);
    }

    result.deltaNDVI.save(outputDir + "/delta_ndvi.asc");
    result.vhi.save(outputDir + "/vhi.asc");
    result.sdp.save(outputDir + "/sdp.asc");
    result.irdaScore.save(outputDir + "/irda_score.asc");
    result.riskClassification.save(outputDir + "/riesgo_clasificacion.asc");

    std::cout << "\nArchivos generados en: " << outputDir << std::endl;
    std::cout << "  - delta_ndvi.asc        (Cambio de NDVI)" << std::endl;
    std::cout << "  - vhi.asc               (Indice de Salud Vegetal)" << std::endl;
    std::cout << "  - sdp.asc               (Presion Espacial de Deforestacion)" << std::endl;
    std::cout << "  - irda_score.asc        (Score IRDA continuo)" << std::endl;
    std::cout << "  - riesgo_clasificacion.asc (Clasificacion de riesgo)" << std::endl;
}

double DeforestationModel::calculateAreaHa(const RasterGrid& raster, double cellSize) const {
    double cellAreaHa = (cellSize * cellSize) / 10000.0;
    long long count = 0;
    double noData = raster.getNoDataValue();

    for (int r = 0; r < raster.getRows(); ++r) {
        for (int c = 0; c < raster.getCols(); ++c) {
            if (!raster.isNoData(r, c)) {
                ++count;
            }
        }
    }
    return count * cellAreaHa;
}
