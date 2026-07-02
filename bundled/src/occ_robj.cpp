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
}
