cmake --build ./../build

# add program to PATH -> path should be in build/bin
export PATH=$PATH:$(realpath ../build/bin)

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    echo "If Nova is not in PATH, run sourced."
fi