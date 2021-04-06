mkdir -p perVertexLight.app/Contents/MacOS

Clang -o perVertexLight.app/Contents/MacOS/perVertexLight perVertexLight.mm Sphere.mm Mesh.mm  -framework Cocoa -framework QuartzCore -framework OpenGL
