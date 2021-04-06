mkdir -p InterleavedArray.app/Contents/MacOS

Clang -o InterleavedArray.app/Contents/MacOS/InterleavedArray InterleavedArray.mm -framework Cocoa -framework QuartzCore -framework OpenGL
