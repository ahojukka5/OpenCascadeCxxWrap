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
// version, so it avoids DynamicType() (absent in 8.0.0, since
// Standard_Failure is no longer Standard_Transient there). The message
// accessor is selected by OCC_VERSION_HEX: GetMessageString() is the
// 7.9.3 name, and what() is the 8.0 name. GetMessageString() remains in
// 8.0 only as a deprecated alias, so an 8.0 build does not call it.
// A missing message falls back to typeid(e).name(), which is the
// compiler-mangled name rather than OCCT's own type name.
//
// Wrap any OCCT call that can throw on malformed/degenerate input:
//   mod.method("Foo", [](T& obj, double x) {
//     return occ_guard([&]{ return obj.Foo(x); });
//   });
#pragma once

#include <Standard_Failure.hxx>
#include <Standard_Version.hxx>
#include <stdexcept>
#include <string>
#include <typeinfo>

template <typename F>
auto occ_guard(F&& f) -> decltype(f()) {
  try {
    return f();
  } catch (const Standard_Failure& e) {
#if OCC_VERSION_HEX >= 0x080000
    const char* msg = e.what();
#else
    const char* msg = e.GetMessageString();
#endif
    std::string what = (msg && *msg) ? std::string(msg) : std::string(typeid(e).name());
    throw std::runtime_error(what);
  }
}
