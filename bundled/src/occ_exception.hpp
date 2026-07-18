// occ_exception.hpp — translate OCCT's Standard_Failure hierarchy into a
// std::exception that JlCxx's own call-site try/catch (jlcxx/module.hpp's
// CallFunctor::apply, which catches std::exception and calls jl_error()) can
// turn into a normal, catchable Julia exception.
//
// In OCCT 7.9.3, Standard_Failure derives from Standard_Transient, NOT
// std::exception, so without this, an OCCT-level throw (e.g. a malformed
// BRepPrimAPI_MakeBox argument, or BRepGProp_VolumeProperties on a
// degenerate shape) unwinds straight through the Julia ccall boundary as
// an unrecognized C++ exception -- undefined behavior, observed in
// practice as a hard process abort/segfault rather than a Julia-catchable
// error. OCCT 8.0.0 changed Standard_Failure to derive from std::exception
// directly (dropping Standard_Transient/RTTI entirely, per its own header
// comment) -- this guard still needs to work for a build against either
// version, so it avoids both what() (absent in 7.9.3) and DynamicType()
// (absent in 8.0.0, since Standard_Failure is no longer Standard_Transient
// there), using only APIs present in both: GetMessageString() (the
// original, non-deprecated name in 7.9.3, a deprecated-but-functional
// alias for what() in 8.0.0) and, in the rare "no message" fallback case,
// the standard (compiler-mangled, not OCCT's own pretty type name)
// typeid(e).name() rather than any OCCT-specific RTTI.
//
// Wrap any OCCT call that can throw on malformed/degenerate input:
//   mod.method("Foo", [](T& obj, double x) {
//     return occ_guard([&]{ return obj.Foo(x); });
//   });
#pragma once

#include <Standard_Failure.hxx>
#include <stdexcept>
#include <string>
#include <typeinfo>

template <typename F>
auto occ_guard(F&& f) -> decltype(f()) {
  try {
    return f();
  } catch (const Standard_Failure& e) {
    const char* msg = e.GetMessageString();
    std::string what = (msg && *msg) ? std::string(msg) : std::string(typeid(e).name());
    throw std::runtime_error(what);
  }
}
