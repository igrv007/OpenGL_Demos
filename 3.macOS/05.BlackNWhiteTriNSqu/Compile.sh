mkdir -p BlackNWhite.app/Contents/MacOS

Clang -o BlackNWhite.app/Contents/MacOS/BlackNWhite BlackNWhite.mm -framework Cocoa -framework QuartzCore -framework OpenGL
