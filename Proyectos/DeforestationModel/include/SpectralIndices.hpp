#ifndef SPECTRAL_INDICES_HPP
#define SPECTRAL_INDICES_HPP

#include "RasterGrid.hpp"

class SpectralIndices {
public:
    static RasterGrid computeNDVI(const RasterGrid& nir, const RasterGrid& red);

    static RasterGrid computeEVI(const RasterGrid& nir, const RasterGrid& red, const RasterGrid& blue,
                                  double G = 2.5, double C1 = 6.0, double C2 = 7.5, double L = 1.0);

    static RasterGrid computeNDMI(const RasterGrid& nir, const RasterGrid& swir);

    static RasterGrid temporalDifference(const RasterGrid& rasterT1, const RasterGrid& rasterT2);

    static RasterGrid normalize(const RasterGrid& raster);

    static RasterGrid applyThreshold(const RasterGrid& raster, double threshold, bool above = true);
};

#endif
