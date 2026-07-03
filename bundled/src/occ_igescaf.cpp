// occ_igescaf.cpp — 1:1 CxxWrap bindings for IGESCAFControl_Reader/Writer:
// IGES's equivalent of STEPCAFControl_Reader/Writer, bridging IGES import/
// export to an XCAF document (names/colors/layers, not just a bare shape).
// IGES has no native multi-part assembly entity the way STEP AP242 does, so
// this is shallower than the STEP bridge, but names/colors still round-trip.
#include "occ_handle_traits.hpp"
#include <jlcxx/jlcxx.hpp>

#include <IGESCAFControl_Reader.hxx>
#include <IGESCAFControl_Writer.hxx>
#include <TDocStd_Document.hxx>
#include <string>

void register_occ_igescaf(jlcxx::Module& mod) {
  mod.add_type<IGESCAFControl_Reader>("IGESCAFControl_Reader").constructor<>();
  mod.method("Perform", [](IGESCAFControl_Reader& r, const std::string& f, Handle(TDocStd_Document)& doc) {
    return bool(r.Perform(f.c_str(), doc));
  });
  mod.method("SetColorMode", [](IGESCAFControl_Reader& r, bool value) { r.SetColorMode(value); });
  mod.method("SetNameMode",  [](IGESCAFControl_Reader& r, bool value) { r.SetNameMode(value); });
  mod.method("SetLayerMode", [](IGESCAFControl_Reader& r, bool value) { r.SetLayerMode(value); });

  mod.add_type<IGESCAFControl_Writer>("IGESCAFControl_Writer").constructor<>();
  mod.method("Transfer", [](IGESCAFControl_Writer& w, Handle(TDocStd_Document)& doc) {
    return bool(w.Transfer(doc));
  });
  mod.method("Write", [](IGESCAFControl_Writer& w, const std::string& f) {
    return bool(w.Write(f.c_str()));
  });
  mod.method("SetColorMode", [](IGESCAFControl_Writer& w, bool value) { w.SetColorMode(value); });
  mod.method("SetNameMode",  [](IGESCAFControl_Writer& w, bool value) { w.SetNameMode(value); });
  mod.method("SetLayerMode", [](IGESCAFControl_Writer& w, bool value) { w.SetLayerMode(value); });
}
