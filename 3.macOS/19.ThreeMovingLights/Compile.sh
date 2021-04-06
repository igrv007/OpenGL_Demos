mkdir -p ThreeMovingLights.app/Contents/MacOS

Clang -o ThreeMovingLights.app/Contents/MacOS/ThreeMovingLights ThreeMovingLights.mm Sphere.mm   -framework Cocoa -framework QuartzCore -framework OpenGL
