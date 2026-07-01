// opencascade.cpp — entry point for the OpenCASCADE CxxWrap module.
// Each register_* function is defined in its own translation unit.
#include <jlcxx/jlcxx.hpp>

void register_occ_toptools_list(jlcxx::Module&);
void register_occ_gp(jlcxx::Module&);
void register_occ_topology(jlcxx::Module&);
void register_occ_geom(jlcxx::Module&);
void register_occ_builders(jlcxx::Module&);
void register_occ_io(jlcxx::Module&);
void register_occ_props(jlcxx::Module&);
void register_occ_fillet(jlcxx::Module&);
void register_occ_topo2(jlcxx::Module&);
void register_occ_adaptor(jlcxx::Module&);
void register_occ_analysis(jlcxx::Module&);
void register_occ_sweep(jlcxx::Module&);
void register_occ_wire(jlcxx::Module&);
void register_occ_sewing(jlcxx::Module&);
void register_occ_extrema(jlcxx::Module&);
void register_occ_builder2(jlcxx::Module&);
void register_occ_intersect(jlcxx::Module&);
void register_occ_shape_analysis(jlcxx::Module&);
void register_occ_breptools(jlcxx::Module&);
void register_occ_mesh(jlcxx::Module&);
void register_occ_shapefix2(jlcxx::Module&);
void register_occ_shapeupgrade(jlcxx::Module&);
void register_occ_feat(jlcxx::Module&);
void register_occ_thicksolid(jlcxx::Module&);
void register_occ_draft(jlcxx::Module&);
void register_occ_breplib(jlcxx::Module&);

JLCXX_MODULE define_julia_module(jlcxx::Module& mod)
{
  register_occ_gp(mod);
  register_occ_topology(mod);
  register_occ_toptools_list(mod);
  register_occ_geom(mod);
  register_occ_builders(mod);
  register_occ_io(mod);
  register_occ_props(mod);
  register_occ_fillet(mod);
  register_occ_topo2(mod);
  register_occ_adaptor(mod);
  register_occ_analysis(mod);
  register_occ_sweep(mod);
  register_occ_wire(mod);
  register_occ_sewing(mod);
  register_occ_extrema(mod);
  register_occ_builder2(mod);
  register_occ_intersect(mod);
  register_occ_shape_analysis(mod);
  register_occ_breptools(mod);
  register_occ_mesh(mod);
  register_occ_shapefix2(mod);
  register_occ_shapeupgrade(mod);
  register_occ_feat(mod);
  register_occ_thicksolid(mod);
  register_occ_draft(mod);
  register_occ_breplib(mod);
}
