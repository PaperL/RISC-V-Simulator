#pragma region PaperL_Header
#ifndef PTL_PTF_H
#define PTL_PTF_H

#include <cstdio>
#include <cassert>
#include <stdexcept>
#include <type_traits>
#include <iostream>

namespace PTF {
#pragma region PTF_DESCRIPTION
/*
 * "PaperL's Template Function"
 *
 * Version: 1.199
 * Last Stable Version: 1.01 / 1.18
 * Last Update Time: 2021.5.10
 * Last Update Content:
 *      增加 PTF_ALGORITHM, 新增 sort 相关功能 (未完成)
 *      增加 PTF_MATH 中整型log相关功能
 *      修复 IO 函数对带符号整型最小值 (如int(1<<31)) 无法正常处理
 * Going to develop:
 *      若干处 getchar 和 putchar 应修改为 cin 和 cout
 *      研究如何让函数支持修改右值(&&)
 *      让同一函数同时支持数组和指针
 *      IO 函数支持浮点
 *      增加 PTF_MATH 三角函数、快速幂等计算函数
 */
#pragma endregion PTF_DESCRIPTION

#pragma region PTF_TYPE

    // std::is_same_v<T1,T2>

    template<typename T1, typename T2>
    struct sameTypeJudge {
        static constexpr bool _value = false;
    };

    template<typename T>
    struct sameTypeJudge<T, T> {
        static constexpr bool _value = true;
    };

    template<typename T1, typename T2>
    constexpr bool sameType = sameTypeJudge<T1, T2>::_value;

#pragma endregion PTF_TYPE

#pragma region PTF_MEMORY

    template<typename T>
    inline void swapT(T &_x, T &_y) {
        T _temp(_x);
        _x = _y;
        _y = _temp;
    }

    template<typename T>
    inline void setT(T &_array, const auto &_value, size_t _n = 0) {
        static_assert(std::is_array_v<T> || std::is_pointer_v<T>,
                      "In PTF: setT get first argument of not array type");
        static_assert(sameType<decltype(_value), std::remove_all_extents_t<T>> ||
        sameType<decltype(_value), std::remove_pointer_t<T>>,
        "In PTF: setT get array and value of different type");
        if constexpr (std::is_array_v<T>)
            for (auto &_item:_array)_item = _value;
        else if constexpr (std::is_pointer_v<T>) {
            for (size_t _p = 0; _p < _n; _p++)
                _array[_p] = _value;
        }
    }

#pragma endregion PTF_MEMORY

#pragma region PTF_MATH

    template<typename T, T _zero = 0>
    inline T absT(const T &_k) { return ((_k >= _zero) ? (_k) : (-_k)); }

    // 注意 maxN 使用了 operator >
    template<typename T>
    inline T maxN(const T &_first, const auto &... _argList) {
        static_assert((sameType<decltype(_argList), const T &> && ...), "Call maxN with different type arguments.");
        const T *_maxArg = &_first;
        ((_maxArg = (_argList > *_maxArg) ? (&_argList) : (_maxArg)), ...);
        return *_maxArg;
    }

    template<typename T>
    inline T minN(const T &_first, const auto &... _argList) {
        static_assert((sameType<decltype(_argList), const T &> && ...), "Call minN with different type arguments.");
        const T *_minArg = &_first;
        ((_minArg = (_argList < *_minArg) ? (&_argList) : (_minArg)), ...);
        return *_minArg;
    }

    inline int intLog2(int _k) {
        int _ans = 0;
        if (_k & 0xffff0000) _ans += 16, _k >>= 16;
        if (_k & 0x0000ff00) _ans += 8, _k >>= 8;
        if (_k & 0x000000f0) _ans += 4, _k >>= 4;
        if (_k & 0x0000000c) _ans += 2, _k >>= 2;
        if (_k & 0x00000002) _ans += 1;
        return _ans;
    }

    inline unsigned int uintLog2(unsigned int _k) {
        unsigned int _ans = 0;
        if (_k & 0xffff0000) _ans += 16, _k >>= 16;
        if (_k & 0x0000ff00) _ans += 8, _k >>= 8;
        if (_k & 0x000000f0) _ans += 4, _k >>= 4;
        if (_k & 0x0000000c) _ans += 2, _k >>= 2;
        if (_k & 0x00000002) _ans += 1;
        return _ans;
    }

    inline size_t ullLog2(size_t _k) {
        size_t _ans = 0;
        if (_k & 0xffffffff00000000) _ans |= 32, _k >>= 32;
        if (_k & 0x00000000ffff0000) _ans |= 16, _k >>= 16;
        if (_k & 0x000000000000ff00) _ans |= 8, _k >>= 8;
        if (_k & 0x00000000000000f0) _ans |= 4, _k >>= 4;
        if (_k & 0x000000000000000c) _ans |= 2, _k >>= 2;
        if (_k & 0x0000000000000002) _ans |= 1;
        return _ans;
    }

#pragma endregion PTF_MATH

#pragma region PTF_IO

