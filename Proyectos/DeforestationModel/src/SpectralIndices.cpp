#include "SpectralIndices.hpp"

RasterGrid SpectralIndices::computeNDVI(const RasterGrid& nir, const RasterGrid& red) {
    if (nir.getRows() != red.getRows() || nir.getCols() != red.getCols()) {
        throw std::invalid_argument("Las dimensiones de las bandas NIR y RED no coinciden");
    }

    RasterGrid ndvi(nir.getRows(), nir.getCols(),
                     nir.getXLLCorner(), nir.getYLLCorner(),
                     nir.getCellSize(), nir.getNoDataValue());

    double noData = nir.getNoDataValue();

    for (int r = 0; r < nir.getRows(); ++r) {
        for (int c = 0; c < nir.getCols(); ++c) {
            double nirVal = nir.getValue(r, c);
            double redVal = red.getValue(r, c);

            if (nir.isNoData(nirVal) || red.isNoData(redVal)) {
                ndvi.setValue(r, c, noData);
                continue;
            }

            double denominator = nirVal + redVal;
            if (std::abs(denominator) < 1e-10) {
                ndvi.setValue(r, c, noData);
            } else {
                double ndviVal = (nirVal - redVal) / denominator;
                ndvi.setValue(r, c, ndviVal);
            }
        }
    }
    return ndvi;
}

RasterGrid SpectralIndices::computeEVI(const RasterGrid& nir, const RasterGrid& red,
                                         const RasterGrid& blue, double G, double C1, double C2, double L) {
    if (nir.getRows() != red.getRows() || nir.getCols() != red.getCols()) {
        throw std::invalid_argument("Dimensiones de bandas no coinciden para EVI");
    }

    RasterGrid evi(nir.getRows(), nir.getCols(),
                    nir.getXLLCorner(), nir.getYLLCorner(),
                    nir.getCellSize(), nir.getNoDataValue());
    double noData = nir.getNoDataValue();

    for (int r = 0; r < nir.getRows(); ++r) {
        for (int c = 0; c < nir.getCols(); ++c) {
            double nirVal = nir.getValue(r, c);
            double redVal = red.getValue(r, c);
            double blueVal = blue.getValue(r, c);

            if (nir.isNoData(nirVal) || red.isNoData(redVal) || blue.isNoData(blueVal)) {
                evi.setValue(r, c, noData);
                continue;
            }

            double denominator = nirVal + C1 * redVal - C2 * blueVal + L;
            if (std::abs(denominator) < 1e-10) {
                evi.setValue(r, c, noData);
            } else {
                double eviVal = G * (nirVal - redVal) / denominator;
                evi.setValue(r, c, eviVal);
            }
        }
    }
    return evi;
}

RasterGrid SpectralIndices::computeNDMI(const RasterGrid& nir, const RasterGrid& swir) {
    if (nir.getRows() != swir.getRows() || nir.getCols() != swir.getCols()) {
        throw std::invalid_argument("Dimensiones de bandas no coinciden para NDMI");
    }

    RasterGrid ndmi(nir.getRows(), nir.getCols(),
                     nir.getXLLCorner(), nir.getYLLCorner(),
                     nir.getCellSize(), nir.getNoDataValue());
    double noData = nir.getNoDataValue();

    for (int r = 0; r < nir.getRows(); ++r) {
        for (int c = 0; c < nir.getCols(); ++c) {
            double nirVal = nir.getValue(r, c);
            double swirVal = swir.getValue(r, c);

            if (nir.isNoData(nirVal) || swir.isNoData(swirVal)) {
                ndmi.setValue(r, c, noData);
                continue;
            }

            double denominator = nirVal + swirVal;
            if (std::abs(denominator) < 1e-10) {
                ndmi.setValue(r, c, noData);
            } else {
                ndmi.setValue(r, c, (nirVal - swirVal) / denominator);
            }
        }
    }
    return ndmi;
}

RasterGrid SpectralIndices::temporalDifference(const RasterGrid& rasterT1, const RasterGrid& rasterT2) {
    if (rasterT1.getRows() != rasterT2.getRows() || rasterT1.getCols() != rasterT2.getCols()) {
        throw std::invalid_argument("Las dimensiones temporales no coinciden");
    }

    RasterGrid diff(rasterT1.getRows(), rasterT1.getCols(),
                     rasterT1.getXLLCorner(), rasterT1.getYLLCorner(),
                     rasterT1.getCellSize(), rasterT1.getNoDataValue());
    double noData = rasterT1.getNoDataValue();

    for (int r = 0; r < rasterT1.getRows(); ++r) {
        for (int c = 0; c < rasterT1.getCols(); ++c) {
            double v1 = rasterT1.getValue(r, c);
            double v2 = rasterT2.getValue(r, c);

            if (rasterT1.isNoData(v1) || rasterT2.isNoData(v2)) {
                diff.setValue(r, c, noData);
            } else {
                diff.setValue(r, c, v1 - v2);
            }
        }
    }
    return diff;
}

RasterGrid SpectralIndices::normalize(const RasterGrid& raster) {
    double minVal = raster.getMin();
    double maxVal = raster.getMax();
    double noData = raster.getNoDataValue();

    RasterGrid normalized(raster.getRows(), raster.getCols(),
                           raster.getXLLCorner(), raster.getYLLCorner(),
                           raster.getCellSize(), noData);

    double range = maxVal - minVal;

    for (int r = 0; r < raster.getRows(); ++r) {
        for (int c = 0; c < raster.getCols(); ++c) {
            double val = raster.getValue(r, c);
            if (raster.isNoData(val)) {
                normalized.setValue(r, c, noData);
            } else if (range < 1e-10) {
                normalized.setValue(r, c, 0.0);
            } else {
                normalized.setValue(r, c, (val - minVal) / range);
            }
        }
    }
    return normalized;
}

RasterGrid SpectralIndices::applyThreshold(const RasterGrid& raster, double threshold, bool above) {
    RasterGrid result(raster.getRows(), raster.getCols(),
                       raster.getXLLCorner(), raster.getYLLCorner(),
                       raster.getCellSize(), raster.getNoDataValue());
    double noData = raster.getNoDataValue();

    for (int r = 0; r < raster.getRows(); ++r) {
        for (int c = 0; c < raster.getCols(); ++c) {
            double val = raster.getValue(r, c);
            if (raster.isNoData(val)) {
                result.setValue(r, c, noData);
            } else if (above) {
                result.setValue(r, c, (val >= threshold) ? 1.0 : 0.0);
            } else {
                result.setValue(r, c, (val <= threshold) ? 1.0 : 0.0);
            }
        }
    }
    return result;
}
