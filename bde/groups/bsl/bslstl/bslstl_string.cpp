// bslstl_string.cpp                                                  -*-C++-*-
#include <bslstl_string.h>

#include <bsls_ident.h>
BSLS_IDENT("$Id$ $CSID$")

#include <bslstl_forwarditerator.h>     // for testing only

#include <bsls_assert.h>
#include <bsls_platform.h>

#include <ctype.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

// IMPLEMENTATION NOTES:
//
// This string class implements a "short string optimization" which optimizes
// the handling of short strings (strings shorter than a certain length) by
// putting them into an internal short string buffer.  The short string buffer
// is a part of the 'string' object and it doesn't require any memory
// allocations.  This avoids memory allocations/deallocations on operations on
// short strings such as: construction, copy-construction, copy-assignment,
// destruction.  Those operations are much cheaper now than they used to be
// when they required memory allocations.
//
// There are also some side-effects of the short string optimization.  The
// footprint of the 'string' object is larger now than it used to be due to the
// short string buffer.  And the default-constructed object may now have a
// non-zero capacity.  But the default constructor of 'string' still doesn't
// require any memory and cannot fail.

#ifndef INCLUDED_BSLSTL_STRING_CPP
#define INCLUDED_BSLSTL_STRING_CPP

#ifdef BSLS_COMPILERFEATURES_SUPPORT_EXTERN_TEMPLATE
template class bsl::String_Imp<char, bsl::string::size_type>;
template class bsl::String_Imp<wchar_t, bsl::wstring::size_type>;
template class bsl::basic_string<char>;
template class bsl::basic_string<wchar_t>;
#endif

#if defined(BSLS_PLATFORM_CMP_MSVC) || defined(BSLS_PLATFORM_CMP_CLANG)
# define BSLSTL_STRING_PLATFORM_NEEDS_GUIDANCE_FOR_0X_STRINGS 1
#endif

namespace {
// "0x" should return '0' for bases 0 and 16, but some C runtime libraries
// disagree.  Intercept a possible error, and check for this special condition,
// including leading whitespace and (optionally) an immediately preceding '+'
// or '-'.
#if defined(BSLSTL_STRING_PLATFORM_NEEDS_GUIDANCE_FOR_0X_STRINGS)
static
bool isAwkwardZero(const bsl::string& str, int base)
    // Return 'true' if the specified 'str' is an awkwardly represented "0x"
    // zero in the specified 'base', and 'false' otherwise.
{
    if (0 != base && 16 != base) {
        return false;                                                 // RETURN
    }

    size_t index = str.size();
    if (str.size() < 2) { // || (0 != base && 16 != base)) {
        return false;                                                 // RETURN
    }

    --index;
    if ('x' != str[index] && 'X' != str[index]) {
        return false;                                                 // RETURN
    }

    if (str[--index] != '0') {
        return false;                                                 // RETURN
    }

    if (!index) {
        return true;                                                  // RETURN
    }

    --index;
    if ('+' == str[index] || '-' == str[index]) {
        if (!index) {
            return true;                                              // RETURN
        }
        --index;
    }

    while (isspace(str[index])) {
        if (!index) {
            return true;                                              // RETURN
        }
        --index;
    }
    return false;                                                     // RETURN
}

static
bool isAwkwardZero(const bsl::wstring& str, int base)
    // Return 'true' if the specified 'str' is an awkwardly represented "0x"
    // zero in the specified 'base', and 'false' otherwise.
{
    if (0 != base && 16 != base) {
        return false;                                                 // RETURN
    }

    size_t index = str.size();
    if (str.size() < 2) { // || (0 != base && 16 != base)) {
        return false;                                                 // RETURN
    }

    --index;
    if (L'x' != str[index] && L'X' != str[index]) {
        return false;                                                 // RETURN
    }

    if (L'0' != str[--index]) {
        return false;                                                 // RETURN
    }

    if (!index) {
        return true;                                                  // RETURN
    }

    --index;
    if (L'+' == str[index] || L'-' == str[index]) {
        if (!index) {
            return true;                                              // RETURN
        }
        --index;
    }

    while (isspace(str[index])) {
        if (!index) {
            return true;                                              // RETURN
        }
        --index;
    }

    return false;                                                     // RETURN
}
#else
# define isAwkwardZero(A, B) false
#endif

}  // close unnamed namespace

