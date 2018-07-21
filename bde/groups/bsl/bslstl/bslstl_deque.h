// bslstl_deque.h                                                     -*-C++-*-
#ifndef INCLUDED_BSLSTL_DEQUE
#define INCLUDED_BSLSTL_DEQUE

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide an STL-compliant deque class.
//
//@CLASSES:
//  bsl::deque: STL-compliant deque template
//
//@SEE_ALSO: bslstl_vector
//
//@DESCRIPTION: This component defines a single class template, 'bsl::deque',
// implementing the standard sequential container, 'std::deque', holding a
// dynamic sequence of values of a template parameter type.
//
// An instantiation of 'deque' is an allocator-aware, value-semantic type
// whose salient attributes are its size (number of contained elements) and the
// (ordered) sequence of values the deque contains.  If 'deque' is instantiated
// with a value type that is not value-semantic, then the deque will not retain
// all of its value-semantic qualities.  In particular, if a value type cannot
// be tested for equality, then a 'deque' containing elements of that type
// cannot be tested for equality.  It is even possible to instantiate 'deque'
// with a value type that does not have a copy-constructor, in which case the
// 'deque' will not be copyable.
//
// A deque meets the requirements of a sequential container with random access
// iterators in section [deque] of the C++ standard.  The 'deque' implemented
// here adheres to the C++11 standard when compiled with a C++11 compiler, and
// makes the best approximation when compiled with a C++03 compiler.  In
// particular, for C++03 we emulate move semantics, but limit forwarding (in
// 'emplace') to 'const' lvalues, and make no effort to emulate 'noexcept' or
// initializer-lists.
//
///Requirements on 'VALUE_TYPE'
///----------------------------
// A 'deque' is a fully Value-Semantic Type (see {'bsldoc_glossary'}) only if
// the supplied 'VALUE_TYPE' template parameter is fully value-semantic.  It is
// possible to instantiate a 'deque' with a 'VALUE_TYPE' parameter that does
// not have a full set of value-semantic operations, but then some methods of
// the container may not be instantiable.  The following terminology, adopted
// from the C++11 standard, is used in the function documentation of 'deque'
// to describe a function's requirements for the 'VALUE_TYPE' template
// parameter.  These terms are also defined in section [17.6.3.1] of the C++11
// standard.  Note that, in the context of a 'deque' instantiation, the
// requirements apply specifically to the deque's entry type, 'value_type',
// which is an alias for 'VALUE_TYPE'.
//
// Legend
// ------
// 'X'    - denotes an allocator-aware container type (e.g., 'deque')
// 'T'    - 'value_type' associated with 'X'
// 'A'    - type of the allocator used by 'X'
// 'm'    - lvalue of type 'A' (allocator)
// 'p',   - address ('T *') of uninitialized storage for a 'T' within an 'X'
// 'rv'   - rvalue of type (non-'const') 'T'
// 'v'    - rvalue or lvalue of type (possibly 'const') 'T'
// 'args' - 0 or more arguments
//
// The following terms are used to more precisely specify the requirements on
// template parameter types in function-level documentation.
//:
//: *default-insertable*: 'T' has a default constructor.  More precisely, 'T'
//:     is 'default-insertable' into 'X' means that the following expression is
//:     well-formed:
//:
//:      'allocator_traits<A>::construct(m, p)'
//:
//: *move-insertable*: 'T' provides a constructor that takes an rvalue of type
//:     (non-'const') 'T'.  More precisely, 'T' is 'move-insertable' into 'X'
//:     means that the following expression is well-formed:
//:
//:      'allocator_traits<A>::construct(m, p, rv)'
//:
//: *copy-insertable*: 'T' provides a constructor that takes an lvalue or
//:     rvalue of type (possibly 'const') 'T'.  More precisely, 'T' is
//:     'copy-insertable' into 'X' means that the following expression is
//:     well-formed:
//:
//:      'allocator_traits<A>::construct(m, p, v)'
//:
//: *move-assignable*: 'T' provides an assignment operator that takes an rvalue
//:     of type (non-'const') 'T'.
//:
//: *copy-assignable*: 'T' provides an assignment operator that takes an lvalue
//:     or rvalue of type (possibly 'const') 'T'.
//:
//: *emplace-constructible*: 'T' is 'emplace-constructible' into 'X' from
//:     'args' means that the following expression is well-formed:
//:
//:      'allocator_traits<A>::construct(m, p, args)'
//:
//: *erasable*: 'T' provides a destructor.  More precisely, 'T' is 'erasable'
//:     from 'X' means that the following expression is well-formed:
//:
//:      'allocator_traits<A>::destroy(m, p)'
//:
//: *equality-comparable*: The type provides an equality-comparison operator
//:     that defines an equivalence relationship and is both reflexive and
//:     transitive.
//
///Memory Allocation
///-----------------
// The type supplied as a deque's 'ALLOCATOR' template parameter determines
// how that deque will allocate memory.  The 'deque' template supports
// allocators meeting the requirements of the C++03 standard, in addition it
// supports scoped-allocators derived from the 'bslma::Allocator' memory
// allocation protocol.  Clients intending to use 'bslma' style allocators
// should use the template's default 'ALLOCATOR' type: The default type for the
// 'ALLOCATOR' template parameter, 'bsl::allocator', provides a C++11
// standard-compatible adapter for a 'bslma::Allocator' object.
//
///'bslma'-Style Allocators
/// - - - - - - - - - - - -
// If the (template parameter) type 'ALLOCATOR' of a 'deque' instantiation' is
// 'bsl::allocator', then objects of that deque type will conform to the
// standard behavior of a 'bslma'-allocator-enabled type.  Such a deque
// accepts an optional 'bslma::Allocator' argument at construction.  If the
// address of a 'bslma::Allocator' object is explicitly supplied at
// construction, it is used to supply memory for the deque throughout its
// lifetime; otherwise, the deque will use the default allocator installed at
// the time of the deque's construction (see 'bslma_default').  In addition to
// directly allocating memory from the indicated 'bslma::Allocator', a deque
// supplies that allocator's address to the constructors of contained objects
// of the (template parameter) 'VALUE_TYPE' if it defines the
// 'bslma::UsesBslmaAllocator' trait.
//
///Operations
///----------
// This section describes the run-time complexity of operations on instances
// of 'deque':
//..
//  Legend
//  ------
//  'V'                - (template parameter) 'VALUE_TYPE' of the deque
//  'a', 'b'           - two distinct objects of type 'deque<V>'
//  'rv'               - modifiable rvalue of type 'deque<V>'
//  'n', 'm'           - number of elements in 'a' and 'b', respectively
//  'i'                - valid index into the deque
//  'k'                - non-negative integer
//  'al                - an STL-style memory allocator
//  'i1', 'i2'         - two iterators defining a sequence of 'V' objects
//  'il'               - object of type 'std::initializer_list<V>'
//  'lil'              - length of 'il'
//  'vt'               - object of type 'VALUE_TYPE'
//  'rvt'              - modifiable rvalue of type 'VALUE_TYPE'
//  'p1', 'p2'         - two 'const_iterator's belonging to 'a'
//  'distance(i1, i2)' - number of elements in the range '[i1 .. i2)'
//  'minHalf(p1)'      - 'min(distance(a.begin(), p1), distance(p1, a.end()))'
//  'args'             - 0 or more arguments
//
//  |-----------------------------------------+-------------------------------|
//  | Operation                               | Complexity                    |
//  |=========================================+===============================|
//  | deque<V> a       (default construction) | O[1]                          |
//  | deque<V> a(al)                          |                               |
//  |-----------------------------------------+-------------------------------|
//  | deque<V> a(b)    (copy construction)    | O[n]                          |
//  | deque<V> a(b, al)                       |                               |
//  |-----------------------------------------+-------------------------------|
//  | deque<V> a(rv)   (move construction)    | O[1] if 'a' and 'rv' use the  |
//  | deque<V> a(rv, al)                      | same allocator; O[n] otherwise|
//  |-----------------------------------------+-------------------------------|
//  | deque<V> a(k)                           | O[k]                          |
//  | deque<V> a(k, al)                       |                               |
//  |-----------------------------------------+-------------------------------|
//  | deque<V> a(k, vt)                       | O[k]                          |
//  | deque<V> a(k, vt, al)                   |                               |
//  |-----------------------------------------+-------------------------------|
//  | deque<V> a(i1, i2)                      | O[distance(i1, i2)]           |
//  | deque<V> a(i1, i2, al)                  |                               |
//  |-----------------------------------------+-------------------------------|
//  | deque<V> a(il)                          | O[lil]                        |
//  | deque<V> a(il, al)                      |                               |
//  |-----------------------------------------+-------------------------------|
//  | a.~deque<V>()   (destruction)           | O[n]                          |
//  |-----------------------------------------+-------------------------------|
//  | a.assign(k, vt)                         | O[k]                          |
//  |-----------------------------------------+-------------------------------|
//  | a.assign(i1, i2)                        | O[distance(i1, i2)]           |
//  |-----------------------------------------+-------------------------------|
//  | a.assign(il)                            | O[lil]                        |
//  |-----------------------------------------+-------------------------------|
//  | get_allocator()                         | O[1]                          |
//  |-----------------------------------------+-------------------------------|
//  | a.begin(), a.end()                      | O[1]                          |
//  | a.cbegin(), a.cend()                    |                               |
//  | a.rbegin(), a.rend()                    |                               |
//  | a.crbegin(), a.crend()                  |                               |
//  |-----------------------------------------+-------------------------------|
//  | a.size()                                | O[1]                          |
//  |-----------------------------------------+-------------------------------|
//  | a.max_size()                            | O[1]                          |
//  |-----------------------------------------+-------------------------------|
//  | a.resize(k)                             | O[k]                          |
//  | a.resize(k, vt)                         |                               |
//  |-----------------------------------------+-------------------------------|
//  | a.empty()                               | O[1]                          |
//  |-----------------------------------------+-------------------------------|
//  | a.reserve(k)                            | O[k]                          |
//  |-----------------------------------------+-------------------------------|
//  | a.shrink_to_fit()                       | O[n]                          |
//  |-----------------------------------------+-------------------------------|
//  | a[i]                                    | O[1]                          |
//  |-----------------------------------------+-------------------------------|
//  | a.at(i)                                 | O[1]                          |
//  |-----------------------------------------+-------------------------------|
//  | a.front()                               | O[1]                          |
//  |-----------------------------------------+-------------------------------|
//  | a.back()                                | O[1]                          |
//  |-----------------------------------------+-------------------------------|
//  | a.push_back(vt)                         | O[1]                          |
//  | a.push_front(vt)                        |                               |
//  |-----------------------------------------+-------------------------------|
//  | a.push_back(rvt)                        | O[1]                          |
//  | a.push_front(rvt)                       |                               |
//  |-----------------------------------------+-------------------------------|
//  | a.pop_back()                            | O[1]                          |
//  | a.pop_front()                           |                               |
//  |-----------------------------------------+-------------------------------|
//  | a.emplace_back(args)                    | O[1]                          |
//  | a.emplace_front(args)                   |                               |
//  |-----------------------------------------+-------------------------------|
//  | a.emplace(p1, args)                     | O[1 + minHalf(p1)]            |
//  |-----------------------------------------+-------------------------------|
//  | a.insert(p1, vt)                        | O[1 + minHalf(p1)]            |
//  |-----------------------------------------+-------------------------------|
//  | a.insert(p1, rvt)                       | O[1 + minHalf(p1)]            |
//  |-----------------------------------------+-------------------------------|
//  | a.insert(p1, k, vt)                     | O[k + minHalf(p1)]            |
//  |-----------------------------------------+-------------------------------|
//  | a.insert(p1, i1, i2)                    | O[distance(i1, i2)            |
//  |                                         |                + minHalf(p1)] |
//  |-----------------------------------------+-------------------------------|
//  | a.insert(p1, il)                        | O[lil + minHalf(p1)]          |
//  |-----------------------------------------+-------------------------------|
//  | a.erase(p1)                             | O[1 + minHalf(p1)]            |
//  |-----------------------------------------+-------------------------------|
//  | a.erase(p1, p2)                         | O[distance(p1, p2)            |
//  |                                         |  + min(distance(a.begin, p1), |
//  |                                         |        distance(p2, a.end())] |
//  |-----------------------------------------+-------------------------------|
//  | a.swap(b), swap(a,b)                    | O[1] if 'a' and 'b' use the   |
//  |                                         | same allocator; O[n + m]      |
//  |                                         | otherwise                     |
//  |-----------------------------------------+-------------------------------|
//  | a.clear()                               | O[n]                          |
//  |-----------------------------------------+-------------------------------|
//  | a = b;           (copy assignment)      | O[n]                          |
//  |-----------------------------------------+-------------------------------|
//  | a = rv;          (move assignment)      | O[1] if 'a' and 'rv' use the  |
//  |                                         | same allocator; O[n] otherwise|
//  |-----------------------------------------+-------------------------------|
//  | a = il;                                 | O[lil]                        |
//  |-----------------------------------------+-------------------------------|
//  | a == b, a != b                          | O[n]                          |
//  |-----------------------------------------+-------------------------------|
//  | a < b, a <= b, a > b, a >= b            | O[n]                          |
//  |-----------------------------------------+-------------------------------|
//..
//
///Exceptional Behavior
///--------------------
// Since this component is below the BSL STL, we centralize all the exceptional
// behavior into a 'bslstl::StdExceptUtil' class, which has a dual purpose:
//
//: o Remove the dependency of this header on the '<exception>' header, so that
//:   this implementation can offer an exception handler with the native
//:   exceptions, and so that all the C-strings may be defined in a single
//:   library ('bsl') and not in all the translation units including this
//:   header.
//:
//: o Allow installation of exception handlers at a higher level to throw BSL
//:   STL exceptions (which differ from the native exceptions) and thus
//:   establish a full standard compliance for this component when used as
//:   'bsl::deque' in the BSL STL.
//
///Usage
///-----
// In this section we show intended usage of this component.
//
///Example 1: Using a 'deque' to Implement a Laundry Queue
///- - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Suppose we want to define a class to maintain a process queue of names of
// customers who are dropping off their laundry at a drop-off laundry service.
// We can accomplish this by defining a new class characterizing a
// laundry-process queue that uses 'bsl::deque' in its implementation.
//
// The process queue provides two methods, 'push' and 'expeditedPush', for
// inserting names of customers onto the queue.  When calling the 'push'
// method, the customer's name will be inserted at the back of the queue -- his
// laundry will be done after the laundry of customers previously on the queue.
// The 'expeditedPush' method is reserved for customers who have bribed the
// merchant for expedited service.  When calling the 'expeditedPush' method,
// the customer's name will be inserted onto the front of the queue -- his
// laundry will be done before customers previously on the queue.
//
// When the workers are ready to do some laundry, they call the 'next' method
// of the queue, which returns the name of the customer whose laundry is to be
// done next.  For brevity of the usage example, we do not show how customers
// are track while or after their laundry is being done.
//
// In addition, the laundry queue also provides the 'find' method, which
// returns a 'bool' to indicate whether a given customer is still in the queue.
//
// First, we declare a class 'LaundryQueue' based on a deque, to store names of
// customers at a drop-off laundry:
//..
//  class LaundryQueue {
//      // This 'class' keeps track of customers enqueued to have their laundry
//      // done by a laundromat.
//
//      // DATA
//      bsl::deque<bsl::string> d_queue;
//
//    public:
//      // CREATORS
//      LaundryQueue(bslma::Allocator *basicAllocator = 0);
//          // Create a 'LaundryQueue' object using the specified
//          // 'basicAllocator'.  If 'basicAllocator' is not provided, use the
//          // default allocator.
//
//      // MANIPULATORS
//      void push(const bsl::string& customerName);
//          // Add the specified 'customerName' to the back of the laundry
//          // queue.
//
//      void expeditedPush(const bsl::string& customerName);
//          // Add the specified 'customerName' to the laundry queue at the
//          // front.
//
//      bsl::string next();
//          // Return the name from the front of the queue, removing it from
//          // the queue.  If the queue is empty, return '(* empty *)' which is
//          // not a valid name for a customer.
//
//      // ACCESSORS
//      bool find(const bsl::string& customerName);
//          // Return 'true' if 'customerName' is in the queue, and 'false'
//          // otherwise.
//  };
//..
// Then, we define the implementation of the methods of 'LaundryQueue'
//..
// CREATORS
//  LaundryQueue::LaundryQueue(bslma::Allocator *basicAllocator)
//  : d_queue(basicAllocator)
//  {
//      // Note that the allocator is propagated to the underlying 'deque',
//      // which will use the default allocator is '0 == basicAllocator'.
//  }
//
// MANIPULATORS
//  void LaundryQueue::push(const bsl::string& customerName)
//  {
//      d_queue.push_back(customerName);     // note constant time
//  }
//
//  void LaundryQueue::expeditedPush(const bsl::string& customerName)
//  {
//      d_queue.push_front(customerName);    // note constant time
//  }
//
//  bsl::string LaundryQueue::next()
//  {
//      if (d_queue.empty()) {
//          return "(* empty *)";
//      }
//
//      bsl::string ret = d_queue.front();   // note constant time
//
//      d_queue.pop_front();                 // note constant time
//
//      return ret;
//  }
//
//  // ACCESSORS
//  bool LaundryQueue::find(const bsl::string& customerName)
//  {
//      // Note 'd_queue.empty() || d_queue[0] == d_queue.front()'
//
//      for (size_t i = 0; i < d_queue.size(); ++i) {
//          if (customerName == d_queue[i]) {    // note '[]' is constant time
//              return true;
//          }
//      }
//
//      return false;
//  }
//..

// Prevent 'bslstl' headers from being included directly in 'BSL_OVERRIDES_STD'
// mode.  Doing so is unsupported, and is likely to cause compilation errors.
#if defined(BSL_OVERRIDES_STD) && !defined(BSL_STDHDRS_PROLOGUE_IN_EFFECT)
#error "include <bsl_deque.h> instead of <bslstl_deque.h> in \
BSL_OVERRIDES_STD mode"
#endif

#ifndef INCLUDED_BSLSCM_VERSION
#include <bslscm_version.h>
#endif

#ifndef INCLUDED_BSLSTL_ITERATOR
#include <bslstl_iterator.h>
#endif

#ifndef INCLUDED_BSLSTL_RANDOMACCESSITERATOR
#include <bslstl_randomaccessiterator.h>
#endif

#ifndef INCLUDED_BSLSTL_STDEXCEPTUTIL
#include <bslstl_stdexceptutil.h>
#endif

#ifndef INCLUDED_BSLALG_CONTAINERBASE
#include <bslalg_containerbase.h>
#endif

#ifndef INCLUDED_BSLALG_DEQUEIMPUTIL
#include <bslalg_dequeimputil.h>
#endif

#ifndef INCLUDED_BSLALG_DEQUEITERATOR
#include <bslalg_dequeiterator.h>
#endif

#ifndef INCLUDED_BSLALG_DEQUEPRIMITIVES
#include <bslalg_dequeprimitives.h>
#endif

#ifndef INCLUDED_BSLALG_RANGECOMPARE
#include <bslalg_rangecompare.h>
#endif

#ifndef INCLUDED_BSLALG_SCALARDESTRUCTIONPRIMITIVES
#include <bslalg_scalardestructionprimitives.h>
#endif

#ifndef INCLUDED_BSLALG_SWAPUTIL
#include <bslalg_swaputil.h>
#endif

#ifndef INCLUDED_BSLALG_TYPETRAITHASSTLITERATORS
#include <bslalg_typetraithasstliterators.h>
#endif

#ifndef INCLUDED_BSLMA_ALLOCATORTRAITS
#include <bslma_allocatortraits.h>
#endif

#ifndef INCLUDED_BSLMA_STDALLOCATOR
#include <bslma_stdallocator.h>
#endif

#ifndef INCLUDED_BSLMA_USESBSLMAALLOCATOR
#include <bslma_usesbslmaallocator.h>
#endif

#ifndef INCLUDED_BSLMF_ASSERT
#include <bslmf_assert.h>
#endif

#ifndef INCLUDED_BSLMF_ISSAME
#include <bslmf_issame.h>
#endif

#ifndef INCLUDED_BSLMF_MATCHANYTYPE
#include <bslmf_matchanytype.h>
#endif

#ifndef INCLUDED_BSLMF_MATCHARITHMETICTYPE
#include <bslmf_matcharithmetictype.h>
#endif

#ifndef INCLUDED_BSLMF_MOVABLEREF
#include <bslmf_movableref.h>
#endif

#ifndef INCLUDED_BSLMF_NIL
#include <bslmf_nil.h>
#endif

#ifndef INCLUDED_BSLS_ASSERT
#include <bsls_assert.h>
#endif

#ifndef INCLUDED_BSLS_COMPILERFEATURES
#include <bsls_compilerfeatures.h>
#endif

#ifndef INCLUDED_BSLS_CPP11
#include <bsls_cpp11.h>
#endif

#ifndef INCLUDED_BSLS_NATIVESTD
#include <bsls_nativestd.h>
#endif

#ifndef INCLUDED_BSLS_PERFORMANCEHINT
#include <bsls_performancehint.h>
#endif

#ifndef INCLUDED_BSLS_UTIL
#include <bsls_util.h>
#endif

#ifndef INCLUDED_CSTRING
#include <cstring>
#define INCLUDED_CSTRING
#endif

#if defined(BSLS_COMPILERFEATURES_SUPPORT_GENERALIZED_INITIALIZERS)
#ifndef INCLUDED_INITIALIZER_LIST
#include <initializer_list>
#define INCLUDED_INITIALIZER_LIST
#endif
#endif

#ifndef BDE_DONT_ALLOW_TRANSITIVE_INCLUDES

#ifndef INCLUDED_STDEXCEPT
#include <stdexcept>
#define INCLUDED_STDEXCEPT
#endif

#endif

namespace bsl {

template <class VALUE_TYPE, class ALLOCATOR>
class Deque_BlockCreator;
template <class VALUE_TYPE, class ALLOCATOR>
class Deque_BlockProctor;
template <class VALUE_TYPE, class ALLOCATOR>
class Deque_ClearGuard;
template <class VALUE_TYPE, class ALLOCATOR>
class Deque_Guard;

                       // ================================
                       // struct Deque_BlockLengthCalcUtil
                       // ================================

template <class VALUE_TYPE>
struct Deque_BlockLengthCalcUtil {
    // This 'struct' provides a namespace for the calculation of block length
    // (the number of elements per block within a 'deque').  This ensures that
    // each block in the deque can hold at least 16 elements.

    // TYPES
    enum {
        DEFAULT_BLOCK_SIZE = 200,  // number of bytes per block

        BLOCK_LENGTH       = (16 * sizeof(VALUE_TYPE) >= DEFAULT_BLOCK_SIZE)
                             ? 16
                             : (DEFAULT_BLOCK_SIZE / sizeof(VALUE_TYPE))
                                   // number of elements per block
    };
};

                          // =================
                          // struct Deque_Util
                          // =================

struct Deque_Util {
    // This 'struct' provides a namespace to implement the 'swap' member
    // function of 'deque<VALUE_TYPE, ALLOCATOR>'.  This function can be
    // implemented irrespective of the 'VALUE_TYPE' or 'ALLOCATOR' template
    // parameters, which is why we implement it in this non-parameterized,
    // non-inlined utility.

    // CLASS METHODS
    static void move(void *dst, void *src);
        // Assign the value of the specified 'dst' deque to that of the
        // specified 'src' deque, and reset the 'src' deque to a raw state.

    static void swap(void *a, void *b);
        // Exchange the value of the specified 'a' deque with that of the
        // specified 'b' deque.
};

                        // ================
                        // class Deque_Base
                        // ================

template <class VALUE_TYPE>
class Deque_Base {
    // This class describes the basic layout for a deque class.  It is
    // important that this class has the same layout as the deque class
    // implementation.  It is parameterized by 'VALUE_TYPE' only and implements
    // the portion of 'bsl::deque' that does not need to know about its
    // (template parameter) type 'ALLOCATOR' (in order to generate shorter
    // debug strings).  Note that this class must have the same layout as
    // 'Deque_Imp' (see implementation file).

    // PRIVATE TYPES
    enum {
        BLOCK_LENGTH = Deque_BlockLengthCalcUtil<VALUE_TYPE>::BLOCK_LENGTH
    };

    typedef BloombergLP::bslalg::DequeImpUtil<VALUE_TYPE,
                                              BLOCK_LENGTH>    Imp;
    typedef typename Imp::Block                                Block;
    typedef typename Imp::BlockPtr                             BlockPtr;
    typedef BloombergLP::bslalg::DequeIterator<VALUE_TYPE,
                                               BLOCK_LENGTH>   IteratorImp;
    typedef BloombergLP::
            bslstl::RandomAccessIterator<VALUE_TYPE, IteratorImp>
                                                               Iterator;

    typedef BloombergLP::
            bslstl::RandomAccessIterator<const VALUE_TYPE, IteratorImp>
                                                               ConstIterator;

  public:
    // PUBLIC TYPES
    typedef VALUE_TYPE&                             reference;
    typedef const VALUE_TYPE&                       const_reference;
    typedef Iterator                                iterator;
    typedef ConstIterator                           const_iterator;
    typedef std::size_t                             size_type;
    typedef std::ptrdiff_t                          difference_type;
    typedef VALUE_TYPE                              value_type;

    // For consistent behavior on all compilers, we must 'bsl::'-qualify
    // 'reverse_iterator'.  (For most compilers, 'reverse_iterator' is in
    // namespace 'native_std', and NOT in namespace 'bsl'.  Hence, we need to
    // actively look into a different namespace to find the iterator.  However,
    // on Solaris we explicitly provide a 'reverse_iterator' implementation, IN
    // namespace 'bsl', to replace their broken one.  See 'bslstl_iterator.h'.)

