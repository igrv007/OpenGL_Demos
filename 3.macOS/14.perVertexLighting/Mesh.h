#import <OpenGL/gl3.h>
#import <OpenGL/gl3ext.h>

#import <Foundation/Foundation.h>
#include <math.h>

@interface Mesh : NSObject
{
     short *elements;
    GLfloat *verts;
    GLfloat *norms;
    GLfloat *texCoords;
    int numElements;
    int maxElements;
    int numVertices;
    
    GLuint vbo_position,vbo_normal,vbo_texture,vbo_index,vao;
}

- (id)init;

- (void)allocate:(int)numIndices;

- (void)addTriangle:(GLfloat [4][3])single_vertex
             Normal:(GLfloat [4][3])single_normal
            Texture:(GLfloat [4][2])single_texture;

- (void)prepareToDraw;

- (void)draw;

- (int)getIndexCount;

- (int)getVertexCount;

- (BOOL)isFoundIdentical:(GLfloat)val1
                  Value2:(GLfloat)val2
              difference:(GLfloat)diff;

- (void)cleanupMeshData;

-(void)normalizeVector:(GLfloat *)v;

- (void) deallocate;

@end