//-----------------------------------------------------------------------------

std::size_t bsl::hashBasicString(const string& str)
{
    return hashBasicString<char, char_traits<char>, allocator<char> >(str);
}

std::size_t bsl::hashBasicString(const wstring& str)
{
    return
      hashBasicString<wchar_t, char_traits<wchar_t>, allocator<wchar_t> >(str);
}

int bsl::stoi(const string& str, std::size_t *pos, int base)
{
    BSLS_ASSERT( 0 <= base);
    BSLS_ASSERT( 1 != base);
    BSLS_ASSERT(36 >= base);

    if (isAwkwardZero(str, base)) {
        if (pos) {
            *pos = str.size();
        }
        return 0;                                                     // RETURN
    }

    char *ptr;
    int   original_errno = errno;

    errno = 0;
    long value = strtol(str.c_str(), &ptr, base);

    if (errno == ERANGE) {
        BloombergLP::bslstl::StdExceptUtil::throwOutOfRange("stoi");
    }
    else if (0 != errno) {
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stoi");
    }
    else if (ptr == str.c_str()){
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stoi");
    }
    else if (sizeof(int) < sizeof(long) // proxy for 'long' has a wider range
         && (value > native_std::numeric_limits<int>::max() ||
             value < native_std::numeric_limits<int>::min() ) ) {
        BloombergLP::bslstl::StdExceptUtil::throwOutOfRange("stoi");
    }

    if (pos) {
        *pos = static_cast<size_t>(ptr - str.c_str());
    }

    errno = original_errno;
    return static_cast<int>(value);
}

int bsl::stoi(const wstring& str, std::size_t *pos, int base)
{
    BSLS_ASSERT( 0 <= base);
    BSLS_ASSERT( 1 != base);
    BSLS_ASSERT(36 >= base);

    if (isAwkwardZero(str, base)) {
        if (pos) {
            *pos = str.size();
        }
        return 0;                                                     // RETURN
    }

    wchar_t *ptr;
    int      original_errno = errno;

    errno = 0;
    long value = wcstol(str.c_str(), &ptr, base);

    if (errno == ERANGE){
        BloombergLP::bslstl::StdExceptUtil::throwOutOfRange("stoi");
    }
    else if (0 != errno) {
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stoi");
    }
    else if (ptr == str.c_str()){
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stoi");
    }
    else if (sizeof(int) < sizeof(long) // proxy for 'long' has a wider range
         && (value > native_std::numeric_limits<int>::max() ||
             value < native_std::numeric_limits<int>::min() ) ) {
        BloombergLP::bslstl::StdExceptUtil::throwOutOfRange("stoi");
    }

    if (pos) {
        *pos = static_cast<size_t>(ptr - str.c_str());
    }

    errno = original_errno;
    return static_cast<int>(value);
}

long bsl::stol(const string& str, std::size_t* pos, int base)
{
    BSLS_ASSERT( 0 <= base);
    BSLS_ASSERT( 1 != base);
    BSLS_ASSERT(36 >= base);

    if (isAwkwardZero(str, base)) {
        if (pos) {
            *pos = str.size();
        }
        return 0;                                                     // RETURN
    }

    char *ptr;
    int   original_errno = errno;

    errno = 0;
    long value = strtol(str.c_str(), &ptr, base);

    if (errno == ERANGE){
        BloombergLP::bslstl::StdExceptUtil::throwOutOfRange("stol");
    }
    else if (0 != errno) {
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stol");
    }
    else if (ptr == str.c_str()){
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stol");
    }

    if (pos){
        *pos = static_cast<size_t>(ptr - str.c_str());
    }

    errno = original_errno;
    return value;
}

