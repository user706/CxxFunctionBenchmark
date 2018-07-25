#include <iostream>
#include <random>
#include <string>
#include <chrono>
#include <memory>
#include <boost/type_erasure/any.hpp>
#include <boost/type_erasure/builtin.hpp>
#include <boost/type_erasure/callable.hpp>
#include <boost/function.hpp>
#include "function.h"
#include "delegate.hpp"
#include "FastFunc.hpp"
#include "stdex.hpp"
#include "function2.hpp"
#include "fixed_size_function.hpp"
#include "forwarder.hpp"
#include "StaticFunction.h"
#include "Function.h"
#include "FastDelegate.h"
#include "folly/Function.h"

#ifdef ADD_BDE
#include "bslstl_function.h"
#define PERF_BDE (Perf< bsl::function<int(int)> >)
#else
#define PERF_BDE /**/
#endif

#ifndef _WIN32
  #include "cxx_function.hpp"
#else
  #include "cxx_function_msvc.hpp"
#endif

#include "measure.hpp"  


#define MAX_REPEAT 100000


template<class... Sig>
using multifunction =
    boost::type_erasure::any<
        boost::mpl::vector<
            boost::type_erasure::copy_constructible<>,
            boost::type_erasure::typeid_<>,
            boost::type_erasure::relaxed,
            boost::type_erasure::callable<Sig>...
        >
    >;

int plain(int val)
{
    return val * 2;
}

struct func1
{
    int operator()(int val)
    {
        return val * 2;
    }
    
    int a[10];
};

struct func2
{
    int operator()(int val)
    {
        return val * a;
    }
    
    int& a;
};

struct A
{
    A(): a(2) {}

    int f(int val)
    {
        return val * a;
    }
    
    int a;
};

struct no_abstraction;

typedef generic::delegate<int(int)> generic_delegate;
typedef gnr::forwarder<int(int), 48> gnr_forwarder;
typedef embxx::util::StaticFunction<int(int), 48> embxx_util_StaticFunction;
typedef Function<int(int), 56> Function_;
typedef fastdelegate::FastDelegate1<int, int> FastDelegate1;

namespace cases
{
    template<class F>
    struct base : test::base
    {
        base() {}

        template<class Fn>
        explicit base(Fn&& fn)
          : f(std::forward<Fn>(fn))
        {}
        
        void benchmark()
        {
            this->val += f(this->val);
        }
        
        F f;
    };

    template<class F>
    struct function_pointer : base<F>
    {
        function_pointer()
          : base<F>(&plain)
        {}
    };
    
    template<>
    struct function_pointer<no_abstraction>
      : function_pointer<int(*)(int)>
    {};

    template<class F>
    struct compile_time_function_pointer : base<F>
    {
        compile_time_function_pointer()
          : base<F>(stdex::function_wrapper<int(int), &plain>())
        {}
    };
    
    template<>
    struct compile_time_function_pointer<generic_delegate>
      : base<generic_delegate>
    {
        compile_time_function_pointer()
          : base<generic_delegate>(generic_delegate::from<&plain>())
        {}
    };
    
    template<>
    struct compile_time_function_pointer<no_abstraction>
      : compile_time_function_pointer<stdex::function_wrapper<int(int), &plain> >
    {};

    template<class F>
    struct compile_time_delegate : base<F>
    {
        compile_time_delegate()
          : base<F>(stdex::method_wrapper<A, int(int), &A::f>(&a))
        {}
        
        A a;
    };
    
    template<>
    struct compile_time_delegate<generic_delegate>
      : base<generic_delegate>
    {
        compile_time_delegate()
          : base<generic_delegate>(generic_delegate::from<A, &A::f>(&a))
        {}
        
        A a;
    };
    
    template<>
    struct compile_time_delegate<no_abstraction>
      : compile_time_delegate<stdex::method_wrapper<A, int(int), &A::f> >
    {};
            
    template<class F>
    struct lambda : base<F>
    {
        lambda()
        {
            this->f = [](int val)
            {
                return val * 2;
            };
        }
    };
    
    template<class F>
    struct lambda_capture : base<F>
    {
        lambda_capture()
        {
            int a = 2;
            this->f = [a](int val)
            {
                return val * a;
            };
        }
    };
    
    template<class F>
    struct heavy_functor : base<F>
    {
        heavy_functor()
          : base<F>(func1())
        {}
    };
    
