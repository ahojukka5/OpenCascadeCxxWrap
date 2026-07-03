// occ_units.cpp — 1:1 CxxWrap bindings for UnitsAPI: real dimensional-
// analysis unit conversion (not just STEP's own "MM"/"INCH" header
// string), free functions only, no class/state to wrap.
#include <jlcxx/jlcxx.hpp>

#include <UnitsAPI.hxx>
#include <string>

void register_occ_units(jlcxx::Module& mod) {
  mod.method("UnitsAPI_AnyToAny", [](double data, const std::string& fromUnit, const std::string& toUnit) -> double {
    return UnitsAPI::AnyToAny(data, fromUnit.c_str(), toUnit.c_str());
  });
  mod.method("UnitsAPI_AnyToSI", [](double data, const std::string& unit) -> double {
    return UnitsAPI::AnyToSI(data, unit.c_str());
  });
  mod.method("UnitsAPI_AnyToLS", [](double data, const std::string& unit) -> double {
    return UnitsAPI::AnyToLS(data, unit.c_str());
  });
  mod.method("UnitsAPI_AnyFromSI", [](double data, const std::string& unit) -> double {
    return UnitsAPI::AnyFromSI(data, unit.c_str());
  });
  mod.method("UnitsAPI_AnyFromLS", [](double data, const std::string& unit) -> double {
    return UnitsAPI::AnyFromLS(data, unit.c_str());
  });
  mod.method("UnitsAPI_SetCurrentUnit", [](const std::string& quantity, const std::string& unit) {
    UnitsAPI::SetCurrentUnit(quantity.c_str(), unit.c_str());
  });
  mod.method("UnitsAPI_CurrentUnit", [](const std::string& quantity) -> std::string {
    return std::string(UnitsAPI::CurrentUnit(quantity.c_str()));
  });
  mod.method("UnitsAPI_Check", [](const std::string& quantity, const std::string& unit) -> bool {
    return bool(UnitsAPI::Check(quantity.c_str(), unit.c_str()));
  });
}
