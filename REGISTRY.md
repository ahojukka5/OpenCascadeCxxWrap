# Registry layout (once packages are registered in General)

```
OCCT_jll
├── OpenCascadeCxxWrap_jll  →  Monge.jl  (private; deps: OpenCascadeCxxWrap_jll, OCCT_jll, CxxWrap)
└── NGSolveNetgen_jll        →  NetgenCxxWrap_jll
```

This repository is the public [`OpenCascadeCxxWrap`](https://github.com/ahojukka5/OpenCascadeCxxWrap)
source and BinaryBuilder recipe. It is not a monorepo and does not contain
`OpenCascade.jl/`. Monge.jl does **not** depend on the Netgen wrapper. Users
compose at the BREP boundary.

## Local build

The recipe is `build_tarballs.jl` at this repository root, not
`OpenCascadeCxxWrap_jll/build_tarballs.jl`:

```julia
julia build_tarballs.jl --deploy=local
```

Until a Yggdrasil / General JLL exists,
[`oodi-artifacts`](https://github.com/ahojukka5/oodi-artifacts) hosts the
temporary `libopencascade_cxxwrap` release assets. After registration, point
Monge.jl's `Artifacts.toml` at the published JLL version instead of a local or
`oodi-artifacts` download.
