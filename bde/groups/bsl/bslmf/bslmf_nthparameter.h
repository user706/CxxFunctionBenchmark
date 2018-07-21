// bslmf_nthparameter.h                                               -*-C++-*-
#ifndef INCLUDED_BSLMF_NTHPARAMETER
#define INCLUDED_BSLMF_NTHPARAMETER

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Metafunction to return the Nth type parameter in a parameter pack
//
//@CLASSES: bslmf::NthParameter<N, PARAM_0, PARAMS...>
//
//@SEE_ALSO:
//
//@DESCRIPTION: This component contains a metafunction that treats a
// parameter pack of types as compile-time array of types, returning the Nth
// type (counting from zero).  It is useful for implementing types like
// 'tuple' that need access to a specific element of a parameter pack.
//
///Usage
///-----
// We wish to implement a 'tuple'-like class that holds a heterogeneous
// collection of elements, each of which might have a different type.  The
// metafunction, 'my_tuple_element<I, my_tuple<ELEMS...>>::Type' would be type
// of the 'I'th element in the tuple (where 'I' is zero-based).
//
// First, we define our 'my_tuple' class template.  The body of the class is
// unimportant for this usage examples:
//..
// template <class... ELEMS>
// class my_tuple {
//     // ...
// };
//..
// Then, we use 'bslmf::NthParameter' to implement 'my_tuple_element':
//..
// #include <bslmf_nthparameter.h>
//
// template <std::size_t I, class TUPLE>
// struct my_tuple_element;  // Not defined
//
// template <std::size_t I, class... ELEMS>
// struct my_tuple_element<I, my_tuple<ELEMS...> > {
//     typedef typename bslmf::NthParameter<I, ELEMS...>::Type Type;
// };
//..
// Finally, we test this implementation using 'bsl::is_same':
//..
// #include <bslmf_issame.h>
//
// int main()
// {
//    typedef my_tuple<int, short, char*> ttype;
//
//    assert((bsl::is_same<int,    my_tuple_element<0, ttype>::Type>::value));
//    assert((bsl::is_same<short,  my_tuple_element<1, ttype>::Type>::value));
//    assert((bsl::is_same<char *, my_tuple_element<2, ttype>::Type>::value));
//
//    assert(! (bsl::is_same<short, my_tuple_element<0, ttype>::Type>::value));
// }
//..

#ifndef INCLUDED_BSLS_COMPILERFEATURES
#include <bsls_compilerfeatures.h>
#endif

#ifndef INCLUDED_BSLMF_ASSERT
#include <bslmf_assert.h>
#endif

#ifndef INCLUDED_CSTDLIB
#include <cstdlib>
#define INCLUDED_CSTDLIB
#endif

namespace BloombergLP {

namespace bslmf {

                        // ===========================
                        // class template NthParameter
                        // ===========================

#if defined(BSLS_PLATFORM_CMP_SUN) && BSLS_PLATFORM_CMP_VERSION == 0x5130 \
 && defined(BSLS_COMPILERFEATURES_SUPPORT_VARIADIC_TEMPLATES)
template <std::size_t N, class... PARAMS>
struct NthParameter {
    using Type = typename NthParameter<N - 1, PARAMS...>::Type;
        // The type of the Nth parameter, computed by recursively stripping off
        // the first parameter until 'N == 0'.
};

template <class FIRST_PARAM, class... PARAMS>
struct NthParameter<0, FIRST_PARAM, PARAMS...> {
    // Specialization of 'NthParameter' the matches finding the first item in
    // the list.

    using Type = FIRST_PARAM;
};

template <std::size_t N>
struct NthParameter<N> {
    // Specialization of 'NthParameter' for when 'N' exceeds the actual number
    // of parameters.

    // No 'Type' member is defined, so that failure by specifying too large a
    // value for 'N' fails in a SFINAE-friendly manner.
};
#else

struct NthParameter_Sentinel;  // Declared but not defined

#if !BSLS_COMPILERFEATURES_SIMULATE_CPP11_FEATURES // $var-args=15

template <std::size_t N, class FIRST_PARAM = NthParameter_Sentinel,
          class... PARAMS>
struct NthParameter {
    // Metafunction to compute the specified 'N'th element of the spcified
    // 'PARAMS' template parameter pack.  The 'Type' nested typedef will match
    // the 'N'th element of 'PARAMS', where 'N' is zero-based (so that an 'N'
    // of zero corresponds to the first parameter.

