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

#ifndef FIR_H
#define FIR_H

#include <cstdint>
#include <fstream>

enum class FirMethod {
    FirLS,
    Fir1,
};

class Fir {
public:
    Fir(FirMethod method, std::uint16_t cardC, std::uint16_t piC, double noiseLevel);

    std::uint64_t getPiC() const;
    std::uint64_t getCardC() const;
    double getNoiseLevel() const;
    FirMethod getMethod() const;
    std::int64_t getPiFir() const;
    std::string getFilterName() const;

public:
    friend std::ostream& operator<<(std::ostream& os, const Fir& fir);

private:
    const FirMethod m_method;
    const std::uint16_t m_cardC;
    const std::uint16_t m_piC;
    const double m_noiseLevel;
};

#endif // FIR_H
