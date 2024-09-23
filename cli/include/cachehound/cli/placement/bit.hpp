#ifndef CACHEHOUND_CLI_PLACEMENT_BIT_HPP
#define CACHEHOUND_CLI_PLACEMENT_BIT_HPP

#include <cassert>
#include <Eigen/Eigen>
#include <ostream>

namespace cachehound::cli {

class bit {
    bool value_;

public:
    bit() : bit(false) {}
    bit(bool value) : value_(value) {}
    operator bool() const { return value_; }

    bit& operator+=(bit other) { value_ ^= other.value_; return *this; };
    bit& operator-=(bit other) { value_ ^= other.value_; return *this; };
    bit& operator*=(bit other) { value_ = value_ && other.value_; return *this; };

    bit operator+(bit other) const { return other.value_ ^ value_; }
    bit operator-(bit other) const { return other.value_ ^ value_; }
    bit operator*(bit other) const { return other.value_ && value_; }

    bool operator==(bit other) const { return value_ == other.value_; }
    bool operator!=(bit other) const { return value_ != other.value_; }

    friend std::ostream& operator<<(std::ostream& os, bit b) {
        return os << int(b.value_);
    }
};

}

namespace Eigen {

template<> struct NumTraits<cachehound::cli::bit>
{
  typedef cachehound::cli::bit Real;
  typedef cachehound::cli::bit Nested;
  enum {
    IsComplex = 0,
    IsInteger = 1,
    IsSigned = 0,
    RequireInitialization = 0,
    ReadCost = 1,
    AddCost = 2,
    MulCost = 2
  };
  static Real epsilon() { return 1; }
  static int digits10() { return 0; }
};

}

#endif /* CACHEHOUND_CLI_PLACEMENT_BIT_HPP */
