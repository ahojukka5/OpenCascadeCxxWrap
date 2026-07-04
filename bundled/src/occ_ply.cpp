// occ_ply.cpp — 1:1 CxxWrap binding for RWPly_CafWriter, OCCT's native PLY
// exporter. Same idiom as RWObj_CafWriter/RWGltf_CafWriter (occ_robj.cpp/
// occ_gltf.cpp): Standard_Transient-derived, but nothing downstream needs
// the Handle, so it's bound as a plain lambda constructor + value methods.
#include "occ_handle_traits.hpp"
#include <jlcxx/jlcxx.hpp>

#include <RWPly_CafWriter.hxx>
#include <TDocStd_Document.hxx>
#include <TCollection_AsciiString.hxx>
#include <NCollection_IndexedDataMap.hxx>
#include <Message_ProgressRange.hxx>

void register_occ_ply(jlcxx::Module& mod) {
  mod.add_type<RWPly_CafWriter>("RWPly_CafWriter")
     .constructor([](const std::string& file) -> RWPly_CafWriter* {
       return new RWPly_CafWriter(TCollection_AsciiString(file.c_str()));
     });

  mod.method("Perform", [](RWPly_CafWriter& w, const Handle(TDocStd_Document)& doc) -> bool {
    NCollection_IndexedDataMap<TCollection_AsciiString, TCollection_AsciiString> fileInfo;
    Message_ProgressRange progress;
    return bool(w.Perform(doc, fileInfo, progress));
  });

  mod.method("SetDoublePrecision", [](RWPly_CafWriter& w, bool v) { w.SetDoublePrecision(v); });
  mod.method("IsDoublePrecision", [](const RWPly_CafWriter& w) -> bool { return w.IsDoublePrecision(); });
  mod.method("SetNormals", [](RWPly_CafWriter& w, bool v) { w.SetNormals(v); });
  mod.method("HasNormals", [](const RWPly_CafWriter& w) -> bool { return w.HasNormals(); });
  mod.method("SetColors", [](RWPly_CafWriter& w, bool v) { w.SetColors(v); });
  mod.method("HasColors", [](const RWPly_CafWriter& w) -> bool { return w.HasColors(); });
  mod.method("SetTexCoords", [](RWPly_CafWriter& w, bool v) { w.SetTexCoords(v); });
  mod.method("HasTexCoords", [](const RWPly_CafWriter& w) -> bool { return w.HasTexCoords(); });
}
