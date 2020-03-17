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

/**
 * @brief Enumeration for the FIR generation method
 */
enum class FirMethod {
    FirLS,          /*!< Generate by firls octave function */
    Fir1,           /*!< Generate by fir1 octave function */
};

/**
 * @brief Encapsulate a FIR filter model
 */
class Fir {
public:
    /**
     * @brief Constructor
     *
     * @param method The method used to generate the filter
     * @param cardC The number of coefficients
     * @param piC The size of coefficients
     * @param noiseLevel The filter rejection
     */
    Fir(FirMethod method, std::uint16_t cardC, std::uint16_t piC, double noiseLevel);

    /**
     * @brief Get the size of coeffcients
     */
    std::uint64_t getPiC() const;

    /**
     * @brief Get the number of coefficients
     */
    std::uint64_t getCardC() const;

    /**
     * @brief Get the rejection
     */
    double getNoiseLevel() const;

    /**
     * @brief Get the generation method
     */
    FirMethod getMethod() const;

    /**
     * @brief Get the number of bit added by the FIR
     */
    std::int64_t getPiFir() const;

    /**
     * @brief Get the path for the filter
     */
    std::string getFilterName() const;

public:
    /**
     * @brief Stream operator to write the filter
     *
     * @param os Output stream
     * @param fir The FIR filter
     */
    friend std::ostream& operator<<(std::ostream& os, const Fir& fir);

private:
    const FirMethod m_method;
    const std::uint16_t m_cardC;
    const std::uint16_t m_piC;
    const double m_noiseLevel;
};

#endif // FIR_H
