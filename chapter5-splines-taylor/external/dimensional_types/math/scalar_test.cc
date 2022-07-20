/**
 * Copyright (c) 2016 Eric Bruneton
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "math/scalar.h"

#include <string>

#include "test/test_case.h"

namespace dimensional {

class ScalarTest : public TestCase {
 public:
  template<typename T>
  ScalarTest(const std::string& name, T test)
      : TestCase("ScalarTest " + name, static_cast<Test>(test)) {}

  void TestScalars() {
    Scalar<1, 2, 3, 4, 5> a = Scalar<1, 2, 3, 4, 5>::Unit();
    Scalar<1, 2, 3, 4, 5> b = 2.0 * a;
    Scalar<1, 2, 3, 4, 5> c = a + b;
    Scalar<1, 2, 3, 4, 5> d = c - b;
    Scalar<10, 20, 30, 40, 50> k = 4.0 * Scalar<10, 20, 30, 40, 50>::Unit();
    Scalar<11, 22, 33, 44, 55> kc = k * c;
    Scalar<11, 22, 33, 44, 55> ck = c * k;
    Scalar<1, 2, 3, 4, 5> e = c * 3.0;
    Scalar<9, 18, 27, 36, 45> koc = k / c;
    Scalar<-9, -18, -27, -36, -45> cok = c / k;
    Scalar<1, 2, 3, 4, 5> f = e / 3.0;
    f += a;
    Scalar<1, 2, 3, 4, 5> g = -e;
    Scalar<-1, -2, -3, -4, -5> h = 9.0 / g;
    ExpectEquals(2.0, b.to(a));
    ExpectEquals(3.0, c.to(a));
    ExpectEquals(1.0, d.to(a));
    ExpectEquals(12.0, kc.to(Scalar<11, 22, 33, 44, 55>::Unit()));
    ExpectEquals(12.0, ck.to(Scalar<11, 22, 33, 44, 55>::Unit()));
    ExpectEquals(9.0, e.to(a));
    ExpectEquals(4.0 / 3.0, koc.to(Scalar<9, 18, 27, 36, 45>::Unit()));
    ExpectEquals(3.0 / 4.0, cok.to(Scalar<-9, -18, -27, -36, -45>::Unit()));
    ExpectEquals(4.0, f.to(a));
    ExpectEquals(-9.0, g.to(a));
    ExpectEquals(-1.0, h.to(Scalar<-1, -2, -3, -4, -5>::Unit()));
    ExpectTrue(a == a);
    ExpectFalse(a == b);
    ExpectTrue(a < b);
    ExpectFalse(a < a);
    ExpectTrue(a <= a);
    ExpectTrue(a <= b);
    ExpectFalse(b <= a);
    ExpectTrue(b > a);
    ExpectFalse(b > b);
    ExpectTrue(a >= a);
    ExpectTrue(b >= a);
    ExpectFalse(a >= b);
  }

  void TestNumbers() {
    Number x = 1.0;
    Number y = 1.0 + x;
    Number z = y + 1.0;
    Number t = 1.0 - z;
    ExpectEquals(1.0, x());
    ExpectEquals(2.0, y());
    ExpectEquals(3.0, z());
    ExpectEquals(-2.0, t());
    ExpectTrue(x < 2.0);
    ExpectFalse(x < 1.0);
    ExpectFalse(y < 1.0);
    ExpectTrue(1.0 < y);
    ExpectFalse(2.0 < y);
    ExpectFalse(2.0 < x);
    ExpectTrue(x <= 2.0);
    ExpectFalse(y <= 1.0);
    ExpectTrue(1.0 <= y);
    ExpectFalse(2.0 <= x);
    ExpectTrue(y > 1.0);
    ExpectFalse(x > 2.0);
    ExpectFalse(x > 1.0);
    ExpectTrue(2.0 > x);
    ExpectFalse(1.0 > y);
    ExpectFalse(2.0 > y);
    ExpectTrue(y >= 1.0);
    ExpectFalse(x >= 2.0);
    ExpectTrue(2.0 >= x);
    ExpectFalse(1.0 >= y);
  }

  void TestMath() {
    Scalar<1, 2, 3, 4, 5> a = Scalar<1, 2, 3, 4, 5>::Unit();
    ExpectEquals(Number(std::exp(1.0)), exp(Number(1.0)));
    ExpectEquals(Number(4.0), floor(Number(4.8)));
    ExpectEquals(Number(std::log(2.0)), log(Number(2.0)));
    ExpectEquals(Number(4.0), max(Number(4.0), 3.0));
    ExpectEquals(Number(4.0), max(4.0, Number(3.0)));
    ExpectEquals(Number(3.0), min(Number(4.0), 3.0));
    ExpectEquals(Number(3.0), min(4.0, Number(3.0)));
    ExpectEquals(Number(8.0), pow(Number(2.0), Number(3.0)));
    ExpectEquals(Number(8.0), pow(Number(2.0), 3.0));
    ExpectEquals(Number(8.0), pow(2.0, Number(3.0)));
    ExpectEquals(Number(2.0), sqrt(Number(4.0)));
    ExpectEquals(2.0 * a, sqrt(4.0 * a * a));
    ExpectEquals(2.0, mod(12.0, 10.0));
    ExpectEquals(Number(2.0), mod(Number(12.0), 10.0));
    ExpectEquals(Number(2.0), mod(12.0, Number(10.0)));
    ExpectEquals(12.0, lerp(10.0, 20.0, 0.2));
    ExpectEquals(12.0 * a, lerp(10.0 * a, 20.0 * a, 0.2));
  }
};

namespace {

ScalarTest scalars("scalars", &ScalarTest::TestScalars);
ScalarTest numbers("numbers", &ScalarTest::TestNumbers);
ScalarTest math("math", &ScalarTest::TestMath);

}  // anonymous namespace

}  // namespace dimensional
