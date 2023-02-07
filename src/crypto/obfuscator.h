/*
MIT License

Copyright (c) 2018

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once
#include <array>
#include <limits>

namespace snowapril {
    constexpr int RandomSeed(void) {
        return '0'      * -40271 + // offset accounting for digits' ANSI offsets
            __TIME__[7] *      1 +
            __TIME__[6] *     10 +
            __TIME__[4] *     60 +
            __TIME__[3] *    600 +
            __TIME__[1] *   3600 +
            __TIME__[0] *  36000;
    };

    template <unsigned int a,
              unsigned int c,
              unsigned int seed,
              unsigned int Limit>
    struct LinearCongruentialEngine {
        enum { value = (a * LinearCongruentialEngine<a, c - 1, seed, Limit>::value + c) % Limit };
    };

    template <unsigned int a,
              unsigned int seed,
              unsigned int Limit>
    struct LinearCongruentialEngine<a, 0, seed, Limit> {
        enum { value = (a * seed) % Limit };
    };

    template <int N, int Limit>
    struct MetaRandom {
        enum { value = LinearCongruentialEngine<16807, N, RandomSeed(), Limit>::value };
    };

	template <int A, int B>
	struct ExtendedEuclidian { enum { 
		d = ExtendedEuclidian<B, A % B>::d,
		x = ExtendedEuclidian<B, A % B>::y,
		y = ExtendedEuclidian<B, A % B>::x - (A/B) * ExtendedEuclidian<B, A % B>::y
	}; };

	template <int A>
	struct ExtendedEuclidian<A, 0> { enum { 
		d = A,
		x = 1,
		y = 0
	}; };

	constexpr std::array<int, 30> PrimeNumbers = {
		2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 
		31, 37, 41, 43, 47, 53, 59, 61, 67,
		71, 73, 79, 83, 89, 97, 101, 103, 
		107, 109, 113
	};

	constexpr int positive_modulo(int a, int n) {
		return (a % n + n) % n;
	}

	template <typename Indexes, int A, int B>
	class MetaString;

	template <size_t... I, int A, int B>
	class MetaString<std::index_sequence<I...>, A, B> {
	public:
		constexpr MetaString(char const* str)
			: encrypted_buffer{ encrypt(str[I])... } {};
	public:
		inline const char* decrypt(void) {
			for (size_t i = 0; i < sizeof...(I); ++i) {
				buffer[i] = decrypt(encrypted_buffer[i]);
			}
			buffer[sizeof...(I)] = 0;
			return buffer;
		}
	private:
		constexpr int  encrypt(char c) const { return (A * c + B) % 127; } ;
		constexpr char decrypt(int c) const { return positive_modulo(ExtendedEuclidian<127, A>::y * (c - B), 127); } ;
	private:
		char buffer[sizeof...(I) + 1] {};
		int  encrypted_buffer[sizeof...(I)] {};
	};
}

#define TEST(str) (snowapril::MetaString<std::make_index_sequence<sizeof(str) - 1>, \
					      std::get<snowapril::MetaRandom<__COUNTER__, 30>::value>(snowapril::PrimeNumbers), \
					      snowapril::MetaRandom<__COUNTER__, 126>::value>(str))

#define OBFUSCATE(str) (snowapril::MetaString<std::make_index_sequence<sizeof(str) - 1>, \
					      std::get<snowapril::MetaRandom<__COUNTER__, 30>::value>(snowapril::PrimeNumbers), \
					      snowapril::MetaRandom<__COUNTER__, 126>::value>(str).decrypt())

