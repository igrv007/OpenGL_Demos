mkdir -p KundaliNStone.app/Contents/MacOS

Clang -o KundaliNStone.app/Contents/MacOS/KundaliNStone KundaliNStone.mm -framework Cocoa -framework QuartzCore -framework OpenGL
