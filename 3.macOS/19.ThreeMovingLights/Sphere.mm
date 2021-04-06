#import <OpenGL/gl3.h>
#import <OpenGL/gl3ext.h>

#import <Foundation/Foundation.h>
#import "Sphere.h"
#import "Vertices.h"

@implementation Sphere

- (id)init
{
    self = [super init];
    numElements =0 ;
    maxElements =0 ;
    numVertices =0;
    return  self;
}

- (void) getSphereVertexDataWithPositionCoords:(float [])spherePositionCoords
                                  NormalCoords:(float [])sphereNormalCoords
                                 TextureCoords:(float [])sphereTexCoords
                                      Elements:(short [])sphereElements
{
    for (int i = 0 ; i < 1146 ; i++)
    {
        model_vertices[i] = spherePositionCoords[i];
    }
    for (int i = 0 ; i < 1146 ; i++)
    {
        model_normals[i] = sphereNormalCoords[i];
    }
    for (int i = 0 ; i < 764 ; i++)
    {
        model_textures[i] = sphereTexCoords[i];
    }
    for (int i = 0 ; i < 2280 ; i++)
    {
        model_elements[i] = sphereElements[i];
    }
    
    //process sphere's data using sphere header file
    [self processSphereData];
    
    // return processed data to the user by filling his empty array by our filled arrays
    for (int i = 0; i < 1146; i++)
    {
        spherePositionCoords[i] = model_vertices[i];
    }
    
    for (int i = 0; i < 1146; i++)
    {
        sphereNormalCoords[i] = model_normals[i];
    }
    
    for (int i = 0; i < 764; i++)
    {
        sphereTexCoords[i] = model_textures[i];
    }
    
    for (int i = 0; i < 2280; i++)
    {
        sphereElements[i] = model_elements[i];
    }
    
}

- (int) getNumberOfSphereVertices
{
    return numVertices;
}

- (int) getNumberOfSphereElements
{
    return  numElements;
}

- (void) processSphereData
{
    int numIndices = 760;
    
    maxElements = numIndices * 3;
    
    float vert[3][3];
    float norm[3][3];
    float tex[3][2];
    
    for (int i = 0; i < numIndices; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            vert[j][0] = vertices[indices[i][j]][0];
            vert[j][1] = vertices[indices[i][j]][1];
            vert[j][2] = vertices[indices[i][j]][2];
            
            norm[j][0] = normals[indices[i][j + 3]][0];
            norm[j][1] = normals[indices[i][j + 3]][1];
            norm[j][2] = normals[indices[i][j + 3]][2];
            
            tex[j][0] = textures[indices[i][j + 6]][0];
            tex[j][1] = textures[indices[i][j + 6]][1];
        }
       [self addTriangleWithVertex:vert normal:norm texture:tex];
    }
}

- (void)addTriangleWithVertex:(float [3][3])single_vertex
                       normal:(float [3][3])single_normal
                      texture:(float [3][2])single_texture
{
    const float diff = 0.00001f;
    int i,j;
    
    //normals should be unit of length
    [self normalizeVector:single_normal[0]];
    [self normalizeVector:single_normal[1]];
    [self normalizeVector:single_normal[2]];
    
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < numVertices; j++) //for the first ever iteration of 'j', numVertices will be 0 because of it's initialization in the parameterized constructor
        {
            if ([self isFoundIdentical:model_vertices[j * 3] Value2:single_vertex[i][0] difference:diff] &&
                [self isFoundIdentical:model_vertices[(j * 3) + 1] Value2:single_vertex[i][1] difference:diff] &&
                [self isFoundIdentical:model_vertices[(j * 3) + 2] Value2:single_vertex[i][2] difference:diff] &&
                
                [self isFoundIdentical:model_normals[j * 3] Value2:single_normal[i][0] difference:diff] &&
                [self isFoundIdentical:model_normals[(j * 3) + 1] Value2:single_normal[i][1] difference:diff] &&
                [self isFoundIdentical:model_normals[(j * 3) + 2] Value2:single_normal[i][2] difference:diff] &&
                
                [self isFoundIdentical:model_textures[j * 2] Value2:single_texture[i][0] difference:diff] &&
                [self isFoundIdentical:model_textures[(j * 2) + 1] Value2:single_texture[i][1] difference:diff] )
                
            {
                model_elements[numElements] = (short)j;
                numElements++;
                break;
            }
        }
        
        //If the single vertex, normal and texture do not match with the given, then add the corressponding triangle to the end of the list
        if (j == numVertices && numVertices < maxElements && numElements < maxElements)
        {
            model_vertices[numVertices * 3] = single_vertex[i][0];
            model_vertices[(numVertices * 3) + 1] = single_vertex[i][1];
            model_vertices[(numVertices * 3) + 2] = single_vertex[i][2];
            
            model_normals[numVertices * 3] = single_normal[i][0];
            model_normals[(numVertices * 3) + 1] = single_normal[i][1];
            model_normals[(numVertices * 3) + 2] = single_normal[i][2];
            
            model_textures[numVertices * 2] = single_texture[i][0];
            model_textures[(numVertices * 2) + 1] = single_texture[i][1];
            
            model_elements[numElements] = (short)numVertices; //adding the index to the end of the list of elements/indices
            numElements++; //incrementing the 'end' of the list
            numVertices++; //incrementing coun of vertices
        }
    }
}

-(BOOL)isFoundIdentical:(float)val1 Value2:(float)val2 difference:(float)diff
{
    //code
                if(fabsf(val1 - val2) < diff)
                    return YES;
                else
                    return NO;
                
}

- (void) normalizeVector:(float [])v
{
    // code
    
    // square the vector length
    float squaredVectorLength=(v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]);
    
    // get square root of above 'squared vector length'
    float squareRootOfSquaredVectorLength=(float)sqrt(squaredVectorLength);
    
    // scale the vector with 1/squareRootOfSquaredVectorLength
    v[0] = v[0] * 1.0f/squareRootOfSquaredVectorLength;
    v[1] = v[1] * 1.0f/squareRootOfSquaredVectorLength;
    v[2] = v[2] * 1.0f/squareRootOfSquaredVectorLength;
}










@end
