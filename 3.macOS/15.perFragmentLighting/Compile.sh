mkdir -p perFragmentLight.app/Contents/MacOS

Clang -o perFragmentLight.app/Contents/MacOS/perFragmentLight perFragmentLight.mm Sphere.mm   -framework Cocoa -framework QuartzCore -framework OpenGL
