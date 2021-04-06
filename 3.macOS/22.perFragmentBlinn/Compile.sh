mkdir -p perFragmentBlinn.app/Contents/MacOS

Clang -o perFragmentBlinn.app/Contents/MacOS/perFragmentBlinn perFragmentBlinn.mm Sphere.mm   -framework Cocoa -framework QuartzCore -framework OpenGL
