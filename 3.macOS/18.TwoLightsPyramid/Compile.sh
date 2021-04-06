mkdir -p TwoLightsPyramid.app/Contents/MacOS

Clang -o TwoLightsPyramid.app/Contents/MacOS/TwoLightsPyramid TwoLightsPyramid.mm Sphere.mm   -framework Cocoa -framework QuartzCore -framework OpenGL
