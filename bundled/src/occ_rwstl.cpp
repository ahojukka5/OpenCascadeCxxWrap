// occ_rwstl.cpp — 1:1 CxxWrap bindings for RWStl: direct STL <->
// Poly_Triangulation I/O, more efficient than StlAPI_Reader/Writer for
// large meshes since it skips TopoDS_Shape/BRep_Builder entirely.
// WriteBinary/WriteAscii only offer OSD_Path or Standard_OStream
// overloads (no raw-path convenience like ReadFile has) -- bind via the
// Standard_OStream& overload and build a std::ofstream from a
// std::string path inside the lambda, avoiding any need to expose
// OSD_Path to Julia.
#include "occ_handle_traits.hpp"
#include <jlcxx/jlcxx.hpp>

#include <RWStl.hxx>
#include <Poly_Triangulation.hxx>
#include <fstream>
#include <string>

void register_occ_rwstl(jlcxx::Module& mod) {
  mod.method("RWStl_ReadFile", [](const std::string& path, double mergeAngle) -> Handle(Poly_Triangulation) {
    return RWStl::ReadFile(path.c_str(), mergeAngle);
  });
  mod.method("RWStl_WriteBinary", [](const Handle(Poly_Triangulation)& mesh, const std::string& path) -> bool {
    std::ofstream out(path, std::ios::binary);
    return RWStl::WriteBinary(mesh, out);
  });
  mod.method("RWStl_WriteAscii", [](const Handle(Poly_Triangulation)& mesh, const std::string& path) -> bool {
    std::ofstream out(path);
    return RWStl::WriteAscii(mesh, out);
  });
}