long bsl::stol(const wstring& str, std::size_t *pos, int base)
{
    BSLS_ASSERT( 0 <= base);
    BSLS_ASSERT( 1 != base);
    BSLS_ASSERT(36 >= base);

    if (isAwkwardZero(str, base)) {
        if (pos) {
            *pos = str.size();
        }
        return 0;                                                     // RETURN
    }

    wchar_t *ptr;
    int      original_errno = errno;

    errno = 0;
    long value = wcstol(str.c_str(), &ptr, base);

    if (errno == ERANGE){
        BloombergLP::bslstl::StdExceptUtil::throwOutOfRange("stol");
    }
    else if (0 != errno) {
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stol");
    }
    else if (ptr == str.c_str()){
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stol");
    }

    if (pos) {
        *pos = static_cast<size_t>(ptr - str.c_str());
    }

    errno = original_errno;
    return value;
}

unsigned long bsl::stoul(const string& str, std::size_t *pos, int base)
{
    BSLS_ASSERT( 0 <= base);
    BSLS_ASSERT( 1 != base);
    BSLS_ASSERT(36 >= base);

    if (isAwkwardZero(str, base)) {
        if (pos) {
            *pos = str.size();
        }
        return 0;                                                     // RETURN
    }

    char *ptr;
    int   original_errno = errno;

    errno = 0;
    unsigned long value = strtoul(str.c_str(), &ptr, base);

    if (errno == ERANGE){
        BloombergLP::bslstl::StdExceptUtil::throwOutOfRange("stoul");
    }
    else if (0 != errno) {
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stoul");
    }
    else if (ptr == str.c_str()){
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stoul");
    }

    if (pos){
        *pos = ptr - str.c_str();
    }

    errno = original_errno;
    return value;
}

unsigned long bsl::stoul(const wstring&  str,
                         std::size_t    *pos,
                         int             base)
{
    BSLS_ASSERT( 0 <= base);
    BSLS_ASSERT( 1 != base);
    BSLS_ASSERT(36 >= base);

    if (isAwkwardZero(str, base)) {
        if (pos) {
            *pos = str.size();
        }
        return 0;                                                     // RETURN
    }

    wchar_t *ptr;
    int      original_errno = errno;

    errno = 0;
    unsigned long value = wcstoul(str.c_str(), &ptr, base);

    if (errno == ERANGE){
        BloombergLP::bslstl::StdExceptUtil::throwOutOfRange("stoul");
    }
    else if (0 != errno) {
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stoul");
    }
    else if (ptr == str.c_str()){
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stoul");
    }

    if (pos) {
        *pos = ptr - str.c_str();
    }

    errno = original_errno;
    return value;
}

long long bsl::stoll(const string& str, std::size_t *pos, int base)
{
    BSLS_ASSERT( 0 <= base);
    BSLS_ASSERT( 1 != base);
    BSLS_ASSERT(36 >= base);

    if (isAwkwardZero(str, base)) {
        if (pos) {
            *pos = str.size();
        }
        return 0;                                                     // RETURN
    }

    char *ptr;
    int   original_errno = errno;

    errno = 0;
#if !(defined(BSLS_PLATFORM_CMP_MSVC) && BSLS_PLATFORM_CMP_VER_MAJOR < 1800)
    long long value = strtoll(str.c_str(), &ptr, base);
#else
    long long value = _strtoi64(str.c_str(), &ptr, base);
#endif

    if (errno == ERANGE){
        BloombergLP::bslstl::StdExceptUtil::throwOutOfRange("stoll");
    }
    else if (0 != errno) {
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stoll");
    }
    else if (ptr == str.c_str()){
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stoll");
    }

    if (pos) {
        *pos = ptr - str.c_str();
    }

    errno = original_errno;
    return value;
}