    template<typename T>
    inline void qRead(T &_k) {
        static_assert(!std::is_const_v<T>,
                      "In PTF: qRead get unexpected const argument");
        if constexpr(sameType<T, char>) _k = std::cin.get();
            // here use "_k = cin.get()" instead of "cin.get(_k)"
            // cin.get() returns int_type(int)
            // cin.get(&) calls for char_type(char)
            // if cin.get(&) get unexpected character, failbit(cin.fail()) and eofbit(cin.eof()) are set
        else if constexpr (sameType<T, char *>) {
            int _c;
            do {
                _c = std::cin.get();
                if (_c < 0 || _c > 255)
                    throw std::runtime_error("In PTF: qRead get unexpected character (may be EOF)\n");
            } while (_c == ' ' || _c == '\n' || _c == '\r');
            size_t _p = -1;
            do {
                _k[++_p] = _c;
                _c = std::cin.get();
                if (_c < 0 || _c > 255)
                    throw std::runtime_error("In PTF: qRead get unexpected character (may be EOF)\n");
            } while (_k[_p] != ' ' && _k[_p] != '\n' && _k[_p] != '\r' && _k[_p] != '\0');
            // todo 以下分支均需同样的安全性检查
        }
        else if constexpr (sameType<std::remove_extent_t<T>, char>) {
            size_t _p = 0;
            const size_t _l = std::extent_v<T>;
            _k[_p] = std::cin.get();
            while (_k[_p] != ' ' && _k[_p] != '\n' && _k[_p] != '\r' && _k[_p] != '\0' && _p < _l)
                _k[++_p] = std::cin.get();
            _k[_p] = '\0';
        }
        else if constexpr(std::is_integral_v<T>) {
            int _c = getchar();
            bool _sign = false;
            _k = 0;
            while (_c < '0' || _c > '9') {
                if (_c < 0 || _c > 255)
                    throw std::runtime_error("In PTF: qRead get unexpected character (may be EOF)\n");
                if (_c == '-') _sign = true;
                _c = getchar();
            }
            while (_c >= '0' && _c <= '9') {
                _k = _k * 10 - 48 + _c; // 此处位运算替代 *10 可能不一定更快
                // todo (_k * 10) 显然会炸
                _c = getchar();
            }
            if (_sign) _k = -_k;
        }
        else static_assert(sameType<T, std::remove_reference_t<T>[1]>,
                           "In PTF: qRead get argument of unexpected type\n");
    }

    template<typename T>
    inline T qRead() {
        T _k;
        qRead(_k);
        return _k;
    }

    // c++20 fold expression
    inline void qRead(auto &... _argList) { (qRead(_argList), ...); }

    template<typename T>
    inline void qWrite(const T &_k) {
        if constexpr(sameType<T, char>) putchar(_k);
        else if constexpr (sameType<T, char *>)
            for (size_t _p = 0; _k[_p] != '\0'; _p++) putchar(_k[_p]);
        else if constexpr (sameType<std::remove_extent_t<T>, char>)
            for (size_t _p = 0, _l = std::extent_v<T>; _k[_p] != '\0' && _p < _l; _p++) putchar(_k[_p]);
        else if constexpr (std::is_integral_v<T>) {
            if (_k == 0) putchar('0');
            else {
                size_t _p = 0;
                T _ck(_k);
                char _c[32];
                if constexpr (std::is_signed_v<T>) {
                    if (_ck < 0) {
                        if (_ck == T(1 << ((sizeof(T) << 3) - 1))) {
                            if constexpr(sizeof(T) == 1) qWrite("-128");
                            else if constexpr(sizeof(T) == 2)qWrite("-32768");
                            else if constexpr(sizeof(T) == 4)qWrite("-2147483648");
                            else if constexpr(sizeof(T) == 8)qWrite("-9223372036854775808");
                            else
                                throw std::runtime_error("In PTF: qWrite get unexpected negative integer "
                                                         "(type length isn't 1/2/4/8)\n");
                            return;
                        }
                        else putchar('-'), _ck = -_ck;
                    }
                }
                while (_ck) _c[_p++] = _ck % 10 + 48, _ck /= 10;
                while (_p--) putchar(_c[_p]);
            }
        }
        else static_assert(sameType<T, std::remove_cv<T>[1]>,
                           "In PTF: qWrite get argument of unexpected type\n");
    }

    inline void qWrite(const auto &... _argList) { (qWrite(_argList), ...); }

    // _s for split character(other type is also acceptable)
    inline void qWriteS(const auto &_s, const auto &... _argList) { ((qWrite(_argList), qWrite(_s)), ...); }

    // _eol for end of line character
    inline void qWriteL(const auto &_eol, const auto &... _argList) { (qWrite(_argList), ...), qWrite(_eol); }

    // 注意最后 _eol 之前无 _s
    inline void qWriteSL(const auto &_s, const auto &_eol, const auto &_first, const auto &... _argList) {
        qWrite(_first), ((qWrite(_s), qWrite(_argList)), ...), qWrite(_eol);
    }

#pragma endregion PTF_IO

#pragma region PTF_ALGORITHM

    // in sort functions, target distinction is [first, last)
    template<typename T>
    void _partialSort(T *_first, T *_middle, T *_last)
    // heap sort part of elements
    // assure first to middle are minimum elements
    {
        // todo
    }

    template<typename T>
    void _introSort(T *_first, T *_last, size_t _depthLim)
    // sort all elements
    // basic method is quick sort
    {
        while (_last - _first > 16) {
            if (_depthLim == 0) {
                _partialSort(_first, _last, _last);
                return;
            }
            --_depthLim;
            // following is quick sort main operation
            T *_mid = _first + (_last - _first) / 2;
            // todo
            _introSort(_first, _mid, _depthLim);
            _introSort(_mid, _last, _depthLim);
        }
    }

    template<typename T>
    void _insertionSort(T *_first, T *_last) {
        // todo
    }

    template<typename T>
    void qSort(T *_first, T *_last)
    // datatype should support Random Access
    {
        if (_first != _last) {
            _introSort(_first, _last, ullLog2(_last - _first) * 2);
            _insertionSort(_first, _last);
            // solve as pointer
        }
    }

#pragma endregion PTF_ALGORITHM

}

#endif //PTL_PTF_H
#pragma endregion PaperL_Header