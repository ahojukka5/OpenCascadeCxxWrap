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

- `OCCT_jll` 7.9.3
- `libcxxwrap_julia_jll`
- **No** `NGSolveNetgen_jll`

## OCCT 8.0.1 local build

The published consumer contract remains `OCCT_jll` 7.9.3. A local CMake build
can instead use the OCCT 8.0.1 archives in
[`OodiArtifacts`](https://github.com/ahojukka5/OodiArtifacts). Pass that
prefix's `lib/cmake/opencascade` as `OpenCASCADE_DIR`, pass the Julia
executable and `JlCxx_DIR` explicitly, and set
`OPENCASCADE_CXXWRAP_RECORD_OCCT_RPATH=ON`. The rpath option stays off in
`build_tarballs.jl`, so a 7.9.3 package build does not embed a host prefix.

Those OCCT 8.0.1 archives do not install `RWGltf_CafWriter`. CMake detects
that from the configured prefix only, the glTF writer binding stays unloaded,
and `OCCT_RWGltfCafWriterBound` returns false. An OCCT 7.9.3 prefix installs
the header and keeps the writer. This local build does not publish a JLL and
does not replace the 7.9.3 release.

`scripts/occt8_smoke.jl` loads the built library through CxxWrap and runs a
box, a volume, a Boolean, a validity check, and a STEP round trip. A
successful `dlopen` is not the result.

## Local build

The BinaryBuilder recipe is `build_tarballs.jl` at this repository root:

```julia
julia build_tarballs.jl --deploy=local
```

There is no `OpenCascade.jl/gen/build_local.jl` in this tree. Private Monge.jl
builds or consumes `libopencascade_cxxwrap` on its side. Until a Yggdrasil JLL
exists, published Linux binaries live in
[`oodi-artifacts`](https://github.com/ahojukka5/oodi-artifacts).