long long bsl::stoll(const wstring& str, std::size_t *pos, int base)
{
    BSLS_ASSERT( 0 <= base);
    BSLS_ASSERT( 1 != base);
    BSLS_ASSERT(36 >= base);

    if (isAwkwardZero(str, base)) {
        if (pos) {
            *pos = str.size();
        }
        return 0;                                                     // RETURN
    }

    wchar_t *ptr;
    int      original_errno = errno;

    errno = 0;
#if !(defined(BSLS_PLATFORM_CMP_MSVC) && BSLS_PLATFORM_CMP_VER_MAJOR < 1800)
    long long value = wcstoll(str.c_str(), &ptr, base);
#else
    long long value = _wcstoi64(str.c_str(), &ptr, base);
#endif

    if (errno == ERANGE){
        BloombergLP::bslstl::StdExceptUtil::throwOutOfRange("stoll");
    }
    else if (0 != errno) {
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stoll");
    }
    else if (ptr == str.c_str()){
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stoll");
    }

    if (pos) {
        *pos = ptr - str.c_str();
    }

    errno = original_errno;
    return value;
}

unsigned long long bsl::stoull(const string& str, std::size_t *pos, int base)
{
    BSLS_ASSERT( 0 <= base);
    BSLS_ASSERT( 1 != base);
    BSLS_ASSERT(36 >= base);

    if (isAwkwardZero(str, base)) {
        if (pos) {
            *pos = str.size();
        }
        return 0;                                                     // RETURN
    }

    char *ptr;
    int   original_errno = errno;

    errno = 0;
#if !(defined(BSLS_PLATFORM_CMP_MSVC) && BSLS_PLATFORM_CMP_VER_MAJOR < 1800)
    unsigned long long value = strtoull(str.c_str(), &ptr, base);
#else
    unsigned long long value = _strtoui64(str.c_str(), &ptr, base);
#endif

    if (errno == ERANGE){
        BloombergLP::bslstl::StdExceptUtil::throwOutOfRange("stoull");
    }
    else if (0 != errno) {
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stoull");
    }
    else if (ptr == str.c_str()){
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stoull");
    }

    if (pos) {
        *pos = ptr - str.c_str();
    }

    errno = original_errno;
    return value;
}

unsigned long long bsl::stoull(const wstring& str, std::size_t *pos, int base)
{
    BSLS_ASSERT( 0 <= base);
    BSLS_ASSERT( 1 != base);
    BSLS_ASSERT(36 >= base);

    if (isAwkwardZero(str, base)) {
        if (pos) {
            *pos = str.size();
        }
        return 0;                                                     // RETURN
    }

    wchar_t *ptr;
    int      original_errno = errno;

    errno = 0;
#if !(defined(BSLS_PLATFORM_CMP_MSVC) && BSLS_PLATFORM_CMP_VER_MAJOR < 1800)
    unsigned long long value = wcstoull(str.c_str(), &ptr, base);
#else
    unsigned long long value = _wcstoui64(str.c_str(), &ptr, base);
#endif

    if (errno == ERANGE){
        BloombergLP::bslstl::StdExceptUtil::throwOutOfRange("stoull");
    }
    else if (0 != errno) {
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stoull");
    }
    else if (ptr == str.c_str()){
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stoull");
    }

    if (pos) {
        *pos = ptr - str.c_str();
    }

    errno = original_errno;
    return value;
}

double bsl::stod(const string& str, std::size_t *pos)
{
    char *ptr;
    int   original_errno = errno;

    errno = 0;
    double value = strtod(str.c_str(), &ptr);

    if (errno == ERANGE){
        BloombergLP::bslstl::StdExceptUtil::throwOutOfRange("stod");
    }
    else if (0 != errno) {
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stod");
    }
    else if (ptr == str.c_str()){
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stod");
    }

    if (pos) {
        *pos = ptr - str.c_str();
    }

    errno = original_errno;
    return value;
}