    typedef bsl::reverse_iterator<Iterator>         reverse_iterator;
    typedef bsl::reverse_iterator<ConstIterator>    const_reverse_iterator;

  protected:
    // PROTECTED DATA
    BlockPtr    *d_blocks_p;      // array of pointer to blocks (owned)
    std::size_t  d_blocksLength;  // length of 'd_blocks_p' array
    IteratorImp  d_start;         // iterator to first element
    IteratorImp  d_finish;        // iterator to one past last element

  public:
    // MANIPULATORS

    // *** iterators ***

    iterator begin() BSLS_CPP11_NOEXCEPT;
        // Return an iterator providing modifiable access to the first element
        // in this deque, and the past-the-end iterator if this deque is empty.

    iterator end() BSLS_CPP11_NOEXCEPT;
        // Return the past-the-end (forward) iterator providing modifiable
        // access to this deque.

    reverse_iterator rbegin() BSLS_CPP11_NOEXCEPT;
        // Return a reverse iterator providing modifiable access to the last
        // element in this deque, and the past-the-end reverse iterator if this
        // deque is empty.

    reverse_iterator rend() BSLS_CPP11_NOEXCEPT;
        // Return the past-the-end reverse iterator providing modifiable access
        // to this deque.

    // *** element access ***

    reference operator[](size_type position);
        // Return a reference providing modifiable access to the element at the
        // specified 'position' in this deque.  The behavior is undefined
        // unless 'position < size()'.

    reference at(size_type position);
        // Return a reference providing modifiable access to the element at the
        // specified 'position' in this deque.  Throw a 'bsl::out_of_range'
        // exception if 'position >= size()'.

    reference front();
        // Return a reference providing modifiable access to the first element
        // in this deque.  The behavior is undefined unless this deque is not
        // empty.

    reference back();
        // Return a reference providing modifiable access to the last element
        // in this deque.  The behavior is undefined unless this deque is not
        // empty.

    // ACCESSORS

    // *** iterators ***

    const_iterator  begin() const BSLS_CPP11_NOEXCEPT;
    const_iterator cbegin() const BSLS_CPP11_NOEXCEPT;
        // Return an iterator providing non-modifiable access to the first
        // element in this deque, and the past-the-end iterator if this deque
        // is empty.

    const_iterator  end() const BSLS_CPP11_NOEXCEPT;
    const_iterator cend() const BSLS_CPP11_NOEXCEPT;
        // Return the past-the-end (forward) iterator providing non-modifiable
        // access to this deque.

    const_reverse_iterator  rbegin() const BSLS_CPP11_NOEXCEPT;
    const_reverse_iterator crbegin() const BSLS_CPP11_NOEXCEPT;
        // Return a reverse iterator providing non-modifiable access to the
        // last element in this deque, and the past-the-end reverse iterator if
        // this deque is empty.

    const_reverse_iterator rend() const BSLS_CPP11_NOEXCEPT;
    const_reverse_iterator crend() const BSLS_CPP11_NOEXCEPT;
        // Return the past-the-end reverse iterator providing non-modifiable
        // access to this deque.

    // *** capacity ***

    size_type size() const BSLS_CPP11_NOEXCEPT;
        // Return the number of elements contained by this deque.

    size_type capacity() const BSLS_CPP11_NOEXCEPT;
        // Return the sum of the current size of this deque plus the minimum
        // number of 'push_front' or 'push_back' operations needed to
        // invalidate iterators in this deque.  Note that this method is not
        // part of the C++ standard.

    bool empty() const BSLS_CPP11_NOEXCEPT;
        // Return 'true' if this deque contains no elements, and 'false'
        // otherwise.

    // *** element access ***

    const_reference operator[](size_type position) const;
        // Return a reference providing non-modifiable access to the element at
        // the specified 'position' in this deque.  The behavior is undefined
        // unless 'position < size()'.

    const_reference at(size_type position) const;
        // Return a reference providing non-modifiable access to the element at
        // the specified 'position' in this deque.  Throw a 'bsl::out_of_range'
        // exception if 'position >= size()'.

    const_reference front() const;
        // Return a reference providing non-modifiable access to the first
        // element in this deque.  The behavior is undefined unless this deque
        // is not empty.

    const_reference back() const;
        // Return a reference providing non-modifiable access to the last
        // element in this deque.  The behavior is undefined unless this deque
        // is not empty.
};

                        // ===========
                        // class deque
                        // ===========

template <class VALUE_TYPE, class ALLOCATOR = allocator<VALUE_TYPE> >
class deque : public  Deque_Base<VALUE_TYPE>
            , private BloombergLP::bslalg::ContainerBase<ALLOCATOR> {
    // This class template provides an STL-compliant 'deque' that conforms to
    // the 'bslma::Allocator' model.  For the requirements of a deque class,
    // consult the C++11 standard.  In particular, this implementation offers
    // the general rules that:
    //..
    //: 1 A call to any method that would result in a deque having a size
    //:   greater than the value returned by 'max_size' triggers a call to
    //:   'bslstl::StdExceptUtil::throwLengthError'.
    //:
    //: 2 A call to an 'at' method that attempts to access a position outside
    //:   of the valid range of a deque triggers a call to
    //:   'bslstl::StdExceptUtil::throwOutOfRange'.
    //..
    // Note that portions of the standard methods are implemented in
    // 'Deque_Base', which is parameterized on only 'VALUE_TYPE' in order to
    // generate smaller debug strings.
    //
    // This class:
    //: o supports a complete set of *value-semantic* operations
    //:   o except for 'BDEX' serialization
    //: o is *exception-neutral*
    //: o is *alias-safe*
    //: o is 'const' *thread-safe*
    // For terminology see {'bsldoc_glossary'}.
    //
    // In addition, the following members offer a full guarantee of rollback:
    // if an exception is thrown during the invocation of 'insert',
    // 'push_front', or 'push_back' on a pre-existing object, the object is
    // left in a valid state and its value is unchanged.

    // PRIVATE TYPES
    enum {
        BLOCK_LENGTH = Deque_BlockLengthCalcUtil<VALUE_TYPE>::BLOCK_LENGTH
    };

    typedef Deque_Base<VALUE_TYPE>                             Base;

    typedef BloombergLP::bslalg::ContainerBase<ALLOCATOR>      ContainerBase;

    typedef BloombergLP::bslalg::DequeImpUtil<VALUE_TYPE,
                                             BLOCK_LENGTH>     Imp;
    typedef typename Imp::Block                                Block;
    typedef typename Imp::BlockPtr                             BlockPtr;

    typedef BloombergLP::bslalg::DequeIterator<VALUE_TYPE,
                                              BLOCK_LENGTH>    IteratorImp;

    typedef BloombergLP::
            bslstl::RandomAccessIterator<VALUE_TYPE,
                                        IteratorImp>           Iterator;

    typedef BloombergLP::
            bslstl::RandomAccessIterator<const VALUE_TYPE,
                                        IteratorImp>           ConstIterator;

    typedef Deque_BlockCreator<VALUE_TYPE, ALLOCATOR>          BlockCreator;
    typedef Deque_BlockProctor<VALUE_TYPE, ALLOCATOR>          BlockProctor;
    typedef Deque_ClearGuard<VALUE_TYPE, ALLOCATOR>            ClearGuard;
    typedef Deque_Guard<VALUE_TYPE, ALLOCATOR>                 Guard;

    typedef BloombergLP::bslalg::DequePrimitives<VALUE_TYPE,
                                                BLOCK_LENGTH>  DequePrimitives;

    typedef bsl::allocator_traits<ALLOCATOR>                   AllocatorTraits;

    typedef BloombergLP::bslmf::MovableRefUtil                 MoveUtil;
        // This typedef is a convenient alias for the utility associated with
        // movable references.

    enum RawInit { k_RAW_INIT = 0 };
        // Special type (and value) used to create a "raw" deque, which has a
        // block length of 0, and null start and finish pointers.

  public:
    // PUBLIC TYPES
    typedef VALUE_TYPE&                             reference;
    typedef const VALUE_TYPE&                       const_reference;
    typedef Iterator                                iterator;
    typedef ConstIterator                           const_iterator;
    typedef std::size_t                             size_type;
    typedef std::ptrdiff_t                          difference_type;
    typedef VALUE_TYPE                              value_type;

    typedef ALLOCATOR                               allocator_type;
    typedef typename ALLOCATOR::pointer             pointer;
    typedef typename ALLOCATOR::const_pointer       const_pointer;

    // As above, we must 'bsl::'-qualify 'reverse_iterator'.

    typedef bsl::reverse_iterator<Iterator>         reverse_iterator;
    typedef bsl::reverse_iterator<ConstIterator>    const_reverse_iterator;

  private:
    // STATIC ASSERTIONS

    BSLMF_ASSERT((is_same<reference, typename Base::reference>::value));
    BSLMF_ASSERT((is_same<const_reference,
                  typename Base::const_reference>::value));
        // These need not necessarily be true as per the C++ standard, but are
        // safe assumptions for this implementation and allow us to implement
        // the element access within the 'Base' type (that is parameterized by
        // 'VALUE_TYPE' only).

    // PRIVATE CREATORS
    deque(RawInit, const allocator_type& allocator);
        // Create a "raw" deque (i.e., a deque that obeys the raw deque
        // invariants and is destructible) that uses the specified 'allocator'
        // to supply memory.  The following holds for a raw deque:
        // '0 == d_blocks_p', '0 == d_blocksLength', and 'd_start' and
        // 'd_finish' are singular iterators (i.e., have null internal
        // pointers).  Note that the constructed deque contains no allocated
        // storage.  Also note that the purpose of a raw deque is to provide an
        // exception-safe repository for intermediate calculations.

    // PRIVATE MANIPULATORS
    template <class INPUT_ITERATOR>
    size_type privateAppend(INPUT_ITERATOR                  first,
                            INPUT_ITERATOR                  last,
                            std::input_iterator_tag);
    template <class INPUT_ITERATOR>
    size_type privateAppend(INPUT_ITERATOR                  first,
                            INPUT_ITERATOR                  last,
                            std::random_access_iterator_tag);
        // Append the elements in the range specified by '[first .. last)' to
        // this deque, and return the number of elements appended.  The third
        // argument is used for overload resolution.  The behavior is undefined
        // unless 'first' and 'last' refer to a sequence of valid values where
        // 'first' is at a position at or before 'last'.

    void privateAppendRaw(size_type numElements, const VALUE_TYPE& value);
        // Append the specified 'numElements' copies of the specified 'value'
        // to this deque.

    void privateInit(size_type numElements);
        // Initialize 'd_start' and 'd_finish' for eventual insertion of the
        // specified 'numElements' elements.  After this method returns, this
        // object is fully constructed with memory allocated for 'numElements',
        // with all member variables obeying the class invariants, and with
        // size 0.  The behavior is undefined unless this object is in a "raw"
        // state.  Note that this method must not be called while constructing
        // this object (although a constructor may call it for a temporary
        // object).

    template <class INTEGER_TYPE>
    void privateInsertDispatch(
                           const_iterator                          position,
                           INTEGER_TYPE                            numElements,
                           INTEGER_TYPE                            value,
                           BloombergLP::bslmf::MatchArithmeticType,
                           BloombergLP::bslmf::Nil);
        // Insert the specified 'numElements' copies of the specified 'value'
        // into this deque at the specified 'position'.  This overload matches
        // 'privateInsert' when the second and third arguments are of the same
        // type which happens to be an integral type.  The fourth and fifth
        // arguments are used for overload resolution only.

    template <class INPUT_ITERATOR>
    void privateInsertDispatch(const_iterator                   position,
                               INPUT_ITERATOR                   first,
                               INPUT_ITERATOR                   last,
                               BloombergLP::bslmf::MatchAnyType,
                               BloombergLP::bslmf::MatchAnyType);
        // Insert the elements in the range specified by '[first .. last)' into
        // this deque at the specified 'position'.  The third and fourth
        // arguments are used for overload resolution only, so that this
        // function is not called if 'first' and 'last' are of integral type.
        // The behavior is undefined unless 'first' and 'last' refer to a
        // sequence of valid values where 'first' is at a position at or before
        // 'last'.

    template <class INPUT_ITERATOR>
    void privateInsert(const_iterator                  position,
                       INPUT_ITERATOR                  first,
                       INPUT_ITERATOR                  last,
                       std::input_iterator_tag);
        // Specialized insertion for input iterators.

    template <class INPUT_ITERATOR>
    void privateInsert(const_iterator                  position,
                       INPUT_ITERATOR                  first,
                       INPUT_ITERATOR                  last,
                       std::random_access_iterator_tag);
        // Specialized insertion for forward, bidirectional, and random-access
        // iterators.

    void privateJoinPrepend(deque *other);
        // Join '*this' and the specified 'other' deque into one.  After the
        // join, '*this' contains the concatenated sequence, in order, of
        // '*other' and '*this' elements, and 'other is made a raw deque.

    void privateJoinAppend(deque *other);
        // Join '*this' and the specified 'other' deque into one.  After the
        // join, '*this' contains the concatenated sequence, in order, of
        // '*this' and '*other' elements, and 'other is made a raw deque.

    template <class INPUT_ITERATOR>
    size_type privatePrepend(INPUT_ITERATOR                  first,
                             INPUT_ITERATOR                  last,
                             std::input_iterator_tag);
    template <class INPUT_ITERATOR>
    size_type privatePrepend(INPUT_ITERATOR                  first,
                             INPUT_ITERATOR                  last,
                             std::bidirectional_iterator_tag);
    template <class INPUT_ITERATOR>
    size_type privatePrepend(INPUT_ITERATOR                  first,
                             INPUT_ITERATOR                  last,
                             std::random_access_iterator_tag);
        // Prepend the elements in the range specified by '[first .. last)' to
        // this deque, and return the number of elements prepended.  The third
        // argument is used for overload resolution only.  The behavior is
        // undefined unless 'first' and 'last' refer to a sequence of valid
        // values where 'first' is at a position at or before 'last'.

    void privatePrependRaw(size_type numElements, const VALUE_TYPE& value);
        // Prepend the specified 'numElements' copies of the specified 'value'
        // to this deque.

    void privateSplit(deque *other, IteratorImp pos);
        // Split this deque in two such that after the split, '*this' contains
        // elements formerly in the range specified by '[d_start .. pos)' and
        // the specified 'other' deque contains elements formerly in the range
        // '[pos .. d_finish)'.  The behavior is undefined unless 'other' is a
        // raw deque, i.e., the 'RawInit' constructor was used to create
        // 'other'.

    // FRIENDS
    template <class VALUE_TYPE2, class ALLOCATOR2>
    friend class Deque_BlockCreator;

    template <class VALUE_TYPE2, class ALLOCATOR2>
    friend class Deque_BlockProctor;

    template <class VALUE_TYPE2, class ALLOCATOR2>
    friend class Deque_Guard;

  public:
    // CREATORS

    // *** construct/copy/destroy ***

    deque();
    explicit deque(const ALLOCATOR& basicAllocator);
        // Create an empty deque.  Optionally specify a 'basicAllocator' used
        // to supply memory.  If 'basicAllocator' is not supplied, a
        // default-constructed object of the (template parameter) type
        // 'ALLOCATOR' is used.  If the type 'ALLOCATOR' is 'bsl::allocator'
        // (the default), then 'basicAllocator', if supplied, shall be
        // convertible to 'bslma::Allocator *'.  If the type 'ALLOCATOR' is
        // 'bsl::allocator' and 'basicAllocator' is not supplied, the currently
        // installed default allocator is used.

    explicit
    deque(size_type        numElements,
          const ALLOCATOR& basicAllocator = ALLOCATOR());
        // Create a deque of the specified 'numElements' size whose every
        // element is default-constructed.  Optionally specify a
        // 'basicAllocator' used to supply memory.  If 'basicAllocator' is not
        // supplied, a default-constructed object of the (template parameter)
        // type 'ALLOCATOR' is used.  If the type 'ALLOCATOR' is
        // 'bsl::allocator' (the default), then 'basicAllocator', if supplied,
        // shall be convertible to 'bslma::Allocator *'.  If the type
        // 'ALLOCATOR' is 'bsl::allocator' and 'basicAllocator' is not
        // supplied, the currently installed default allocator is used.  Throw
        // 'bsl::length_error' if 'numElements > max_size()'.  This method
        // requires that the (template parameter) 'VALUE_TYPE' be
        // 'default-insertable' into this deque (see {Requirements on
        // 'VALUE_TYPE'}).

    deque(size_type         numElements,
          const VALUE_TYPE& value,
          const ALLOCATOR&  basicAllocator = ALLOCATOR());
        // Create a deque of the specified 'numElements' size whose every
        // element has the specified 'value'.  Optionally specify a
        // 'basicAllocator' used to supply memory.  If 'basicAllocator' is not
        // supplied, a default-constructed object of the (template parameter)
        // type 'ALLOCATOR' is used.  If the type 'ALLOCATOR' is
        // 'bsl::allocator' (the default), then 'basicAllocator', if supplied,
        // shall be convertible to 'bslma::Allocator *'.  If the type
        // 'ALLOCATOR' is 'bsl::allocator' and 'basicAllocator' is not
        // supplied, the currently installed default allocator is used.  Throw
        // 'bsl::length_error' if 'numElements > max_size()'.  This method
        // requires that the (template parameter) 'VALUE_TYPE' be
        // 'copy-insertable' into this deque (see {Requirements on
        // 'VALUE_TYPE'}).

    template <class INPUT_ITERATOR>
    deque(INPUT_ITERATOR   first,
          INPUT_ITERATOR   last,
          const ALLOCATOR& basicAllocator = ALLOCATOR());
        // Create a deque initially containing copies of the values in the
        // range starting at the specified 'first' and ending immediately
        // before the specified 'last' iterators of the (template parameter)
        // type 'INPUT_ITERATOR'.  Optionally specify a 'basicAllocator' used
        // to supply memory.  If 'basicAllocator' is not supplied, a
        // default-constructed object of the (template parameter) type
        // 'ALLOCATOR' is used.  If the type 'ALLOCATOR' is 'bsl::allocator'
        // (the default), then 'basicAllocator', if supplied, shall be
        // convertible to 'bslma::Allocator *'.  If the type 'ALLOCATOR' is
        // 'bsl::allocator' and 'basicAllocator' is not supplied, the currently
        // installed default allocator is used.  Throw 'bsl::length_error' if
        // the number of elements in '[first .. last)' exceeds the size
        // returned by 'max_size'.  The (template parameter) type
        // 'INPUT_ITERATOR' shall meet the requirements of an input iterator
        // defined in the C++11 standard [input.iterators] providing access to
        // values of a type convertible to 'value_type', and 'value_type' must
        // be 'emplace-constructible' from '*i' into this deque, where 'i' is a
        // dereferenceable iterator in the range '[first .. last)' (see
        // {Requirements on 'VALUE_TYPE'}).  The behavior is undefined unless
        // 'first' and 'last' refer to a sequence of valid values where 'first'
        // is at a position at or before 'last'.

    deque(const deque& original);
        // Create a deque that has the same value as the specified 'original'
        // object.  Use the allocator returned by
        // 'bsl::allocator_traits<ALLOCATOR>::
        // select_on_container_copy_construction(original.get_allocator())' to
        // supply memory.  If the (template parameter) type 'ALLOCATOR' is
        // 'bsl::allocator' (the default), the currently installed default
        // allocator is used.  This method requires that the (template
        // parameter) 'VALUE_TYPE' be 'copy-insertable' into this deque (see
        // {Requirements on 'VALUE_TYPE'}).

    deque(const deque& original, const ALLOCATOR& basicAllocator);
        // Create a deque that has the same value as the specified 'original'
        // object and that uses the specified 'basicAllocator' to supply
        // memory.  This method requires that the (template parameter)
        // 'VALUE_TYPE' be 'copy-insertable' into this deque (see
        // {Requirements on 'VALUE_TYPE'}).  Note that a 'bslma::Allocator *'
        // can be supplied for 'basicAllocator' if the (template parameter)
        // type 'ALLOCATOR' is 'bsl::allocator' (the default).

    deque(BloombergLP::bslmf::MovableRef<deque> original);          // IMPLICIT
        // Create a deque having the same value as the specified 'original'
        // object by moving (in constant time) the contents of 'original' to
        // the new deque.  The allocator associated with 'original' is
        // propagated for use in the newly-created deque.  'original' is left
        // in a valid but unspecified state.

    deque(BloombergLP::bslmf::MovableRef<deque> original,
          const ALLOCATOR&                      basicAllocator);
        // Create a deque having the same value as the specified 'original'
        // object that uses the specified 'basicAllocator' to supply memory.
        // The contents of 'original' are moved (in constant time) to the new
        // deque if 'basicAllocator == original.get_allocator()', and are move-
        // inserted (in linear time) using 'basicAllocator' otherwise.
        // 'original' is left in a valid but unspecified state.  This method
        // requires that the (template parameter) 'VALUE_TYPE' be
        // 'move-insertable' into this deque (see {Requirements on
        // 'VALUE_TYPE'}).  Note that a 'bslma::Allocator *' can be supplied
        // for 'basicAllocator' if the (template parameter) 'ALLOCATOR' is
        // 'bsl::allocator' (the default).

#if defined(BSLS_COMPILERFEATURES_SUPPORT_GENERALIZED_INITIALIZERS)
    deque(std::initializer_list<value_type> values,
          const ALLOCATOR&                  basicAllocator = ALLOCATOR());
        // Create a deque and append each 'value_type' object in the specified
        // 'values' initializer list.  Optionally specify a 'basicAllocator'
        // used to supply memory.  If 'basicAllocator' is not supplied, a
        // default-constructed object of the (template parameter) type
        // 'ALLOCATOR' is used.  If the type 'ALLOCATOR' is 'bsl::allocator'
        // (the default), then 'basicAllocator', if supplied, shall be
        // convertible to 'bslma::Allocator *'.  If the type 'ALLOCATOR' is
        // 'bsl::allocator' and 'basicAllocator' is not supplied, the currently
        // installed default allocator is used.  This method requires that the
        // (template parameter) 'VALUE_TYPE' be 'copy-insertable' into this
        // deque (see {Requirements on 'VALUE_TYPE'}).
#endif

    ~deque();
        // Destroy this object.

    // MANIPULATORS
    deque& operator=(const deque& rhs);
        // Assign to this object the value of the specified 'rhs' object,
        // propagate to this object the allocator of 'rhs' if the 'ALLOCATOR'
        // type has trait 'propagate_on_container_copy_assignment', and return
        // a reference providing modifiable access to this object.  If an
        // exception is thrown, '*this' is left in a valid but unspecified
        // state.  This method requires that the (template parameter)
        // 'VALUE_TYPE' be 'copy-assignable' and 'copy-insertable' into this
        // deque (see {Requirements on 'VALUE_TYPE'}).

    deque& operator=(BloombergLP::bslmf::MovableRef<deque> rhs)
             BSLS_CPP11_NOEXCEPT_SPECIFICATION(BSLS_CPP11_PROVISIONALLY_FALSE);
        // Assign to this object the value of the specified 'rhs' object,
        // propagate to this object the allocator of 'rhs' if the 'ALLOCATOR'
        // type has trait 'propagate_on_container_move_assignment', and return
        // a reference providing modifiable access to this object.  The
        // contents of 'rhs' are moved (in constant time) to this deque if
        // 'get_allocator() == rhs.get_allocator()' (after accounting for the
        // aforementioned trait); otherwise, all elements in this deque are
        // either destroyed or move-assigned to and each additional element in
        // 'rhs' is move-inserted into this deque.  'rhs' is left in a valid
        // but unspecified state, and if an exception is thrown, '*this' is
        // left in a valid but unspecified state.  This method requires that
        // the (template parameter) 'VALUE_TYPE' be 'move-assignable' and
        // 'move-insertable' into this deque (see {Requirements on
        // 'VALUE_TYPE'}).

#if defined(BSLS_COMPILERFEATURES_SUPPORT_GENERALIZED_INITIALIZERS)
    deque& operator=(std::initializer_list<value_type> values);
        // Assign to this object the value resulting from first clearing this
        // deque and then appending each 'value_type' object in the specified
        // 'values' initializer list; return a reference providing modifiable
        // access to this object.  This method requires that the (template
        // parameter) 'VALUE_TYPE' be 'copy-insertable' into this deque (see
        // {Requirements on 'VALUE_TYPE'}).
#endif

    template <class INPUT_ITERATOR>
    void assign(INPUT_ITERATOR first, INPUT_ITERATOR last);
        // Assign to this deque the values in the range starting at the
        // specified 'first' and ending immediately before the specified 'last'
        // iterators of the (template parameter) type 'INPUT_ITERATOR'.  The
        // (template parameter) type 'INPUT_ITERATOR' shall meet the
        // requirements of an input iterator defined in the C++11 standard
        // [input.iterators] providing access to values of a type convertible
        // to 'value_type', and 'value_type' must be 'copy-assignable' and
        // 'emplace-constructible' from '*i' into this deque, where 'i' is a
        // dereferenceable iterator in the range '[first .. last)' (see
        // {Requirements on 'VALUE_TYPE'}).  The behavior is undefined unless
        // 'first' and 'last' refer to a sequence of valid values where 'first'
        // is at a position at or before 'last'.

    void assign(size_type numElements, const VALUE_TYPE& value);
        // Assign to this object the value resulting from first clearing this
        // deque and then appending the specified 'numElements' having the
        // specified 'value'.  This method requires that the (template
        // parameter) 'VALUE_TYPE' be 'copy-assignable' and 'copy-insertable'
        // into this deque (see {Requirements on 'VALUE_TYPE'}).

#if defined(BSLS_COMPILERFEATURES_SUPPORT_GENERALIZED_INITIALIZERS)
    void assign(std::initializer_list<value_type> values);
        // Assign to this object the value resulting from first clearing this
        // deque and then appending each 'value_type' object in the specified
        // 'values' initializer list.  This method requires that the (template
        // parameter) 'VALUE_TYPE' be 'copy-assignable' and 'copy-insertable'
        // into this deque (see {Requirements on 'VALUE_TYPE'}).
#endif

    // *** capacity ***

    void reserve(size_type numElements);
        // Change the capacity of this deque such that, after this method
        // returns, iterators remain valid provided that no more than the
        // specified 'numElements' objects are pushed to the front or back of
        // the deque after this call.  If it is already possible to push
        // 'numElements' objects to either end of this deque without
        // invalidating iterators, this method has no effect.  Note that
        // inserting elements into the deque may still incur memory allocation.
        // Also note that this method, if it has any effect, will invalidate
        // iterators initialized prior to the call.  Also note that this method
        // is not part of the C++ standard.

    void resize(size_type newSize);
    void resize(size_type newSize, const VALUE_TYPE& value);
        // Change the size of this deque to the specified 'newSize'.  Erase
        // 'size() - newSize' elements at the back if 'newSize < size()'.
        // Append 'newSize - size()' elements at the back having the optionally
        // specified 'value' if 'newSize > size()'; if 'value' is not
        // specified, default-constructed objects of the (template parameter)
        // 'VALUE_TYPE' are emplaced.  This method has no effect if
        // 'newSize == size()'.  Throw 'bsl::length_error' if
        // 'newSize > max_size()'.

    void shrink_to_fit();
        // Minimize the memory used by this deque to the extent possible
        // without moving any contained elements.  If an exception is thrown,
        // the value of this object is unchanged.  Note that this method has no
        // effect on the memory used by individual elements of the (template
        // parameter) 'VALUE_TYPE'.

    // *** modifiers ***

    void push_front(const VALUE_TYPE& value);
        // Prepend to the front of this deque a copy of the specified 'value'.
        // This method offers full guarantee of rollback in case an exception
        // is thrown.  This method requires that the (template parameter)
        // 'VALUE_TYPE' be 'copy-constructible' (see {Requirements on
        // 'VALUE_TYPE'}).

    void push_front(BloombergLP::bslmf::MovableRef<value_type> value);
        // Prepend to the front of this deque the specified move-insertable
        // 'value'.  'value' is left in a valid but unspecified state.  If an
        // exception is thrown (other than by the move constructor of a
        // non-copy-insertable 'value_type'), this method has no effect.  This
        // method requires that the (template parameter) 'VALUE_TYPE' be
        // 'move-insertable' into this deque (see {Requirements on
        // 'VALUE_TYPE'}).

    void push_back(const VALUE_TYPE& value);
        // Append to the back of this deque a copy of the specified 'value'.
        // This method offers full guarantee of rollback in case an exception
        // is thrown.  This method requires that the (template parameter)
        // 'VALUE_TYPE' be 'copy-constructible' (see {Requirements on
        // 'VALUE_TYPE'}).

    void push_back(BloombergLP::bslmf::MovableRef<value_type> value);
        // Append to the back of this deque the specified move-insertable
        // 'value'.  'value' is left in a valid but unspecified state.  If an
        // exception is thrown (other than by the move constructor of a
        // non-copy-insertable 'value_type'), this method has no effect.  This
        // method requires that the (template parameter) 'VALUE_TYPE' be
        // 'move-insertable' into this deque (see {Requirements on
        // 'VALUE_TYPE'}).

#if !BSLS_COMPILERFEATURES_SIMULATE_CPP11_FEATURES
    template <class... Args>
    void emplace_front(Args&&... arguments);
        // Prepend to the front of this deque a newly created 'value_type'
        // object, constructed by forwarding 'get_allocator()' (if required)
        // and the specified (variable number of) 'arguments' to the
        // corresponding constructor of 'value_type'.  If an exception is
        // thrown (other than by the move constructor of a non-copy-insertable
        // 'value_type'), this method has no effect.  This method requires that
        // the (template parameter) 'VALUE_TYPE' be 'move-insertable' into this
        // deque and 'emplace-constructible' from 'arguments' (see
        // {Requirements on 'VALUE_TYPE'}).

    template <class... Args>
    void emplace_back(Args&&... arguments);
        // Append to the back of this deque a newly created 'value_type'
        // object, constructed by forwarding 'get_allocator()' (if required)
        // and the specified (variable number of) 'arguments' to the
        // corresponding constructor of 'value_type'.  If an exception is
        // thrown (other than by the move constructor of a non-copy-insertable
        // 'value_type'), this method has no effect.  This method requires that
        // the (template parameter) 'VALUE_TYPE' be 'move-insertable' into this
        // deque and 'emplace-constructible' from 'arguments' (see
        // {Requirements on 'VALUE_TYPE'}).
#elif BSLS_COMPILERFEATURES_SIMULATE_VARIADIC_TEMPLATES
// {{{ BEGIN GENERATED CODE
// The following section is automatically generated.  **DO NOT EDIT**
// Generator command line: sim_cpp11_features.pl bslstl_deque.h
    void emplace_front();

    template <class Args_01>
    void emplace_front(
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01);

    template <class Args_01,
              class Args_02>
    void emplace_front(
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02);

    template <class Args_01,
              class Args_02,
              class Args_03>
    void emplace_front(
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03);

    template <class Args_01,
              class Args_02,
              class Args_03,
              class Args_04>
    void emplace_front(
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04);

    template <class Args_01,
              class Args_02,
              class Args_03,
              class Args_04,
              class Args_05>
    void emplace_front(
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05);

    template <class Args_01,
              class Args_02,
              class Args_03,
              class Args_04,
              class Args_05,
              class Args_06>
    void emplace_front(
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06);

    template <class Args_01,
              class Args_02,
              class Args_03,
              class Args_04,
              class Args_05,
              class Args_06,
              class Args_07>
    void emplace_front(
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_07) arguments_07);

