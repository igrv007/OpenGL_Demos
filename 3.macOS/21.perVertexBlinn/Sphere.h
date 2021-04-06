
@interface Sphere : NSObject
{
    int maxElements;
    int numElements;
    int numVertices;
    float model_vertices[1146];
    float model_normals[1146];
    float model_textures[764];
    float model_elements[2280];
}
- (id)init;

- (void)getSphereVertexDataWithPositionCoords:(float [])spherePositionCoords
                                 NormalCoords:(float [])sphereNormalCoords
                                TextureCoords:(float [])sphereTexCoords
                                     Elements:(short [])sphereElements
;

- (int) getNumberOfSphereVertices;

- (int) getNumberOfSphereElements;
@end
