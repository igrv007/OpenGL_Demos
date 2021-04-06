mkdir -p ColorTriNSqu.app/Contents/MacOS

Clang -o ColorTriNSqu.app/Contents/MacOS/ColorTriNSqu ColorTriNSqu.mm -framework Cocoa -framework QuartzCore -framework OpenGL
