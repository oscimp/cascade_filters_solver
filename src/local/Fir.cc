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
    return m_piC + 0.5 * std::log2(m_cardC);
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
