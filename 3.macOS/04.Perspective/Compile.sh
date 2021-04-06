mkdir -p Perspective.app/Contents/MacOS

Clang -o Perspective.app/Contents/MacOS/Perspective Perspective.mm -framework Cocoa -framework QuartzCore -framework OpenGL