double bsl::stod(const wstring& str, std::size_t *pos)
{
    wchar_t *ptr;
    int      original_errno = errno;

    errno = 0;
    double value = wcstod(str.c_str(), &ptr);

    if (errno == ERANGE){
        BloombergLP::bslstl::StdExceptUtil::throwOutOfRange("stod");
    }
    else if (0 != errno) {
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stod");
    }
    else if (ptr == str.c_str()){
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stod");
    }

    if (pos) {
        *pos = ptr - str.c_str();
    }

    errno = original_errno;
    return value;
}

#if !defined(BSLS_PLATFORM_CMP_MSVC) || BSLS_PLATFORM_CMP_VER_MAJOR > 1800
float bsl::stof(const string& str, std::size_t *pos)
{
    char *ptr;
    int   original_errno = errno;

    errno = 0;

    float value = strtof(str.c_str(), &ptr);

    if (errno == ERANGE){
        BloombergLP::bslstl::StdExceptUtil::throwOutOfRange("stof");
    }
    else if (0 != errno) {
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stof");
    }
    else if (ptr == str.c_str()){
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stof");
    }

    if (pos) {
        *pos = ptr - str.c_str();
    }

    errno = original_errno;
    return value;
}

float bsl::stof(const wstring& str, std::size_t *pos)
{
    wchar_t *ptr;
    int      original_errno = errno;

    errno = 0;
    float value = wcstof(str.c_str(), &ptr);

    if (errno == ERANGE){
        BloombergLP::bslstl::StdExceptUtil::throwOutOfRange("stof");
    }
    else if (0 != errno) {
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stof");
    }
    else if (ptr == str.c_str()){
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stof");
    }

    if (pos) {
        *pos = ptr - str.c_str();
    }

    errno = original_errno;
    return value;
}

long double bsl::stold(const string& str, std::size_t *pos)
{
    char *ptr;
    int   original_errno = errno;

    errno = 0;
    long double value = strtold(str.c_str(), &ptr);

    if (errno == ERANGE){
        BloombergLP::bslstl::StdExceptUtil::throwOutOfRange("stold");
    }
    else if (0 != errno) {
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stold");
    }
    else if (ptr == str.c_str()){
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stold");
    }

    if (pos) {
        *pos = ptr - str.c_str();
    }

    errno = original_errno;
    return value;
}

long double bsl::stold(const wstring& str, std::size_t *pos)
{
    wchar_t *ptr;
    int      original_errno = errno;

    errno = 0;
    long double value = wcstold(str.c_str(), &ptr);

    if (errno == ERANGE){
        BloombergLP::bslstl::StdExceptUtil::throwOutOfRange("stold");
    }
    else if (0 != errno) {
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stold");
    }
    else if (ptr == str.c_str()){
        BloombergLP::bslstl::StdExceptUtil::throwInvalidArgument("stold");
    }

    if (pos) {
        *pos = ptr - str.c_str();
    }

    errno = original_errno;
    return value;
}
#else
// Prior to C++17, the C++ standard specified this function using the C90
// library, which did not have 'strtof'.  That is the implementation we
// continue to support on older Microsoft compilers, lacking the necessary C99
// conversion functions.  Note that 'errno' and invalid strings are detected
// and thrown by the 'stod' function that is forwarded to, so need only to
// worry about performing a safe floating-point conversion.

namespace {
inline
float convertDoubleToFloatOrThrow(double value) {
    if ((value <=  native_std::numeric_limits<float>::max()) &&
        (value >= -native_std::numeric_limits<float>::max())) {
        return static_cast<float>(value);
    }

    BloombergLP::bslstl::StdExceptUtil::throwOutOfRange("stolf");
}

}
float bsl::stof(const string& str, std::size_t *pos)
{
    return convertDoubleToFloatOrThrow(stod(str, pos));
}

