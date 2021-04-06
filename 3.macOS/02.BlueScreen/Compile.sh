mkdir -p BlueScreen.app/Contents/MacOS

Clang -o BlueScreen.app/Contents/MacOS/BlueScreen BlueScreen.mm -framework Cocoa -framework QuartzCore -framework OpenGL
