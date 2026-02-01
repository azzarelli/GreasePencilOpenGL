# OpenGLPlayGround

Working my way through the tutorials on 
https://www.opengl-tutorial.org/
until I can grease pencil some meshes.


## Commits correspond each tutorial + extra work I did for fun:
1. "Tutorial 1": Operating a window
2. "Tutorial 2": 2-D Triangle rendering
3. "Tutorial 3": Intrinsic/Extrinsic/Model Transforms
4. "Tutorial 4 + ..": Rendering a cube. Extra work: re-shading the same cube 2x with different Model transforms (multi-instance from single object/modelMatrix)
4. "Moving Cube from -1 < x < 1 ": Moving a cube from the -1 x and +1 x positions, each frame
5. "Tutorial 5": Loading BMP and DSS files for texturing vertices
6. "Tutorial 6 + more funcs": Implemented mouse and keyboard controls and updated the scroll functionality for compatibility with glfw3. Also added some buttons for other motion characteristics
7. "Control with moving object": Added the moving cube from commit #4 to make it more exciting.
8. "Tutorial 7": Implementing a really simple .obj loader for vertex and texture shader
9. "Tutorial 8": Implementing a very basic shader + fixed some code bugs from the original tutorial
10. "Tutorial 9": Constructing and Using an Index Buffer for VAO
11. "Tutorial 10": Adding transparency by using alphablending on fixed material alpha = 0.3
12. "T10+ViewDependentAlpha": Extended shader to make triangles parallel to camera -> using Cos()
13. "T10+ViewDepHighGloss": Using smoothstep, we interpollate 
This essentially interpollates normalizes and skeys the angle between a triangle and the fragment view, between pre-determined "full" and peak "drop-off" angles. E.g. only triangles with dot(tri_norm, cam_dir) < 20Deg will be rendered (with alpha>0) and triangle-fragments with < 5Deg will be alpha=1.
# GreasePencilOpenGL