float bsl::stof(const wstring& str, std::size_t *pos)
{
    return convertDoubleToFloatOrThrow(stod(str, pos));
}

// 'long double' has an identical representation to 'double' on these Windows
// compilers, so lacking the 'strtold' function is not an issue.

long double bsl::stold(const string& str, std::size_t *pos)
{
    return stod(str, pos);
}

long double bsl::stold(const wstring& str, std::size_t *pos)
{
    return stod(str, pos);
}
#endif

bsl::string bsl::to_string(int value) {
    BSLS_ASSERT(std::numeric_limits<int>::digits * 100/332 +
                std::numeric_limits<int>::is_signed + 1 <
                bsl::string::SHORT_BUFFER_LENGTH);

    bsl::string str;

    int len = sprintf(str.dataPtr(), "%d", value);

    BSLS_ASSERT_SAFE(len < bsl::string::SHORT_BUFFER_LENGTH);

    str.d_length = len;
    return str;
}

bsl::string bsl::to_string(unsigned value) {
    BSLS_ASSERT(std::numeric_limits<unsigned>::digits * 100/332 +
                std::numeric_limits<unsigned>::is_signed + 1 <
                bsl::string::SHORT_BUFFER_LENGTH);

    bsl::string str;

    int len = sprintf(str.dataPtr(), "%u", value);

    BSLS_ASSERT_SAFE(len < bsl::string::SHORT_BUFFER_LENGTH);

    str.d_length = len;
    return str;
}

bsl::string bsl::to_string(long value) {
    BSLS_ASSERT(std::numeric_limits<long>::digits * 100/332 +
                std::numeric_limits<long>::is_signed + 1 <
                bsl::string::SHORT_BUFFER_LENGTH);

    bsl::string str;

    int len = sprintf(str.dataPtr(), "%ld", value);

    BSLS_ASSERT_SAFE(len < bsl::string::SHORT_BUFFER_LENGTH);

    str.d_length = len;
    return str;
}

bsl::string bsl::to_string(unsigned long value) {
    BSLS_ASSERT(std::numeric_limits<unsigned long >::digits * 100/332 +
                std::numeric_limits<unsigned long >::is_signed + 1 <
                bsl::string::SHORT_BUFFER_LENGTH);

    bsl::string str;

    int len = sprintf(str.dataPtr(), "%lu", value);

    BSLS_ASSERT_SAFE(len < bsl::string::SHORT_BUFFER_LENGTH);

    str.d_length = len;
    return str;
}

bsl::string bsl::to_string(long long value) {
    char tempBuf[e_MAX_INT64_STRLEN10];
    int  len = sprintf(tempBuf, "%lld", value);

    BSLS_ASSERT_SAFE(len < e_MAX_INT64_STRLEN10);
    return string (tempBuf, len);
}

bsl::string bsl::to_string(unsigned long long value) {
    char tempBuf[e_MAX_INT64_STRLEN10];
    int  len = sprintf(tempBuf, "%llu", value);

    BSLS_ASSERT_SAFE(len < e_MAX_INT64_STRLEN10);
    return string (tempBuf, len);
}

bsl::string bsl::to_string(float value) {
    char tempBuf[e_MAX_FLOAT_STRLEN10];
    int  len = sprintf(tempBuf, "%f", value);

    BSLS_ASSERT_SAFE(len < e_MAX_FLOAT_STRLEN10);
    return string (tempBuf, len);
}

bsl::string bsl::to_string(double value) {
    char tempBuf[e_MAX_DOUBLE_STRLEN10];
    int  len = sprintf(tempBuf, "%f", value);

    BSLS_ASSERT_SAFE(len < e_MAX_DOUBLE_STRLEN10);
    return string (tempBuf, len);
}

