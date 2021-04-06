mkdir -p geometryShader.app/Contents/MacOS

Clang -o geometryShader.app/Contents/MacOS/geometryShader geometryShader.mm -framework Cocoa -framework QuartzCore -framework OpenGL
