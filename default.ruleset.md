# Comments on disabled Microsoft C++ Code Analysis Rules

This document contains the rationales why some Microsoft
C++ warnings are disabled in the file default.ruleset
It is not possible to add this info to the .ruleset file itself as edit actions
with the VS GUI would cause the comments to get lost.
Most of these Microsoft rules\warning are based on the C++ Core Guidelines.

## Warnings

- C26052: Potentially unconstrained access using expression  
**Rationale**: false warnings (VS 2019 16.9.0 Preview 2)

- C26429: Use a not_null to indicate that "null" is not a valid value  
**Rationale**: Prefast attributes (\_In_, etc.) are better than gsl::not_null.

- C26446: Prefer to use gsl::at() instead of unchecked subscript operator.  
**Rationale**: gsl:at() cannot be used as gsl project is by design not included. MSVC STL in debug mode already checks access.

- C26459: You called an STL function 'std::transform' with a raw pointer parameter. Consider wrapping your range in a
gsl::span and pass as a span iterator (stl.1)  
**Rationale**: gsl:span() cannot be used. Update to std:span when available (C++20).

- C26466: Don't use static_cast downcasts. A cast from a polymorphic type should use dynamic_cast.  
**Rationale**: not using RTTI to support dynamic_cast.

- C26472: Don't use static_cast for arithmetic conversions  
**Rationale**: can only be solved with gsl::narrow_cast

- C26474: No implicit cast  
**Rationale**: false warnings (VS 2019 16.8.0 Preview 3)

- C26481: Do not pass an array as a single pointer.  
**Rationale**: gsl::span is not available.

- C26482:Only index into arrays using constant expressions.  
**Rationale**: static analysis can verify access.

- C26485: Do not pass an array as a single pointer  
**Rationale**: see rationale for C26481.

- C26490: Don't use reinterpret_cast  
**Rationale**: required to work with win32 API, manual review required.

- C26494: Variable 'x' is uninitialized. Always initialize an object  
**Rationale**: many false warnings due to output parameters. Other analyzers (prefast) provide
flow inspection and can be used.
