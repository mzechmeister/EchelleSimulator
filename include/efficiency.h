/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2016  Julian Stürmer <julian.stuermer@online.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef EFFICIENCY_H
#define EFFICIENCY_H

#include <string>
#include <vector>
#include <map>

class Efficiency {
public:
    Efficiency();

    virtual ~Efficiency();

    virtual std::vector<double> get_efficiency(int order, std::vector<double> &wavelength);

    virtual std::vector<double> get_efficiency(int order, std::vector<double> &wavelength, int N);

private:

};

class ConstantEfficiency : public Efficiency {
public:
    ConstantEfficiency(double efficiency);

    std::vector<double> get_efficiency(int order, std::vector<double> &wavelength);

    std::vector<double> get_efficiency(int order, std::vector<double> &wavelength, int N);

private:
    double eff;
};

/**
 * \class GratingEfficiency
 * \brief implements the efficiency curve of a echelle grating based on theory
 *
 */
class GratingEfficiency : public Efficiency {
public:
    /**
     * Constructor.
     * @param peak_efficiency peak efficiency of the grating
     * @param alpha alpha angle
     * @param blaze blaze angle
     * @param gpmm grooves per mm
     */
    GratingEfficiency(double peak_efficiency, double alpha, double blaze, double gpmm);

    std::vector<double> get_efficiency(int order, std::vector<double> &wavelength);

    std::vector<double> get_efficiency(int order, std::vector<double> &wavelength, int N);

private:
    double peak_efficiency;
    double alpha;
    double blaze;
    double gpmm;

    double calc_eff(double scalingfactor, int order, double alpha, double blaze, double wl, double n);

};


class CSVEfficiency : public Efficiency {
public:
    CSVEfficiency(std::string path);

    std::vector<double> get_efficiency(int order, std::vector<double> &wavelength);

    std::vector<double> get_efficiency(int order, std::vector<double> &wavelength, int N);

private:
    std::vector<double> wl;
    std::vector<double> ef;
    std::map<double, double> data;
};

#endif // EFFICIENCY_H
