cmake -S vendor/sdl -B vendor/sdl/build \
    -DCMAKE_BUILD_TYPE=Debug \
    -DSDL_SHARED=OFF \
    -DSDL_STATIC=ON

cmake --build vendor/sdl/build -j
