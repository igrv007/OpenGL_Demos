mkdir -p SmileyOnKeys.app/Contents/MacOS

Clang -o SmileyOnKeys.app/Contents/MacOS/SmileyOnKeys SmileyOnKeys.mm -framework Cocoa -framework QuartzCore -framework OpenGL
