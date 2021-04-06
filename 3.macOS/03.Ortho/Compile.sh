mkdir -p Ortho.app/Contents/MacOS

Clang -o Ortho.app/Contents/MacOS/Ortho Ortho.mm -framework Cocoa -framework QuartzCore -framework OpenGL