    typedef typename NthParameter<N - 1, PARAMS...>::Type Type;
        // The type of the Nth parameter, computed by recursively stripping
        // off the first parameter until N == 0.
};

// ===========================================================================
//                      IMPLEMENTATION
// ===========================================================================

template <class FIRST_PARAM, class... PARAMS>
struct NthParameter<0, FIRST_PARAM, PARAMS...> {
    // Specialization of 'NthParameter' for when 'N' is zero.

    typedef FIRST_PARAM Type;
        // The type of the 0th parameter.
};

#elif BSLS_COMPILERFEATURES_SIMULATE_VARIADIC_TEMPLATES
// {{{ BEGIN GENERATED CODE
// The following section is automatically generated.  **DO NOT EDIT**
// Generator command line: sim_cpp11_features.pl bslmf_nthparameter.h

template <std::size_t N,
          class FIRST_PARAM = NthParameter_Sentinel,
          class PARAMS_0 = BSLS_COMPILERFEATURES_NILT,
          class PARAMS_1 = BSLS_COMPILERFEATURES_NILT,
          class PARAMS_2 = BSLS_COMPILERFEATURES_NILT,
          class PARAMS_3 = BSLS_COMPILERFEATURES_NILT,
          class PARAMS_4 = BSLS_COMPILERFEATURES_NILT,
          class PARAMS_5 = BSLS_COMPILERFEATURES_NILT,
          class PARAMS_6 = BSLS_COMPILERFEATURES_NILT,
          class PARAMS_7 = BSLS_COMPILERFEATURES_NILT,
          class PARAMS_8 = BSLS_COMPILERFEATURES_NILT,
          class PARAMS_9 = BSLS_COMPILERFEATURES_NILT,
          class PARAMS_10 = BSLS_COMPILERFEATURES_NILT,
          class PARAMS_11 = BSLS_COMPILERFEATURES_NILT,
          class PARAMS_12 = BSLS_COMPILERFEATURES_NILT,
          class PARAMS_13 = BSLS_COMPILERFEATURES_NILT,
          class PARAMS_14 = BSLS_COMPILERFEATURES_NILT,
          class = BSLS_COMPILERFEATURES_NILT>
struct NthParameter;

template <std::size_t N, class FIRST_PARAM>
struct NthParameter<N, FIRST_PARAM> {

    typedef typename NthParameter<N - 1>::Type Type;
};

template <std::size_t N, class FIRST_PARAM, class PARAMS_01>
struct NthParameter<N, FIRST_PARAM, PARAMS_01> {