bsl::string bsl::to_string(long double value) {
    char tempBuf[e_MAX_LONGDOUBLE_STRLEN10];
    int  len = sprintf(tempBuf, "%Lf", value);

    BSLS_ASSERT_SAFE(len < e_MAX_LONGDOUBLE_STRLEN10);
    return string (tempBuf, len);
}

bsl::wstring bsl::to_wstring(int value) {
    wchar_t tempBuf[e_MAX_INT_STRLEN10];
    int     len = swprintf(tempBuf,
                           sizeof tempBuf / sizeof *tempBuf,
                           L"%d", value);

    BSLS_ASSERT_SAFE(len < e_MAX_INT_STRLEN10);
    return wstring (tempBuf, len);
}

bsl::wstring bsl::to_wstring(long value) {
    wchar_t tempBuf[e_MAX_INT64_STRLEN10];
    int     len = swprintf(tempBuf,
                           sizeof tempBuf / sizeof *tempBuf,
                           L"%ld", value);

    BSLS_ASSERT_SAFE(len < e_MAX_INT64_STRLEN10);
    return wstring (tempBuf, len);
}

bsl::wstring bsl::to_wstring(long long value) {
    wchar_t tempBuf[e_MAX_INT64_STRLEN10];
    int     len = swprintf(tempBuf,
                           sizeof tempBuf / sizeof *tempBuf,
                           L"%lld", value);

    BSLS_ASSERT_SAFE(len < e_MAX_INT64_STRLEN10);
    return wstring (tempBuf, len);
}

bsl::wstring bsl::to_wstring(unsigned value) {
    wchar_t tempBuf[e_MAX_INT_STRLEN10];
    int     len = swprintf(tempBuf,
                           sizeof tempBuf / sizeof *tempBuf,
                           L"%u", value);

    BSLS_ASSERT_SAFE(len < e_MAX_INT_STRLEN10);
    return wstring (tempBuf, len);
}

bsl::wstring bsl::to_wstring(unsigned long value) {
    wchar_t tempBuf[e_MAX_INT64_STRLEN10];
    int     len = swprintf(tempBuf,
                           sizeof tempBuf / sizeof *tempBuf,
                           L"%lu", value);

    BSLS_ASSERT_SAFE(len < e_MAX_INT64_STRLEN10);
    return wstring (tempBuf, len);
}

bsl::wstring bsl::to_wstring(unsigned long long value) {
    wchar_t tempBuf[e_MAX_INT64_STRLEN10];
    int     len = swprintf(tempBuf,
                           sizeof tempBuf / sizeof *tempBuf,
                           L"%llu", value);

    BSLS_ASSERT_SAFE(len < e_MAX_INT64_STRLEN10);
    return wstring (tempBuf, len);
}

bsl::wstring bsl::to_wstring(float value) {
    wchar_t tempBuf[e_MAX_FLOAT_STRLEN10];
    int     len = swprintf(tempBuf,
                           sizeof tempBuf / sizeof *tempBuf,
                           L"%f", value);

    BSLS_ASSERT_SAFE(len < e_MAX_FLOAT_STRLEN10);
    return wstring (tempBuf, len);
}

bsl::wstring bsl::to_wstring(double value) {
    wchar_t tempBuf[e_MAX_DOUBLE_STRLEN10];
    int     len = swprintf(tempBuf,
                           sizeof tempBuf / sizeof *tempBuf,
                           L"%f", value);

    BSLS_ASSERT_SAFE(len < e_MAX_DOUBLE_STRLEN10);
    return wstring (tempBuf, len);
}

bsl::wstring bsl::to_wstring(long double value) {
    wchar_t tempBuf[e_MAX_LONGDOUBLE_STRLEN10];
    int     len = swprintf(tempBuf,
                           sizeof tempBuf / sizeof *tempBuf,
                           L"%Lf", value);

    BSLS_ASSERT_SAFE(len < e_MAX_LONGDOUBLE_STRLEN10);
    return wstring (tempBuf, len);
}

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
