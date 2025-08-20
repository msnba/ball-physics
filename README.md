# ball-physics

_An OpenGL physics simulation._
What I plan to do in this project:

- Simulate physics in OpenGl
- (Hopefully) Get to work with shaders
- Figure out how to use OpenGl

### Build Requirements

---

1. [A C++ Compiler](https://code.visualstudio.com/docs/languages/cpp#_install-a-compiler)
2. [CMake](https://cmake.org)
3. [Vcpkg](https://vcpkg.io/en)
4. [Git](https://git-scm.com)

### Build Instructions

---

(In Order)

1. Clone the repo:
   - `git clone https://github.com/msnba/ball-physics.git`
2. CD into the new directory:
   - `cd ./ball-physics`
3. Install dependencies:
   - `vcpkg install`
4. Get the cmake toolchain file path:
   - `vcpkg integrate install`
5. Configure project with CMake (using the file path from prev. step):
   - `cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake`
6. Build the project:
   - `cmake --build build`
