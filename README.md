# A modular video game modding library

This is a set of commonly used utilities in video game modifications, such as a pattern scanner as well as a high performance vector math library. Some game independant features, such as aimbot are also in this library (does not support physics bullets yet).

This library is intended to be used as the core module of any project project, wrapped in a layer of game engine specific functions and data structures, then wrapped in a game specific implementation (which provides initialization, shutdown, data preparation and execution of features). This way the least code is duplicated, and the most of it is reused. The library was primarilly focused around internal hacks, with direct access to pointer dereferencing, but some tools can be used externally (provided correct memory access functions are implemented by other layers).

Another great focus of the library was efficient data layout (data oriented design), hence SoA style vector structs, SoA style player data structure, etc. This allows for good cache performance and more efficient vectorization (not having to loose 4th value in a SSE register when dealing with 3D vectors and being able to scale up even to AVX512). Out of these 3 compilers tested: LLVM (together with Apple's LLVM), GCC and MSVC, LLVM seems to do the best job at auto vectorization and is naturally the recommended compiler to be used.

##### TODO
- Make the HistoryList structure use standard naming
