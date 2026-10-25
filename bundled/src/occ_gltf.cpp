// occ_gltf.cpp — 1:1 CxxWrap binding for RWGltf_CafWriter, OCCT's native
// glTF/GLB exporter. Previously the only glTF path available to Julia was a
// hand-rolled writer parsing a temporary ASCII STL file -- mesh-only, no
// names/colors, and paying for a full file round-trip for something already
// available in memory. This calls into OCCT's own, standards-compliant
// writer directly against an XCAF document (so part names/colors carry
// through), operating on whatever triangulation is already attached to the
// shapes (BRepMesh_IncrementalMesh must be run first -- this binding does
// not mesh implicitly, matching RWGltf_CafWriter's own documented contract).
//
// CMake defines OPENCASCADE_CXXWRAP_HAS_RWGLTF only when the configured
// OCCT prefix installs RWGltf_CafWriter.hxx. OCCT 7.9.3 does, and this file
// keeps that writer. The published OCCT 8.0.1 archives build without
// RapidJSON, so they omit the header and the toolkit. This translation unit
// still compiles there and reports the writer as unbound. It does not
// invent a replacement exporter.
#include <jlcxx/jlcxx.hpp>

#if defined(OPENCASCADE_CXXWRAP_HAS_RWGLTF)

#include "occ_handle_traits.hpp"

#include <RWGltf_CafWriter.hxx>
#include <TDocStd_Document.hxx>
#include <TCollection_AsciiString.hxx>
#include <TColStd_IndexedDataMapOfStringString.hxx>
#include <Message_ProgressRange.hxx>

void register_occ_gltf(jlcxx::Module& mod) {
  mod.method("OCCT_RWGltfCafWriterBound", []() { return true; });

  mod.add_type<RWGltf_CafWriter>("RWGltf_CafWriter")
     .constructor([](const std::string& file, bool isBinary) -> RWGltf_CafWriter* {
       return new RWGltf_CafWriter(TCollection_AsciiString(file.c_str()), isBinary);
     });

  mod.method("Perform", [](RWGltf_CafWriter& w, const Handle(TDocStd_Document)& doc) -> bool {
    TColStd_IndexedDataMapOfStringString fileInfo;
    Message_ProgressRange progress;
    return bool(w.Perform(doc, fileInfo, progress));
  });
}

#else

void register_occ_gltf(jlcxx::Module& mod) {
  mod.method("OCCT_RWGltfCafWriterBound", []() { return false; });
}

#endif
