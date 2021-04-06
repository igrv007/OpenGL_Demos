mkdir -p 24_Spheres.app/Contents/MacOS

Clang -o 24_Spheres.app/Contents/MacOS/24_Spheres 24_Spheres.mm Sphere.mm   -framework Cocoa -framework QuartzCore -framework OpenGL
