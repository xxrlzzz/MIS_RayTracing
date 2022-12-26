
add_executable(RayTracing main.cpp Object.hpp Vector.cpp Vector.hpp Sphere.hpp global.hpp Triangle.hpp Scene.cpp
    Scene.hpp Light.hpp BVH.cpp BVH.hpp Bounds3.hpp Ray.hpp Material.hpp Material.cpp Intersection.hpp VirtualScreen.cpp VirtualScreen.hpp
    Renderer.cpp Renderer.hpp Profiler.h global.cpp)


if(SFML_OS_WINDOWS AND SFML_COMPILER_MSVC)
    find_package(SFML 2 COMPONENTS main audio graphics window system REQUIRED)
else()
    find_package(SFML 2 COMPONENTS audio graphics window system REQUIRED)
endif()

if(SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
else()
    set(SFML_ROOT "" CACHE PATH "SFML top-level directory")
    message("\nSFML directory not found. Set SFML_ROOT to SFML's top-level path (containing \"include\" and \"lib\" directories).")
    message("Make sure the SFML libraries with the same configuration (Release/Debug, Static/Dynamic) exist.\n")
endif()

target_link_libraries(RayTracing ${SFML_LIBRARIES} ${SFML_DEPENDENCIES})