    template <class Args_01,
              class Args_02,
              class Args_03,
              class Args_04,
              class Args_05,
              class Args_06,
              class Args_07,
              class Args_08>
    void emplace_front(
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_07) arguments_07,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_08) arguments_08);

    template <class Args_01,
              class Args_02,
              class Args_03,
              class Args_04,
              class Args_05,
              class Args_06,
              class Args_07,
              class Args_08,
              class Args_09>
    void emplace_front(
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_07) arguments_07,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_08) arguments_08,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_09) arguments_09);

    template <class Args_01,
              class Args_02,
              class Args_03,
              class Args_04,
              class Args_05,
              class Args_06,
              class Args_07,
              class Args_08,
              class Args_09,
              class Args_10>
    void emplace_front(
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_07) arguments_07,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_08) arguments_08,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_09) arguments_09,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_10) arguments_10);


    void emplace_back();

    template <class Args_01>
    void emplace_back(BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01);

    template <class Args_01,
              class Args_02>
    void emplace_back(BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02);

    template <class Args_01,
              class Args_02,
              class Args_03>
    void emplace_back(BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03);

    template <class Args_01,
              class Args_02,
              class Args_03,
              class Args_04>
    void emplace_back(BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04);

    template <class Args_01,
              class Args_02,
              class Args_03,
              class Args_04,
              class Args_05>
    void emplace_back(BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05);

    template <class Args_01,
              class Args_02,
              class Args_03,
              class Args_04,
              class Args_05,
              class Args_06>
    void emplace_back(BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06);

    template <class Args_01,
              class Args_02,
              class Args_03,
              class Args_04,
              class Args_05,
              class Args_06,
              class Args_07>
    void emplace_back(BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_07) arguments_07);

    template <class Args_01,
              class Args_02,
              class Args_03,
              class Args_04,
              class Args_05,
              class Args_06,
              class Args_07,
              class Args_08>
    void emplace_back(BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_07) arguments_07,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_08) arguments_08);

    template <class Args_01,
              class Args_02,
              class Args_03,
              class Args_04,
              class Args_05,
              class Args_06,
              class Args_07,
              class Args_08,
              class Args_09>
    void emplace_back(BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_07) arguments_07,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_08) arguments_08,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_09) arguments_09);

    template <class Args_01,
              class Args_02,
              class Args_03,
              class Args_04,
              class Args_05,
              class Args_06,
              class Args_07,
              class Args_08,
              class Args_09,
              class Args_10>
    void emplace_back(BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_07) arguments_07,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_08) arguments_08,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_09) arguments_09,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_10) arguments_10);

#else
// The generated code below is a workaround for the absence of perfect
// forwarding in some compilers.
    template <class... Args>
    void emplace_front(BSLS_COMPILERFEATURES_FORWARD_REF(Args)... arguments);

    template <class... Args>
    void emplace_back(BSLS_COMPILERFEATURES_FORWARD_REF(Args)... arguments);
// }}} END GENERATED CODE
#endif

#if !BSLS_COMPILERFEATURES_SIMULATE_CPP11_FEATURES
    template <class... Args>
    iterator emplace(const_iterator position, Args&&... arguments);
        // Insert at the specified 'position' in this deque a newly created
        // 'value_type' object, constructed by forwarding 'get_allocator()' (if
        // required) and the specified (variable number of) 'arguments' to the
        // corresponding constructor of 'value_type', and return an iterator
        // providing modifiable access to the newly created and inserted
        // element.  If an exception is thrown (other than by the copy
        // constructor, move constructor, assignment operator, or move
        // assignment operator of 'value_type'), this method has no effect.
        // This method requires that the (template parameter) 'VALUE_TYPE' be
        // 'move-insertable' into this deque and 'emplace-constructible' from
        // 'arguments' (see {Requirements on 'VALUE_TYPE'}).  The behavior is
        // undefined unless 'position' is an iterator in the range
        // '[cbegin() .. cend()]' (both endpoints included).
#elif BSLS_COMPILERFEATURES_SIMULATE_VARIADIC_TEMPLATES
// {{{ BEGIN GENERATED CODE
// The following section is automatically generated.  **DO NOT EDIT**
// Generator command line: sim_cpp11_features.pl bslstl_deque.h
    iterator emplace(const_iterator position);

    template <class Args_01>
    iterator emplace(const_iterator position,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01);

    template <class Args_01,
              class Args_02>
    iterator emplace(const_iterator position,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02);

    template <class Args_01,
              class Args_02,
              class Args_03>
    iterator emplace(const_iterator position,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03);

    template <class Args_01,
              class Args_02,
              class Args_03,
              class Args_04>
    iterator emplace(const_iterator position,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04);

    template <class Args_01,
              class Args_02,
              class Args_03,
              class Args_04,
              class Args_05>
    iterator emplace(const_iterator position,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05);

    template <class Args_01,
              class Args_02,
              class Args_03,
              class Args_04,
              class Args_05,
              class Args_06>
    iterator emplace(const_iterator position,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06);

    template <class Args_01,
              class Args_02,
              class Args_03,
              class Args_04,
              class Args_05,
              class Args_06,
              class Args_07>
    iterator emplace(const_iterator position,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_07) arguments_07);

    template <class Args_01,
              class Args_02,
              class Args_03,
              class Args_04,
              class Args_05,
              class Args_06,
              class Args_07,
              class Args_08>
    iterator emplace(const_iterator position,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_07) arguments_07,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_08) arguments_08);

    template <class Args_01,
              class Args_02,
              class Args_03,
              class Args_04,
              class Args_05,
              class Args_06,
              class Args_07,
              class Args_08,
              class Args_09>
    iterator emplace(const_iterator position,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_07) arguments_07,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_08) arguments_08,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_09) arguments_09);

    template <class Args_01,
              class Args_02,
              class Args_03,
              class Args_04,
              class Args_05,
              class Args_06,
              class Args_07,
              class Args_08,
              class Args_09,
              class Args_10>
    iterator emplace(const_iterator position,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_07) arguments_07,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_08) arguments_08,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_09) arguments_09,
                      BSLS_COMPILERFEATURES_FORWARD_REF(Args_10) arguments_10);

#else
// The generated code below is a workaround for the absence of perfect
// forwarding in some compilers.
    template <class... Args>
    iterator emplace(const_iterator position,
                         BSLS_COMPILERFEATURES_FORWARD_REF(Args)... arguments);
// }}} END GENERATED CODE
#endif

    void pop_front();
        // Erase the first element from this deque.  The behavior is undefined
        // if this deque is empty.

    void pop_back();
        // Erase the last element from this deque.  The behavior is undefined
        // if this deque is empty.

    iterator insert(const_iterator position, const VALUE_TYPE& value);
        // Insert at the specified 'position' in this deque a copy of the
        // specified 'value', and return an iterator providing modifiable
        // access to the newly inserted element.  This method offers full
        // guarantee of rollback in case an exception is thrown other than by
        // the 'VALUE_TYPE' copy constructor or assignment operator.  This
        // method requires that the (template parameter) 'VALUE_TYPE' be
        // 'copy-insertable' into this deque (see {Requirements on
        // 'VALUE_TYPE'}).  The behavior is undefined unless 'position' is an
        // iterator in the range '[cbegin() .. cend()] (both endpoints
        // included)'.

    iterator insert(const_iterator                             position,
                    BloombergLP::bslmf::MovableRef<value_type> value);
        // Insert at the specified 'position' in this deque the specified
        // move-insertable 'value', and return an iterator providing modifiable
        // access to the newly inserted element.  'value' is left in a valid
        // but unspecified state.  If an exception is thrown (other than by the
        // copy constructor, move constructor, assignment operator, or move
        // assignment operator of 'value_type'), this method has no effect.
        // This method requires that the (template parameter) 'VALUE_TYPE' be
        // 'move-insertable' into this deque (see {Requirements on
        // 'VALUE_TYPE'}).  The behavior is undefined unless 'position' is an
        // iterator in the range '[cbegin() .. cend()]' (both endpoints
        // included).

    iterator insert(const_iterator    position,
                    size_type         numElements,
                    const VALUE_TYPE& value);
        // Insert at the specified 'position' in this deque the specified
        // 'numElements' copies of the specified 'value', and return an
        // iterator providing modifiable access to the first element in the
        // newly inserted sequence of elements.  This method offers full
        // guarantee of rollback in case an exception is thrown other than by
        // the 'VALUE_TYPE' copy constructor or assignment operator.  This
        // method requires that the (template parameter) 'VALUE_TYPE' be
        // 'copy-insertable' into this deque (see {Requirements on
        // 'VALUE_TYPE'}).  The behavior is undefined unless 'position' is an
        // iterator in the range '[cbegin() .. cend()]' (both endpoints
        // included).

    template <class INPUT_ITERATOR>
    iterator insert(const_iterator position,
                    INPUT_ITERATOR first,
                    INPUT_ITERATOR last);
        // Insert at the specified 'position' in this deque the values in the
        // range starting at the specified 'first' and ending immediately
        // before the specified 'last' iterators of the (template parameter)
        // type 'INPUT_ITERATOR', and return an iterator providing modifiable
        // access to the first element in the newly inserted sequence of
        // elements.  This method offers full guarantee of rollback in case an
        // exception is thrown other than by the 'VALUE_TYPE' copy constructor
        // or assignment operator.  The (template parameter) type
        // 'INPUT_ITERATOR' shall meet the requirements of an input iterator
        // defined in the C++11 standard [input.iterators] providing access to
        // values of a type convertible to 'value_type', and 'value_type' must
        // be 'emplace-constructible' from '*i' into this deque, where 'i' is a
        // dereferenceable iterator in the range '[first .. last)' (see
        // {Requirements on 'VALUE_TYPE'}).  The behavior is undefined unless
        // 'position' is an iterator in the range '[cbegin() .. cend()]' (both
        // endpoints included), and 'first' and 'last' refer to a sequence of
        // valid values where 'first' is at a position at or before 'last'.

#if defined(BSLS_COMPILERFEATURES_SUPPORT_GENERALIZED_INITIALIZERS)
    iterator insert(const_iterator                    position,
                    std::initializer_list<value_type> values);
        // Insert at the specified 'position' in this deque the value of each
        // 'value_type' object in the specified 'values' initializer list, and
        // return an iterator providing modifiable access to the first element
        // in the newly inserted sequence of elements.  This method offers full
        // guarantee of rollback in case an exception is thrown other than by
        // the 'VALUE_TYPE' copy constructor or assignment operator.  This
        // method requires that the (template parameter) 'VALUE_TYPE' be
        // 'copy-insertable' into this deque (see {Requirements on
        // 'VALUE_TYPE'}).  The behavior is undefined unless 'position' is an
        // iterator in the range '[cbegin() .. cend()]' (both endpoints
        // included).
#endif

    iterator erase(const_iterator position);
        // Remove from this deque the element at the specified 'position', and
        // return an iterator providing modifiable access to the element
        // immediately following the removed element, or the position returned
        // by the method 'end' if the removed element was the last in the
        // sequence.  The behavior is undefined unless 'position' is an
        // iterator in the range '[cbegin() .. cend())'.

    iterator erase(const_iterator first, const_iterator last);
        // Remove from this deque the sequence of elements starting at the
        // specified 'first' position and ending before the specified 'last'
        // position, and return an iterator providing modifiable access to the
        // element immediately following the last removed element, or the
        // position returned by the method 'end' if the removed elements were
        // last in the sequence.  The behavior is undefined unless 'first' is
        // an iterator in the range '[cbegin() .. cend()]' (both endpoints
        // included) and 'last' is an iterator in the range
        // '[first .. cend()]' (both endpoints included).

    void swap(deque<VALUE_TYPE, ALLOCATOR>& other)
             BSLS_CPP11_NOEXCEPT_SPECIFICATION(BSLS_CPP11_PROVISIONALLY_FALSE);
        // Exchange the value of this object with the value of the specified
        // 'other' object.  Additionally, if
        // 'bsl::allocator_traits<ALLOCATOR>::propagate_on_container_swap' is
        // 'true', then exchange the allocator of this object with that of the
        // 'other' object, and do not modify either allocator otherwise.  This
        // method provides the no-throw exception-safety guarantee and
        // guarantees 'O[1]' complexity.  The behavior is undefined unless
        // either this object was created with the same allocator as 'other' or
        // 'propagate_on_container_swap' is 'true'.

    void clear() BSLS_CPP11_NOEXCEPT;
        // Remove all elements from this deque making its size 0.  Note that
        // although this deque is empty after this method returns, it preserves
        // the same capacity it had before the method was called.

    // ACCESSORS

    // *** construct/copy/destroy ***

    allocator_type get_allocator() const BSLS_CPP11_NOEXCEPT;
        // Return the allocator used by this deque to supply memory.

    size_type max_size() const BSLS_CPP11_NOEXCEPT;
        // Return the maximum possible size of this deque.  Note that this is a
        // theoretical maximum (such as the maximum value that can be held by
        // 'size_type').  Also note that any request to create or enlarge a
        // deque to a size greater than 'max_size()' is guaranteed to raise a
        // 'bsl::length_error' exception.
};

// FREE OPERATORS
template <class VALUE_TYPE, class ALLOCATOR>
bool operator==(const deque<VALUE_TYPE, ALLOCATOR>& lhs,
                const deque<VALUE_TYPE, ALLOCATOR>& rhs);
    // Return 'true' if the specified 'lhs' and 'rhs' objects have the same
    // value, and 'false' otherwise.  Two 'deque' objects 'lhs' and 'rhs' have
    // the same value if they have the same number of elements, and each
    // element in the ordered sequence of elements of 'lhs' has the same value
    // as the corresponding element in the ordered sequence of elements of
    // 'rhs'.  This method requires that the (template parameter) type
    // 'VALUE_TYPE' be 'equality-comparable' (see {Requirements on
    // 'VALUE_TYPE'}).

template <class VALUE_TYPE, class ALLOCATOR>
bool operator!=(const deque<VALUE_TYPE, ALLOCATOR>& lhs,
                const deque<VALUE_TYPE, ALLOCATOR>& rhs);
    // Return 'true' if the specified 'lhs' and 'rhs' objects do not have the
    // same value, and 'false' otherwise.  Two 'deque' objects 'lhs' and 'rhs'
    // do not have the same value if they do not have the same number of
    // elements, or some element in the ordered sequence of elements of 'lhs'
    // does not have the same value as the corresponding element in the ordered
    // sequence of elements of 'rhs'.  This method requires that the (template
    // parameter) type 'VALUE_TYPE' be 'equality-comparable' (see {Requirements
    // on 'VALUE_TYPE'}).

template <class VALUE_TYPE, class ALLOCATOR>
bool operator<(const deque<VALUE_TYPE, ALLOCATOR>& lhs,
               const deque<VALUE_TYPE, ALLOCATOR>& rhs);
    // Return 'true' if the value of the specified 'lhs' deque is
    // lexicographically less than that of the specified 'rhs' deque, and
    // 'false' otherwise.  Given iterators 'i' and 'j' over the respective
    // sequences '[lhs.begin() .. lhs.end())' and '[rhs.begin() .. rhs.end())',
    // the value of deque 'lhs' is lexicographically less than that of deque
    // 'rhs' if 'true == *i < *j' for the first pair of corresponding iterator
    // positions where '*i < *j' and '*j < *i' are not both 'false'.  If no
    // such corresponding iterator position exists, the value of 'lhs' is
    // lexicographically less than that of 'rhs' if 'lhs.size() < rhs.size()'.
    // This method requires that 'operator<', inducing a total order, be
    // defined for 'value_type'.

template <class VALUE_TYPE, class ALLOCATOR>
bool operator>(const deque<VALUE_TYPE, ALLOCATOR>& lhs,
               const deque<VALUE_TYPE, ALLOCATOR>& rhs);
    // Return 'true' if the value of the specified 'lhs' deque is
    // lexicographically greater than that of the specified 'rhs' deque, and
    // 'false' otherwise.  The value of deque 'lhs' is lexicographically
    // greater than that of deque 'rhs' if 'rhs' is lexicographically less than
    // 'lhs' (see 'operator<').  This method requires that 'operator<',
    // inducing a total order, be defined for 'value_type'.  Note that this
    // operator returns 'rhs < lhs'.

template <class VALUE_TYPE, class ALLOCATOR>
bool operator<=(const deque<VALUE_TYPE, ALLOCATOR>& lhs,
                const deque<VALUE_TYPE, ALLOCATOR>& rhs);
    // Return 'true' if the value of the specified 'lhs' deque is
    // lexicographically less than or equal to that of the specified 'rhs'
    // deque, and 'false' otherwise.  The value of deque 'lhs' is
    // lexicographically less than or equal to that of deque 'rhs' if 'rhs' is
    // not lexicographically less than 'lhs' (see 'operator<').  This method
    // requires that 'operator<', inducing a total order, be defined for
    // 'value_type'.  Note that this operator returns '!(rhs < lhs)'.

template <class VALUE_TYPE, class ALLOCATOR>
bool operator>=(const deque<VALUE_TYPE, ALLOCATOR>& lhs,
                const deque<VALUE_TYPE, ALLOCATOR>& rhs);
    // Return 'true' if the value of the specified 'lhs' deque is
    // lexicographically greater than or equal to that of the specified 'rhs'
    // deque, and 'false' otherwise.  The value of deque 'lhs' is
    // lexicographically greater than or equal to that of deque 'rhs' if 'lhs'
    // is not lexicographically less than 'rhs' (see 'operator<').  This method
    // requires that 'operator<', inducing a total order, be defined for
    // 'value_type'.  Note that this operator returns '!(lhs < rhs)'.

// FREE FUNCTIONS
template <class VALUE_TYPE, class ALLOCATOR>
void swap(deque<VALUE_TYPE, ALLOCATOR>& a, deque<VALUE_TYPE, ALLOCATOR>& b)
             BSLS_CPP11_NOEXCEPT_SPECIFICATION(BSLS_CPP11_PROVISIONALLY_FALSE);
    // Exchange the value of the specified 'a' object with the value of the
    // specified 'b' object.  Additionally, if
    // 'bsl::allocator_traits<ALLOCATOR>::propagate_on_container_swap' is
    // 'true', then exchange the allocator of 'a' with that of 'b'.  If
    // 'propagate_on_container_swap' is 'true' or 'a' and 'b' were created with
    // the same allocator, then this method provides the no-throw
    // exception-safety guarantee and has 'O[1]' complexity; otherwise, this
    // method has 'O[n + m]' complexity, where 'n' and 'm' are the number of
    // elements in 'a' and 'b', respectively.  Note that 'a' and 'b' are left
    // in valid but unspecified states if an exception is thrown, e.g., in the
    // case where 'propagate_on_container_swap' is 'false' and 'a' and 'b' were
    // created with different allocators.

                      // ========================
                      // class Deque_BlockCreator
                      // ========================

template <class VALUE_TYPE, class ALLOCATOR>
class Deque_BlockCreator {
    // This class allocates blocks at the front or back of a deque and
    // tentatively adds them to the deque.  It also keeps track of how many of
    // the newly allocated blocks have actually been used by the deque.  The
    // destructor automatically frees any unused blocks (e.g., in case an
    // exception is thrown).

    // PRIVATE TYPES
    enum {
        BLOCK_LENGTH = Deque_BlockLengthCalcUtil<VALUE_TYPE>::BLOCK_LENGTH
    };

    typedef BloombergLP::bslalg::DequeImpUtil<VALUE_TYPE,
                                             BLOCK_LENGTH> Imp;
    typedef typename Imp::Block                            Block;
    typedef typename Imp::BlockPtr                         BlockPtr;
    typedef std::size_t                                    size_type;

    // DATA
    deque<VALUE_TYPE, ALLOCATOR> *d_deque_p;
    BlockPtr                     *d_boundary_p;

  private:
    // NOT IMPLEMENTED
    Deque_BlockCreator(const Deque_BlockCreator&);
    Deque_BlockCreator& operator=(const Deque_BlockCreator&);

