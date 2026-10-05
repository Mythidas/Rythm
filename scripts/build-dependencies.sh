cmake -S vendor/sdl -B vendor/sdl/build \
    -DCMAKE_BUILD_TYPE=Debug \
    -DSDL_SHARED=OFF \
    -DSDL_STATIC=ON

cmake --build vendor/sdl/build -j

cmake -S vendor/ktx/lib -B vendor/ktx/lib/build \
    -DBUILD_SHARED_LIBS=OFF

cmake --build vendor/ktx/lib/build -j

cmake -S vendor/spdlog -B vendor/spdlog/build
cmake --build vendor/spdlog/build -j
