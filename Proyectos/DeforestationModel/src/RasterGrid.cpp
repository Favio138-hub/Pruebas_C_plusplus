#include "RasterGrid.hpp"
#include <algorithm>
#include <iostream>
#include <limits>

RasterGrid::RasterGrid()
    : m_rows(0), m_cols(0), m_xllcorner(0.0), m_yllcorner(0.0),
      m_cellsize(1.0), m_nodata(-9999.0) {}

RasterGrid::RasterGrid(int nrows, int ncols, double xll, double yll, double cellSize, double noData)
    : m_rows(nrows), m_cols(ncols), m_xllcorner(xll), m_yllcorner(yll),
      m_cellsize(cellSize), m_nodata(noData) {
    m_data.resize(m_rows, std::vector<double>(m_cols, m_nodata));
}

bool RasterGrid::load(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: no se pudo abrir " << filepath << std::endl;
        return false;
    }

    std::string keyword;
    file >> keyword;
    if (keyword != "ncols" && keyword != "NCOLS") {
        std::cerr << "Error: formato ASCII grid invalido (se esperaba ncols)" << std::endl;
        return false;
    }
    file >> m_cols;

    file >> keyword; file >> m_rows;
    file >> keyword; file >> m_xllcorner;
    file >> keyword; file >> m_yllcorner;
    file >> keyword; file >> m_cellsize;
    file >> keyword; file >> m_nodata;

    m_data.resize(m_rows, std::vector<double>(m_cols));
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            file >> m_data[r][c];
        }
    }

    file.close();
    return true;
}

bool RasterGrid::save(const std::string& filepath) const {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: no se pudo escribir " << filepath << std::endl;
        return false;
    }

    file << "ncols         " << m_cols << std::endl;
    file << "nrows         " << m_rows << std::endl;
    file << "xllcorner     " << m_xllcorner << std::endl;
    file << "yllcorner     " << m_yllcorner << std::endl;
    file << "cellsize      " << m_cellsize << std::endl;
    file << "NODATA_value  " << m_nodata << std::endl;

    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            file << m_data[r][c];
            if (c < m_cols - 1) file << " ";
        }
        if (r < m_rows - 1) file << std::endl;
    }

    file.close();
    return true;
}

double& RasterGrid::at(int row, int col) {
    if (row < 0 || row >= m_rows || col < 0 || col >= m_cols)
        throw std::out_of_range("RasterGrid indices fuera de rango");
    return m_data[row][col];
}

const double& RasterGrid::at(int row, int col) const {
    if (row < 0 || row >= m_rows || col < 0 || col >= m_cols)
        throw std::out_of_range("RasterGrid indices fuera de rango");
    return m_data[row][col];
}

double RasterGrid::getValue(int row, int col) const {
    return at(row, col);
}

void RasterGrid::setValue(int row, int col, double value) {
    at(row, col) = value;
}

void RasterGrid::fill(double value) {
    for (int r = 0; r < m_rows; ++r)
        for (int c = 0; c < m_cols; ++c)
            m_data[r][c] = value;
}

int RasterGrid::getRows() const { return m_rows; }
int RasterGrid::getCols() const { return m_cols; }
double RasterGrid::getXLLCorner() const { return m_xllcorner; }
double RasterGrid::getYLLCorner() const { return m_yllcorner; }
double RasterGrid::getCellSize() const { return m_cellsize; }
double RasterGrid::getNoDataValue() const { return m_nodata; }

bool RasterGrid::isNoData(int row, int col) const {
    return std::abs(m_data[row][col] - m_nodata) < 1e-6;
}

bool RasterGrid::isNoData(double value) const {
    return std::abs(value - m_nodata) < 1e-6;
}

double RasterGrid::getMin() const {
    double minVal = std::numeric_limits<double>::max();
    for (int r = 0; r < m_rows; ++r)
        for (int c = 0; c < m_cols; ++c)
            if (!isNoData(r, c) && m_data[r][c] < minVal)
                minVal = m_data[r][c];
    return minVal;
}

double RasterGrid::getMax() const {
    double maxVal = std::numeric_limits<double>::lowest();
    for (int r = 0; r < m_rows; ++r)
        for (int c = 0; c < m_cols; ++c)
            if (!isNoData(r, c) && m_data[r][c] > maxVal)
                maxVal = m_data[r][c];
    return maxVal;
}

double RasterGrid::getMean() const {
    double sum = 0.0;
    long long count = 0;
    for (int r = 0; r < m_rows; ++r)
        for (int c = 0; c < m_cols; ++c)
            if (!isNoData(r, c)) {
                sum += m_data[r][c];
                ++count;
            }
    return (count > 0) ? sum / count : m_nodata;
}

double RasterGrid::getStdDev() const {
    double mean = getMean();
    if (isNoData(mean)) return m_nodata;

    double sumSq = 0.0;
    long long count = 0;
    for (int r = 0; r < m_rows; ++r)
        for (int c = 0; c < m_cols; ++c)
            if (!isNoData(r, c)) {
                sumSq += (m_data[r][c] - mean) * (m_data[r][c] - mean);
                ++count;
            }
    return (count > 1) ? std::sqrt(sumSq / (count - 1)) : 0.0;
}

std::pair<double, double> RasterGrid::getWindowMean(int row, int col, int windowSize) const {
    int half = windowSize / 2;
    double sum = 0.0;
    int count = 0;

    for (int dr = -half; dr <= half; ++dr) {
        for (int dc = -half; dc <= half; ++dc) {
            int nr = row + dr;
            int nc = col + dc;
            if (nr >= 0 && nr < m_rows && nc >= 0 && nc < m_cols) {
                if (!isNoData(nr, nc)) {
                    sum += m_data[nr][nc];
                    ++count;
                }
            }
        }
    }
    double mean = (count > 0) ? sum / count : m_nodata;
    return {mean, static_cast<double>(count)};
}

RasterGrid RasterGrid::clone() const {
    RasterGrid copy(m_rows, m_cols, m_xllcorner, m_yllcorner, m_cellsize, m_nodata);
    for (int r = 0; r < m_rows; ++r)
        for (int c = 0; c < m_cols; ++c)
            copy.m_data[r][c] = m_data[r][c];
    return copy;
}
