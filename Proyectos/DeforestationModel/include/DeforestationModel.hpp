#ifndef DEFORESTATION_MODEL_HPP
#define DEFORESTATION_MODEL_HPP

#include "RasterGrid.hpp"
#include <string>
#include <unordered_map>

enum class RiskLevel {
    NO_DEFORESTATION = 0,
    LOW = 1,
    MODERATE = 2,
    HIGH = 3,
    VERY_HIGH = 4
};

struct IRDAParams {
    double weightDeltaNDVI = 0.35;
    double weightVHI = 0.30;
    double weightSDP = 0.25;
    double weightNRF = 0.10;
};

struct DeforestationResult {
    RasterGrid deltaNDVI;
    RasterGrid vhi;
    RasterGrid sdp;
    RasterGrid irdaScore;
    RasterGrid riskClassification;
    std::unordered_map<int, long long> pixelCountByClass;
    double totalDeforestedAreaHa;
    double percentAreaDeforested;
};

class DeforestationModel {
public:
    explicit DeforestationModel(const IRDAParams& params = IRDAParams());

    DeforestationResult analyze(const RasterGrid& ndviT1, const RasterGrid& ndviT2,
                                 double timeIntervalYears = 1.0);

    void setParameters(const IRDAParams& params);
    IRDAParams getParameters() const;

    static std::string riskLevelToString(RiskLevel level);
    static RiskLevel classifyRisk(double irdaScore);

    void exportResults(const DeforestationResult& result, const std::string& outputDir) const;

private:
    IRDAParams m_params;

    RasterGrid computeDeltaNDVI(const RasterGrid& ndviT1, const RasterGrid& ndviT2) const;
    RasterGrid computeVHI(const RasterGrid& deltaNDVI, double timeIntervalYears) const;
    RasterGrid computeSDP(const RasterGrid& deltaNDVI, int windowSize = 3) const;
    RasterGrid computeIRDAScore(const RasterGrid& deltaNDVI, const RasterGrid& vhi,
                                 const RasterGrid& sdp) const;

    double calculateAreaHa(const RasterGrid& raster, double cellSize) const;
};

#endif
