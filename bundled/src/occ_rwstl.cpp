// occ_rwstl.cpp — 1:1 CxxWrap bindings for RWStl: direct STL <->
// Poly_Triangulation I/O, more efficient than StlAPI_Reader/Writer for
// large meshes since it skips TopoDS_Shape/BRep_Builder entirely.
// OCCT 7.9.3 only offers the OSD_Path overload of WriteBinary/WriteAscii
// (the Standard_OStream& overload this file used to bind against is a
// new-in-8.0.0 addition, absent from 7.9.3's RWStl.hxx entirely) -- build
// an OSD_Path from the std::string path instead.
#include "occ_handle_traits.hpp"
#include <jlcxx/jlcxx.hpp>

#include <RWStl.hxx>
#include <Poly_Triangulation.hxx>
#include <OSD_Path.hxx>
#include <TCollection_AsciiString.hxx>
#include <string>

void register_occ_rwstl(jlcxx::Module& mod) {
  mod.method("RWStl_ReadFile", [](const std::string& path, double mergeAngle) -> Handle(Poly_Triangulation) {
    return RWStl::ReadFile(path.c_str(), mergeAngle);
  });
  mod.method("RWStl_WriteBinary", [](const Handle(Poly_Triangulation)& mesh, const std::string& path) -> bool {
    OSD_Path osdPath{TCollection_AsciiString(path.c_str())};
    return RWStl::WriteBinary(mesh, osdPath);
  });
  mod.method("RWStl_WriteAscii", [](const Handle(Poly_Triangulation)& mesh, const std::string& path) -> bool {
    OSD_Path osdPath{TCollection_AsciiString(path.c_str())};
    return RWStl::WriteAscii(mesh, osdPath);
  });
}
