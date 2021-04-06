mkdir -p tessellationShader.app/Contents/MacOS

Clang -o tessellationShader.app/Contents/MacOS/tessellationShader tessellationShader.mm    -framework Cocoa -framework QuartzCore -framework OpenGL