  public:
    // CREATORS
    explicit
    Deque_BlockCreator(deque<VALUE_TYPE, ALLOCATOR> *deque);
        // Construct a block allocator for the specified 'deque'.

    ~Deque_BlockCreator();
        // Free any blocks that have been allocated by this allocator but have
        // not yet been used by the deque.

    // MANIPULATORS
    void insertAtFront(size_type n);
        // Allocate the specified 'n' blocks at the front of the block array.
        // This method invalidates all iterators except 'd_deque_p->d_start'
        // and 'd_deque_p->d_finish'.

    void insertAtBack(size_type n);
        // Allocate the specified 'n' blocks at the back of the block array.
        // This method invalidates all iterators except 'd_deque_p->d_start'
        // and 'd_deque_p->d_finish'.

    BlockPtr *reserveBlockSlots(size_type numNewBlocks, bool atFront);
        // Make room for the specified 'numNewBlocks' pointers in the blocks
        // array.  If the specified 'atFront' is 'true', then make room at the
        // front of the array, else make room at the back of the array.  Return
        // a pointer to the insertion point, i.e., the point where new blocks
        // can be stored into the array, working backwards if 'atFront' is
        // 'true', or working forwards if 'atFront' is 'false'.  This method
        // invalidates all iterators and updates 'd_deque_p->d_start' and
        // 'd_deque_p->d_finish'.

    void release();
        // Relinquish control over any allocated blocks.  The destructor will
        // do nothing following a call to this method.
};

                        // ========================
                        // class Deque_BlockProctor
                        // ========================

template <class VALUE_TYPE, class ALLOCATOR>
class Deque_BlockProctor {
    // This class implements a proctor that, upon destruction and unless its
    // 'release' method has previously been invoked, deallocates empty blocks
    // at one end (i.e., front or back) of a proctored deque.  The end at which
    // empty blocks are to be proctored is indicated by a flag supplied at
    // construction.  See 'emplace' for a use case.

    // PRIVATE TYPES
    enum {
        BLOCK_LENGTH = Deque_BlockLengthCalcUtil<VALUE_TYPE>::BLOCK_LENGTH
    };

    typedef BloombergLP::bslalg::DequeImpUtil<VALUE_TYPE,
                                              BLOCK_LENGTH> Imp;

    typedef typename Imp::BlockPtr                          BlockPtr;

    // DATA
    deque<VALUE_TYPE, ALLOCATOR> *d_deque_p;     // proctored deque

    BlockPtr                     *d_boundary_p;  // boundary of proctored
                                                 // blocks

    bool                          d_atFront;     // if 'true', proctor blocks
                                                 // at the front of the deque;
                                                 // otherwise, proctor blocks
                                                 // at the block

  private:
    // NOT IMPLEMENTED
    Deque_BlockProctor(const Deque_BlockProctor&);
    Deque_BlockProctor& operator=(const Deque_BlockProctor&);

  public:
    // CREATORS
    Deque_BlockProctor(deque<VALUE_TYPE, ALLOCATOR> *deque, bool atFront);
        // Create a block proctor object to proctor blocks at one end (i.e.,
        // front or back) of the specified 'deque' as indicated by the
        // specified 'atFront' flag.  If 'atFront' is 'true', blocks at the
        // front of 'deque' are proctored; otherwise, blocks at the back of
        // 'deque' are proctored.

    ~Deque_BlockProctor();
        // Destroy this block proctor, and deallocate empty blocks at the back
        // of the proctored deque as indicated by the 'atFront' flag supplied
        // at construction.  If no deque is currently being managed, this
        // method has no effect.

    // MANIPULATORS
    void release();
        // Release from management the deque currently managed by this proctor.
        // If no deque is currently being managed, this method has no effect.
};

                        // ======================
                        // class Deque_ClearGuard
                        // ======================

template <class VALUE_TYPE, class ALLOCATOR>
class Deque_ClearGuard {
    // This class provides a proctor which, at destruction, calls 'clear' on
    // the deque supplied at construction, unless the guard has been released
    // prior.

    // PRIVATE TYPES
    typedef BloombergLP::bslalg::ContainerBase<ALLOCATOR> ContainerBase;

    // DATA
    deque<VALUE_TYPE, ALLOCATOR> *d_deque_p;  // proctored object

  private:
    // NOT IMPLEMENTED
    Deque_ClearGuard(const Deque_ClearGuard&);
    Deque_ClearGuard& operator=(const Deque_ClearGuard&);

  public:
    // CREATORS
    explicit
    Deque_ClearGuard(deque<VALUE_TYPE, ALLOCATOR> *deque);
        // Create a clear guard object to proctor the specified 'deque'.

    ~Deque_ClearGuard();
        // Destroy this guard, and call 'clear' on the deque supplied at
        // construction, unless 'release' has been called on this object.

    // MANIPULATORS
    void release();
        // Release from management the deque proctored by this object.
};

                        // =================
                        // class Deque_Guard
                        // =================

template <class VALUE_TYPE, class ALLOCATOR>
class Deque_Guard {
    // This class provides a proctor that maintains a count of the number of
    // elements constructed at the front or back of a deque, but not yet
    // committed to the deque's range of valid elements; if the count is
    // non-zero at destruction, the destructor destroys the elements in the
    // range '[d_deque_p->end() .. d_deque_p->end() + d_count)', or the range
    // '[d_deque_p->begin() - d_count .. d_deque_p->begin())', depending on
    // whether this proctor guards the back or front.  This guard is used to
    // undo element constructors in the event of an exception.  It is up to the
    // client code to increment the count whenever a new element is constructed
    // and to decrement the count whenever 'd_start' or 'd_finish' of the
    // guarded deque is moved to incorporate more elements.

    // PRIVATE TYPES
    enum {
        BLOCK_LENGTH = Deque_BlockLengthCalcUtil<VALUE_TYPE>::BLOCK_LENGTH
    };

    typedef BloombergLP::bslalg::DequeIterator<VALUE_TYPE,
                                               BLOCK_LENGTH>   IteratorImp;
    typedef BloombergLP::bslalg::DequePrimitives<VALUE_TYPE,
                                                 BLOCK_LENGTH> DequePrimitives;

    // DATA
    deque<VALUE_TYPE, ALLOCATOR> *d_deque_p;
    std::size_t                   d_count;
    bool                          d_isTail;

  private:
    // NOT IMPLEMENTED
    Deque_Guard(const Deque_Guard&);
    Deque_Guard& operator=(const Deque_Guard&);

  public:
    // CREATORS
    Deque_Guard(deque<VALUE_TYPE, ALLOCATOR> *deque, bool isTail);
        // Initializes object to guard 0 items from the specified 'deque'.
        // This guards either the tail or the head, as determined by the
        // specified 'isTail' boolean.

    ~Deque_Guard();
        // Call the (template parameter) 'VALUE_TYPE' destructor on objects in
        // the range '[d.end() .. d.end() + count())' if 'isTail' was specified
        // as 'true' during construction, or
        // '[d.start() - count() .. d.start()]' if 'isTail' was specified as
        // 'false' during construction, where 'd' is the deque used to
        // construct this guard.

    // MANIPULATORS
    std::size_t operator++();
        // Increment the count of this guard, and return the new count.

    std::size_t operator--();
        // Decrement the count of this guard, and return the new count.

    void release();
        // Set the count of this guard to 0.  Note that this guard's destructor
        // will do nothing if count is not incremented again after this call.

    // ACCESSORS
    std::size_t count() const BSLS_CPP11_NOEXCEPT;
        // Return the current count maintained by this guard.

    IteratorImp begin() const BSLS_CPP11_NOEXCEPT;
        // Return a pointer after the first item in the guarded range.

    IteratorImp end() const BSLS_CPP11_NOEXCEPT;
        // Return a pointer after the last item in the guarded range.
};

// ============================================================================
//                  TEMPLATE AND INLINE FUNCTION DEFINITIONS
// ============================================================================

// See IMPLEMENTATION NOTES in the .cpp before modifying anything below.

                             // ----------------
                             // class Deque_Base
                             // ----------------

// MANIPULATORS
template <class VALUE_TYPE>
inline
typename Deque_Base<VALUE_TYPE>::iterator
Deque_Base<VALUE_TYPE>::begin() BSLS_CPP11_NOEXCEPT
{
    return d_start;
}

template <class VALUE_TYPE>
inline
typename Deque_Base<VALUE_TYPE>::iterator
Deque_Base<VALUE_TYPE>::end() BSLS_CPP11_NOEXCEPT
{
    return d_finish;
}

template <class VALUE_TYPE>
inline
typename Deque_Base<VALUE_TYPE>::reverse_iterator
Deque_Base<VALUE_TYPE>::rbegin() BSLS_CPP11_NOEXCEPT
{
    return reverse_iterator(end());
}

template <class VALUE_TYPE>
inline
typename Deque_Base<VALUE_TYPE>::reverse_iterator
Deque_Base<VALUE_TYPE>::rend() BSLS_CPP11_NOEXCEPT
{
    return reverse_iterator(begin());
}

template <class VALUE_TYPE>
inline
typename Deque_Base<VALUE_TYPE>::reference
Deque_Base<VALUE_TYPE>::operator[](size_type position)
{
    BSLS_ASSERT_SAFE(position < size());

    return *(begin() + position);
}

template <class VALUE_TYPE>
typename Deque_Base<VALUE_TYPE>::reference
Deque_Base<VALUE_TYPE>::at(size_type position)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(position >= size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwOutOfRange(
                                        "deque<...>::at(n): invalid position");
    }
    return *(begin() + position);
}

template <class VALUE_TYPE>
inline
typename Deque_Base<VALUE_TYPE>::reference
Deque_Base<VALUE_TYPE>::front()
{
    BSLS_ASSERT_SAFE(!empty());

    return *d_start;
}

template <class VALUE_TYPE>
inline
typename Deque_Base<VALUE_TYPE>::reference
Deque_Base<VALUE_TYPE>::back()
{
    BSLS_ASSERT_SAFE(!empty());

    IteratorImp backIterator = d_finish;
    --backIterator;
    return *backIterator;
}

// ACCESSORS
template <class VALUE_TYPE>
inline
typename Deque_Base<VALUE_TYPE>::const_iterator
Deque_Base<VALUE_TYPE>::begin() const BSLS_CPP11_NOEXCEPT
{
    return d_start;
}

template <class VALUE_TYPE>
inline
typename Deque_Base<VALUE_TYPE>::const_iterator
Deque_Base<VALUE_TYPE>::cbegin() const BSLS_CPP11_NOEXCEPT
{
    return d_start;
}

template <class VALUE_TYPE>
inline
typename Deque_Base<VALUE_TYPE>::const_iterator
Deque_Base<VALUE_TYPE>::end() const BSLS_CPP11_NOEXCEPT
{
    return d_finish;
}

template <class VALUE_TYPE>
inline
typename Deque_Base<VALUE_TYPE>::const_iterator
Deque_Base<VALUE_TYPE>::cend() const BSLS_CPP11_NOEXCEPT
{
    return d_finish;
}

template <class VALUE_TYPE>
inline
typename Deque_Base<VALUE_TYPE>::const_reverse_iterator
Deque_Base<VALUE_TYPE>::rbegin() const BSLS_CPP11_NOEXCEPT
{
    return const_reverse_iterator(end());
}

template <class VALUE_TYPE>
inline
typename Deque_Base<VALUE_TYPE>::const_reverse_iterator
Deque_Base<VALUE_TYPE>::crbegin() const BSLS_CPP11_NOEXCEPT
{
    return const_reverse_iterator(end());
}

template <class VALUE_TYPE>
inline
typename Deque_Base<VALUE_TYPE>::const_reverse_iterator
Deque_Base<VALUE_TYPE>::rend() const BSLS_CPP11_NOEXCEPT
{
    return const_reverse_iterator(begin());
}

template <class VALUE_TYPE>
inline
typename Deque_Base<VALUE_TYPE>::const_reverse_iterator
Deque_Base<VALUE_TYPE>::crend() const BSLS_CPP11_NOEXCEPT
{
    return const_reverse_iterator(begin());
}

template <class VALUE_TYPE>
inline
typename Deque_Base<VALUE_TYPE>::size_type
Deque_Base<VALUE_TYPE>::size() const BSLS_CPP11_NOEXCEPT
{
    return d_finish - d_start;
}

template <class VALUE_TYPE>
typename Deque_Base<VALUE_TYPE>::size_type
Deque_Base<VALUE_TYPE>::capacity() const BSLS_CPP11_NOEXCEPT
{
    // 'ContainerBase::allocateN', which creates the 'd_blocks_p' array, does
    // not, in its contract, guarantee to initialize the array to 0.  Since we
    // read these values, we have to make sure they're initialized to avoid
    // purify 'read before write' errors.  Note that we initialize them to
    // null in which case they are not valid pointers, but we never dereference
    // them, and the pointer arithmetic we do on them will still work.

    if (d_start.blockPtr() > d_blocks_p) {
        d_blocks_p[0] = 0;
    }
    if (d_finish.blockPtr() < d_blocks_p + d_blocksLength - 1) {
        d_blocks_p[d_blocksLength - 1] = 0;
    }

    const IteratorImp first(d_blocks_p);

    // 'last' points to the empty slot at the end of the last block in
    // 'd_blocks_p', which might not actually be there.

    IteratorImp last(d_blocks_p + d_blocksLength - 1);
    last += BLOCK_LENGTH - 1;

    BSLS_ASSERT_SAFE(!(d_start < first));          // 'IteratorImp' has no '>='
    BSLS_ASSERT_SAFE(!(last    < d_finish));

    const size_type frontCapacity = d_finish - first;
    const size_type backCapacity  = last - d_start;

    // Return the min (we are below 'bsl_algorithm.h', so we have to do it by
    // hand).

    return frontCapacity < backCapacity ? frontCapacity : backCapacity;
}

template <class VALUE_TYPE>
inline
bool Deque_Base<VALUE_TYPE>::empty() const BSLS_CPP11_NOEXCEPT
{
    return d_start == d_finish;
}

template <class VALUE_TYPE>
inline
typename Deque_Base<VALUE_TYPE>::const_reference
Deque_Base<VALUE_TYPE>::operator[](size_type position) const
{
    BSLS_ASSERT_SAFE(position < size());

    return *(begin() + position);
}

template <class VALUE_TYPE>
typename Deque_Base<VALUE_TYPE>::const_reference
Deque_Base<VALUE_TYPE>::at(size_type position) const
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(position >= size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwOutOfRange(
                                  "const deque<...>::at(n): invalid position");
    }
    return *(begin() + position);
}

template <class VALUE_TYPE>
inline
typename Deque_Base<VALUE_TYPE>::const_reference
Deque_Base<VALUE_TYPE>::front() const
{
    BSLS_ASSERT_SAFE(!empty());

    return *d_start;
}

template <class VALUE_TYPE>
inline
typename Deque_Base<VALUE_TYPE>::const_reference
Deque_Base<VALUE_TYPE>::back() const
{
    BSLS_ASSERT_SAFE(!empty());

    IteratorImp backIterator = d_finish;
    --backIterator;
    return *backIterator;
}

                             // -----------
                             // class deque
                             // -----------

// PRIVATE CREATORS
template <class VALUE_TYPE, class ALLOCATOR>
inline
deque<VALUE_TYPE, ALLOCATOR>::deque(RawInit, const ALLOCATOR& allocator)
: Deque_Base<VALUE_TYPE>()
, ContainerBase(allocator)
{
    this->d_blocks_p = 0;
}

// PRIVATE MANIPULATORS
template <class VALUE_TYPE, class ALLOCATOR>
template <class INPUT_ITERATOR>
typename deque<VALUE_TYPE, ALLOCATOR>::size_type
deque<VALUE_TYPE, ALLOCATOR>::privateAppend(
                                         INPUT_ITERATOR                  first,
                                         INPUT_ITERATOR                  last,
                                         std::random_access_iterator_tag)
{
    BlockCreator newBlocks(this);
    Guard guard(this, true);

    const size_type numElements = bsl::distance(first, last);
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
                                    numElements > max_size() - this->size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                 "deque<...>::insert(pos,n,v): deque too big");
    }

    for ( ; first != last; ++first) {
        IteratorImp insertPoint = guard.end();

        // There must be room for the iterator to be incremented.  Allocate new
        // block now if necessary.  We cannot wait until after the new element
        // is constructed or else we won't be able to increment the guard right
        // away and there will be a window where an exception will cause a
        // resource leak.

        if (1 == insertPoint.remainingInBlock()) {
            newBlocks.insertAtBack(1);
            insertPoint = guard.end();  // 'insertAtBack(1)' invalidated
                                        // iterator
        }
        AllocatorTraits::construct(ContainerBase::allocator(),
                                   BSLS_UTIL_ADDRESSOF(*insertPoint),
                                   *first);
        ++guard;
    }

    this->d_finish += guard.count();

    guard.release();
    return numElements;
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class INPUT_ITERATOR>
typename deque<VALUE_TYPE, ALLOCATOR>::size_type
deque<VALUE_TYPE, ALLOCATOR>::privateAppend(INPUT_ITERATOR          first,
                                            INPUT_ITERATOR          last,
                                            std::input_iterator_tag)
{
    BlockCreator newBlocks(this);
    Guard guard(this, true);

    size_type numElements = 0;
    size_type maxNumElements = max_size() - this->size();
    for ( ; first != last; ++first) {
        ++numElements;
        if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
                                               numElements > maxNumElements)) {
            BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

            BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                 "deque<...>::insert(pos,n,v): deque too big");
        }
        IteratorImp insertPoint = guard.end();

        // There must be room for the iterator to be incremented.  Allocate new
        // block now if necessary.  We cannot wait until after the new element
        // is constructed or else we won't be able to increment the guard right
        // away and there will be a window where an exception will cause a
        // resource leak.

        if (1 == insertPoint.remainingInBlock()) {
            newBlocks.insertAtBack(1);
            insertPoint = guard.end();  // 'insertAtBack(1)' invalidated
                                        // iterator
        }

        AllocatorTraits::construct(ContainerBase::allocator(),
                                   BSLS_UTIL_ADDRESSOF(*insertPoint),
                                   *first);
        ++guard;
    }

    this->d_finish += guard.count();

    guard.release();
    return numElements;
}

template <class VALUE_TYPE, class ALLOCATOR>
void deque<VALUE_TYPE, ALLOCATOR>::privateAppendRaw(
                                                 size_type         numElements,
                                                 const VALUE_TYPE& value)
{
    // Create new blocks at the back.  In case an exception is thrown, any
    // unused blocks are returned to the allocator.

    size_type numNewBlocks = (this->d_finish.offsetInBlock() + numElements) /
                                                                  BLOCK_LENGTH;
    BlockCreator newBlocks(this);
    newBlocks.insertAtBack(numNewBlocks);

    DequePrimitives::uninitializedFillNBack(&this->d_finish,
                                            this->d_finish,
                                            numElements,
                                            value,
                                            ContainerBase::allocator());
}