    typedef typename NthParameter<N - 1, PARAMS_01>::Type Type;
};

template <std::size_t N, class FIRST_PARAM, class PARAMS_01,
                                            class PARAMS_02>
struct NthParameter<N, FIRST_PARAM, PARAMS_01,
                                    PARAMS_02> {

    typedef typename NthParameter<N - 1, PARAMS_01,
                                         PARAMS_02>::Type Type;
};

template <std::size_t N, class FIRST_PARAM, class PARAMS_01,
                                            class PARAMS_02,
                                            class PARAMS_03>
struct NthParameter<N, FIRST_PARAM, PARAMS_01,
                                    PARAMS_02,
                                    PARAMS_03> {

    typedef typename NthParameter<N - 1, PARAMS_01,
                                         PARAMS_02,
                                         PARAMS_03>::Type Type;
};

template <std::size_t N, class FIRST_PARAM, class PARAMS_01,
                                            class PARAMS_02,
                                            class PARAMS_03,
                                            class PARAMS_04>
struct NthParameter<N, FIRST_PARAM, PARAMS_01,
                                    PARAMS_02,
                                    PARAMS_03,
                                    PARAMS_04> {

    typedef typename NthParameter<N - 1, PARAMS_01,
                                         PARAMS_02,
                                         PARAMS_03,
                                         PARAMS_04>::Type Type;
};

template <std::size_t N, class FIRST_PARAM, class PARAMS_01,
                                            class PARAMS_02,
                                            class PARAMS_03,
                                            class PARAMS_04,
                                            class PARAMS_05>
struct NthParameter<N, FIRST_PARAM, PARAMS_01,
                                    PARAMS_02,
                                    PARAMS_03,
                                    PARAMS_04,
                                    PARAMS_05> {

    typedef typename NthParameter<N - 1, PARAMS_01,
                                         PARAMS_02,
                                         PARAMS_03,
                                         PARAMS_04,
                                         PARAMS_05>::Type Type;
};

template <std::size_t N, class FIRST_PARAM, class PARAMS_01,
                                            class PARAMS_02,
                                            class PARAMS_03,
                                            class PARAMS_04,
                                            class PARAMS_05,
                                            class PARAMS_06>
struct NthParameter<N, FIRST_PARAM, PARAMS_01,
                                    PARAMS_02,
                                    PARAMS_03,
                                    PARAMS_04,
                                    PARAMS_05,
                                    PARAMS_06> {

    typedef typename NthParameter<N - 1, PARAMS_01,
                                         PARAMS_02,
                                         PARAMS_03,
                                         PARAMS_04,
                                         PARAMS_05,
                                         PARAMS_06>::Type Type;
};

template <std::size_t N, class FIRST_PARAM, class PARAMS_01,
                                            class PARAMS_02,
                                            class PARAMS_03,
                                            class PARAMS_04,
                                            class PARAMS_05,
                                            class PARAMS_06,
                                            class PARAMS_07>
struct NthParameter<N, FIRST_PARAM, PARAMS_01,
                                    PARAMS_02,
                                    PARAMS_03,
                                    PARAMS_04,
                                    PARAMS_05,
                                    PARAMS_06,
                                    PARAMS_07> {

    typedef typename NthParameter<N - 1, PARAMS_01,
                                         PARAMS_02,
                                         PARAMS_03,
                                         PARAMS_04,
                                         PARAMS_05,
                                         PARAMS_06,
                                         PARAMS_07>::Type Type;
};

template <std::size_t N, class FIRST_PARAM, class PARAMS_01,
                                            class PARAMS_02,
                                            class PARAMS_03,
                                            class PARAMS_04,
                                            class PARAMS_05,
                                            class PARAMS_06,
                                            class PARAMS_07,
                                            class PARAMS_08>
struct NthParameter<N, FIRST_PARAM, PARAMS_01,
                                    PARAMS_02,
                                    PARAMS_03,
                                    PARAMS_04,
                                    PARAMS_05,
                                    PARAMS_06,
                                    PARAMS_07,
                                    PARAMS_08> {

    typedef typename NthParameter<N - 1, PARAMS_01,
                                         PARAMS_02,
                                         PARAMS_03,
                                         PARAMS_04,
                                         PARAMS_05,
                                         PARAMS_06,
                                         PARAMS_07,
                                         PARAMS_08>::Type Type;
};

template <std::size_t N, class FIRST_PARAM, class PARAMS_01,
                                            class PARAMS_02,
                                            class PARAMS_03,
                                            class PARAMS_04,
                                            class PARAMS_05,
                                            class PARAMS_06,
                                            class PARAMS_07,
                                            class PARAMS_08,
                                            class PARAMS_09>
struct NthParameter<N, FIRST_PARAM, PARAMS_01,
                                    PARAMS_02,
                                    PARAMS_03,
                                    PARAMS_04,
                                    PARAMS_05,
                                    PARAMS_06,
                                    PARAMS_07,
                                    PARAMS_08,
                                    PARAMS_09> {

    typedef typename NthParameter<N - 1, PARAMS_01,
                                         PARAMS_02,
                                         PARAMS_03,
                                         PARAMS_04,
                                         PARAMS_05,
                                         PARAMS_06,
                                         PARAMS_07,
                                         PARAMS_08,
                                         PARAMS_09>::Type Type;
};

template <std::size_t N, class FIRST_PARAM, class PARAMS_01,
                                            class PARAMS_02,
                                            class PARAMS_03,
                                            class PARAMS_04,
                                            class PARAMS_05,
                                            class PARAMS_06,
                                            class PARAMS_07,
                                            class PARAMS_08,
                                            class PARAMS_09,
                                            class PARAMS_10>
struct NthParameter<N, FIRST_PARAM, PARAMS_01,
                                    PARAMS_02,
                                    PARAMS_03,
                                    PARAMS_04,
                                    PARAMS_05,
                                    PARAMS_06,
                                    PARAMS_07,
                                    PARAMS_08,
                                    PARAMS_09,
                                    PARAMS_10> {

    typedef typename NthParameter<N - 1, PARAMS_01,
                                         PARAMS_02,
                                         PARAMS_03,
                                         PARAMS_04,
                                         PARAMS_05,
                                         PARAMS_06,
                                         PARAMS_07,
                                         PARAMS_08,
                                         PARAMS_09,
                                         PARAMS_10>::Type Type;
};

template <std::size_t N, class FIRST_PARAM, class PARAMS_01,
                                            class PARAMS_02,
                                            class PARAMS_03,
                                            class PARAMS_04,
                                            class PARAMS_05,
                                            class PARAMS_06,
                                            class PARAMS_07,
                                            class PARAMS_08,
                                            class PARAMS_09,
                                            class PARAMS_10,
                                            class PARAMS_11>
struct NthParameter<N, FIRST_PARAM, PARAMS_01,
                                    PARAMS_02,
                                    PARAMS_03,
                                    PARAMS_04,
                                    PARAMS_05,
                                    PARAMS_06,
                                    PARAMS_07,
                                    PARAMS_08,
                                    PARAMS_09,
                                    PARAMS_10,
                                    PARAMS_11> {

    typedef typename NthParameter<N - 1, PARAMS_01,
                                         PARAMS_02,
                                         PARAMS_03,
                                         PARAMS_04,
                                         PARAMS_05,
                                         PARAMS_06,
                                         PARAMS_07,
                                         PARAMS_08,
                                         PARAMS_09,
                                         PARAMS_10,
                                         PARAMS_11>::Type Type;
};

template <std::size_t N, class FIRST_PARAM, class PARAMS_01,
                                            class PARAMS_02,
                                            class PARAMS_03,
                                            class PARAMS_04,
                                            class PARAMS_05,
                                            class PARAMS_06,
                                            class PARAMS_07,
                                            class PARAMS_08,
                                            class PARAMS_09,
                                            class PARAMS_10,
                                            class PARAMS_11,
                                            class PARAMS_12>
struct NthParameter<N, FIRST_PARAM, PARAMS_01,
                                    PARAMS_02,
                                    PARAMS_03,
                                    PARAMS_04,
                                    PARAMS_05,
                                    PARAMS_06,
                                    PARAMS_07,
                                    PARAMS_08,
                                    PARAMS_09,
                                    PARAMS_10,
                                    PARAMS_11,
                                    PARAMS_12> {

    typedef typename NthParameter<N - 1, PARAMS_01,
                                         PARAMS_02,
                                         PARAMS_03,
                                         PARAMS_04,
                                         PARAMS_05,
                                         PARAMS_06,
                                         PARAMS_07,
                                         PARAMS_08,
                                         PARAMS_09,
                                         PARAMS_10,
                                         PARAMS_11,
                                         PARAMS_12>::Type Type;
};

template <std::size_t N, class FIRST_PARAM, class PARAMS_01,
                                            class PARAMS_02,
                                            class PARAMS_03,
                                            class PARAMS_04,
                                            class PARAMS_05,
                                            class PARAMS_06,
                                            class PARAMS_07,
                                            class PARAMS_08,
                                            class PARAMS_09,
                                            class PARAMS_10,
                                            class PARAMS_11,
                                            class PARAMS_12,
                                            class PARAMS_13>
struct NthParameter<N, FIRST_PARAM, PARAMS_01,
                                    PARAMS_02,
                                    PARAMS_03,
                                    PARAMS_04,
                                    PARAMS_05,
                                    PARAMS_06,
                                    PARAMS_07,
                                    PARAMS_08,
                                    PARAMS_09,
                                    PARAMS_10,
                                    PARAMS_11,
                                    PARAMS_12,
                                    PARAMS_13> {

    typedef typename NthParameter<N - 1, PARAMS_01,
                                         PARAMS_02,
                                         PARAMS_03,
                                         PARAMS_04,
                                         PARAMS_05,
                                         PARAMS_06,
                                         PARAMS_07,
                                         PARAMS_08,
                                         PARAMS_09,
                                         PARAMS_10,
                                         PARAMS_11,
                                         PARAMS_12,
                                         PARAMS_13>::Type Type;
};

template <std::size_t N, class FIRST_PARAM, class PARAMS_01,
                                            class PARAMS_02,
                                            class PARAMS_03,
                                            class PARAMS_04,
                                            class PARAMS_05,
                                            class PARAMS_06,
                                            class PARAMS_07,
                                            class PARAMS_08,
                                            class PARAMS_09,
                                            class PARAMS_10,
                                            class PARAMS_11,
                                            class PARAMS_12,
                                            class PARAMS_13,
                                            class PARAMS_14>
struct NthParameter<N, FIRST_PARAM, PARAMS_01,
                                    PARAMS_02,
                                    PARAMS_03,
                                    PARAMS_04,
                                    PARAMS_05,
                                    PARAMS_06,
                                    PARAMS_07,
                                    PARAMS_08,
                                    PARAMS_09,
                                    PARAMS_10,
                                    PARAMS_11,
                                    PARAMS_12,
                                    PARAMS_13,
                                    PARAMS_14> {

    typedef typename NthParameter<N - 1, PARAMS_01,
                                         PARAMS_02,
                                         PARAMS_03,
                                         PARAMS_04,
                                         PARAMS_05,
                                         PARAMS_06,
                                         PARAMS_07,
                                         PARAMS_08,
                                         PARAMS_09,
                                         PARAMS_10,
                                         PARAMS_11,
                                         PARAMS_12,
                                         PARAMS_13,
                                         PARAMS_14>::Type Type;
};

template <std::size_t N, class FIRST_PARAM, class PARAMS_01,
                                            class PARAMS_02,
                                            class PARAMS_03,
                                            class PARAMS_04,
                                            class PARAMS_05,
                                            class PARAMS_06,
                                            class PARAMS_07,
                                            class PARAMS_08,
                                            class PARAMS_09,
                                            class PARAMS_10,
                                            class PARAMS_11,
                                            class PARAMS_12,
                                            class PARAMS_13,
                                            class PARAMS_14,
                                            class PARAMS_15>
struct NthParameter<N, FIRST_PARAM, PARAMS_01,
                                    PARAMS_02,
                                    PARAMS_03,
                                    PARAMS_04,
                                    PARAMS_05,
                                    PARAMS_06,
                                    PARAMS_07,
                                    PARAMS_08,
                                    PARAMS_09,
                                    PARAMS_10,
                                    PARAMS_11,
                                    PARAMS_12,
                                    PARAMS_13,
                                    PARAMS_14,
                                    PARAMS_15> {

    typedef typename NthParameter<N - 1, PARAMS_01,
                                         PARAMS_02,
                                         PARAMS_03,
                                         PARAMS_04,
                                         PARAMS_05,
                                         PARAMS_06,
                                         PARAMS_07,
                                         PARAMS_08,
                                         PARAMS_09,
                                         PARAMS_10,
                                         PARAMS_11,
                                         PARAMS_12,
                                         PARAMS_13,
                                         PARAMS_14,
                                         PARAMS_15>::Type Type;
};



template <class FIRST_PARAM>
struct NthParameter<0, FIRST_PARAM> {

