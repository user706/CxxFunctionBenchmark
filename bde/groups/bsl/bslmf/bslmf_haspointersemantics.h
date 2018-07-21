// bslmf_haspointersemantics.h                                        -*-C++-*-
#ifndef INCLUDED_BSLMF_HASPOINTERSEMANTICS
#define INCLUDED_BSLMF_HASPOINTERSEMANTICS

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id: $")

//@PURPOSE: Provide a type trait for pointer semantics.
//
//@CLASSES:
//  bslmf::HasPointerSemantics: meta-function indicating pointer-like types
//
//@SEE_ALSO:
//  bslmf_nestedtraitdeclaration
//
//@DESCRIPTION: This component defines a tag type 'HasPointerSemantics' derived
// from 'bsl::true_type' to indicate that a type has pointer semantics, and
// from 'bsl::false_type' otherwise.  A type has pointer-like semantics must
// define (at a minimum) 'operator*' and 'operator->'.  All pointer types have
// pointer semantics; other types must explicitly add this trait, either
// with the macro 'BSLMF_NESTED_TRAIT_DECLARATION', or by explicit template
// specialization.

#ifndef INCLUDED_BSLSCM_VERSION
#include <bslscm_version.h>
#endif

#ifndef INCLUDED_BSLMF_DETECTNESTEDTRAIT
#include <bslmf_detectnestedtrait.h>
#endif

#ifndef INCLUDED_BSLMF_INTEGRALCONSTANT
#include <bslmf_integralconstant.h>
#endif

#ifndef INCLUDED_BSLMF_ISPOINTER
#include <bslmf_ispointer.h>
#endif

namespace BloombergLP {
namespace bslmf {

template <class TYPE>
struct HasPointerSemantics
    : bsl::integral_constant<bool,
                             bsl::is_pointer<TYPE>::value
                             || DetectNestedTrait<TYPE,
                                                  HasPointerSemantics>::value>
{};

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
