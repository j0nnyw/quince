#ifndef QUINCE__mappers__detail__array_of_int16_h
#define QUINCE__mappers__detail__array_of_int16_h

//          Copyright James Earnshaw 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <boost/serialization/strong_typedef.hpp>>

/*
    Everything in this file is for quince internal use only.
*/

// Low-level representation of int16_t[], as used in a cell.
//
// It's the same as the DBMS's on-the-wire representation,
// so any conversions between this and the representation in user code must be made in wrappers.
//

namespace quince {
    BOOST_STRONG_TYPEDEF(std::string, array_of_int16)
}

#endif
