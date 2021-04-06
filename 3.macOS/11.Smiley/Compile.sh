mkdir -p Smiley.app/Contents/MacOS

Clang -o Smiley.app/Contents/MacOS/Smiley Smiley.mm -framework Cocoa -framework QuartzCore -framework OpenGL
