mkdir -p 3DRotation.app/Contents/MacOS

Clang -o 3DRotation.app/Contents/MacOS/3DRotation 3DRotation.mm -framework Cocoa -framework QuartzCore -framework OpenGL
