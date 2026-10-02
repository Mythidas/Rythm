#!/bin/bash

./vendor/premake/premake5 gmake
bear -- make config=debug verbose=1

export LD_LIBRARY_PATH="$VULKAN_SDK/lib:$LD_LIBRARY_PATH"
gdb -ex run -ex bt ./bin/Debug-linux-x86_64/Rythm/Rythm