template <class VALUE_TYPE, class ALLOCATOR>
void deque<VALUE_TYPE, ALLOCATOR>::privateInit(size_type numElements)
{
    size_type blocksLength = numElements / BLOCK_LENGTH + 1 +
                                                  2 * Imp::BLOCK_ARRAY_PADDING;

    // Allocate block pointer array.

    this->d_blocks_p     = this->allocateN((BlockPtr *)0, blocksLength);
    this->d_blocksLength = blocksLength;

    // Allocate the first block and store its pointer into the array.  Leave a
    // little room at the front and back of the array for growth.

    BlockPtr *firstBlockPtr = &this->d_blocks_p[Imp::BLOCK_ARRAY_PADDING];
    *firstBlockPtr = this->allocateN((Block *)0, 1);

    // Calculate the offset into the first block such that 'n' elements will
    // leave equal space at the front of the first block and at the end of the
    // last block, remembering that the last element of the last block cannot
    // be used.  Centering the elements reduces the chance that either
    // 'push_back' or 'push_front' will need to allocate a new block.  In case
    // of an odd number of unused elements, slight preference is given to
    // 'push_back' over 'push_front'.

    const int offset = static_cast<int>(
                          (BLOCK_LENGTH - 1 - numElements % BLOCK_LENGTH) / 2);

    // Initialize the begin and end iterators.

    this->d_start = this->d_finish = IteratorImp(
                                            firstBlockPtr,
                                            (*firstBlockPtr)->d_data + offset);
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class INTEGRAL_TYPE>
inline
void deque<VALUE_TYPE, ALLOCATOR>::privateInsertDispatch(
                           const_iterator                          position,
                           INTEGRAL_TYPE                           numElements,
                           INTEGRAL_TYPE                           value,
                           BloombergLP::bslmf::MatchArithmeticType,
                           BloombergLP::bslmf::Nil)
{
    insert(position,
           static_cast<size_type>(numElements),
           static_cast<VALUE_TYPE>(value));
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class INPUT_ITERATOR>
void deque<VALUE_TYPE, ALLOCATOR>::privateInsertDispatch(
                                     const_iterator                   position,
                                     INPUT_ITERATOR                   first,
                                     INPUT_ITERATOR                   last,
                                     BloombergLP::bslmf::MatchAnyType,
                                     BloombergLP::bslmf::MatchAnyType)
{
    typedef typename iterator_traits<INPUT_ITERATOR>::iterator_category Tag;

    if (first == last) {
        return;                                                       // RETURN
    }

    if (position == this->cbegin()) {
        privatePrepend(first, last, Tag());
        return;                                                       // RETURN
    }

    if (position == this->cend()) {
        privateAppend(first, last, Tag());
        return;                                                       // RETURN
    }

    privateInsert(position, first, last, Tag());
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class INPUT_ITERATOR>
void deque<VALUE_TYPE, ALLOCATOR>::privateInsert(
                                              const_iterator          position,
                                              INPUT_ITERATOR          first,
                                              INPUT_ITERATOR          last,
                                              std::input_iterator_tag tag)
{
    BSLS_ASSERT(first != last);

    iterator pos(position.imp());
    const size_type currentSize = this->size();
    const size_type posIdx      = pos - this->begin();

    deque temp(k_RAW_INIT, this->get_allocator());
    privateSplit(&temp, position.imp());

    if (posIdx <= currentSize / 2) {
        Deque_Util::swap(
                        static_cast<Base *>(this), static_cast<Base *>(&temp));
        privatePrepend(first, last, tag);
        privateJoinPrepend(&temp);
    }
    else {
        privateAppend(first, last, tag);
        privateJoinAppend(&temp);
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
void deque<VALUE_TYPE, ALLOCATOR>::privateSplit(
                                           deque<VALUE_TYPE, ALLOCATOR> *other,
                                           IteratorImp                   pos)
{
    // BEFORE:
    //..
    //  this->d_start.valuePtr() -----------+
    //                                      V
    //  this->d_start.blockPtr() --> H -> __AAA
    //                               I -> AAAAA
    //             pos.blockPtr() -> J -> BBBxx <- pos.valuePtr() (at 1st x)
    //                               K -> yyyyy
    //  this->d_finish.blockPtr() -> L -> y____
    //                                     ^
    //                                     +- this->d_finish.valuePtr()
    //
    // AFTER:
    //  this->d_start.valuePtr() -----------+
    //                                      V
    //  this->d_start.blockPtr() --> H -> __AAA
    //                               I -> AAAAA
    //  this->d_finish.blockPtr() -> J -> BBB__
    //                                       ^
    //                                       +- this->d_finish.valuePtr()
    //
    // other->d_start.valuePtr() ------------+
    //                                       V
    // other->d_start.blockPtr() --> M -> ___xx
    //                               K -> yyyyy
    // other->d_finish.blockPtr() -> L -> y____
    //                                     ^
    //                                     +- other->d_finish.valuePtr()
    //
    // assert(! other.d_blocks_p);
    //..

    if (pos.blockPtr() == this->d_finish.blockPtr()) {
        // Split point is in last block.  Just copy portion after the split to
        // new block in 'other'.

        difference_type numAfter = this->d_finish.valuePtr() - pos.valuePtr();
        other->privateInit(numAfter);
        BloombergLP::bslalg::ArrayPrimitives::destructiveMove(
                                                  other->d_start.valuePtr(),
                                                  pos.valuePtr(),
                                                  this->d_finish.valuePtr(),
                                                  ContainerBase::allocator());
        other->d_finish += numAfter;
        this->d_finish = pos;
        return;                                                       // RETURN
    }

    if (pos.blockPtr() == this->d_start.blockPtr()) {
        // Split point is in first block.  Copy portion before the split to new
        // block in 'other' and swap.

        difference_type numBefore = pos.valuePtr() - this->d_start.valuePtr();
        other->privateInit(numBefore);
        BloombergLP::bslalg::ArrayPrimitives::destructiveMove(
                                                  other->d_start.valuePtr(),
                                                  this->d_start.valuePtr(),
                                                  pos.valuePtr(),
                                                  ContainerBase::allocator());
        other->d_finish += numBefore;
        this->d_start = pos;
        Deque_Util::swap(
                        static_cast<Base *>(this), static_cast<Base *>(other));
        return;                                                       // RETURN
    }

    // Compute number of unsplit blocks to move.

    difference_type numMoveBlocks = this->d_finish.blockPtr() - pos.blockPtr();

    size_type otherBlocksLength = numMoveBlocks + 1 +
                                              2 * Imp::BLOCK_ARRAY_PADDING;

    other->d_blocks_p     = this->allocateN((BlockPtr *)0, otherBlocksLength);
    other->d_blocksLength = otherBlocksLength;

    // Good time to allocate block for exception safety.

    Block *newBlock = this->allocateN((Block *)0, 1);

    // The following chunk of code will never throw an exception.  Move unsplit
    // blocks from 'this' to 'other', then adjust the iterators.

    std::memcpy(other->d_blocks_p + 1 + Imp::BLOCK_ARRAY_PADDING,
                pos.blockPtr() + 1,
                sizeof(BlockPtr) * numMoveBlocks);

    other->d_start = IteratorImp(&other->d_blocks_p[
                                                1 + Imp::BLOCK_ARRAY_PADDING]);
    other->d_finish = IteratorImp(other->d_start.blockPtr() +
                                  numMoveBlocks - 1,
                                  this->d_finish.valuePtr());

    BlockPtr *newBlockPtr = pos.blockPtr() + 1;
    *newBlockPtr = newBlock;
    this->d_finish = IteratorImp(newBlockPtr);

    // Current situation:
    //..
    //  this->d_start.valuePtr() -----------+
    //                                      V
    //  this->d_start.blockPtr() --> H -> __AAA
    //                               I -> AAAAA
    //             pos.blockPtr() -> J -> BBBxx <- pos.valuePtr() (1st x)
    //  this->d_finish.blockPtr() -> M -> _____
    //                              /        ^
    //                newBlockPtr -+         +- this->d_finish.valuePtr()
    //
    //  other->d_start.valuePtr() ---------+
    //                                     V
    //  other->d_start.blockPtr() --> K -> yyyyy
    //  other->d_finish.blockPtr() -> L -> y____
    //                                      ^
    //                                      +- other->d_finish.valuePtr()
    //..
    // Now we split the block containing "BBBxx" into two blocks, with the "xx"
    // part going into '*newBlockPtr'.  An exception can safely occur here
    // because the 'bslalg::ArrayPrimitives' functions are exception-neutral
    // and because all class invariants for both '*this' and 'other' hold going
    // in to this section.

    size_type splitOffset = pos.offsetInBlock();
    if (splitOffset >= pos.remainingInBlock()) {
        // Move the tail part of the block into the new block.

        value_type *splitValuePtr = newBlock->d_data + splitOffset;
        BloombergLP::bslalg::ArrayPrimitives::destructiveMove(
                                                  splitValuePtr,
                                                  pos.valuePtr(),
                                                  pos.blockEnd(),
                                                  ContainerBase::allocator());
    }
    else {
        // Move the head part of the block into the new block, then swap the
        // blocks within the 'd_blocks_p' array.

        BloombergLP::bslalg::ArrayPrimitives::destructiveMove(
                                                  newBlock->d_data,
                                                  pos.blockBegin(),
                                                  pos.valuePtr(),
                                                  ContainerBase::allocator());
        *newBlockPtr    = *pos.blockPtr();
        *pos.blockPtr() = newBlock;
    }

    // Move block to 'other' and adjust the iterators.  This will not throw.

    this->d_finish = IteratorImp(&newBlockPtr[-1],
                                 newBlockPtr[-1]->d_data + splitOffset);
    other->d_start.previousBlock();
    *(other->d_start.blockPtr()) = *newBlockPtr;
    other->d_start = IteratorImp(other->d_start.blockPtr(),
                                 other->d_start.blockBegin() + splitOffset);
}

template <class VALUE_TYPE, class ALLOCATOR>
inline
void deque<VALUE_TYPE, ALLOCATOR>::privateJoinPrepend(
                                           deque<VALUE_TYPE, ALLOCATOR> *other)
{
    privatePrepend(other->begin(),
                   other->end(),
                   std::random_access_iterator_tag());

    // Make 'other' raw again, and free its resources.

    deque<VALUE_TYPE, ALLOCATOR> temp(k_RAW_INIT, other->allocator());
    Deque_Util::move(static_cast<Base *>(&temp), static_cast<Base *>(other));
}

template <class VALUE_TYPE, class ALLOCATOR>
inline
void deque<VALUE_TYPE, ALLOCATOR>::privateJoinAppend(
                                           deque<VALUE_TYPE, ALLOCATOR> *other)
{
    privateAppend(other->begin(),
                  other->end(),
                  std::random_access_iterator_tag());

    // Make 'other' raw again, and free its resources.

    deque<VALUE_TYPE, ALLOCATOR> temp(k_RAW_INIT, other->allocator());
    Deque_Util::move(static_cast<Base *>(&temp), static_cast<Base *>(other));
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class INPUT_ITERATOR>
void deque<VALUE_TYPE, ALLOCATOR>::privateInsert(
                                      const_iterator                  position,
                                      INPUT_ITERATOR                  first,
                                      INPUT_ITERATOR                  last,
                                      std::random_access_iterator_tag tag)
{
    BSLS_ASSERT(first != last);

    if (position == this->cbegin()) {
        privatePrepend(first, last, tag);
        return;                                                       // RETURN
    }

    if (position == this->cend()) {
        privateAppend(first, last, tag);
        return;                                                       // RETURN
    }

    const size_type currentSize = this->size();
    const size_type numElements = bsl::distance(first, last);
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
                                     numElements > max_size() - currentSize)) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                 "deque<...>::insert(pos,n,v): deque too big");
    }

    iterator pos(position.imp());
    const size_type posIdx = position - this->cbegin();
    if (posIdx <= currentSize / 2) {
        // Create new blocks at the front.  In case an exception is thrown, any
        // unused blocks are returned to the allocator.

        size_type numNewBlocks = (this->d_start.remainingInBlock()
                                             + numElements - 1) / BLOCK_LENGTH;
        BlockCreator newBlocks(this);
        newBlocks.insertAtFront(numNewBlocks);

        DequePrimitives::insertAndMoveToFront(&this->d_start,
                                              this->d_start,
                                              this->d_start + posIdx,
                                              first,
                                              last,
                                              numElements,
                                              ContainerBase::allocator());
    } else {
        // Create new blocks at front.  In case an exception is thrown, any
        // unused blocks are returned to the allocator.

        size_type numNewBlocks = (this->d_finish.offsetInBlock() + numElements)
                                                                / BLOCK_LENGTH;
        BlockCreator newBlocks(this);
        newBlocks.insertAtBack(numNewBlocks);

        DequePrimitives::insertAndMoveToBack(&this->d_finish,
                                             this->d_finish,
                                             this->d_start + posIdx,
                                             first,
                                             last,
                                             numElements,
                                             ContainerBase::allocator());
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
void deque<VALUE_TYPE, ALLOCATOR>::privatePrependRaw(
                                                 size_type         numElements,
                                                 const VALUE_TYPE& value)
{
    // Create new blocks at front.  In case an exception is thrown, any unused
    // blocks are returned to the allocator.

    size_type numNewBlocks = (this->d_start.remainingInBlock() +
                                               numElements - 1) / BLOCK_LENGTH;
    BlockCreator newBlocks(this);
    newBlocks.insertAtFront(numNewBlocks);

    DequePrimitives::uninitializedFillNFront(&this->d_start,
                                             this->d_start,
                                             numElements,
                                             value,
                                             ContainerBase::allocator());
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class INPUT_ITERATOR>
typename deque<VALUE_TYPE, ALLOCATOR>::size_type
deque<VALUE_TYPE, ALLOCATOR>::privatePrepend(INPUT_ITERATOR          first,
                                             INPUT_ITERATOR          last,
                                             std::input_iterator_tag tag)
{
    deque temp(k_RAW_INIT, this->get_allocator());
    temp.privateInit(this->size() + 1);
    size_type numElements = temp.privateAppend(first, last, tag);

    // Check whether appending or prepending is more economical.

    if (numElements > this->size()) {
        Deque_Util::swap((Base *)this, (Base *)&temp);
        privateJoinAppend(&temp);
    }
    else {
        privateJoinPrepend(&temp);
    }

    return numElements;
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class INPUT_ITERATOR>
typename deque<VALUE_TYPE, ALLOCATOR>::size_type
deque<VALUE_TYPE, ALLOCATOR>::privatePrepend(
                                         INPUT_ITERATOR                  first,
                                         INPUT_ITERATOR                  last,
                                         std::bidirectional_iterator_tag)
{

    BlockCreator newBlocks(this);
    Guard guard(this, false);

    size_type numElements = 0;
    size_type maxNumElements = max_size() - this->size();
    do {
        ++numElements;
        if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
                                               numElements > maxNumElements)) {
            BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

            BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                 "deque<...>::insert(pos,n,v): deque too big");
        }

        IteratorImp insertPoint = guard.begin();

        // There must be room for the iterator to be decremented.  Allocate new
        // block now if necessary, with same caveat as above.

        if (insertPoint.valuePtr() == insertPoint.blockBegin()) {
            newBlocks.insertAtFront(1);
            insertPoint = guard.begin();  // 'insertAtFront' invalidates
                                          // 'insertPoint'
        }
        --insertPoint;
        AllocatorTraits::construct(ContainerBase::allocator(),
                                   BSLS_UTIL_ADDRESSOF(*insertPoint),
                                   *--last);
        ++guard;
    } while (first != last);

    this->d_start -= guard.count();
    guard.release();
    return numElements;
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class INPUT_ITERATOR>
typename deque<VALUE_TYPE, ALLOCATOR>::size_type
deque<VALUE_TYPE, ALLOCATOR>::privatePrepend(
                                         INPUT_ITERATOR                  first,
                                         INPUT_ITERATOR                  last,
                                         std::random_access_iterator_tag)
{

    const size_type numElements = bsl::distance(first, last);
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
                                    numElements > max_size() - this->size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                 "deque<...>::insert(pos,n,v): deque too big");
    }

    BlockCreator newBlocks(this);
    Guard guard(this, false);

    do {
        IteratorImp insertPoint = guard.begin();

        // There must be room for the iterator to be decremented.  Allocate new
        // block now if necessary, with same caveat as above.

        if (insertPoint.valuePtr() == insertPoint.blockBegin()) {
            newBlocks.insertAtFront(1);
            insertPoint = guard.begin();  // 'insertAtFront' invalidates
                                          // 'insertPoint'
        }
        --insertPoint;
        AllocatorTraits::construct(ContainerBase::allocator(),
                                   BSLS_UTIL_ADDRESSOF(*insertPoint),
                                   *--last);
        ++guard;
    } while (first != last);

    this->d_start -= guard.count();
    guard.release();
    return numElements;
}

// CREATORS
template <class VALUE_TYPE, class ALLOCATOR>
deque<VALUE_TYPE, ALLOCATOR>::deque()
: Deque_Base<VALUE_TYPE>()
, ContainerBase(ALLOCATOR())
{
    deque temp(k_RAW_INIT, this->get_allocator());
    temp.privateInit(0);
    Deque_Util::move(static_cast<Base *>(this), static_cast<Base *>(&temp));
}

template <class VALUE_TYPE, class ALLOCATOR>
deque<VALUE_TYPE, ALLOCATOR>::deque(const ALLOCATOR& basicAllocator)
: Deque_Base<VALUE_TYPE>()
, ContainerBase(basicAllocator)
{
    deque temp(k_RAW_INIT, this->get_allocator());
    temp.privateInit(0);
    Deque_Util::move(static_cast<Base *>(this), static_cast<Base *>(&temp));
}

template <class VALUE_TYPE, class ALLOCATOR>
deque<VALUE_TYPE, ALLOCATOR>::deque(size_type        numElements,
                                    const ALLOCATOR& basicAllocator)
: Deque_Base<VALUE_TYPE>()
, ContainerBase(basicAllocator)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(numElements > max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                        "deque<...>::deque(n): deque too big");
    }
    deque temp(k_RAW_INIT, this->get_allocator());
    temp.privateInit(numElements);
    temp.privateAppendRaw(numElements, VALUE_TYPE());
    Deque_Util::move(static_cast<Base *>(this), static_cast<Base *>(&temp));
}

template <class VALUE_TYPE, class ALLOCATOR>
deque<VALUE_TYPE, ALLOCATOR>::deque(size_type         numElements,
                                    const VALUE_TYPE& value,
                                    const ALLOCATOR&  basicAllocator)
: Deque_Base<VALUE_TYPE>()
, ContainerBase(basicAllocator)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(numElements > max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                      "deque<...>::deque(n,v): deque too big");
    }
    deque temp(k_RAW_INIT, this->get_allocator());
    temp.privateInit(numElements);
    temp.privateAppendRaw(numElements, value);
    Deque_Util::move(static_cast<Base *>(this), static_cast<Base *>(&temp));
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class INPUT_ITERATOR>
deque<VALUE_TYPE, ALLOCATOR>::deque(INPUT_ITERATOR   first,
                                    INPUT_ITERATOR   last,
                                    const ALLOCATOR& basicAllocator)
: Deque_Base<VALUE_TYPE>()
, ContainerBase(basicAllocator)
{
    deque temp(k_RAW_INIT, this->get_allocator());
    temp.privateInit(0);
    temp.insert(temp.begin(), first, last);
    Deque_Util::move(static_cast<Base *>(this), static_cast<Base *>(&temp));
}

template <class VALUE_TYPE, class ALLOCATOR>
deque<VALUE_TYPE, ALLOCATOR>::deque(
                                  const deque<VALUE_TYPE, ALLOCATOR>& original)
: Deque_Base<VALUE_TYPE>()
, ContainerBase(AllocatorTraits::select_on_container_copy_construction(
                                                     original.get_allocator()))
{
    deque temp(k_RAW_INIT, this->get_allocator());
    temp.privateInit(original.size());
    temp.privateAppend(original.begin(),
                       original.end(),
                       std::random_access_iterator_tag());
    Deque_Util::move(static_cast<Base *>(this), static_cast<Base *>(&temp));
}

template <class VALUE_TYPE, class ALLOCATOR>
deque<VALUE_TYPE, ALLOCATOR>::deque(
                            const deque<VALUE_TYPE, ALLOCATOR>& original,
                            const ALLOCATOR&                    basicAllocator)
: Deque_Base<VALUE_TYPE>()
, ContainerBase(basicAllocator)
{
    deque temp(k_RAW_INIT, this->get_allocator());
    temp.privateInit(original.size());
    temp.privateAppend(original.begin(),
                       original.end(),
                       std::random_access_iterator_tag());
    Deque_Util::move(static_cast<Base *>(this), static_cast<Base *>(&temp));
}

template <class VALUE_TYPE, class ALLOCATOR>
deque<VALUE_TYPE, ALLOCATOR>::deque(
                                BloombergLP::bslmf::MovableRef<deque> original)
: Deque_Base<VALUE_TYPE>()
, ContainerBase(MoveUtil::access(original).get_allocator())
{
    deque temp(k_RAW_INIT, this->get_allocator());
    temp.privateInit(0);
    Deque_Util::move(static_cast<Base *>(this), static_cast<Base *>(&temp));

    deque& lvalue = original;
    Deque_Util::swap(static_cast<Base *>(this), static_cast<Base *>(&lvalue));
}

template <class VALUE_TYPE, class ALLOCATOR>
deque<VALUE_TYPE, ALLOCATOR>::deque(
                          BloombergLP::bslmf::MovableRef<deque> original,
                          const ALLOCATOR&                      basicAllocator)
: Deque_Base<VALUE_TYPE>()
, ContainerBase(basicAllocator)
{
    deque temp(k_RAW_INIT, this->get_allocator());
    temp.privateInit(0);

    deque& lvalue = original;

    if (BSLS_PERFORMANCEHINT_PREDICT_LIKELY(
                                  get_allocator() == lvalue.get_allocator())) {
        Deque_Util::move(static_cast<Base *>(this),
                         static_cast<Base *>(&temp));
        Deque_Util::swap(static_cast<Base *>(this),
                         static_cast<Base *>(&lvalue));
    }
    else {
        const size_type size = lvalue.size();
        temp.reserve(size);
        for (size_type pos = 0; pos < size; ++pos) {
            temp.push_back(MoveUtil::move(lvalue[pos]));
        }
        Deque_Util::move(static_cast<Base *>(this),
                         static_cast<Base *>(&temp));
    }
}

#if defined(BSLS_COMPILERFEATURES_SUPPORT_GENERALIZED_INITIALIZERS)
template <class VALUE_TYPE, class ALLOCATOR>
inline
deque<VALUE_TYPE, ALLOCATOR>::deque(
                              std::initializer_list<value_type> values,
                              const ALLOCATOR&                  basicAllocator)
: deque(values.begin(), values.end(), basicAllocator)
{
}
#endif

template <class VALUE_TYPE, class ALLOCATOR>
deque<VALUE_TYPE, ALLOCATOR>::~deque()
{
    if (0 == this->d_blocks_p) {
        // Nothing to do when destroying raw deques.

        return;                                                       // RETURN
    }

    if (0 != this->d_start.blockPtr()) {
        // Destroy all elements and deallocate all but one block.

        clear();

        // Deallocate the remaining (empty) block.

        this->deallocateN(*this->d_start.blockPtr(), 1);
    }

    // Deallocate the array of block pointers.

    this->deallocateN(this->d_blocks_p, this->d_blocksLength);
}

// MANIPULATORS
template <class VALUE_TYPE, class ALLOCATOR>
deque<VALUE_TYPE, ALLOCATOR>&
deque<VALUE_TYPE, ALLOCATOR>::operator=(
                                       const deque<VALUE_TYPE, ALLOCATOR>& rhs)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_LIKELY(this != &rhs)) {
        if (AllocatorTraits::propagate_on_container_copy_assignment::value) {
            deque other(rhs, rhs.get_allocator());

            Deque_Util::swap(static_cast<Base *>(this),
                             static_cast<Base *>(&other));
            BloombergLP::bslalg::SwapUtil::swap(
                                            &ContainerBase::allocator(),
                                            &other.ContainerBase::allocator());
        }
        else {
            size_type origSize = this->size();
            size_type rhsSize  = rhs.size();
            size_type minSize;

            if (origSize > rhsSize) {
                // Make shorter by deleting excess elements.

                minSize = rhsSize;
                erase(this->begin() + minSize, this->end());
            }
            else {
                // Make longer by appending new elements.

                minSize = origSize;
                privateAppend(rhs.begin() + minSize,
                              rhs.end(),
                              std::random_access_iterator_tag());
            }

            // Copy the smaller of the number of elements in 'rhs' and '*this'.

            IteratorImp from = rhs.d_start;
            IteratorImp to   = this->d_start;
            for (size_type i = 0; i < minSize; ++i) {
                *to = *from;
                ++to;
                ++from;
            }
        }
    }

    return *this;
}

template <class VALUE_TYPE, class ALLOCATOR>
deque<VALUE_TYPE, ALLOCATOR>&
deque<VALUE_TYPE, ALLOCATOR>::operator=(
                                     BloombergLP::bslmf::MovableRef<deque> rhs)
              BSLS_CPP11_NOEXCEPT_SPECIFICATION(BSLS_CPP11_PROVISIONALLY_FALSE)
{
    deque& lvalue = rhs;

    if (BSLS_PERFORMANCEHINT_PREDICT_LIKELY(this != &lvalue)) {
        if (get_allocator() == lvalue.get_allocator()) {
            Deque_Util::swap(static_cast<Base *>(this),
                             static_cast<Base *>(&lvalue));
        }
        else if (
              AllocatorTraits::propagate_on_container_move_assignment::value) {
            deque other(MoveUtil::move(lvalue));
            Deque_Util::swap(static_cast<Base *>(this),
                             static_cast<Base *>(&other));
            BloombergLP::bslalg::SwapUtil::swap(
                                            &ContainerBase::allocator(),
                                            &other.ContainerBase::allocator());
        }
        else {
            deque other(MoveUtil::move(lvalue), get_allocator());
            Deque_Util::swap(static_cast<Base *>(this),
                             static_cast<Base *>(&other));
        }
    }
    return *this;
}

#if defined(BSLS_COMPILERFEATURES_SUPPORT_GENERALIZED_INITIALIZERS)
template <class VALUE_TYPE, class ALLOCATOR>
inline
deque<VALUE_TYPE, ALLOCATOR>&
deque<VALUE_TYPE, ALLOCATOR>::operator=(
                                      std::initializer_list<value_type> values)
{
    assign(values.begin(), values.end());
    return *this;
}
#endif

template <class VALUE_TYPE, class ALLOCATOR>
template <class INPUT_ITERATOR>
void deque<VALUE_TYPE, ALLOCATOR>::assign(INPUT_ITERATOR first,
                                          INPUT_ITERATOR last)
{
    typedef typename iterator_traits<INPUT_ITERATOR>::iterator_category Tag;

    // If an exception is thrown, clear the deque to provide standard behavior,
    // which is:
    //..
    //  erase(begin(), end());
    //  insert(begin(), first, last);
    //..

    ClearGuard guard(this);

    // Copy over existing elements.

    IteratorImp i;
    for (i = this->d_start; !(i == this->d_finish) && first != last;
                                                                ++i, ++first) {
        *i = *first;
    }

    if (!(i == this->d_finish)) {
        // Erase elements past the last one copied.

        erase(i, this->end());
    }
    else {
        // Still more elements to copy.  Append them.

        privateAppend(first, last, Tag());
    }

    guard.release();
}

template <class VALUE_TYPE, class ALLOCATOR>
void deque<VALUE_TYPE, ALLOCATOR>::assign(size_type         numElements,
                                          const VALUE_TYPE& value)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(numElements > max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                     "deque<...>::assign(n,v): deque too big");
    }

    // If an exception is thrown, clear the deque to provide standard behavior,
    // which is:
    //..
    //  erase(begin(), end());
    //  insert(begin(), first, last);
    //..

    ClearGuard guard(this);

    size_type origSize = this->size();
    size_type minSize;

    if (numElements < origSize) {
        minSize = numElements;
        erase(this->begin() + numElements, this->end());
    }
    else {
        minSize = origSize;
        privateAppendRaw(numElements - origSize, value);
    }

    IteratorImp to = this->d_start;
    for (size_type i = 0; i < minSize; ++i) {
        *to = value;
        ++to;
    }

    guard.release();
}

#if defined(BSLS_COMPILERFEATURES_SUPPORT_GENERALIZED_INITIALIZERS)
template <class VALUE_TYPE, class ALLOCATOR>
inline
void deque<VALUE_TYPE, ALLOCATOR>::assign(
                                      std::initializer_list<value_type> values)
{
    assign(values.begin(), values.end());
}
#endif

template <class VALUE_TYPE, class ALLOCATOR>
void deque<VALUE_TYPE, ALLOCATOR>::reserve(size_type numElements)
{
    // Make sure 'numElements' isn't high enough to make the calculations of
    // 'num(Front|Back)Blocks' overflow.

    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(numElements >
                                            max_size() - (BLOCK_LENGTH - 1))) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                      "deque<...>::reserve(n): deque too big");
    }

    // 'ContainerBase::allocateN', which creates the 'd_blocks_p' array, does
    // not, in its contract, guarantee to initialize the array to 0.  Since we
    // read these values, we have to make sure they're initialized to avoid
    // purify 'read before write' errors.  Note that we initialize them to 0,
    // making them invalid pointers, but we never dereference them, and the
    // pointer arithmetic we do on them will still work.

    if (this->d_start.blockPtr() > this->d_blocks_p) {
        this->d_blocks_p[0] = 0;
    }
    if (this->d_finish.blockPtr() < this->d_blocks_p + this->d_blocksLength-1){
        this->d_blocks_p[this->d_blocksLength - 1] = 0;
    }

    const IteratorImp first(this->d_blocks_p);
    IteratorImp       last( this->d_blocks_p + this->d_blocksLength - 1);
    last += BLOCK_LENGTH - 1;

    const size_type frontRoom = this->d_start - first;
    const size_type backRoom  = last - this->d_finish;

    size_type numFrontBlocks = numElements > frontRoom
                             ? (numElements - frontRoom + BLOCK_LENGTH - 1) /
                                                                   BLOCK_LENGTH
                             : 0;
    size_type numBackBlocks  = numElements > backRoom
                             ? (numElements - backRoom  + BLOCK_LENGTH - 1) /
                                                                   BLOCK_LENGTH
                             : 0;

    if (0 == numFrontBlocks && 0 == numBackBlocks) {
        return;                                                       // RETURN
    }

    // Make sure that if we throw, it's before we modify the deque.

    size_type existingSpace = last - first;
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(numFrontBlocks >
                               (max_size() - existingSpace) / BLOCK_LENGTH
       || (existingSpace += numFrontBlocks * BLOCK_LENGTH,
                           numBackBlocks >
                               (max_size() - existingSpace) / BLOCK_LENGTH))) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                      "deque<...>::reserve(n): deque too big");
    }

    // When 'numFrontBlocks' or 'numBackBlocks' are 0, the respective calls
    // will be no-ops.

    BlockCreator newBlocks(this);
    newBlocks.reserveBlockSlots(numFrontBlocks, true);
    newBlocks.reserveBlockSlots(numBackBlocks,  false);
}

template <class VALUE_TYPE, class ALLOCATOR>
inline
void deque<VALUE_TYPE, ALLOCATOR>::resize(size_type newSize)
{
    resize(newSize, VALUE_TYPE());
}

