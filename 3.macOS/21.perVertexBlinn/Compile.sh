mkdir -p perVertexBlinn.app/Contents/MacOS

Clang -o perVertexBlinn.app/Contents/MacOS/perVertexBlinn perVertexBlinn.mm Sphere.mm   -framework Cocoa -framework QuartzCore -framework OpenGL
