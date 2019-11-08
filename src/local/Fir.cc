/* Cascade filters solver - Solver to choose the best configuration to design a
 * cascade of filters
 * Copyright (C) 2019  Arthur HUGEAT
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>
 */

#include "Fir.h"

#include <cmath>

Fir::Fir(FirMethod method, std::uint16_t cardC, std::uint16_t piC, double noiseLevel)
: m_method(method)
, m_cardC(cardC)
, m_piC(piC)
, m_noiseLevel(noiseLevel) {
    // ctor
}

std::size_t Fir::getPiC() const {
    return m_piC;
}

std::size_t Fir::getCardC() const {
    return m_cardC;
}

double Fir::getNoiseLevel() const {
    return m_noiseLevel;
}

FirMethod Fir::getMethod() const {
    return m_method;
}

std::int64_t Fir::getPiFir() const {
    return m_piC;
}

std::string Fir::getFilterName() const {
    char filterName[128] = {0};
    switch (getMethod()) {
    case FirMethod::Fir1:
        std::snprintf(filterName, 128, "filters/fir1/fir1_%03lu_int%02lu", getCardC(), getPiC());
        break;
    case FirMethod::FirLS:
        std::snprintf(filterName, 128, "filters/firls/firls_%03lu_int%02lu", getCardC(), getPiC());
        break;
    }

    return std::string(filterName);
}

std::ostream& operator<<(std::ostream& os, const Fir& fir) {
    std::string methodString;

    switch (fir.m_method) {
    case FirMethod::FirLS:
        methodString = "FirLS";
        break;
    case FirMethod::Fir1:
        methodString = "Fir1";
        break;
    }

    os << "fir('" << methodString << "', C:" << fir.m_cardC << ", PiC:" << fir.m_piC << ", " << fir.m_noiseLevel << " dB, PiFir: " << fir.getPiFir() << " bit)";
    return os;
}