    typedef FIRST_PARAM Type;
};

template <class FIRST_PARAM, class PARAMS_01>
struct NthParameter<0, FIRST_PARAM, PARAMS_01> {

    typedef FIRST_PARAM Type;
};

template <class FIRST_PARAM, class PARAMS_01,
                             class PARAMS_02>
struct NthParameter<0, FIRST_PARAM, PARAMS_01,
                                    PARAMS_02> {

    typedef FIRST_PARAM Type;
};

template <class FIRST_PARAM, class PARAMS_01,
                             class PARAMS_02,
                             class PARAMS_03>
struct NthParameter<0, FIRST_PARAM, PARAMS_01,
                                    PARAMS_02,
                                    PARAMS_03> {

    typedef FIRST_PARAM Type;
};

template <class FIRST_PARAM, class PARAMS_01,
                             class PARAMS_02,
                             class PARAMS_03,
                             class PARAMS_04>
struct NthParameter<0, FIRST_PARAM, PARAMS_01,
                                    PARAMS_02,
                                    PARAMS_03,
                                    PARAMS_04> {

    typedef FIRST_PARAM Type;
};

template <class FIRST_PARAM, class PARAMS_01,
                             class PARAMS_02,
                             class PARAMS_03,
                             class PARAMS_04,
                             class PARAMS_05>
struct NthParameter<0, FIRST_PARAM, PARAMS_01,
                                    PARAMS_02,
                                    PARAMS_03,
                                    PARAMS_04,
                                    PARAMS_05> {

    typedef FIRST_PARAM Type;
};

template <class FIRST_PARAM, class PARAMS_01,
                             class PARAMS_02,
                             class PARAMS_03,
                             class PARAMS_04,
                             class PARAMS_05,
                             class PARAMS_06>
struct NthParameter<0, FIRST_PARAM, PARAMS_01,
                                    PARAMS_02,
                                    PARAMS_03,
                                    PARAMS_04,
                                    PARAMS_05,
                                    PARAMS_06> {

    typedef FIRST_PARAM Type;
};

template <class FIRST_PARAM, class PARAMS_01,
                             class PARAMS_02,
                             class PARAMS_03,
                             class PARAMS_04,
                             class PARAMS_05,
                             class PARAMS_06,
                             class PARAMS_07>
struct NthParameter<0, FIRST_PARAM, PARAMS_01,
                                    PARAMS_02,
                                    PARAMS_03,
                                    PARAMS_04,
                                    PARAMS_05,
                                    PARAMS_06,
                                    PARAMS_07> {

    typedef FIRST_PARAM Type;
};

template <class FIRST_PARAM, class PARAMS_01,
                             class PARAMS_02,
                             class PARAMS_03,
                             class PARAMS_04,
                             class PARAMS_05,
                             class PARAMS_06,
                             class PARAMS_07,
                             class PARAMS_08>
struct NthParameter<0, FIRST_PARAM, PARAMS_01,
                                    PARAMS_02,
                                    PARAMS_03,
                                    PARAMS_04,
                                    PARAMS_05,
                                    PARAMS_06,
                                    PARAMS_07,
                                    PARAMS_08> {

    typedef FIRST_PARAM Type;
};

template <class FIRST_PARAM, class PARAMS_01,
                             class PARAMS_02,
                             class PARAMS_03,
                             class PARAMS_04,
                             class PARAMS_05,
                             class PARAMS_06,
                             class PARAMS_07,
                             class PARAMS_08,
                             class PARAMS_09>
struct NthParameter<0, FIRST_PARAM, PARAMS_01,
                                    PARAMS_02,
                                    PARAMS_03,
                                    PARAMS_04,
                                    PARAMS_05,
                                    PARAMS_06,
                                    PARAMS_07,
                                    PARAMS_08,
                                    PARAMS_09> {

    typedef FIRST_PARAM Type;
};

template <class FIRST_PARAM, class PARAMS_01,
                             class PARAMS_02,
                             class PARAMS_03,
                             class PARAMS_04,
                             class PARAMS_05,
                             class PARAMS_06,
                             class PARAMS_07,
                             class PARAMS_08,
                             class PARAMS_09,
                             class PARAMS_10>
struct NthParameter<0, FIRST_PARAM, PARAMS_01,
                                    PARAMS_02,
                                    PARAMS_03,
                                    PARAMS_04,
                                    PARAMS_05,
                                    PARAMS_06,
                                    PARAMS_07,
                                    PARAMS_08,
                                    PARAMS_09,
                                    PARAMS_10> {

    typedef FIRST_PARAM Type;
};

template <class FIRST_PARAM, class PARAMS_01,
                             class PARAMS_02,
                             class PARAMS_03,
                             class PARAMS_04,
                             class PARAMS_05,
                             class PARAMS_06,
                             class PARAMS_07,
                             class PARAMS_08,
                             class PARAMS_09,
                             class PARAMS_10,
                             class PARAMS_11>
struct NthParameter<0, FIRST_PARAM, PARAMS_01,
                                    PARAMS_02,
                                    PARAMS_03,
                                    PARAMS_04,
                                    PARAMS_05,
                                    PARAMS_06,
                                    PARAMS_07,
                                    PARAMS_08,
                                    PARAMS_09,
                                    PARAMS_10,
                                    PARAMS_11> {

    typedef FIRST_PARAM Type;
};

template <class FIRST_PARAM, class PARAMS_01,
                             class PARAMS_02,
                             class PARAMS_03,
                             class PARAMS_04,
                             class PARAMS_05,
                             class PARAMS_06,
                             class PARAMS_07,
                             class PARAMS_08,
                             class PARAMS_09,
                             class PARAMS_10,
                             class PARAMS_11,
                             class PARAMS_12>
struct NthParameter<0, FIRST_PARAM, PARAMS_01,
                                    PARAMS_02,
                                    PARAMS_03,
                                    PARAMS_04,
                                    PARAMS_05,
                                    PARAMS_06,
                                    PARAMS_07,
                                    PARAMS_08,
                                    PARAMS_09,
                                    PARAMS_10,
                                    PARAMS_11,
                                    PARAMS_12> {

    typedef FIRST_PARAM Type;
};

template <class FIRST_PARAM, class PARAMS_01,
                             class PARAMS_02,
                             class PARAMS_03,
                             class PARAMS_04,
                             class PARAMS_05,
                             class PARAMS_06,
                             class PARAMS_07,
                             class PARAMS_08,
                             class PARAMS_09,
                             class PARAMS_10,
                             class PARAMS_11,
                             class PARAMS_12,
                             class PARAMS_13>
struct NthParameter<0, FIRST_PARAM, PARAMS_01,
                                    PARAMS_02,
                                    PARAMS_03,
                                    PARAMS_04,
                                    PARAMS_05,
                                    PARAMS_06,
                                    PARAMS_07,
                                    PARAMS_08,
                                    PARAMS_09,
                                    PARAMS_10,
                                    PARAMS_11,
                                    PARAMS_12,
                                    PARAMS_13> {

    typedef FIRST_PARAM Type;
};

template <class FIRST_PARAM, class PARAMS_01,
                             class PARAMS_02,
                             class PARAMS_03,
                             class PARAMS_04,
                             class PARAMS_05,
                             class PARAMS_06,
                             class PARAMS_07,
                             class PARAMS_08,
                             class PARAMS_09,
                             class PARAMS_10,
                             class PARAMS_11,
                             class PARAMS_12,
                             class PARAMS_13,
                             class PARAMS_14>
struct NthParameter<0, FIRST_PARAM, PARAMS_01,
                                    PARAMS_02,
                                    PARAMS_03,
                                    PARAMS_04,
                                    PARAMS_05,
                                    PARAMS_06,
                                    PARAMS_07,
                                    PARAMS_08,
                                    PARAMS_09,
                                    PARAMS_10,
                                    PARAMS_11,
                                    PARAMS_12,
                                    PARAMS_13,
                                    PARAMS_14> {

    typedef FIRST_PARAM Type;
};

template <class FIRST_PARAM, class PARAMS_01,
                             class PARAMS_02,
                             class PARAMS_03,
                             class PARAMS_04,
                             class PARAMS_05,
                             class PARAMS_06,
                             class PARAMS_07,
                             class PARAMS_08,
                             class PARAMS_09,
                             class PARAMS_10,
                             class PARAMS_11,
                             class PARAMS_12,
                             class PARAMS_13,
                             class PARAMS_14,
                             class PARAMS_15>
struct NthParameter<0, FIRST_PARAM, PARAMS_01,
                                    PARAMS_02,
                                    PARAMS_03,
                                    PARAMS_04,
                                    PARAMS_05,
                                    PARAMS_06,
                                    PARAMS_07,
                                    PARAMS_08,
                                    PARAMS_09,
                                    PARAMS_10,
                                    PARAMS_11,
                                    PARAMS_12,
                                    PARAMS_13,
                                    PARAMS_14,
                                    PARAMS_15> {

    typedef FIRST_PARAM Type;
};

#else
// The generated code below is a workaround for the absence of perfect
// forwarding in some compilers.

template <std::size_t N, class FIRST_PARAM = NthParameter_Sentinel,
          class... PARAMS>
struct NthParameter {

    typedef typename NthParameter<N - 1, PARAMS...>::Type Type;
};


template <class FIRST_PARAM, class... PARAMS>
struct NthParameter<0, FIRST_PARAM, PARAMS...> {

    typedef FIRST_PARAM Type;
};

// }}} END GENERATED CODE
#endif

template <>
struct NthParameter<0, NthParameter_Sentinel> {
    // Specialization of 'NthParameter' for when 'N' exceeds the actual number
    // of parameters.

#ifdef BSLS_COMPILERFEATURES_SUPPORT_VARIADIC_TEMPLATES
    // No 'Type' member is defined.
#else
    // There are no dependent parameters because this is a full specialization.
    // When used in another simulated variadic template, the compiler may
    // attempt to evaluate the 'Type' member even when that client is not
    // actually instantiated.  To avoid a spurious compilation error, we must
    // therefore make sure that 'Type' is defined, even if it is defined as an
    // incomplete class.
    typedef NthParameter_Sentinel Type;
#endif
};

#endif // Sun C++11 workaround

}  // close package namespace

}  // close enterprise namespace

#endif // ! defined(INCLUDED_BSLMF_NTHPARAMETER)

// ----------------------------------------------------------------------------
// Copyright 2013 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
