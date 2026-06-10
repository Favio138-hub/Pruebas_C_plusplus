#ifndef RASTER_GRID_HPP
#define RASTER_GRID_HPP

#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <cmath>

class RasterGrid {
public:
    RasterGrid();
    RasterGrid(int nrows, int ncols, double xll, double yll, double cellSize, double noData = -9999.0);

    bool load(const std::string& filepath);
    bool save(const std::string& filepath) const;

    double& at(int row, int col);
    const double& at(int row, int col) const;
    double getValue(int row, int col) const;
    void setValue(int row, int col, double value);

    void fill(double value);

    int getRows() const;
    int getCols() const;
    double getXLLCorner() const;
    double getYLLCorner() const;
    double getCellSize() const;
    double getNoDataValue() const;

    bool isNoData(int row, int col) const;
    bool isNoData(double value) const;

    double getMin() const;
    double getMax() const;
    double getMean() const;
    double getStdDev() const;

    std::pair<double, double> getWindowMean(int row, int col, int windowSize = 3) const;

    RasterGrid clone() const;

private:
    int m_rows;
    int m_cols;
    double m_xllcorner;
    double m_yllcorner;
    double m_cellsize;
    double m_nodata;
    std::vector<std::vector<double>> m_data;
};

#endif
