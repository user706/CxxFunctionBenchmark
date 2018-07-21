// bslmf_isconvertibletoany.h                                         -*-C++-*-
#ifndef INCLUDED_BSLMF_ISCONVERTIBLETOANY
#define INCLUDED_BSLMF_ISCONVERTIBLETOANY

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide a compile-time check for types convertible to any type.
//
//@CLASSES:
//  bslmf::IsConvertibleToAny: meta-function for detecting conversion
//
//@SEE_ALSO: bslmf_isconvertible, bslmf_integralconstant
//
//@DESCRIPTION: This component provides a meta-function,
// 'bslmf::IsConvertibleToAny', that may be used to query (at compile-time)
// whether a type has a templatized conversion operator allowing it to be
// converted to any other type.
//
///Usage
///-----
// In this section we show the intended use of this component.
//
///Example 1: Determine If a Class Has a Template Conversion Operator
/// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Suppose that we want to assert whether a particular type has a template
// conversion operator.
//
// First, we define a type with the template conversion operator:
//..
//  struct TypeWithTemplateConversion {
//      template <class TYPE>
//      operator TYPE() {
//          return TYPE();
//      }
//  };
//..
// Now, we instantiate the 'bslmf::IsConvertibleToAny' template for
// 'TypeWithTemplateConversion' and assert the 'value' of the instantiation:
//..
//  assert(bslmf::IsConvertibleToAny<TypeWithTemplateConversion>::value);
//..

#ifndef INCLUDED_BSLSCM_VERSION
#include <bslscm_version.h>
#endif

#ifndef INCLUDED_BSLMF_INTEGRALCONSTANT
#include <bslmf_integralconstant.h>
#endif

#ifndef INCLUDED_BSLMF_ISCONVERTIBLE
#include <bslmf_isconvertible.h>
#endif

namespace BloombergLP {

namespace bslmf {

                          // =========================
                          // struct IsConvertibleToAny
                          // =========================

template <class TYPE>
class IsConvertibleToAny_Imp {
    // This class template implements a component-private meta-function to
    // determine if the (template parameter) 'TYPE' is convertible to any other
    // type.

    struct UniqueType {
        // A type convertible to this private type must have a template
        // conversion operator.
    };

  public:
    typedef typename bsl::is_convertible<TYPE, UniqueType>::type Type;
        // 'Type' is defined as 'bsl::true_type' if 'TYPE' is convertible to
        // 'UniqueType' and 'bsl::false_type' otherwise.
};

template <class TYPE>
struct IsConvertibleToAny : IsConvertibleToAny_Imp<TYPE>::Type {
    // This 'struct' template implements a meta-function to determine if the
    // (template parameter) 'TYPE' is convertible to any other type.  This
    // 'struct' derives from 'bsl::true_type' if 'TYPE' is convertible to any
    // type, and 'bsl::false_type' otherwise.
};

}  // close package namespace

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
