mkdir -p perVerNFragLight.app/Contents/MacOS

Clang -o perVerNFragLight.app/Contents/MacOS/perVerNFragLight perVerNFragLight.mm Sphere.mm   -framework Cocoa -framework QuartzCore -framework OpenGL
