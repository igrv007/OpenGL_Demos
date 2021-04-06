mkdir -p MultiColorTri.app/Contents/MacOS

Clang -o MultiColorTri.app/Contents/MacOS/MultiColorTri MultiColorTri.mm -framework Cocoa -framework QuartzCore -framework OpenGL