template <class VALUE_TYPE, class ALLOCATOR>
void deque<VALUE_TYPE, ALLOCATOR>::resize(size_type         newSize,
                                          const VALUE_TYPE& value)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(newSize > max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                     "deque<...>::resize(n,v): deque too big");
    }

    size_type origSize = this->size();

    if (newSize <= origSize) {
        erase(this->begin() + newSize, this->end());
    }
    else {
        privateAppendRaw(newSize - origSize, value);
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
void deque<VALUE_TYPE, ALLOCATOR>::shrink_to_fit()
{
    // Minimize the length of 'd_blocks_p' without moving any elements.  A more
    // complex algorithm is not justified.  At most 'BLOCK_LENGTH' bytes are
    // wasted.

    const size_type newBlocksLength =
                      this->d_finish.blockPtr() - this->d_start.blockPtr() + 1;

    if (newBlocksLength == this->d_blocksLength) {
        return;                                                       // RETURN
    }

    const size_type offsetStart  = this->d_start.offsetInBlock();
    const size_type offsetFinish = this->d_finish.offsetInBlock();

    BlockPtr *newBlocks = this->allocateN((BlockPtr *)0, newBlocksLength);

    std::memmove(newBlocks,
                 this->d_start.blockPtr(),
                 newBlocksLength * sizeof(BlockPtr));

    this->deallocateN(this->d_blocks_p, this->d_blocksLength);

    this->d_blocks_p     = newBlocks;
    this->d_blocksLength = newBlocksLength;

    this->d_start.setBlock(newBlocks);
    this->d_start += offsetStart;

    this->d_finish.setBlock(newBlocks + newBlocksLength - 1);
    this->d_finish += offsetFinish;
}

template <class VALUE_TYPE, class ALLOCATOR>
void deque<VALUE_TYPE, ALLOCATOR>::push_front(const VALUE_TYPE& value)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                   "deque<...>::push_front(v): deque too big");
    }

    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
                                         0 == this->d_start.offsetInBlock())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtFront(1);  // The deque's value is not modified.

        AllocatorTraits::construct(
            ContainerBase::allocator(), (this->d_start - 1).valuePtr(), value);

        --this->d_start;
    }
    else {
        // Since the offset is non-zero, it is safe to directly decrement the
        // pointer.  This is much quicker than calling 'operator--'.

        AllocatorTraits::construct(
            ContainerBase::allocator(), this->d_start.valuePtr() - 1, value);
        this->d_start.valuePtrDecrement();
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
void deque<VALUE_TYPE, ALLOCATOR>::push_front(
                              BloombergLP::bslmf::MovableRef<value_type> value)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                   "deque<...>::push_front(v): deque too big");
    }

    VALUE_TYPE& lvalue = value;

    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
                                         0 == this->d_start.offsetInBlock())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtFront(1);  // The deque's value is not modified.

        AllocatorTraits::construct(ContainerBase::allocator(),
                                   (this->d_start - 1).valuePtr(),
                                   MoveUtil::move(lvalue));
        --this->d_start;
    }
    else {
        // Since the offset is non-zero, it is safe to directly decrement the
        // pointer.  This is much quicker than calling 'operator--'.

        AllocatorTraits::construct(ContainerBase::allocator(),
                                   this->d_start.valuePtr() - 1,
                                   MoveUtil::move(lvalue));
        this->d_start.valuePtrDecrement();
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
void deque<VALUE_TYPE, ALLOCATOR>::push_back(const VALUE_TYPE& value)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                    "deque<...>::push_back(v): deque too big");
    }

    if (BSLS_PERFORMANCEHINT_PREDICT_LIKELY(
                                      1 < this->d_finish.remainingInBlock())) {
        AllocatorTraits::construct(
                 ContainerBase::allocator(), this->d_finish.valuePtr(), value);
        this->d_finish.valuePtrIncrement();
    }
    else {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtBack(1);  // The deque's value is not modified.

        AllocatorTraits::construct(
                 ContainerBase::allocator(), this->d_finish.valuePtr(), value);
        this->d_finish.nextBlock();
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
void deque<VALUE_TYPE, ALLOCATOR>::push_back(
                              BloombergLP::bslmf::MovableRef<value_type> value)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                    "deque<...>::push_back(v): deque too big");
    }

    VALUE_TYPE& lvalue = value;

    if (BSLS_PERFORMANCEHINT_PREDICT_LIKELY(
                                      1 < this->d_finish.remainingInBlock())) {
        AllocatorTraits::construct(ContainerBase::allocator(),
                                   this->d_finish.valuePtr(),
                                   MoveUtil::move(lvalue));
        this->d_finish.valuePtrIncrement();
    }
    else {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtBack(1);  // The deque's value is not modified.

        AllocatorTraits::construct(ContainerBase::allocator(),
                                   this->d_finish.valuePtr(),
                                   MoveUtil::move(lvalue));
        this->d_finish.nextBlock();
    }
}

#if !BSLS_COMPILERFEATURES_SIMULATE_CPP11_FEATURES
template <class VALUE_TYPE, class ALLOCATOR>
template <class... Args>
void deque<VALUE_TYPE, ALLOCATOR>::emplace_front(Args&&...arguments)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                             "deque<...>::emplace_front(args): deque too big");
    }

    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
                                         0 == this->d_start.offsetInBlock())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtFront(1);  // The deque's value is not modified.

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            (this->d_start - 1).valuePtr(),
                            BSLS_COMPILERFEATURES_FORWARD(Args, arguments)...);
        --this->d_start;
    }
    else {
        // Since the offset is non-zero, it is safe to directly decrement the
        // pointer.  This is much quicker than calling 'operator--'.

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_start.valuePtr() - 1,
                            BSLS_COMPILERFEATURES_FORWARD(Args, arguments)...);
        this->d_start.valuePtrDecrement();
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class... Args>
void deque<VALUE_TYPE, ALLOCATOR>::emplace_back(Args&&...arguments)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                              "deque<...>::emplace_back(args): deque too big");
    }

    if (BSLS_PERFORMANCEHINT_PREDICT_LIKELY(
                                      1 < this->d_finish.remainingInBlock())) {
        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_finish.valuePtr(),
                            BSLS_COMPILERFEATURES_FORWARD(Args, arguments)...);
        this->d_finish.valuePtrIncrement();
    }
    else {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtBack(1);  // The deque's value is not modified.

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_finish.valuePtr(),
                            BSLS_COMPILERFEATURES_FORWARD(Args, arguments)...);
        this->d_finish.nextBlock();
    }
}
#elif BSLS_COMPILERFEATURES_SIMULATE_VARIADIC_TEMPLATES
// {{{ BEGIN GENERATED CODE
// The following section is automatically generated.  **DO NOT EDIT**
// Generator command line: sim_cpp11_features.pl bslstl_deque.h
template <class VALUE_TYPE, class ALLOCATOR>
void deque<VALUE_TYPE, ALLOCATOR>::emplace_front(
                           )
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                             "deque<...>::emplace_front(args): deque too big");
    }

    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
                                         0 == this->d_start.offsetInBlock())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtFront(1);

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            (this->d_start - 1).valuePtr());
        --this->d_start;
    }
    else {

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_start.valuePtr() - 1);
        this->d_start.valuePtrDecrement();
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01>
void deque<VALUE_TYPE, ALLOCATOR>::emplace_front(
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                             "deque<...>::emplace_front(args): deque too big");
    }

    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
                                         0 == this->d_start.offsetInBlock())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtFront(1);

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            (this->d_start - 1).valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01));
        --this->d_start;
    }
    else {

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_start.valuePtr() - 1,
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01));
        this->d_start.valuePtrDecrement();
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01,
          class Args_02>
void deque<VALUE_TYPE, ALLOCATOR>::emplace_front(
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                             "deque<...>::emplace_front(args): deque too big");
    }

    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
                                         0 == this->d_start.offsetInBlock())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtFront(1);

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            (this->d_start - 1).valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02));
        --this->d_start;
    }
    else {

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_start.valuePtr() - 1,
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02));
        this->d_start.valuePtrDecrement();
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01,
          class Args_02,
          class Args_03>
void deque<VALUE_TYPE, ALLOCATOR>::emplace_front(
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                             "deque<...>::emplace_front(args): deque too big");
    }

    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
                                         0 == this->d_start.offsetInBlock())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtFront(1);

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            (this->d_start - 1).valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03));
        --this->d_start;
    }
    else {

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_start.valuePtr() - 1,
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03));
        this->d_start.valuePtrDecrement();
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01,
          class Args_02,
          class Args_03,
          class Args_04>
void deque<VALUE_TYPE, ALLOCATOR>::emplace_front(
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                             "deque<...>::emplace_front(args): deque too big");
    }

    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
                                         0 == this->d_start.offsetInBlock())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtFront(1);

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            (this->d_start - 1).valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04));
        --this->d_start;
    }
    else {

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_start.valuePtr() - 1,
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04));
        this->d_start.valuePtrDecrement();
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01,
          class Args_02,
          class Args_03,
          class Args_04,
          class Args_05>
void deque<VALUE_TYPE, ALLOCATOR>::emplace_front(
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                             "deque<...>::emplace_front(args): deque too big");
    }

    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
                                         0 == this->d_start.offsetInBlock())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtFront(1);

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            (this->d_start - 1).valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05));
        --this->d_start;
    }
    else {

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_start.valuePtr() - 1,
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05));
        this->d_start.valuePtrDecrement();
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01,
          class Args_02,
          class Args_03,
          class Args_04,
          class Args_05,
          class Args_06>
void deque<VALUE_TYPE, ALLOCATOR>::emplace_front(
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                             "deque<...>::emplace_front(args): deque too big");
    }

    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
                                         0 == this->d_start.offsetInBlock())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtFront(1);

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            (this->d_start - 1).valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06));
        --this->d_start;
    }
    else {

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_start.valuePtr() - 1,
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06));
        this->d_start.valuePtrDecrement();
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01,
          class Args_02,
          class Args_03,
          class Args_04,
          class Args_05,
          class Args_06,
          class Args_07>
void deque<VALUE_TYPE, ALLOCATOR>::emplace_front(
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_07) arguments_07)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                             "deque<...>::emplace_front(args): deque too big");
    }

    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
                                         0 == this->d_start.offsetInBlock())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtFront(1);

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            (this->d_start - 1).valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                         BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07));
        --this->d_start;
    }
    else {

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_start.valuePtr() - 1,
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                         BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07));
        this->d_start.valuePtrDecrement();
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01,
          class Args_02,
          class Args_03,
          class Args_04,
          class Args_05,
          class Args_06,
          class Args_07,
          class Args_08>
void deque<VALUE_TYPE, ALLOCATOR>::emplace_front(
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_07) arguments_07,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_08) arguments_08)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                             "deque<...>::emplace_front(args): deque too big");
    }

    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
                                         0 == this->d_start.offsetInBlock())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtFront(1);

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            (this->d_start - 1).valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                         BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07),
                         BSLS_COMPILERFEATURES_FORWARD(Args_08, arguments_08));
        --this->d_start;
    }
    else {

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_start.valuePtr() - 1,
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                         BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07),
                         BSLS_COMPILERFEATURES_FORWARD(Args_08, arguments_08));
        this->d_start.valuePtrDecrement();
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01,
          class Args_02,
          class Args_03,
          class Args_04,
          class Args_05,
          class Args_06,
          class Args_07,
          class Args_08,
          class Args_09>
void deque<VALUE_TYPE, ALLOCATOR>::emplace_front(
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_07) arguments_07,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_08) arguments_08,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_09) arguments_09)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                             "deque<...>::emplace_front(args): deque too big");
    }

    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
                                         0 == this->d_start.offsetInBlock())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtFront(1);

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            (this->d_start - 1).valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                         BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07),
                         BSLS_COMPILERFEATURES_FORWARD(Args_08, arguments_08),
                         BSLS_COMPILERFEATURES_FORWARD(Args_09, arguments_09));
        --this->d_start;
    }
    else {

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_start.valuePtr() - 1,
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                         BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07),
                         BSLS_COMPILERFEATURES_FORWARD(Args_08, arguments_08),
                         BSLS_COMPILERFEATURES_FORWARD(Args_09, arguments_09));
        this->d_start.valuePtrDecrement();
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01,
          class Args_02,
          class Args_03,
          class Args_04,
          class Args_05,
          class Args_06,
          class Args_07,
          class Args_08,
          class Args_09,
          class Args_10>
void deque<VALUE_TYPE, ALLOCATOR>::emplace_front(
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_07) arguments_07,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_08) arguments_08,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_09) arguments_09,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_10) arguments_10)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                             "deque<...>::emplace_front(args): deque too big");
    }

    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
                                         0 == this->d_start.offsetInBlock())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtFront(1);

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            (this->d_start - 1).valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                         BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07),
                         BSLS_COMPILERFEATURES_FORWARD(Args_08, arguments_08),
                         BSLS_COMPILERFEATURES_FORWARD(Args_09, arguments_09),
                         BSLS_COMPILERFEATURES_FORWARD(Args_10, arguments_10));
        --this->d_start;
    }
    else {

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_start.valuePtr() - 1,
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                         BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07),
                         BSLS_COMPILERFEATURES_FORWARD(Args_08, arguments_08),
                         BSLS_COMPILERFEATURES_FORWARD(Args_09, arguments_09),
                         BSLS_COMPILERFEATURES_FORWARD(Args_10, arguments_10));
        this->d_start.valuePtrDecrement();
    }
}


template <class VALUE_TYPE, class ALLOCATOR>
void deque<VALUE_TYPE, ALLOCATOR>::emplace_back(
                           )
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                              "deque<...>::emplace_back(args): deque too big");
    }

    if (BSLS_PERFORMANCEHINT_PREDICT_LIKELY(
                                      1 < this->d_finish.remainingInBlock())) {
        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_finish.valuePtr());
        this->d_finish.valuePtrIncrement();
    }
    else {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtBack(1);

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_finish.valuePtr());
        this->d_finish.nextBlock();
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01>
void deque<VALUE_TYPE, ALLOCATOR>::emplace_back(
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                              "deque<...>::emplace_back(args): deque too big");
    }

    if (BSLS_PERFORMANCEHINT_PREDICT_LIKELY(
                                      1 < this->d_finish.remainingInBlock())) {
        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_finish.valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01));
        this->d_finish.valuePtrIncrement();
    }
    else {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtBack(1);

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_finish.valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01));
        this->d_finish.nextBlock();
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01,
          class Args_02>
void deque<VALUE_TYPE, ALLOCATOR>::emplace_back(
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                              "deque<...>::emplace_back(args): deque too big");
    }

    if (BSLS_PERFORMANCEHINT_PREDICT_LIKELY(
                                      1 < this->d_finish.remainingInBlock())) {
        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_finish.valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02));
        this->d_finish.valuePtrIncrement();
    }
    else {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtBack(1);

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_finish.valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02));
        this->d_finish.nextBlock();
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01,
          class Args_02,
          class Args_03>
void deque<VALUE_TYPE, ALLOCATOR>::emplace_back(
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                              "deque<...>::emplace_back(args): deque too big");
    }

    if (BSLS_PERFORMANCEHINT_PREDICT_LIKELY(
                                      1 < this->d_finish.remainingInBlock())) {
        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_finish.valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03));
        this->d_finish.valuePtrIncrement();
    }
    else {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtBack(1);

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_finish.valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03));
        this->d_finish.nextBlock();
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01,
          class Args_02,
          class Args_03,
          class Args_04>
void deque<VALUE_TYPE, ALLOCATOR>::emplace_back(
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                              "deque<...>::emplace_back(args): deque too big");
    }

    if (BSLS_PERFORMANCEHINT_PREDICT_LIKELY(
                                      1 < this->d_finish.remainingInBlock())) {
        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_finish.valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04));
        this->d_finish.valuePtrIncrement();
    }
    else {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtBack(1);

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_finish.valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04));
        this->d_finish.nextBlock();
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01,
          class Args_02,
          class Args_03,
          class Args_04,
          class Args_05>
void deque<VALUE_TYPE, ALLOCATOR>::emplace_back(
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                              "deque<...>::emplace_back(args): deque too big");
    }

    if (BSLS_PERFORMANCEHINT_PREDICT_LIKELY(
                                      1 < this->d_finish.remainingInBlock())) {
        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_finish.valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05));
        this->d_finish.valuePtrIncrement();
    }
    else {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtBack(1);

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_finish.valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05));
        this->d_finish.nextBlock();
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01,
          class Args_02,
          class Args_03,
          class Args_04,
          class Args_05,
          class Args_06>
void deque<VALUE_TYPE, ALLOCATOR>::emplace_back(
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                              "deque<...>::emplace_back(args): deque too big");
    }

    if (BSLS_PERFORMANCEHINT_PREDICT_LIKELY(
                                      1 < this->d_finish.remainingInBlock())) {
        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_finish.valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06));
        this->d_finish.valuePtrIncrement();
    }
    else {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtBack(1);

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_finish.valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06));
        this->d_finish.nextBlock();
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01,
          class Args_02,
          class Args_03,
          class Args_04,
          class Args_05,
          class Args_06,
          class Args_07>
void deque<VALUE_TYPE, ALLOCATOR>::emplace_back(
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_07) arguments_07)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                              "deque<...>::emplace_back(args): deque too big");
    }

    if (BSLS_PERFORMANCEHINT_PREDICT_LIKELY(
                                      1 < this->d_finish.remainingInBlock())) {
        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_finish.valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                         BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07));
        this->d_finish.valuePtrIncrement();
    }
    else {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtBack(1);

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_finish.valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                         BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07));
        this->d_finish.nextBlock();
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01,
          class Args_02,
          class Args_03,
          class Args_04,
          class Args_05,
          class Args_06,
          class Args_07,
          class Args_08>
void deque<VALUE_TYPE, ALLOCATOR>::emplace_back(
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_07) arguments_07,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_08) arguments_08)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                              "deque<...>::emplace_back(args): deque too big");
    }

    if (BSLS_PERFORMANCEHINT_PREDICT_LIKELY(
                                      1 < this->d_finish.remainingInBlock())) {
        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_finish.valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                         BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07),
                         BSLS_COMPILERFEATURES_FORWARD(Args_08, arguments_08));
        this->d_finish.valuePtrIncrement();
    }
    else {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtBack(1);

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_finish.valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                         BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07),
                         BSLS_COMPILERFEATURES_FORWARD(Args_08, arguments_08));
        this->d_finish.nextBlock();
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01,
          class Args_02,
          class Args_03,
          class Args_04,
          class Args_05,
          class Args_06,
          class Args_07,
          class Args_08,
          class Args_09>
void deque<VALUE_TYPE, ALLOCATOR>::emplace_back(
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_07) arguments_07,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_08) arguments_08,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_09) arguments_09)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                              "deque<...>::emplace_back(args): deque too big");
    }

    if (BSLS_PERFORMANCEHINT_PREDICT_LIKELY(
                                      1 < this->d_finish.remainingInBlock())) {
        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_finish.valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                         BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07),
                         BSLS_COMPILERFEATURES_FORWARD(Args_08, arguments_08),
                         BSLS_COMPILERFEATURES_FORWARD(Args_09, arguments_09));
        this->d_finish.valuePtrIncrement();
    }
    else {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtBack(1);

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_finish.valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                         BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07),
                         BSLS_COMPILERFEATURES_FORWARD(Args_08, arguments_08),
                         BSLS_COMPILERFEATURES_FORWARD(Args_09, arguments_09));
        this->d_finish.nextBlock();
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01,
          class Args_02,
          class Args_03,
          class Args_04,
          class Args_05,
          class Args_06,
          class Args_07,
          class Args_08,
          class Args_09,
          class Args_10>
void deque<VALUE_TYPE, ALLOCATOR>::emplace_back(
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_07) arguments_07,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_08) arguments_08,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_09) arguments_09,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_10) arguments_10)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                              "deque<...>::emplace_back(args): deque too big");
    }

    if (BSLS_PERFORMANCEHINT_PREDICT_LIKELY(
                                      1 < this->d_finish.remainingInBlock())) {
        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_finish.valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                         BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07),
                         BSLS_COMPILERFEATURES_FORWARD(Args_08, arguments_08),
                         BSLS_COMPILERFEATURES_FORWARD(Args_09, arguments_09),
                         BSLS_COMPILERFEATURES_FORWARD(Args_10, arguments_10));
        this->d_finish.valuePtrIncrement();
    }
    else {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtBack(1);

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_finish.valuePtr(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                         BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07),
                         BSLS_COMPILERFEATURES_FORWARD(Args_08, arguments_08),
                         BSLS_COMPILERFEATURES_FORWARD(Args_09, arguments_09),
                         BSLS_COMPILERFEATURES_FORWARD(Args_10, arguments_10));
        this->d_finish.nextBlock();
    }
}

#else
// The generated code below is a workaround for the absence of perfect
// forwarding in some compilers.
template <class VALUE_TYPE, class ALLOCATOR>
template <class... Args>
void deque<VALUE_TYPE, ALLOCATOR>::emplace_front(
                           BSLS_COMPILERFEATURES_FORWARD_REF(Args)...arguments)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                             "deque<...>::emplace_front(args): deque too big");
    }

    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
                                         0 == this->d_start.offsetInBlock())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtFront(1);

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            (this->d_start - 1).valuePtr(),
                            BSLS_COMPILERFEATURES_FORWARD(Args, arguments)...);
        --this->d_start;
    }
    else {

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_start.valuePtr() - 1,
                            BSLS_COMPILERFEATURES_FORWARD(Args, arguments)...);
        this->d_start.valuePtrDecrement();
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class... Args>
void deque<VALUE_TYPE, ALLOCATOR>::emplace_back(
                           BSLS_COMPILERFEATURES_FORWARD_REF(Args)...arguments)
{
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(this->size() >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                              "deque<...>::emplace_back(args): deque too big");
    }

    if (BSLS_PERFORMANCEHINT_PREDICT_LIKELY(
                                      1 < this->d_finish.remainingInBlock())) {
        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_finish.valuePtr(),
                            BSLS_COMPILERFEATURES_FORWARD(Args, arguments)...);
        this->d_finish.valuePtrIncrement();
    }
    else {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BlockCreator newBlocks(this);
        newBlocks.insertAtBack(1);

        AllocatorTraits::construct(
                            ContainerBase::allocator(),
                            this->d_finish.valuePtr(),
                            BSLS_COMPILERFEATURES_FORWARD(Args, arguments)...);
        this->d_finish.nextBlock();
    }
}
// }}} END GENERATED CODE
#endif

#if !BSLS_COMPILERFEATURES_SIMULATE_CPP11_FEATURES
template <class VALUE_TYPE, class ALLOCATOR>
template <class... Args>
typename deque<VALUE_TYPE, ALLOCATOR>::iterator
deque<VALUE_TYPE, ALLOCATOR>::emplace(const_iterator position,
                                      Args&&...      arguments)
{
    BSLS_ASSERT(position >= this->cbegin());
    BSLS_ASSERT(position <= this->cend());

    if (position == this->cbegin()) {
        emplace_front(BSLS_COMPILERFEATURES_FORWARD(Args, arguments)...);
        return this->begin();                                         // RETURN
    }

    if (position == this->cend()) {
        emplace_back(BSLS_COMPILERFEATURES_FORWARD(Args, arguments)...);
        return iterator(this->d_finish - 1);                          // RETURN
    }

    // The test is placed here because 'emplace_front' and 'emplace_back' do
    // the same check.

    const size_type currentSize = this->size();
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(currentSize >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                   "deque<...>::emplace(args): deque too big");
    }

    iterator pos(position.imp());
    const size_type posIdx = position - this->cbegin();
    if (posIdx <= currentSize / 2) {
        BlockCreator newBlocks(this);
        if (this->d_start.remainingInBlock() == BLOCK_LENGTH) {
            newBlocks.insertAtFront(1);
        }

        BlockProctor proctor(this, true);
        DequePrimitives::emplaceAndMoveToFront(
                            &this->d_start,
                            this->d_start,
                            this->d_start + posIdx,
                            ContainerBase::allocator(),
                            BSLS_COMPILERFEATURES_FORWARD(Args, arguments)...);
        proctor.release();
    }
    else {
        BlockCreator newBlocks(this);
        if (this->d_finish.offsetInBlock() == BLOCK_LENGTH - 1) {
            newBlocks.insertAtBack(1);
        }

        BlockProctor proctor(this, false);
        DequePrimitives::emplaceAndMoveToBack(
                            &this->d_finish,
                            this->d_finish,
                            this->d_start + posIdx,
                            ContainerBase::allocator(),
                            BSLS_COMPILERFEATURES_FORWARD(Args, arguments)...);
        proctor.release();
    }
    return this->begin() + posIdx;
}
#elif BSLS_COMPILERFEATURES_SIMULATE_VARIADIC_TEMPLATES
// {{{ BEGIN GENERATED CODE
// The following section is automatically generated.  **DO NOT EDIT**
// Generator command line: sim_cpp11_features.pl bslstl_deque.h
template <class VALUE_TYPE, class ALLOCATOR>
typename deque<VALUE_TYPE, ALLOCATOR>::iterator
deque<VALUE_TYPE, ALLOCATOR>::emplace(const_iterator position)
{
    BSLS_ASSERT(position >= this->cbegin());
    BSLS_ASSERT(position <= this->cend());

    if (position == this->cbegin()) {
        emplace_front();
        return this->begin();
    }

    if (position == this->cend()) {
        emplace_back();
        return iterator(this->d_finish - 1);
    }


    const size_type currentSize = this->size();
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(currentSize >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                   "deque<...>::emplace(args): deque too big");
    }

    iterator pos(position.imp());
    const size_type posIdx = position - this->cbegin();
    if (posIdx <= currentSize / 2) {
        BlockCreator newBlocks(this);
        if (this->d_start.remainingInBlock() == BLOCK_LENGTH) {
            newBlocks.insertAtFront(1);
        }

        BlockProctor proctor(this, true);
        DequePrimitives::emplaceAndMoveToFront(
                            &this->d_start,
                            this->d_start,
                            this->d_start + posIdx,
                            ContainerBase::allocator());
        proctor.release();
    }
    else {
        BlockCreator newBlocks(this);
        if (this->d_finish.offsetInBlock() == BLOCK_LENGTH - 1) {
            newBlocks.insertAtBack(1);
        }

        BlockProctor proctor(this, false);
        DequePrimitives::emplaceAndMoveToBack(
                            &this->d_finish,
                            this->d_finish,
                            this->d_start + posIdx,
                            ContainerBase::allocator());
        proctor.release();
    }
    return this->begin() + posIdx;
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01>
typename deque<VALUE_TYPE, ALLOCATOR>::iterator
deque<VALUE_TYPE, ALLOCATOR>::emplace(const_iterator position,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01)
{
    BSLS_ASSERT(position >= this->cbegin());
    BSLS_ASSERT(position <= this->cend());

    if (position == this->cbegin()) {
        emplace_front(BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01));
        return this->begin();
    }

    if (position == this->cend()) {
        emplace_back(BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01));
        return iterator(this->d_finish - 1);
    }


    const size_type currentSize = this->size();
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(currentSize >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                   "deque<...>::emplace(args): deque too big");
    }

    iterator pos(position.imp());
    const size_type posIdx = position - this->cbegin();
    if (posIdx <= currentSize / 2) {
        BlockCreator newBlocks(this);
        if (this->d_start.remainingInBlock() == BLOCK_LENGTH) {
            newBlocks.insertAtFront(1);
        }

        BlockProctor proctor(this, true);
        DequePrimitives::emplaceAndMoveToFront(
                            &this->d_start,
                            this->d_start,
                            this->d_start + posIdx,
                            ContainerBase::allocator(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01));
        proctor.release();
    }
    else {
        BlockCreator newBlocks(this);
        if (this->d_finish.offsetInBlock() == BLOCK_LENGTH - 1) {
            newBlocks.insertAtBack(1);
        }

        BlockProctor proctor(this, false);
        DequePrimitives::emplaceAndMoveToBack(
                            &this->d_finish,
                            this->d_finish,
                            this->d_start + posIdx,
                            ContainerBase::allocator(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01));
        proctor.release();
    }
    return this->begin() + posIdx;
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01,
          class Args_02>
