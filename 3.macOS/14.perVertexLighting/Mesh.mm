#import "Mesh.h"

typedef enum : NSUInteger {
    VDG_ATTRIBUTE_VERTEX = 0,
    VDG_ATTRIBUTE_COLOR,
    VDG_ATTRIBUTE_NORMAL,
    VDG_ATTRIBUTE_TEXTURE0
} VDG_ATTRIBUTE;

@implementation Mesh


-(id)init
{
    self = [super init];
    numElements =0;
    maxElements =0;
    numVertices =0;
    vbo_position=0;
    vbo_normal=0;
    vbo_texture=0;
    vbo_index =0;
    vao =0;
    return self;
}

-(void)allocate:(int)numIndices
{
    int iNumIndices;
    
    [self cleanupMeshData];
    
    maxElements = numIndices;
    numElements =0 ;
    numVertices =0;
    
    iNumIndices = numIndices /3;
    
    elements = (short *) malloc(sizeof(short) * iNumIndices * 3 * 2 );
    verts = (float *) malloc(sizeof(float) *iNumIndices * 3 * 4);
    norms = (float *) malloc(sizeof(float) * iNumIndices * 3 * 4 );
    texCoords = (float *) malloc(sizeof(float) *iNumIndices * 2 * 4 );
}

-(void)addTriangle:(GLfloat [4][3])single_vertex
            Normal:(GLfloat [4][3])single_normal
           Texture:(GLfloat [4][2])single_texture
{
    const float diff  = 0.00001f;
    int i,j;
    
    //code
    //normals should be of unit length
    [self normalizeVector:single_normal[0]];
    [self normalizeVector:single_normal[1]];
    [self normalizeVector:single_normal[2]];
    
    for (i =0 ;i < 3; i++)
    {
        for (j=0 ; j < numVertices ; j++)
        {
            if([self isFoundIdentical:verts[j*3] Value2:single_vertex[i][0] difference:diff] &&
               [self isFoundIdentical:verts[(j*3) + 1] Value2:single_vertex[i][1] difference:diff] &&
               [self isFoundIdentical:verts[(j*3) + 2] Value2:single_vertex[i][2] difference:diff] &&
               
               [self isFoundIdentical:norms[j*3] Value2:single_normal[i][0] difference:diff] &&
               [self isFoundIdentical:norms[(j*3) + 1] Value2:single_normal[i][1] difference:diff] &&
               [self isFoundIdentical:norms[(j*3) + 2] Value2:single_normal[i][2] difference:diff] &&
               
               [self isFoundIdentical:texCoords[j*2] Value2:single_texture[i][0] difference:diff] &&
               [self isFoundIdentical:texCoords[(j*2) + 1] Value2:single_texture[i][1] difference:diff] )
            {
                elements[numElements] = j;
                numElements++;
                break;
            }
            
        }
        
        //if the single vertex ,normal and texture do not match with the givenn ,then add the corresponding traingle to the end of the list
        if (j == numVertices && numVertices < maxElements && numElements < maxElements)
        {
            verts[numVertices * 3] = single_vertex[i][0];
            verts[(numVertices * 3) + 1] = single_vertex[i][1];
            verts[(numVertices * 3) + 2] = single_vertex[i][2];
            
            norms[numVertices * 3] = single_normal[i][0];
            norms[(numVertices * 3) + 1] = single_normal[i][1];
            norms[(numVertices * 3) + 2] = single_normal[i][2];
            
            texCoords[numVertices * 2] = single_texture[i][0];
            texCoords[(numVertices * 2) + 1] = single_texture[i][1];
            
            elements[numElements] = numVertices; //adding the index to the end of the list of elements/indices
            numElements++;
            numVertices++;
        }
    }
}

-(void)prepareToDraw
{
    glGenVertexArrays(1,&vao);
    glBindVertexArray(vao);
    
    glGenBuffers(1,&vbo_position);
    glBindBuffer(GL_ARRAY_BUFFER,vbo_position);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(verts),
                 verts,
                 GL_STATIC_DRAW
                 );
    glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          0,
                          NULL);
    
    glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
    
    glBindBuffer(GL_ARRAY_BUFFER,0);
    
    
    //vbo for normals
    glGenBuffers(1,&vbo_normal);
    glBindBuffer(GL_ARRAY_BUFFER,vbo_normal);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(norms),
                 norms,
                 GL_STATIC_DRAW
                 );
    glVertexAttribPointer(VDG_ATTRIBUTE_NORMAL,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          0,
                          NULL);
    glEnableVertexAttribArray(VDG_ATTRIBUTE_NORMAL);
    
    glBindBuffer(GL_ARRAY_BUFFER,0);
    
    //vbo for texture
    glGenBuffers(1,&vbo_texture);
    glBindBuffer(GL_ARRAY_BUFFER,vbo_texture);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(texCoords),
                 texCoords,
                 GL_STATIC_DRAW
                 );
    glVertexAttribPointer(VDG_ATTRIBUTE_TEXTURE0,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          0,
                          NULL);
    glEnableVertexAttribArray(VDG_ATTRIBUTE_TEXTURE0);
    
    glBindBuffer(GL_ARRAY_BUFFER,0);
    
    //vbo for index
    glGenBuffers(1,&vbo_index);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vbo_index);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(elements),
                 elements,
                 GL_STATIC_DRAW
                 );
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    
    glBindVertexArray(0);
    
    //after sending data to gpu , now we can free our arrays
    [self cleanupMeshData];
    
}

-(void)draw
{
    //code
    //bind vao
    glBindVertexArray(vao);
    //draw
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vbo_index);
    glDrawElements(GL_TRIANGLES,numElements,GL_UNSIGNED_SHORT,0);
    
    glBindVertexArray(0);
}
-(int)getIndexCount
{
    return (numElements);
}

-(int)getVertexCount
{
    return(numVertices);
}
-(BOOL)isFoundIdentical:(GLfloat)val1 Value2:(GLfloat)val2 difference:(GLfloat)diff
{
    //code
    if(fabsf(val1 - val2) < diff)
        return YES;
    else
        return NO;
    
}
-(void)cleanupMeshData
{
    //code
    if(elements != nil)
    {
        elements = nil;
    }
    
    if(verts != nil)
    {
        verts = nil;
    }
    
    if(norms != nil)
    {
        norms = nil;
    }
    
    if(texCoords != nil)
    {
        texCoords = nil;
    }
    
}

-(void)normalizeVector:(GLfloat[3])v
{
    float squareVectorLength = (v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]);
    
    float squareRootOfSquareVectorLength = sqrt(squareVectorLength);
    
    v[0] = v[0] * 1.0/squareRootOfSquareVectorLength;
    v[1] = v[1] * 1.0/squareRootOfSquareVectorLength;
    v[2] = v[2] * 1.0/squareRootOfSquareVectorLength;
}

-(void)deallocate
{
    if(vao)
    {
        glDeleteVertexArrays(1, &vao);
        vao=0;
    }
    if(vbo_index)
    {
        glDeleteBuffers(1, &vbo_index);
        vbo_index = 0;
    }
    if(vbo_texture)
    {
        glDeleteBuffers(1, &vbo_texture);
        vbo_texture = 0;
    }
    if(vbo_normal)
    {
        glDeleteBuffers(1, &vbo_normal);
        vbo_normal = 0;
    }
    if(vbo_position)
    {
        glDeleteBuffers(1, &vbo_position);
        vbo_position = 0;
    }
}

@end

