mkdir -p 2DRotation.app/Contents/MacOS

Clang -o 2DRotation.app/Contents/MacOS/2DRotation 2DRotation.mm -framework Cocoa -framework QuartzCore -framework OpenGL
