#ifndef INTERACTIVE_APP_HPP
#define INTERACTIVE_APP_HPP

#include "RasterGrid.hpp"
#include "SpectralIndices.hpp"
#include "DeforestationModel.hpp"
#include <string>
#include <memory>

struct SessionState {
    RasterGrid ndviT1;
    RasterGrid ndviT2;
    int yearT1 = 2015;
    int yearT2 = 2025;
    bool dataLoaded = false;
    bool analysisRun = false;
    IRDAParams params;
    DeforestationResult lastResult;
    std::string lastOutputDir = "output";
    std::string locationName = "Loreto, Amazonia Baja Peruana";
    double utmEasting = 450000.0;
    double utmNorthing = 9548000.0;
    int utmZone = 18;
    char utmHemisphere = 'S';
    double cellSize = 30.0;
};

class InteractiveApp {
public:
    InteractiveApp();
    void run();

private:
    SessionState m_state;
    DeforestationModel m_model;

    void showSplash();
    void showMainMenu();
    void showAbout();
    void showModelInfo();
    void showCurrentConfig();
    int readInt(const std::string& prompt, int min, int max);
    double readDouble(const std::string& prompt, double min, double max);
    std::string readString(const std::string& prompt);
    bool readBool(const std::string& prompt);
    void waitEnter();

    void menuGenerateSynthetic();
    void menuLoadRasters();
    void menuAdjustParams();
    void menuRunAnalysis();
    void menuViewReport();
    void menuExportResults();
    void menuChangeZone();

    void printReport(const DeforestationResult& result, double cellSize,
                     int yearT1, int yearT2);
};

#endif
