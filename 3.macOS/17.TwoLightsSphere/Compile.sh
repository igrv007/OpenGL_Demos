mkdir -p TwoLightsSphere.app/Contents/MacOS

Clang -o TwoLightsSphere.app/Contents/MacOS/TwoLightsSphere TwoLightsSphere.mm Sphere.mm   -framework Cocoa -framework QuartzCore -framework OpenGL
