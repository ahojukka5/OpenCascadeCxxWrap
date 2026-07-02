// occ_exception.hpp — translate OCCT's Standard_Failure hierarchy into a
// std::exception that JlCxx's own call-site try/catch (jlcxx/module.hpp's
// CallFunctor::apply, which catches std::exception and calls jl_error()) can
// turn into a normal, catchable Julia exception.
//
// Standard_Failure derives from Standard_Transient, NOT std::exception, so
// without this, an OCCT-level throw (e.g. a malformed BRepPrimAPI_MakeBox
// argument, or BRepGProp_VolumeProperties on a degenerate shape) unwinds
// straight through the Julia ccall boundary as an unrecognized C++ exception
// -- undefined behavior, observed in practice as a hard process abort/segfault
// rather than a Julia-catchable error.
//
// Wrap any OCCT call that can throw on malformed/degenerate input:
//   mod.method("Foo", [](T& obj, double x) {
//     return occ_guard([&]{ return obj.Foo(x); });
//   });
#pragma once

#include <Standard_Failure.hxx>
#include <Standard_Type.hxx>
#include <stdexcept>
#include <string>

template <typename F>
auto occ_guard(F&& f) -> decltype(f()) {
  try {
    return f();
  } catch (const Standard_Failure& e) {
    Standard_CString msg = e.GetMessageString();
    std::string what = (msg && *msg) ? std::string(msg) : std::string(e.DynamicType()->Name());
    throw std::runtime_error(what);
  }
}
