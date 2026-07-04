// occ_robj.cpp — 1:1 CxxWrap binding for RWObj_CafWriter, OCCT's native
// Wavefront OBJ exporter. A working alternative to RWGltf_CafWriter
// (occ_gltf.cpp), which is non-functional on the shipped OCCT_jll binary
// (built without RapidJSON support) -- RWObj is a separate, simpler
// toolkit unaffected by that gap.
//
// RWObj_CafWriter is Standard_Transient-derived, but -- unlike
// HLRBRep_Algo (occ_hlr.cpp) -- nothing else needs to hold a
// Handle(RWObj_CafWriter): it's a standalone writer, constructed, used
// once via Perform, and discarded. The already-shipped RWGltf_CafWriter
// is *also* Transient-derived and is bound exactly this way (plain lambda
// constructor returning a raw pointer, methods taking a plain reference)
// -- proven to work fine as long as nothing downstream needs the Handle.
#include "occ_handle_traits.hpp"
#include <jlcxx/jlcxx.hpp>

#include <RWObj_CafWriter.hxx>
#include <RWObj_CafReader.hxx>
#include <TDocStd_Document.hxx>
#include <TCollection_AsciiString.hxx>
#include <TColStd_IndexedDataMapOfStringString.hxx>
#include <Message_ProgressRange.hxx>

void register_occ_robj(jlcxx::Module& mod) {
  mod.add_type<RWObj_CafWriter>("RWObj_CafWriter")
     .constructor([](const std::string& file) -> RWObj_CafWriter* {
       return new RWObj_CafWriter(TCollection_AsciiString(file.c_str()));
     });

  mod.method("Perform", [](RWObj_CafWriter& w, const Handle(TDocStd_Document)& doc) -> bool {
    TColStd_IndexedDataMapOfStringString fileInfo;
    Message_ProgressRange progress;
    return bool(w.Perform(doc, fileInfo, progress));
  });

  // RWObj_CafReader: OBJ *import* into an XCAF document -- until now
  // occ_robj.cpp only had the writer. Standard_Transient-derived like
  // RWObj_CafWriter above, same "no Handle needed" reasoning (nothing
  // downstream holds a Handle(RWObj_CafReader)) -- but unlike the writer's
  // 1-arg constructor, an explicit 0-arg .constructor() lambda here
  // double-registers against jlcxx's own auto-generated default
  // constructor (RWObj_CafReader has a public default ctor), so this
  // relies on that implicit one instead of adding a redundant explicit one.
  mod.add_type<RWObj_CafReader>("RWObj_CafReader");
  mod.method("SetDocument", [](RWObj_CafReader& r, const Handle(TDocStd_Document)& doc) {
    r.SetDocument(doc);
  });
  mod.method("Perform", [](RWObj_CafReader& r, const std::string& file) -> bool {
    Message_ProgressRange progress;
    return bool(r.Perform(TCollection_AsciiString(file.c_str()), progress));
  });
  mod.method("SetSinglePrecision", [](RWObj_CafReader& r, bool v) { r.SetSinglePrecision(v); });
  mod.method("IsSinglePrecision", [](const RWObj_CafReader& r) -> bool { return r.IsSinglePrecision(); });
}
