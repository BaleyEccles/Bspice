#pragma once
#include <cmath>

template<typename T>
class complexNumber {
public:
  complexNumber(T a = 0.0, T b = 0.0)
    : a(a), b(b) {}

  T a, b;

  inline double magnitude() const {
    return sqrt(pow(a, 2) + pow(b, 2));
  }

  inline double phase() const {
    return atan2(b, a);
  }

  inline void print() const {
    std::cout << a << " + " << b << "j" << std::endl;
  }

  inline complexNumber<T> operator*(const complexNumber<T>& other) const {
    return complexNumber(this->a * other.a - this->b * other.b,
                         this->a * other.b + this->b * other.a);
  }

  inline complexNumber<T> operator+(const complexNumber<T>& other) const {
    return complexNumber(this->a + other.a, this->b + other.b);
  }

  inline complexNumber<T> operator-(const complexNumber<T>& other) const {
    return complexNumber(this->a - other.a, this->b - other.b);
  }

  inline complexNumber<T> operator*(double scalar) const {
    return complexNumber<T>(this->a * scalar, this->b * scalar);
  }

  friend complexNumber<T> operator*(double scalar, const complexNumber<T>& c) {
    return complexNumber<T>(c.a * scalar, c.b * scalar);
  }

  inline complexNumber<T> operator+(double scalar) const {
    return complexNumber(this->a + scalar, this->b);
  }

  inline complexNumber<T> operator-(double scalar) const {
    return complexNumber(this->a - scalar, this->b);
  }

  complexNumber<T>& operator+=(const complexNumber<T>& other) {
    this->a += other.a;
    this->b += other.b;
    return *this;
  }

  complexNumber<T>& operator-=(const complexNumber<T>& other) {
    this->a -= other.a;
    this->b -= other.b;
    return *this;
  }

  complexNumber<T>& operator*=(const complexNumber<T>& other) {
    T new_a = this->a * other.a - this->b * other.b;
    T new_b = this->a * other.b + this->b * other.a;
    this->a = new_a;
    this->b = new_b;
    return *this;
  }

  complexNumber<T>& operator+=(double scalar) {
    this->a += scalar;
    return *this;
  }

  complexNumber<T>& operator-=(double scalar) {
    this->a -= scalar;
    return *this;
  }

  complexNumber<T>& operator*=(double scalar) {
    this->a *= scalar;
    this->b *= scalar;
    return *this;
  }
};

template<typename T>
complexNumber<T> makeComplexNumberFromPolar(double r, double phi) {
  return complexNumber<T>(r*cos(phi), r*sin(phi));
}

#include "complexNumbers_impl.h"
