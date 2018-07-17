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
    std::int64_t getPiOut(std::int64_t piIn) const;

public:
    friend std::ostream& operator<<(std::ostream& os, const Fir& fir);

private:
    const FirMethod m_method;
    const std::uint16_t m_cardC;
    const std::uint16_t m_piC;
    const double m_noiseLevel;
};

#endif // FIR_H