typename deque<VALUE_TYPE, ALLOCATOR>::iterator
deque<VALUE_TYPE, ALLOCATOR>::emplace(const_iterator position,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02)
{
    BSLS_ASSERT(position >= this->cbegin());
    BSLS_ASSERT(position <= this->cend());

    if (position == this->cbegin()) {
        emplace_front(BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                      BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02));
        return this->begin();
    }

    if (position == this->cend()) {
        emplace_back(BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                     BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02));
        return iterator(this->d_finish - 1);
    }


    const size_type currentSize = this->size();
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(currentSize >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                   "deque<...>::emplace(args): deque too big");
    }

    iterator pos(position.imp());
    const size_type posIdx = position - this->cbegin();
    if (posIdx <= currentSize / 2) {
        BlockCreator newBlocks(this);
        if (this->d_start.remainingInBlock() == BLOCK_LENGTH) {
            newBlocks.insertAtFront(1);
        }

        BlockProctor proctor(this, true);
        DequePrimitives::emplaceAndMoveToFront(
                            &this->d_start,
                            this->d_start,
                            this->d_start + posIdx,
                            ContainerBase::allocator(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02));
        proctor.release();
    }
    else {
        BlockCreator newBlocks(this);
        if (this->d_finish.offsetInBlock() == BLOCK_LENGTH - 1) {
            newBlocks.insertAtBack(1);
        }

        BlockProctor proctor(this, false);
        DequePrimitives::emplaceAndMoveToBack(
                            &this->d_finish,
                            this->d_finish,
                            this->d_start + posIdx,
                            ContainerBase::allocator(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02));
        proctor.release();
    }
    return this->begin() + posIdx;
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01,
          class Args_02,
          class Args_03>
typename deque<VALUE_TYPE, ALLOCATOR>::iterator
deque<VALUE_TYPE, ALLOCATOR>::emplace(const_iterator position,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03)
{
    BSLS_ASSERT(position >= this->cbegin());
    BSLS_ASSERT(position <= this->cend());

    if (position == this->cbegin()) {
        emplace_front(BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                      BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                      BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03));
        return this->begin();
    }

    if (position == this->cend()) {
        emplace_back(BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                     BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                     BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03));
        return iterator(this->d_finish - 1);
    }


    const size_type currentSize = this->size();
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(currentSize >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                   "deque<...>::emplace(args): deque too big");
    }

    iterator pos(position.imp());
    const size_type posIdx = position - this->cbegin();
    if (posIdx <= currentSize / 2) {
        BlockCreator newBlocks(this);
        if (this->d_start.remainingInBlock() == BLOCK_LENGTH) {
            newBlocks.insertAtFront(1);
        }

        BlockProctor proctor(this, true);
        DequePrimitives::emplaceAndMoveToFront(
                            &this->d_start,
                            this->d_start,
                            this->d_start + posIdx,
                            ContainerBase::allocator(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03));
        proctor.release();
    }
    else {
        BlockCreator newBlocks(this);
        if (this->d_finish.offsetInBlock() == BLOCK_LENGTH - 1) {
            newBlocks.insertAtBack(1);
        }

        BlockProctor proctor(this, false);
        DequePrimitives::emplaceAndMoveToBack(
                            &this->d_finish,
                            this->d_finish,
                            this->d_start + posIdx,
                            ContainerBase::allocator(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03));
        proctor.release();
    }
    return this->begin() + posIdx;
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01,
          class Args_02,
          class Args_03,
          class Args_04>
typename deque<VALUE_TYPE, ALLOCATOR>::iterator
deque<VALUE_TYPE, ALLOCATOR>::emplace(const_iterator position,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04)
{
    BSLS_ASSERT(position >= this->cbegin());
    BSLS_ASSERT(position <= this->cend());

    if (position == this->cbegin()) {
        emplace_front(BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                      BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                      BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                      BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04));
        return this->begin();
    }

    if (position == this->cend()) {
        emplace_back(BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                     BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                     BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                     BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04));
        return iterator(this->d_finish - 1);
    }


    const size_type currentSize = this->size();
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(currentSize >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                   "deque<...>::emplace(args): deque too big");
    }

    iterator pos(position.imp());
    const size_type posIdx = position - this->cbegin();
    if (posIdx <= currentSize / 2) {
        BlockCreator newBlocks(this);
        if (this->d_start.remainingInBlock() == BLOCK_LENGTH) {
            newBlocks.insertAtFront(1);
        }

        BlockProctor proctor(this, true);
        DequePrimitives::emplaceAndMoveToFront(
                            &this->d_start,
                            this->d_start,
                            this->d_start + posIdx,
                            ContainerBase::allocator(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04));
        proctor.release();
    }
    else {
        BlockCreator newBlocks(this);
        if (this->d_finish.offsetInBlock() == BLOCK_LENGTH - 1) {
            newBlocks.insertAtBack(1);
        }

        BlockProctor proctor(this, false);
        DequePrimitives::emplaceAndMoveToBack(
                            &this->d_finish,
                            this->d_finish,
                            this->d_start + posIdx,
                            ContainerBase::allocator(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04));
        proctor.release();
    }
    return this->begin() + posIdx;
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01,
          class Args_02,
          class Args_03,
          class Args_04,
          class Args_05>
typename deque<VALUE_TYPE, ALLOCATOR>::iterator
deque<VALUE_TYPE, ALLOCATOR>::emplace(const_iterator position,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05)
{
    BSLS_ASSERT(position >= this->cbegin());
    BSLS_ASSERT(position <= this->cend());

    if (position == this->cbegin()) {
        emplace_front(BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                      BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                      BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                      BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                      BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05));
        return this->begin();
    }

    if (position == this->cend()) {
        emplace_back(BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                     BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                     BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                     BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                     BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05));
        return iterator(this->d_finish - 1);
    }


    const size_type currentSize = this->size();
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(currentSize >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                   "deque<...>::emplace(args): deque too big");
    }

    iterator pos(position.imp());
    const size_type posIdx = position - this->cbegin();
    if (posIdx <= currentSize / 2) {
        BlockCreator newBlocks(this);
        if (this->d_start.remainingInBlock() == BLOCK_LENGTH) {
            newBlocks.insertAtFront(1);
        }

        BlockProctor proctor(this, true);
        DequePrimitives::emplaceAndMoveToFront(
                            &this->d_start,
                            this->d_start,
                            this->d_start + posIdx,
                            ContainerBase::allocator(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05));
        proctor.release();
    }
    else {
        BlockCreator newBlocks(this);
        if (this->d_finish.offsetInBlock() == BLOCK_LENGTH - 1) {
            newBlocks.insertAtBack(1);
        }

        BlockProctor proctor(this, false);
        DequePrimitives::emplaceAndMoveToBack(
                            &this->d_finish,
                            this->d_finish,
                            this->d_start + posIdx,
                            ContainerBase::allocator(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05));
        proctor.release();
    }
    return this->begin() + posIdx;
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01,
          class Args_02,
          class Args_03,
          class Args_04,
          class Args_05,
          class Args_06>
typename deque<VALUE_TYPE, ALLOCATOR>::iterator
deque<VALUE_TYPE, ALLOCATOR>::emplace(const_iterator position,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06)
{
    BSLS_ASSERT(position >= this->cbegin());
    BSLS_ASSERT(position <= this->cend());

    if (position == this->cbegin()) {
        emplace_front(BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                      BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                      BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                      BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                      BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                      BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06));
        return this->begin();
    }

    if (position == this->cend()) {
        emplace_back(BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                     BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                     BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                     BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                     BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                     BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06));
        return iterator(this->d_finish - 1);
    }


    const size_type currentSize = this->size();
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(currentSize >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                   "deque<...>::emplace(args): deque too big");
    }

    iterator pos(position.imp());
    const size_type posIdx = position - this->cbegin();
    if (posIdx <= currentSize / 2) {
        BlockCreator newBlocks(this);
        if (this->d_start.remainingInBlock() == BLOCK_LENGTH) {
            newBlocks.insertAtFront(1);
        }

        BlockProctor proctor(this, true);
        DequePrimitives::emplaceAndMoveToFront(
                            &this->d_start,
                            this->d_start,
                            this->d_start + posIdx,
                            ContainerBase::allocator(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06));
        proctor.release();
    }
    else {
        BlockCreator newBlocks(this);
        if (this->d_finish.offsetInBlock() == BLOCK_LENGTH - 1) {
            newBlocks.insertAtBack(1);
        }

        BlockProctor proctor(this, false);
        DequePrimitives::emplaceAndMoveToBack(
                            &this->d_finish,
                            this->d_finish,
                            this->d_start + posIdx,
                            ContainerBase::allocator(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06));
        proctor.release();
    }
    return this->begin() + posIdx;
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01,
          class Args_02,
          class Args_03,
          class Args_04,
          class Args_05,
          class Args_06,
          class Args_07>
typename deque<VALUE_TYPE, ALLOCATOR>::iterator
deque<VALUE_TYPE, ALLOCATOR>::emplace(const_iterator position,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_07) arguments_07)
{
    BSLS_ASSERT(position >= this->cbegin());
    BSLS_ASSERT(position <= this->cend());

    if (position == this->cbegin()) {
        emplace_front(BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                      BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                      BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                      BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                      BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                      BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                      BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07));
        return this->begin();
    }

    if (position == this->cend()) {
        emplace_back(BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                     BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                     BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                     BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                     BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                     BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                     BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07));
        return iterator(this->d_finish - 1);
    }


    const size_type currentSize = this->size();
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(currentSize >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                   "deque<...>::emplace(args): deque too big");
    }

    iterator pos(position.imp());
    const size_type posIdx = position - this->cbegin();
    if (posIdx <= currentSize / 2) {
        BlockCreator newBlocks(this);
        if (this->d_start.remainingInBlock() == BLOCK_LENGTH) {
            newBlocks.insertAtFront(1);
        }

        BlockProctor proctor(this, true);
        DequePrimitives::emplaceAndMoveToFront(
                            &this->d_start,
                            this->d_start,
                            this->d_start + posIdx,
                            ContainerBase::allocator(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                         BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07));
        proctor.release();
    }
    else {
        BlockCreator newBlocks(this);
        if (this->d_finish.offsetInBlock() == BLOCK_LENGTH - 1) {
            newBlocks.insertAtBack(1);
        }

        BlockProctor proctor(this, false);
        DequePrimitives::emplaceAndMoveToBack(
                            &this->d_finish,
                            this->d_finish,
                            this->d_start + posIdx,
                            ContainerBase::allocator(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                         BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07));
        proctor.release();
    }
    return this->begin() + posIdx;
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01,
          class Args_02,
          class Args_03,
          class Args_04,
          class Args_05,
          class Args_06,
          class Args_07,
          class Args_08>
typename deque<VALUE_TYPE, ALLOCATOR>::iterator
deque<VALUE_TYPE, ALLOCATOR>::emplace(const_iterator position,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_07) arguments_07,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_08) arguments_08)
{
    BSLS_ASSERT(position >= this->cbegin());
    BSLS_ASSERT(position <= this->cend());

    if (position == this->cbegin()) {
        emplace_front(BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                      BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                      BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                      BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                      BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                      BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                      BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07),
                      BSLS_COMPILERFEATURES_FORWARD(Args_08, arguments_08));
        return this->begin();
    }

    if (position == this->cend()) {
        emplace_back(BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                     BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                     BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                     BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                     BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                     BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                     BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07),
                     BSLS_COMPILERFEATURES_FORWARD(Args_08, arguments_08));
        return iterator(this->d_finish - 1);
    }


    const size_type currentSize = this->size();
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(currentSize >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                   "deque<...>::emplace(args): deque too big");
    }

    iterator pos(position.imp());
    const size_type posIdx = position - this->cbegin();
    if (posIdx <= currentSize / 2) {
        BlockCreator newBlocks(this);
        if (this->d_start.remainingInBlock() == BLOCK_LENGTH) {
            newBlocks.insertAtFront(1);
        }

        BlockProctor proctor(this, true);
        DequePrimitives::emplaceAndMoveToFront(
                            &this->d_start,
                            this->d_start,
                            this->d_start + posIdx,
                            ContainerBase::allocator(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                         BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07),
                         BSLS_COMPILERFEATURES_FORWARD(Args_08, arguments_08));
        proctor.release();
    }
    else {
        BlockCreator newBlocks(this);
        if (this->d_finish.offsetInBlock() == BLOCK_LENGTH - 1) {
            newBlocks.insertAtBack(1);
        }

        BlockProctor proctor(this, false);
        DequePrimitives::emplaceAndMoveToBack(
                            &this->d_finish,
                            this->d_finish,
                            this->d_start + posIdx,
                            ContainerBase::allocator(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                         BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07),
                         BSLS_COMPILERFEATURES_FORWARD(Args_08, arguments_08));
        proctor.release();
    }
    return this->begin() + posIdx;
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01,
          class Args_02,
          class Args_03,
          class Args_04,
          class Args_05,
          class Args_06,
          class Args_07,
          class Args_08,
          class Args_09>
typename deque<VALUE_TYPE, ALLOCATOR>::iterator
deque<VALUE_TYPE, ALLOCATOR>::emplace(const_iterator position,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_07) arguments_07,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_08) arguments_08,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_09) arguments_09)
{
    BSLS_ASSERT(position >= this->cbegin());
    BSLS_ASSERT(position <= this->cend());

    if (position == this->cbegin()) {
        emplace_front(BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                      BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                      BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                      BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                      BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                      BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                      BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07),
                      BSLS_COMPILERFEATURES_FORWARD(Args_08, arguments_08),
                      BSLS_COMPILERFEATURES_FORWARD(Args_09, arguments_09));
        return this->begin();
    }

    if (position == this->cend()) {
        emplace_back(BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                     BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                     BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                     BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                     BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                     BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                     BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07),
                     BSLS_COMPILERFEATURES_FORWARD(Args_08, arguments_08),
                     BSLS_COMPILERFEATURES_FORWARD(Args_09, arguments_09));
        return iterator(this->d_finish - 1);
    }


    const size_type currentSize = this->size();
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(currentSize >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                   "deque<...>::emplace(args): deque too big");
    }

    iterator pos(position.imp());
    const size_type posIdx = position - this->cbegin();
    if (posIdx <= currentSize / 2) {
        BlockCreator newBlocks(this);
        if (this->d_start.remainingInBlock() == BLOCK_LENGTH) {
            newBlocks.insertAtFront(1);
        }

        BlockProctor proctor(this, true);
        DequePrimitives::emplaceAndMoveToFront(
                            &this->d_start,
                            this->d_start,
                            this->d_start + posIdx,
                            ContainerBase::allocator(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                         BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07),
                         BSLS_COMPILERFEATURES_FORWARD(Args_08, arguments_08),
                         BSLS_COMPILERFEATURES_FORWARD(Args_09, arguments_09));
        proctor.release();
    }
    else {
        BlockCreator newBlocks(this);
        if (this->d_finish.offsetInBlock() == BLOCK_LENGTH - 1) {
            newBlocks.insertAtBack(1);
        }

        BlockProctor proctor(this, false);
        DequePrimitives::emplaceAndMoveToBack(
                            &this->d_finish,
                            this->d_finish,
                            this->d_start + posIdx,
                            ContainerBase::allocator(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                         BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07),
                         BSLS_COMPILERFEATURES_FORWARD(Args_08, arguments_08),
                         BSLS_COMPILERFEATURES_FORWARD(Args_09, arguments_09));
        proctor.release();
    }
    return this->begin() + posIdx;
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class Args_01,
          class Args_02,
          class Args_03,
          class Args_04,
          class Args_05,
          class Args_06,
          class Args_07,
          class Args_08,
          class Args_09,
          class Args_10>
typename deque<VALUE_TYPE, ALLOCATOR>::iterator
deque<VALUE_TYPE, ALLOCATOR>::emplace(const_iterator position,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_01) arguments_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_02) arguments_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_03) arguments_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_04) arguments_04,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_05) arguments_05,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_06) arguments_06,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_07) arguments_07,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_08) arguments_08,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_09) arguments_09,
                       BSLS_COMPILERFEATURES_FORWARD_REF(Args_10) arguments_10)
{
    BSLS_ASSERT(position >= this->cbegin());
    BSLS_ASSERT(position <= this->cend());

    if (position == this->cbegin()) {
        emplace_front(BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                      BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                      BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                      BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                      BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                      BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                      BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07),
                      BSLS_COMPILERFEATURES_FORWARD(Args_08, arguments_08),
                      BSLS_COMPILERFEATURES_FORWARD(Args_09, arguments_09),
                      BSLS_COMPILERFEATURES_FORWARD(Args_10, arguments_10));
        return this->begin();
    }

    if (position == this->cend()) {
        emplace_back(BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                     BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                     BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                     BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                     BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                     BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                     BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07),
                     BSLS_COMPILERFEATURES_FORWARD(Args_08, arguments_08),
                     BSLS_COMPILERFEATURES_FORWARD(Args_09, arguments_09),
                     BSLS_COMPILERFEATURES_FORWARD(Args_10, arguments_10));
        return iterator(this->d_finish - 1);
    }


    const size_type currentSize = this->size();
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(currentSize >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                   "deque<...>::emplace(args): deque too big");
    }

    iterator pos(position.imp());
    const size_type posIdx = position - this->cbegin();
    if (posIdx <= currentSize / 2) {
        BlockCreator newBlocks(this);
        if (this->d_start.remainingInBlock() == BLOCK_LENGTH) {
            newBlocks.insertAtFront(1);
        }

        BlockProctor proctor(this, true);
        DequePrimitives::emplaceAndMoveToFront(
                            &this->d_start,
                            this->d_start,
                            this->d_start + posIdx,
                            ContainerBase::allocator(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                         BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07),
                         BSLS_COMPILERFEATURES_FORWARD(Args_08, arguments_08),
                         BSLS_COMPILERFEATURES_FORWARD(Args_09, arguments_09),
                         BSLS_COMPILERFEATURES_FORWARD(Args_10, arguments_10));
        proctor.release();
    }
    else {
        BlockCreator newBlocks(this);
        if (this->d_finish.offsetInBlock() == BLOCK_LENGTH - 1) {
            newBlocks.insertAtBack(1);
        }

        BlockProctor proctor(this, false);
        DequePrimitives::emplaceAndMoveToBack(
                            &this->d_finish,
                            this->d_finish,
                            this->d_start + posIdx,
                            ContainerBase::allocator(),
                         BSLS_COMPILERFEATURES_FORWARD(Args_01, arguments_01),
                         BSLS_COMPILERFEATURES_FORWARD(Args_02, arguments_02),
                         BSLS_COMPILERFEATURES_FORWARD(Args_03, arguments_03),
                         BSLS_COMPILERFEATURES_FORWARD(Args_04, arguments_04),
                         BSLS_COMPILERFEATURES_FORWARD(Args_05, arguments_05),
                         BSLS_COMPILERFEATURES_FORWARD(Args_06, arguments_06),
                         BSLS_COMPILERFEATURES_FORWARD(Args_07, arguments_07),
                         BSLS_COMPILERFEATURES_FORWARD(Args_08, arguments_08),
                         BSLS_COMPILERFEATURES_FORWARD(Args_09, arguments_09),
                         BSLS_COMPILERFEATURES_FORWARD(Args_10, arguments_10));
        proctor.release();
    }
    return this->begin() + posIdx;
}

#else
// The generated code below is a workaround for the absence of perfect
// forwarding in some compilers.
template <class VALUE_TYPE, class ALLOCATOR>
template <class... Args>
typename deque<VALUE_TYPE, ALLOCATOR>::iterator
deque<VALUE_TYPE, ALLOCATOR>::emplace(const_iterator position,
                          BSLS_COMPILERFEATURES_FORWARD_REF(Args)... arguments)
{
    BSLS_ASSERT(position >= this->cbegin());
    BSLS_ASSERT(position <= this->cend());

    if (position == this->cbegin()) {
        emplace_front(BSLS_COMPILERFEATURES_FORWARD(Args, arguments)...);
        return this->begin();
    }

    if (position == this->cend()) {
        emplace_back(BSLS_COMPILERFEATURES_FORWARD(Args, arguments)...);
        return iterator(this->d_finish - 1);
    }


    const size_type currentSize = this->size();
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(currentSize >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                   "deque<...>::emplace(args): deque too big");
    }

    iterator pos(position.imp());
    const size_type posIdx = position - this->cbegin();
    if (posIdx <= currentSize / 2) {
        BlockCreator newBlocks(this);
        if (this->d_start.remainingInBlock() == BLOCK_LENGTH) {
            newBlocks.insertAtFront(1);
        }

        BlockProctor proctor(this, true);
        DequePrimitives::emplaceAndMoveToFront(
                            &this->d_start,
                            this->d_start,
                            this->d_start + posIdx,
                            ContainerBase::allocator(),
                            BSLS_COMPILERFEATURES_FORWARD(Args, arguments)...);
        proctor.release();
    }
    else {
        BlockCreator newBlocks(this);
        if (this->d_finish.offsetInBlock() == BLOCK_LENGTH - 1) {
            newBlocks.insertAtBack(1);
        }

        BlockProctor proctor(this, false);
        DequePrimitives::emplaceAndMoveToBack(
                            &this->d_finish,
                            this->d_finish,
                            this->d_start + posIdx,
                            ContainerBase::allocator(),
                            BSLS_COMPILERFEATURES_FORWARD(Args, arguments)...);
        proctor.release();
    }
    return this->begin() + posIdx;
}
// }}} END GENERATED CODE
#endif

template <class VALUE_TYPE, class ALLOCATOR>
void deque<VALUE_TYPE, ALLOCATOR>::pop_front()
{
    BSLS_ASSERT(!this->empty());

    BloombergLP::bslalg::ScalarDestructionPrimitives::destroy(
                                                     this->d_start.valuePtr());

    if (1 == this->d_start.remainingInBlock()) {
        this->deallocateN(*this->d_start.blockPtr(), 1);
        this->d_start.nextBlock();
        return;                                                       // RETURN
    }

    this->d_start.valuePtrIncrement();
}

template <class VALUE_TYPE, class ALLOCATOR>
void deque<VALUE_TYPE, ALLOCATOR>::pop_back()
{
    BSLS_ASSERT(!this->empty());

    if (0 == this->d_finish.offsetInBlock()) {
        --this->d_finish;
        BloombergLP::bslalg::ScalarDestructionPrimitives::destroy(
                                                    this->d_finish.valuePtr());
        this->deallocateN(this->d_finish.blockPtr()[1], 1);
        return;                                                       // RETURN
    }

    this->d_finish.valuePtrDecrement();
    BloombergLP::bslalg::ScalarDestructionPrimitives::destroy(
                                                    this->d_finish.valuePtr());
}

template <class VALUE_TYPE, class ALLOCATOR>
typename deque<VALUE_TYPE, ALLOCATOR>::iterator
deque<VALUE_TYPE, ALLOCATOR>::insert(const_iterator    position,
                                     const VALUE_TYPE& value)
{
    BSLS_ASSERT(position >= this->cbegin());
    BSLS_ASSERT(position <= this->cend());

    if (position == this->cbegin()) {
        push_front(value);
        return this->begin();                                         // RETURN
    }

    if (position == this->cend()) {
        push_back(value);
        return iterator(this->d_finish - 1);                          // RETURN
    }

    // The test is placed here because 'push_front' and 'push_back' do the same
    // check.

    const size_type currentSize = this->size();
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(currentSize >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                 "deque<...>::insert(pos,n,v): deque too big");
    }

    iterator pos(position.imp());
    const size_type posIdx = position - this->cbegin();
    if (posIdx <= currentSize / 2) {
        BlockCreator newBlocks(this);
        if (this->d_start.remainingInBlock() == BLOCK_LENGTH) {
            newBlocks.insertAtFront(1);
        }
        DequePrimitives::insertAndMoveToFront(&this->d_start,
                                              this->d_start,
                                              this->d_start + posIdx,
                                              1,
                                              value,
                                              ContainerBase::allocator());
    }
    else {
        BlockCreator newBlocks(this);
        if (this->d_finish.offsetInBlock() == BLOCK_LENGTH - 1) {
            newBlocks.insertAtBack(1);
        }
        DequePrimitives::insertAndMoveToBack(&this->d_finish,
                                             this->d_finish,
                                             this->d_start + posIdx,
                                             1,
                                             value,
                                             ContainerBase::allocator());
    }
    return this->begin() + posIdx;
}

