cat << EOF > .clangd
CompileFlags:
  Add:
    - "-Wall"
    - "-Wextra"
    - "-I$(realpath LPC1769)"
    - "-I$(realpath include)"
    - "-ferror-limit=0"

Diagnostics:
  UnusedIncludes: Strict
EOF

mkdir -p include
