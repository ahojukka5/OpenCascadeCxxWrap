// occ_interface_static.cpp — 1:1 CxxWrap bindings for Interface_Static, the
// static (process-global) parameter registry OCCT's STEP/IGES translators
// read for unit control ("write.step.unit" before STEPControl_Writer::Write,
// "xstep.cascade.unit" to control/inspect the unit STEPControl_Reader
// imports into) and similar translation parameters. Nothing in this repo
// previously read or set these -- STEP export/import silently defaulted to
// whatever OCCT's built-in default is (millimeters), with no way for a
// caller to declare or discover the unit in play.
#include <jlcxx/jlcxx.hpp>

#include <Interface_Static.hxx>
#include <string>

void register_occ_interface_static(jlcxx::Module& mod) {
  mod.method("Interface_Static_CVal", [](const std::string& name) -> std::string {
    Standard_CString v = Interface_Static::CVal(name.c_str());
    return v == nullptr ? std::string() : std::string(v);
  });
  mod.method("Interface_Static_SetCVal", [](const std::string& name, const std::string& val) -> bool {
    return bool(Interface_Static::SetCVal(name.c_str(), val.c_str()));
  });
  mod.method("Interface_Static_IVal", [](const std::string& name) -> int {
    return int(Interface_Static::IVal(name.c_str()));
  });
  mod.method("Interface_Static_SetIVal", [](const std::string& name, int val) -> bool {
    return bool(Interface_Static::SetIVal(name.c_str(), val));
  });
}