template <class VALUE_TYPE, class ALLOCATOR>
typename deque<VALUE_TYPE, ALLOCATOR>::iterator
deque<VALUE_TYPE, ALLOCATOR>::insert(
                           const_iterator                             position,
                           BloombergLP::bslmf::MovableRef<VALUE_TYPE> value)
{
    BSLS_ASSERT(position >= this->cbegin());
    BSLS_ASSERT(position <= this->cend());

    VALUE_TYPE& lvalue = value;

    if (position == this->cbegin()) {
        push_front(MoveUtil::move(lvalue));
        return this->begin();                                         // RETURN
    }

    if (position == this->cend()) {
        push_back(MoveUtil::move(lvalue));
        return iterator(this->d_finish - 1);                          // RETURN
    }

    // The test is placed here because 'push_front' and 'push_back' do the same
    // check.

    const size_type currentSize = this->size();
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(currentSize >= max_size())) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                 "deque<...>::insert(pos,n,v): deque too big");
    }

    iterator pos(position.imp());
    const size_type posIdx = position - this->cbegin();
    if (posIdx <= currentSize / 2) {
        BlockCreator newBlocks(this);
        if (this->d_start.remainingInBlock() == BLOCK_LENGTH) {
            newBlocks.insertAtFront(1);
        }
        DequePrimitives::moveInsertAndMoveToFront(&this->d_start,
                                                  this->d_start,
                                                  this->d_start + posIdx,
                                                  MoveUtil::move(lvalue),
                                                  ContainerBase::allocator());
    }
    else {
        BlockCreator newBlocks(this);
        if (this->d_finish.offsetInBlock() == BLOCK_LENGTH - 1) {
            newBlocks.insertAtBack(1);
        }
        DequePrimitives::moveInsertAndMoveToBack(&this->d_finish,
                                                 this->d_finish,
                                                 this->d_start + posIdx,
                                                 MoveUtil::move(lvalue),
                                                 ContainerBase::allocator());
    }
    return this->begin() + posIdx;
}

template <class VALUE_TYPE, class ALLOCATOR>
typename deque<VALUE_TYPE, ALLOCATOR>::iterator
deque<VALUE_TYPE, ALLOCATOR>::insert(const_iterator    position,
                                     size_type         numElements,
                                     const VALUE_TYPE& value)
{
    BSLS_ASSERT(position >= this->cbegin());
    BSLS_ASSERT(position <= this->cend());

    const size_type posIdx = position - this->cbegin();

    if (0 == numElements) {
        return this->begin() + posIdx;                                // RETURN
    }

    const size_type currentSize = this->size();
    if (BSLS_PERFORMANCEHINT_PREDICT_UNLIKELY(
                                     numElements > max_size() - currentSize)) {
        BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

        BloombergLP::bslstl::StdExceptUtil::throwLengthError(
                                 "deque<...>::insert(pos,n,v): deque too big");
    }

    if (position == this->cbegin()) {
        privatePrependRaw(numElements, value);

        return this->begin();                                         // RETURN
    }

    if (position == this->cend()) {
        privateAppendRaw(numElements, value);

        return this->begin() + posIdx;                                // RETURN
    }

    if (posIdx <= currentSize / 2) {
        // Create new blocks at front.  In case an exception is thrown, any
        // unused blocks are returned to the allocator.

        size_type numNewBlocks = (this->d_start.remainingInBlock()
                                             + numElements - 1) / BLOCK_LENGTH;
        BlockCreator newBlocks(this);
        newBlocks.insertAtFront(numNewBlocks);

        DequePrimitives::insertAndMoveToFront(&this->d_start,
                                              this->d_start,
                                              this->d_start + posIdx,
                                              numElements,
                                              value,
                                              ContainerBase::allocator());
    }
    else {
        // Create new blocks at back.  In case an exception is thrown, any
        // unused blocks are returned to the allocator.

        size_type numNewBlocks = (this->d_finish.offsetInBlock() + numElements)
                                                                / BLOCK_LENGTH;
        BlockCreator newBlocks(this);
        newBlocks.insertAtBack(numNewBlocks);

        DequePrimitives::insertAndMoveToBack(&this->d_finish,
                                             this->d_finish,
                                             this->d_start + posIdx,
                                             numElements,
                                             value,
                                             ContainerBase::allocator());
    }

    return this->begin() + posIdx;
}

template <class VALUE_TYPE, class ALLOCATOR>
template <class INPUT_ITERATOR>
inline
typename deque<VALUE_TYPE, ALLOCATOR>::iterator
deque<VALUE_TYPE, ALLOCATOR>::insert(const_iterator position,
                                     INPUT_ITERATOR first,
                                     INPUT_ITERATOR last)
{
    BSLS_ASSERT_SAFE(position >= this->cbegin());
    BSLS_ASSERT_SAFE(position <= this->cend());

    const size_type posIdx = position - this->cbegin();

    privateInsertDispatch(position,
                          first,
                          last,
                          first,
                          BloombergLP::bslmf::Nil());

    return this->begin() + posIdx;
}

#if defined(BSLS_COMPILERFEATURES_SUPPORT_GENERALIZED_INITIALIZERS)
template <class VALUE_TYPE, class ALLOCATOR>
inline
typename deque<VALUE_TYPE, ALLOCATOR>::iterator
deque<VALUE_TYPE, ALLOCATOR>::insert(
                                    const_iterator                    position,
                                    std::initializer_list<value_type> values)
{
    BSLS_ASSERT_SAFE(position >= this->cbegin());
    BSLS_ASSERT_SAFE(position <= this->cend());

    return insert(position, values.begin(), values.end());
}
#endif

template <class VALUE_TYPE, class ALLOCATOR>
typename deque<VALUE_TYPE, ALLOCATOR>::iterator
deque<VALUE_TYPE, ALLOCATOR>::erase(const_iterator position)
{
    BSLS_ASSERT(position >= this->cbegin());
    BSLS_ASSERT(position <  this->cend());

    if (position == const_iterator(this->d_start)) {
        pop_front();
        return this->begin();                                         // RETURN
    }

    if (position + 1 == const_iterator(this->d_finish)) {
        pop_back();
        return this->end();                                           // RETURN
    }

    return erase(position, position + 1);
}

template <class VALUE_TYPE, class ALLOCATOR>
typename deque<VALUE_TYPE, ALLOCATOR>::iterator
deque<VALUE_TYPE, ALLOCATOR>::erase(const_iterator first, const_iterator last)
{
    BSLS_ASSERT(first >= this->cbegin());
    BSLS_ASSERT(first <= this->cend());
    BSLS_ASSERT(first <= last);
    BSLS_ASSERT(last  <= this->cend());

    iterator first_imp = this->begin() + (first - this->cbegin());
    iterator last_imp  = this->begin() + (last  - this->cbegin());
    iterator oldStart  = this->begin();
    iterator oldFinish = this->end();
    iterator result = iterator(DequePrimitives::erase(
                                                  &this->d_start,
                                                  &this->d_finish,
                                                  this->d_start,
                                                  first_imp.imp(),
                                                  last_imp.imp(),
                                                  this->d_finish,
                                                  ContainerBase::allocator()));

    // Deallocate blocks no longer used.

    for ( ; oldStart.imp().blockPtr() != this->d_start.blockPtr();
                                                  oldStart.imp().nextBlock()) {
        this->deallocateN(oldStart.imp().blockPtr()[0], 1);
    }
    for ( ; oldFinish.imp().blockPtr() != this->d_finish.blockPtr();
                                             oldFinish.imp().previousBlock()) {
        this->deallocateN(oldFinish.imp().blockPtr()[0], 1);
    }
    return result;
}

template <class VALUE_TYPE, class ALLOCATOR>
void deque<VALUE_TYPE, ALLOCATOR>::swap(deque<VALUE_TYPE, ALLOCATOR>& other)
              BSLS_CPP11_NOEXCEPT_SPECIFICATION(BSLS_CPP11_PROVISIONALLY_FALSE)
{
    if (AllocatorTraits::propagate_on_container_swap::value) {
        Deque_Util::swap(static_cast<Base *>(this),
                         static_cast<Base *>(&other));
        BloombergLP::bslalg::SwapUtil::swap(&ContainerBase::allocator(),
                                            &other.ContainerBase::allocator());
    }
    else {
        if (BSLS_PERFORMANCEHINT_PREDICT_LIKELY(
                             this->get_allocator() == other.get_allocator())) {
            Deque_Util::swap(static_cast<Base *>(this),
                             static_cast<Base *>(&other));
        }
        else {
            BSLS_PERFORMANCEHINT_UNLIKELY_HINT;

            deque toOtherCopy(MoveUtil::move(*this), other.get_allocator());
            deque toThisCopy( MoveUtil::move(other), this->get_allocator());

            Deque_Util::swap(static_cast<Base *>(&toThisCopy),
                             static_cast<Base *>(this));
            Deque_Util::swap(static_cast<Base *>(&toOtherCopy),
                             static_cast<Base *>(&other));
        }
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
void deque<VALUE_TYPE, ALLOCATOR>::clear() BSLS_CPP11_NOEXCEPT
{
    DequePrimitives::destruct(this->d_start,
                              this->d_finish,
                              ContainerBase::allocator());

    // Deallocate all blocks except 'finishBlock'.

    BlockPtr *startBlock  = this->d_start.blockPtr();
    BlockPtr *finishBlock = this->d_finish.blockPtr();
    for ( ; startBlock != finishBlock; ++startBlock) {
        this->deallocateN(*startBlock, 1);
    }

    // Reposition in the middle.

    size_type  blockOffset = this->d_blocksLength / 2;
    int        offset      = BLOCK_LENGTH / 2;
    BlockPtr  *blockPtr    = this->d_blocks_p + blockOffset;

    *blockPtr = *finishBlock;

    this->d_start = this->d_finish = IteratorImp(blockPtr,
                                                 (*blockPtr)->d_data + offset);
}

// ACCESSORS
template <class VALUE_TYPE, class ALLOCATOR>
inline
typename deque<VALUE_TYPE, ALLOCATOR>::allocator_type
deque<VALUE_TYPE, ALLOCATOR>::get_allocator() const BSLS_CPP11_NOEXCEPT
{
    return ContainerBase::allocator();
}

template <class VALUE_TYPE, class ALLOCATOR>
inline
typename deque<VALUE_TYPE, ALLOCATOR>::size_type
deque<VALUE_TYPE, ALLOCATOR>::max_size() const BSLS_CPP11_NOEXCEPT
{
    return this->get_allocator().max_size();
}

// FREE OPERATORS
template <class VALUE_TYPE, class ALLOCATOR>
bool operator==(const deque<VALUE_TYPE, ALLOCATOR>& lhs,
                const deque<VALUE_TYPE, ALLOCATOR>& rhs)
{
    if (lhs.size() != rhs.size()) {
        return false;                                                 // RETURN
    }

    enum {
        BLOCK_LENGTH = Deque_BlockLengthCalcUtil<VALUE_TYPE>::BLOCK_LENGTH
    };

    typedef BloombergLP::bslalg::DequeIterator<VALUE_TYPE,
                                               BLOCK_LENGTH> Iterator;

    Iterator lhsBegin = lhs.begin().imp();
    Iterator lhsEnd   = lhs.end().imp();
    Iterator rhsBegin = rhs.begin().imp();

    for (; !(lhsBegin == lhsEnd); ++lhsBegin, ++rhsBegin) {
        if (!(*lhsBegin == *rhsBegin)) {
            return false;                                             // RETURN
        }
    }
    return true;
}

template <class VALUE_TYPE, class ALLOCATOR>
inline
bool operator!=(const deque<VALUE_TYPE, ALLOCATOR>& lhs,
                const deque<VALUE_TYPE, ALLOCATOR>& rhs)
{
    return !(lhs == rhs);
}

template <class VALUE_TYPE, class ALLOCATOR>
inline
bool operator<(const deque<VALUE_TYPE, ALLOCATOR>& lhs,
               const deque<VALUE_TYPE, ALLOCATOR>& rhs)
{
    return 0 > BloombergLP::bslalg::RangeCompare::lexicographical(lhs.begin(),
                                                                  lhs.end(),
                                                                  lhs.size(),
                                                                  rhs.begin(),
                                                                  rhs.end(),
                                                                  rhs.size());
}

template <class VALUE_TYPE, class ALLOCATOR>
inline
bool operator>(const deque<VALUE_TYPE, ALLOCATOR>& lhs,
               const deque<VALUE_TYPE, ALLOCATOR>& rhs)
{
    return rhs < lhs;
}

template <class VALUE_TYPE, class ALLOCATOR>
inline
bool operator<=(const deque<VALUE_TYPE, ALLOCATOR>& lhs,
                const deque<VALUE_TYPE, ALLOCATOR>& rhs)
{
    return !(rhs < lhs);
}

template <class VALUE_TYPE, class ALLOCATOR>
inline
bool operator>=(const deque<VALUE_TYPE, ALLOCATOR>& lhs,
                const deque<VALUE_TYPE, ALLOCATOR>& rhs)
{
    return !(lhs < rhs);
}

// FREE FUNCTIONS
template <class VALUE_TYPE, class ALLOCATOR>
inline
void swap(deque<VALUE_TYPE, ALLOCATOR>& a, deque<VALUE_TYPE, ALLOCATOR>& b)
              BSLS_CPP11_NOEXCEPT_SPECIFICATION(BSLS_CPP11_PROVISIONALLY_FALSE)
{
    a.swap(b);
}

                      // ------------------------
                      // class Deque_BlockCreator
                      // ------------------------

// CREATORS
template <class VALUE_TYPE, class ALLOCATOR>
inline
Deque_BlockCreator<VALUE_TYPE, ALLOCATOR>::Deque_BlockCreator(
                                           deque<VALUE_TYPE, ALLOCATOR> *deque)
: d_deque_p(deque)
, d_boundary_p(0)
{
}

template <class VALUE_TYPE, class ALLOCATOR>
Deque_BlockCreator<VALUE_TYPE, ALLOCATOR>::~Deque_BlockCreator()
{
    if (0 != d_boundary_p) {
        BlockPtr *delFirst, *delLast;
        if (d_boundary_p <= d_deque_p->d_start.blockPtr()) {
            delFirst = d_boundary_p;
            delLast  = d_deque_p->d_start.blockPtr();
        }
        else {
            delFirst = d_deque_p->d_finish.blockPtr() + 1;
            delLast  = d_boundary_p;
        }

        for (; delFirst != delLast; ++delFirst) {
            // Deallocate the block that '*delFirst' points to.

            d_deque_p->deallocateN(*delFirst, 1);
        }
    }
}

// MANIPULATORS
template <class VALUE_TYPE, class ALLOCATOR>
void Deque_BlockCreator<VALUE_TYPE, ALLOCATOR>::insertAtFront(size_type n)
{
    d_boundary_p = reserveBlockSlots(n, true);
    for ( ; n > 0; --n) {
        d_boundary_p[-1] = d_deque_p->allocateN((Block *)0, 1);
        --d_boundary_p;
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
void Deque_BlockCreator<VALUE_TYPE, ALLOCATOR>::insertAtBack(size_type n)
{
    d_boundary_p = reserveBlockSlots(n, false);
    for ( ; n > 0; --n) {
        *d_boundary_p = d_deque_p->allocateN((Block *)0, 1);
        ++d_boundary_p;
    }
}

template <class VALUE_TYPE, class ALLOCATOR>
typename Deque_BlockCreator<VALUE_TYPE, ALLOCATOR>::BlockPtr *
Deque_BlockCreator<VALUE_TYPE, ALLOCATOR>::reserveBlockSlots(
                                                        size_type numNewBlocks,
                                                        bool      atFront)
{
    BlockPtr  *blocks       = d_deque_p->d_blocks_p;
    size_type  blocksLength = d_deque_p->d_blocksLength;

    BlockPtr *firstSlot = d_deque_p->d_start.blockPtr();
    BlockPtr *lastSlot  = d_deque_p->d_finish.blockPtr() + 1;

    if (atFront) {
        if (d_boundary_p) {
            firstSlot = d_boundary_p;
        }
        if (size_type(firstSlot - blocks) >= numNewBlocks) {
            // Enough room to insert at the front.

            return firstSlot;                                         // RETURN
        }
    }
    else {
        if (d_boundary_p) {
            lastSlot = d_boundary_p;
        }
        if (size_type(blocks + blocksLength - lastSlot) >= numNewBlocks) {
            // Enough room to insert at the back.

            return lastSlot;                                          // RETURN
        }
    }

    BlockPtr  *newBlocks          = blocks;
    size_type  newBlocksLength    = blocksLength;
    size_type  numUsedBlocks      = lastSlot - firstSlot;
    size_type  blockOffsetStart   = d_deque_p->d_start.blockPtr() - firstSlot;
    size_type  numCommittedBlocks = (d_deque_p->d_finish.blockPtr() -
                                            d_deque_p->d_start.blockPtr() + 1);
    size_type  newNumUsedBlocks   = numUsedBlocks + numNewBlocks;

    if (newNumUsedBlocks > blocksLength) {
        const size_type newThreshold = newNumUsedBlocks +
                                                  2 * Imp::BLOCK_ARRAY_PADDING;
        while (newThreshold > newBlocksLength) {
            // Insufficient room.  Allocate new blocks array with geometric
            // growth.  Note that this should never overflow because there are
            // at least 16 elements in each block, thus the requested block
            // array pointer will never be close to 'max_size() / 2'.

            newBlocksLength *= 2;
        }
        newBlocks = d_deque_p->allocateN((BlockPtr *)0, newBlocksLength);
    }

    // Center block pointers within new blocks array.

    BlockPtr *newFirstSlot = newBlocks +
                                      (newBlocksLength - newNumUsedBlocks) / 2;

    if (atFront) {
        newFirstSlot += numNewBlocks;
    }

    // Calculate offset for start and finish.  Need to do this before moving
    // around blocks.

    const size_type offsetStart  = d_deque_p->d_start.offsetInBlock();
    const size_type offsetFinish = d_deque_p->d_finish.offsetInBlock();

    // Move old block pointers into new position.

    std::memmove(newFirstSlot, firstSlot, numUsedBlocks * sizeof(BlockPtr));

    if (newBlocks != blocks) {
        // Deallocate old blocks array and install the new one.

        if (blocks) {
            d_deque_p->deallocateN(blocks, d_deque_p->d_blocksLength);
        }
        d_deque_p->d_blocks_p     = newBlocks;
        d_deque_p->d_blocksLength = newBlocksLength;
    }

    // Adjust start and finish iterators.

    d_deque_p->d_start.setBlock(newFirstSlot + blockOffsetStart);
    d_deque_p->d_start += offsetStart;
    d_deque_p->d_finish.setBlock(newFirstSlot + blockOffsetStart +
                                                       numCommittedBlocks - 1);
    d_deque_p->d_finish += offsetFinish;

    BlockPtr *ret = newFirstSlot;
    if (!atFront) {
        ret += numUsedBlocks;
    }

    return ret;
}

template <class VALUE_TYPE, class ALLOCATOR>
inline
void Deque_BlockCreator<VALUE_TYPE, ALLOCATOR>::release()
{
    d_boundary_p = 0;
}

                      // ------------------------
                      // class Deque_BlockProctor
                      // ------------------------

// CREATORS
template <class VALUE_TYPE, class ALLOCATOR>
Deque_BlockProctor<VALUE_TYPE, ALLOCATOR>::Deque_BlockProctor(
                                         deque<VALUE_TYPE, ALLOCATOR> *deque,
                                         bool                          atFront)
: d_deque_p(deque)
, d_boundary_p(atFront
               ? d_deque_p->d_start.blockPtr()
               : d_deque_p->d_finish.blockPtr())
, d_atFront(atFront)
{
}

template <class VALUE_TYPE, class ALLOCATOR>
Deque_BlockProctor<VALUE_TYPE, ALLOCATOR>::~Deque_BlockProctor()
{
    if (0 != d_deque_p) {
        BlockPtr *delFirst, *delLast;

        if (d_atFront && d_boundary_p < d_deque_p->d_start.blockPtr()) {
            // Blocks at the front of the deque have been emptied since this
            // proctor was created.

            delFirst = d_boundary_p;
            delLast  = d_deque_p->d_start.blockPtr();
        }
        else if (!d_atFront && d_boundary_p > d_deque_p->d_finish.blockPtr()) {
            // Blocks at the back of the deque have been emptied since this
            // proctor was created.

            delFirst = d_deque_p->d_finish.blockPtr() + 1;
            delLast  = d_boundary_p + 1;
        }
        else {
            return;                                                   // RETURN
        }

        for (; delFirst != delLast; ++delFirst) {
            // Deallocate the block that '*delFirst' points to.

            d_deque_p->deallocateN(*delFirst, 1);
        }
    }
}

// MANIPULATORS
template <class VALUE_TYPE, class ALLOCATOR>
inline
void Deque_BlockProctor<VALUE_TYPE, ALLOCATOR>::release()
{
    d_deque_p = 0;
}

                        // ----------------------
                        // class Deque_ClearGuard
                        // ----------------------

// CREATORS
template <class VALUE_TYPE, class ALLOCATOR>
inline
Deque_ClearGuard<VALUE_TYPE, ALLOCATOR>::Deque_ClearGuard(
                                           deque<VALUE_TYPE, ALLOCATOR> *deque)
: d_deque_p(deque)
{
}

template <class VALUE_TYPE, class ALLOCATOR>
inline
Deque_ClearGuard<VALUE_TYPE, ALLOCATOR>::~Deque_ClearGuard()
{
    if (d_deque_p) {
        d_deque_p->clear();
    }
}

// MANIPULATORS
template <class VALUE_TYPE, class ALLOCATOR>
inline
void Deque_ClearGuard<VALUE_TYPE, ALLOCATOR>::release()
{
    d_deque_p = 0;
}

                        // -----------------
                        // class Deque_Guard
                        // -----------------

// CREATORS
template <class VALUE_TYPE, class ALLOCATOR>
inline
Deque_Guard<VALUE_TYPE, ALLOCATOR>::Deque_Guard(
                                          deque<VALUE_TYPE, ALLOCATOR> *deque,
                                          bool                          isTail)
: d_deque_p(deque)
, d_count(0)
, d_isTail(isTail)
{
}

template <class VALUE_TYPE, class ALLOCATOR>
Deque_Guard<VALUE_TYPE, ALLOCATOR>::~Deque_Guard()
{
    if (0 == d_count) {
        return;                                                       // RETURN
    }

    IteratorImp begin, end;

    if (d_isTail) {
        begin = d_deque_p->d_finish;
        end   = begin + d_count;
    }
    else {
        end   = d_deque_p->d_start;
        begin = end - d_count;
    }

    DequePrimitives::destruct(begin, end, d_deque_p->allocator());
}

// MANIPULATORS
template <class VALUE_TYPE, class ALLOCATOR>
inline
std::size_t Deque_Guard<VALUE_TYPE, ALLOCATOR>::operator++()
{
    return ++d_count;
}

template <class VALUE_TYPE, class ALLOCATOR>
inline
std::size_t Deque_Guard<VALUE_TYPE, ALLOCATOR>::operator--()
{
    return --d_count;
}

template <class VALUE_TYPE, class ALLOCATOR>
inline
void Deque_Guard<VALUE_TYPE, ALLOCATOR>::release()
{
    d_count = 0;
}

// ACCESSORS
template <class VALUE_TYPE, class ALLOCATOR>
inline
std::size_t
Deque_Guard<VALUE_TYPE, ALLOCATOR>::count() const BSLS_CPP11_NOEXCEPT
{
    return d_count;
}

template <class VALUE_TYPE, class ALLOCATOR>
inline
typename Deque_Guard<VALUE_TYPE, ALLOCATOR>::IteratorImp
Deque_Guard<VALUE_TYPE, ALLOCATOR>::begin() const BSLS_CPP11_NOEXCEPT
{
    return d_deque_p->d_start - d_count;
}

template <class VALUE_TYPE, class ALLOCATOR>
inline
typename Deque_Guard<VALUE_TYPE, ALLOCATOR>::IteratorImp
Deque_Guard<VALUE_TYPE, ALLOCATOR>::end() const BSLS_CPP11_NOEXCEPT
{
    return d_deque_p->d_finish + d_count;
}

}  // close namespace bsl

// ============================================================================
//                                TYPE TRAITS
// ============================================================================

// Type traits for STL *sequence* containers:
//: o A sequence container defines STL iterators.
//: o A sequence container is bitwise-movable if its allocator is
//:   bitwise-movable.
//: o A sequence container uses 'bslma' allocators if the (template parameter)
//:   type 'ALLOCATOR' is convertible from 'bslma::Allocator *'.

namespace BloombergLP {

namespace bslalg {

template <class VALUE_TYPE, class ALLOCATOR>
struct HasStlIterators<bsl::deque<VALUE_TYPE, ALLOCATOR> > : bsl::true_type
{
};

}  // close namespace bslalg

namespace bslmf {

template <class VALUE_TYPE, class ALLOCATOR>
struct IsBitwiseMoveable<bsl::deque<VALUE_TYPE, ALLOCATOR> >
    : IsBitwiseMoveable<ALLOCATOR>
{
};

}  // close namespace bslmf

namespace bslma {

template <class VALUE_TYPE, class ALLOCATOR>
struct UsesBslmaAllocator<bsl::deque<VALUE_TYPE, ALLOCATOR> >
    : bsl::is_convertible<Allocator *, ALLOCATOR>
{
};

}  // close namespace bslma

}  // close enterprise namespace

#endif

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
