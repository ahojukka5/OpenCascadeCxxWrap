# Registry layout (once packages are registered in General):

```
OCCT_jll
├── OpenCascadeCxxWrap_jll  →  OpenCascade.jl  (deps: OpenCascadeCxxWrap_jll, OCCT_jll, CxxWrap)
└── NGSolveNetgen_jll        →  NetgenCxxWrap_jll  →  Netgen.jl
```

OpenCascade.jl does **not** depend on Netgen.jl. Users compose at the BREP boundary.

## Local monorepo

| Package | Build |
|---------|-------|
| `OpenCascadeCxxWrap_jll` | `julia --project=OpenCascade.jl OpenCascade.jl/gen/build_local.jl` |
| `NetgenCxxWrap_jll` | `julia --project=Netgen.jl Netgen.jl/gen/build_local.jl` |

BinaryBuilder (when ready):

```julia
julia OpenCascadeCxxWrap_jll/build_tarballs.jl --deploy=local
julia NetgenCxxWrap_jll/build_tarballs.jl --deploy=local
```

After registration, point `OpenCascade.jl/Artifacts.toml` and `Netgen.jl/Artifacts.toml`
at the published JLL versions instead of local `gen/build_local.jl` hashes.
