# OpenCascadeCxxWrap_jll

BinaryBuilder recipe for **`libopencascade_cxxwrap`** — a strict 1:1 [CxxWrap](https://github.com/JuliaInterop/CxxWrap.jl)
module for the OpenCASCADE (OCCT) **modeling kernel** only.

## Split boundary (Netgen vs OpenCASCADE)

Historically, ~25 `occ_*.cpp` translation units lived inside `NetgenCxxWrap_jll`
alongside Netgen meshing bindings. That mixed two domains in one shared library.

| Stays in **NetgenCxxWrap_jll** | Lives in **OpenCascadeCxxWrap_jll** |
|-------------------------------|-------------------------------------|
| `Mesh`, `Ngx_Mesh`, refinement, topology | `gp_*`, `TopoDS_*`, `BRep*API_*` |
| `LoadOCC_STEP/IGES/BREP` (Netgen nglib import) | `BRepTools_*`, `STEPControl_*`, `IGESControl_*` |
| `OCCGeometry_from_brep_string` (internal BREP → NetgenGeometry) | Fillets, sweeps, sewing, shape fix, Geom handles, … |
| `generate_mesh`, hp, snapshots (Julia: Netgen.jl) | CAD modeling (Julia: OpenCascade.jl) |

**Interop:** `OpenCascade.jl` serializes shapes with `to_brep_string`; `Netgen.jl`
imports via `occ_geometry_from_brep_string`. Do **not** pass `TopoDS_Shape` across
two CxxWrap shared libraries.

`NGSolveNetgen_jll` remains built with `USE_OCC=ON` because Netgen's mesher uses
OCC internally — that is unrelated to this Julia wrapper split.

## Dependencies

- `OCCT_jll`
- `libcxxwrap_julia_jll`
- **No** `NGSolveNetgen_jll`

## Local build

```julia
julia --project=OpenCascade.jl OpenCascade.jl/gen/build_local.jl
```

See [`OpenCascade.jl/README.md`](../OpenCascade.jl/README.md).