    template<class F>
    struct non_assignable : base<F>
    {
        non_assignable()
          : base<F>(func2{a})
        {}
        
        int a = 2;
    };
}

#ifdef CLUGSTON
#define CLUGSTON_FUNCTION        (Perf< FastDelegate1 >)
#else
#define CLUGSTON_FUNCTION        /* nothing */
#endif

#ifdef SSVU
#define SSVU_FUNCTION           (Perf< ssvu::FastFunc<int(int)> >)
#else
#define SSVU_FUNCTION            /* nothing */
#endif

template<template<class> class Perf>
void benchmark1(char const* name)
{
    std::cout << "[" << name << "]\n";
    BOOST_SPIRIT_TEST_BENCHMARK(
        MAX_REPEAT,
        (Perf< no_abstraction >)
        (Perf< stdex::function<int(int)> >)
        (Perf< std::function<int(int)> >)
        (Perf< cxx_function::function<int(int)> >)
        (Perf< multifunction<int(int)> >)
        (Perf< boost::function<int(int)> >)
        (Perf< func::function<int(int)> >)
        (Perf< generic::delegate<int(int)> >)
        (Perf< fu2::function<int(int)> >)
        CLUGSTON_FUNCTION
        SSVU_FUNCTION
        (Perf< fixed_size_function<int(int)> >)
        (Perf< gnr_forwarder >)
        (Perf< embxx_util_StaticFunction >)
        (Perf< Function_ >)
        (Perf< folly::Function<int(int)> >)
        PERF_BDE
    )
    std::cout << std::endl;
}

template<template<class> class Perf>
void benchmark2(char const* name)
{
    std::cout << "[" << name << "]\n";
    BOOST_SPIRIT_TEST_BENCHMARK(
        MAX_REPEAT,
        (Perf< stdex::function<int(int)> >)
        (Perf< std::function<int(int)> >)
        (Perf< cxx_function::function<int(int)> >)
        (Perf< multifunction<int(int)> >)
        (Perf< boost::function<int(int)> >)
        (Perf< func::function<int(int)> >)
        (Perf< generic::delegate<int(int)> >)
        (Perf< fu2::function<int(int)> >)
        CLUGSTON_FUNCTION
        SSVU_FUNCTION
        (Perf< fixed_size_function<int(int)> >)
        (Perf< gnr_forwarder >)
        (Perf< embxx_util_StaticFunction >)
        (Perf< Function_ >)
        (Perf< folly::Function<int(int)> >)
        PERF_BDE
    )
    std::cout << std::endl;
}

#define BENCHMARK(i, name) benchmark##i<cases::name>(#name)
#define SHOW_SIZE(name) \
std::cout << #name << ": " << sizeof(name) << std::endl;


int main(int /*argc*/, char* /*argv*/[])
{
    std::cout << "[size]\n";
    SHOW_SIZE(stdex::function<int(int)>);
    SHOW_SIZE(std::function<int(int)>);
    SHOW_SIZE(cxx_function::function<int(int)>);
    SHOW_SIZE(multifunction<int(int)>);
    SHOW_SIZE(boost::function<int(int)>);
    SHOW_SIZE(func::function<int(int)>);
    SHOW_SIZE(generic::delegate<int(int)>);
    SHOW_SIZE(FastDelegate1);
    SHOW_SIZE(ssvu::FastFunc<int(int)>);
    SHOW_SIZE(fu2::function<int(int)>);
    SHOW_SIZE(fixed_size_function<int(int)>);
    SHOW_SIZE(gnr_forwarder);
    SHOW_SIZE(embxx_util_StaticFunction);
    SHOW_SIZE(Function_);
    SHOW_SIZE(folly::Function<int(int)>);
#ifdef ADD_BDE
    SHOW_SIZE(bsl::function<int(int)>);
#endif
    std::cout << std::endl;
    
    BENCHMARK(1, function_pointer);
#ifndef CLUGSTON

#ifndef SSVU
    BENCHMARK(1, compile_time_function_pointer);
    BENCHMARK(1, compile_time_delegate);
    BENCHMARK(2, heavy_functor);
    BENCHMARK(2, non_assignable);
#endif
    BENCHMARK(2, lambda_capture);

#endif
    BENCHMARK(2, lambda);
    
    // This is ultimately responsible for preventing all the test code
    // from being optimized away.  Change this to return 0 and you
    // unplug the whole test's life support system.
    return test::live_code != 0;
}
