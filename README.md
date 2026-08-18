# OpenCascadeCxxWrap

BinaryBuilder recipe for **`libopencascade_cxxwrap`** — a strict 1:1 [CxxWrap](https://github.com/JuliaInterop/CxxWrap.jl)
module for the OpenCASCADE (OCCT) **modeling kernel** only.

This public repository is [`ahojukka5/OpenCascadeCxxWrap`](https://github.com/ahojukka5/OpenCascadeCxxWrap).
The root is `README.md`, `REGISTRY.md`, `build_tarballs.jl`, and `bundled/`.
There is no `OpenCascade.jl/` directory here, and this is not a monorepo
checkout. The Julia consumer is private **Monge.jl**.

## Split boundary (Netgen vs OpenCASCADE)

Historically, ~25 `occ_*.cpp` translation units lived inside
[`NetgenCxxWrap_jll`](https://github.com/ahojukka5/NetgenCxxWrap_jll)
alongside Netgen meshing bindings. That mixed two domains in one shared library.

| Stays in **NetgenCxxWrap_jll** | Lives in **this repo** |
|-------------------------------|-------------------------------------|
| `Mesh`, `Ngx_Mesh`, refinement, topology | `gp_*`, `TopoDS_*`, `BRep*API_*` |
| `LoadOCC_STEP/IGES/BREP` (Netgen nglib import) | `BRepTools_*`, `STEPControl_*`, `IGESControl_*` |
| `OCCGeometry_from_brep_string` (internal BREP → NetgenGeometry) | Fillets, sweeps, sewing, shape fix, Geom handles, … |
| `generate_mesh`, hp, snapshots | CAD modeling (Julia: private Monge.jl) |

**Interop:** Monge.jl serializes shapes with `to_brep_string`; the Netgen
wrapper imports via `occ_geometry_from_brep_string`. Do **not** pass
`TopoDS_Shape` across two CxxWrap shared libraries.

`NGSolveNetgen_jll` remains built with `USE_OCC=ON` because Netgen's mesher uses
OCC internally — that is unrelated to this Julia wrapper split.

## Dependencies

- `OCCT_jll`
- `libcxxwrap_julia_jll`
- **No** `NGSolveNetgen_jll`

## Local build

The BinaryBuilder recipe is `build_tarballs.jl` at this repository root:

```julia
julia build_tarballs.jl --deploy=local
```

There is no `OpenCascade.jl/gen/build_local.jl` in this tree. Private Monge.jl
builds or consumes `libopencascade_cxxwrap` on its side. Until a Yggdrasil JLL
exists, published Linux binaries live in
[`oodi-artifacts`](https://github.com/ahojukka5/oodi-artifacts).
