//headers
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#import <QuartzCore/CVDisplayLink.h>

#import <OpenGL/gl3.h>
#import <OpenGL/gl3ext.h>
#import "vmath.h"

#import "Sphere.h"
#define STEP_ANGLE 1.0f
enum
{
    VDG_ATTRIBUTE_VERTEX = 0,
    VDG_ATTRIBUTE_COLOR,
    VDG_ATTRIBUTE_NORMAL,
    VDG_ATTRIBUTE_TEXTURE0,
};

GLfloat light_ambient[] = {0.0f,0.0f,0.0f,1.0f};
GLfloat light_diffuse[] = {1.0f,1.0f,1.0f,1.0f};
GLfloat light_specular[] = {1.0f,1.0f,1.0f,1.0f};
GLfloat light_position[] = {100.0f,100.0f,100.0f,1.0f};

GLfloat material_ambient[]  = {0.0f ,0.0f,0.0f,1.0f};
GLfloat material_diffuse[] ={1.0f,1.0f,1.0f,1.0f};
GLfloat material_specular[] = {1.0f,1.0f,1.0f,1.0f};
GLfloat material_shininess = 50.0f;
//'C' style gloable function
CVReturn MyDisplayLinkCallback(CVDisplayLinkRef ,const CVTimeStamp * ,
	const CVTimeStamp * ,CVOptionFlags,CVOptionFlags *, void *);

//global varables
FILE *gpFile = NULL;

//interface declarations
@interface AppDelegate : NSObject <NSApplicationDelegate , NSWindowDelegate>
@end


@interface GLView : NSOpenGLView
@end

//entry point function
int main(int argc, char const *argv[])
{
	/* code */
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc]init];

	NSApp = [NSApplication sharedApplication];

	[NSApp setDelegate:[[AppDelegate alloc]init]];

	[NSApp run];

	[pPool release];

	return (0);
}

//interface implementation
@implementation AppDelegate
{
@private 
	NSWindow *window;
	GLView *glView;

}

-(void)applicationDidFinishLaunching:(NSNotification * )aNotification
{
	//code
	//log file
    NSBundle *mainBundle = [NSBundle mainBundle];
    NSString *appDirName = [mainBundle bundlePath];
    NSString *parentDirPath =[appDirName stringByDeletingLastPathComponent];
    NSString *logFileNameWithPath = [NSString stringWithFormat:@"%@/Log.txt",parentDirPath];
    const char *pszLogFileNameWithPath = [logFileNameWithPath cStringUsingEncoding:NSASCIIStringEncoding];
    gpFile = fopen(pszLogFileNameWithPath,"w");
    if(gpFile == NULL)
    {
        printf("Failed to create log file:\n");
        [self release];
        [NSApp terminate:self];
        
    }
    fprintf(gpFile,"Program is started Successfully\n");
    
    //window
    NSRect win_rect;
    win_rect = NSMakeRect(0.0,0.0,800.0,600.0);
    
    //create simple window
    window =[[NSWindow alloc] initWithContentRect:win_rect styleMask:NSWindowStyleMaskTitled |
                                                NSWindowStyleMaskClosable |
                                                NSWindowStyleMaskMiniaturizable |
                                                NSWindowStyleMaskResizable backing:NSBackingStoreBuffered defer:NO];
    [window setTitle:@"macOS per Fragment Lighting"];
    [window center];
    
    glView = [[GLView alloc] initWithFrame:win_rect];
    
    [window setContentView:glView];
    [window setDelegate:self];
    [window makeKeyAndOrderFront:self];
    
}

-(void)applicationWillTerminate:(NSNotification *)notification
{
    //code
    fprintf(gpFile,"Program is Terminated succesfully\n");
    
    if(gpFile)
    {
        fclose(gpFile);
        gpFile = NULL;
    }
}

-(void)windowWillClose:(NSNotification *)notification
{
    //code
    [NSApp terminate:self];
    
}

-(void)dealloc
{
    //code
    [glView release];
    
    [window release];
    
    [super dealloc];
}

@end

@implementation GLView
{
    @private
    CVDisplayLinkRef displayLink;
    
    GLuint vertexShaderObject;
    GLuint fragmentShaderObject;
    GLuint shaderProgramObject;
    GLuint perVertexShaderProgramObject;
    GLuint perFragmentShaderProgramObject;
    
    GLuint vao_sphere;
    GLuint vbo_sphere_position;
    GLuint vbo_sphere_normal;
    GLuint vbo_sphere_element;
    
    
    
    //int singleTap;
    //int doubleTap;
    int lKeyPressed;
    int fKeyPressed;
    int vKeyPressed;
    int numVertices;
    int numElements;
    GLfloat angle;
    
    GLint modelMatrixUniform,viewMatrixUniform,projectionMatrixUniform;
    GLint laUniform,lsUniform,ldUniform,lightPositionUniform;
    GLint kaUniform,kdUniform,ksUniform,materialShininessUniform;
    
    int lKeyPressedUniform;
    
    
    
    
    vmath::mat4 perspectiveProjectionMatrix;
}

-(id)initWithFrame:(NSRect)frame
{
    //code
    self = [super initWithFrame:frame];
    
    if(self)
    {
        [[self window]setContentView:self];
     NSOpenGLPixelFormatAttribute attrs [] =
        {
        //Must specify the 4.1 Core profile to use OpenGL4.1
          NSOpenGLPFAOpenGLProfile,
          NSOpenGLProfileVersion4_1Core,
          //specify the display ID to associate the GL context with (main display for now)
            NSOpenGLPFAScreenMask,CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay),
            NSOpenGLPFANoRecovery,
            NSOpenGLPFAAccelerated, //for hardware acceleration
            NSOpenGLPFAColorSize,24,
            NSOpenGLPFADepthSize,24,
            NSOpenGLPFAAlphaSize,8,
            NSOpenGLPFADoubleBuffer,
            0 //to terminate an array last parameter must be 0
        };
        NSOpenGLPixelFormat *pixelFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attrs] autorelease];//last autorelease is to tell that release pixelFormat from autoReleasePool
        if(pixelFormat == nil)
        {
            fprintf(gpFile,"No valid OpenGL Pixel Format is Available....\n");
            [self release];
            [NSApp terminate:self];
        }
        
        NSOpenGLContext *glContext = [[[NSOpenGLContext alloc] initWithFormat:pixelFormat
                                                                 shareContext:nil] autorelease];
        [self setPixelFormat:pixelFormat];
        
        [self setOpenGLContext:glContext]; //it automatically releases the older context if present
                                           //and sets the newer one
        
    }
    return(self);
}
-(CVReturn)getFrameForTime:(const CVTimeStamp *)pOutputTime
{
    //code
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc]init];
    
    [self drawView];
    [pool release];
    return (kCVReturnSuccess);
}
-(void)prepareOpenGL
{
    //code
    //OpenGL iNfo
    fprintf(gpFile,"OpneGL version : %s\n",glGetString(GL_VERSION));
    fprintf(gpFile,"GLSL Version   : %s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    [[self openGLContext]makeCurrentContext];
    
    GLint swapInt =1;
    [[self openGLContext]setValues:&swapInt forParameter:NSOpenGLCPSwapInterval]; //cp means context parameter
   
    [self perVertexShaderCode];
  
    [self perFragmentShaderCode];
    
    Sphere *sphere = [[Sphere alloc] init];
    float sphere_vertices[1146];
    float sphere_normals[1146];
    float sphere_textures[764];
    short sphere_elements[2280];
    [sphere getSphereVertexDataWithPositionCoords:sphere_vertices
                                     NormalCoords:sphere_normals
                                    TextureCoords:sphere_textures
                                         Elements:sphere_elements];
    numVertices = [sphere getNumberOfSphereVertices];
    numElements = [sphere getNumberOfSphereElements];
    
    glGenVertexArrays(1,&vao_sphere);
    glBindVertexArray(vao_sphere);
    
    glGenBuffers(1,&vbo_sphere_position);
    glBindBuffer(GL_ARRAY_BUFFER,vbo_sphere_position);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(sphere_vertices),
                 sphere_vertices,
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
    
    glGenBuffers(1,&vbo_sphere_normal);
    glBindBuffer(GL_ARRAY_BUFFER,vbo_sphere_normal);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(sphere_normals),
                 sphere_normals,
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
    
    glGenBuffers(1,&vbo_sphere_element);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vbo_sphere_element);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(sphere_elements),
                 sphere_elements,
                 GL_STATIC_DRAW
                 );
    
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    
    glBindVertexArray(0);
    
    glClearDepth(1.0f);
    
    //enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    //depth test to do
    glDepthFunc(GL_LEQUAL);
    //glEnable(GL_CULL_FACE);
    
    lKeyPressed =0 ;
    vKeyPressed =0;
    fKeyPressed =1;
    //set background color
    glClearColor(0.0f,0.0f,0.0f,0.0f); //blue
    
    //set projection matrix to indentity matrix
    perspectiveProjectionMatrix = vmath::mat4::identity();
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);//1
    
    CVDisplayLinkSetOutputCallback(displayLink,&MyDisplayLinkCallback,self);//2
    
    CGLContextObj cglContext = (CGLContextObj)[[self openGLContext]CGLContextObj]; //3
    
    CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj)[[self pixelFormat]CGLPixelFormatObj];
    
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink,cglContext,cglPixelFormat);
    
    CVDisplayLinkStart(displayLink);
    
}
-(void)setUniformLocation
{
    if(fKeyPressed == 1)
    {
        //get MVP Uniform location
        modelMatrixUniform = glGetUniformLocation(perFragmentShaderProgramObject,"u_model_matrix");
        viewMatrixUniform = glGetUniformLocation(perFragmentShaderProgramObject,"u_view_matrix");
        projectionMatrixUniform = glGetUniformLocation(perFragmentShaderProgramObject,"u_projection_matrix");
        
        lKeyPressedUniform = glGetUniformLocation(perFragmentShaderProgramObject,
                                                "u_lighting_enabled");
        
        laUniform = glGetUniformLocation(perFragmentShaderProgramObject,"u_La");
        ldUniform = glGetUniformLocation(perFragmentShaderProgramObject,"u_Ld");
        lsUniform = glGetUniformLocation(perFragmentShaderProgramObject,"u_Ls");
        
        lightPositionUniform = glGetUniformLocation(perFragmentShaderProgramObject,"u_light_position");
        
        kaUniform = glGetUniformLocation(perFragmentShaderProgramObject,"u_Ka");
        kdUniform = glGetUniformLocation(perFragmentShaderProgramObject,"u_Kd");
        ksUniform = glGetUniformLocation(perFragmentShaderProgramObject,"u_Ks");
        materialShininessUniform = glGetUniformLocation(perFragmentShaderProgramObject,"u_material_shininess");
    }
    else if(vKeyPressed == 1)
    {
        //get MVP Uniform location
        modelMatrixUniform = glGetUniformLocation(perVertexShaderProgramObject,"u_model_matrix");
        viewMatrixUniform = glGetUniformLocation(perVertexShaderProgramObject,"u_view_matrix");
        projectionMatrixUniform = glGetUniformLocation(perVertexShaderProgramObject,"u_projection_matrix");
        
        lKeyPressedUniform = glGetUniformLocation(perVertexShaderProgramObject,
                                                "u_lighting_enabled");
        
        laUniform = glGetUniformLocation(perVertexShaderProgramObject,"u_La");
        ldUniform = glGetUniformLocation(perVertexShaderProgramObject,"u_Ld");
        lsUniform = glGetUniformLocation(perVertexShaderProgramObject,"u_Ls");
        
        lightPositionUniform = glGetUniformLocation(perVertexShaderProgramObject,"u_light_position");
        
        kaUniform = glGetUniformLocation(perVertexShaderProgramObject,"u_Ka");
        kdUniform = glGetUniformLocation(perVertexShaderProgramObject,"u_Kd");
        ksUniform = glGetUniformLocation(perVertexShaderProgramObject,"u_Ks");
        materialShininessUniform = glGetUniformLocation(perVertexShaderProgramObject,"u_material_shininess");
    }
}
-(void)reshape
{
    //code
    CGLLockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
    
    NSRect rect = [self bounds]; //take view's bounds
    
    GLfloat width = rect.size.width;
    GLfloat height = rect.size.height;
    
    if(height ==0 )
        height =1;
    glViewport(0,0,(GLsizei)width,(GLsizei)height);
    perspectiveProjectionMatrix = vmath::perspective(45.0f,width/height,0.1f,100.0f);
    
    CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
    
}
- (void) perVertexShaderCode
{
    //*****VERTEX SHADER *************
    //create shader
    vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    
    //provide source code to shader
    const GLchar *vertexShaderSourceCode =
    "#version 410" \
    "\n" \
    "in vec4 vPosition;" \
    "in vec3 vNormal;"   \
    "uniform mat4 u_model_matrix;" \
    "uniform mat4 u_view_matrix;"  \
    "uniform mat4 u_projection_matrix;" \
    "uniform int u_lighting_enabled;" \
    "uniform vec3 u_La;" \
    "uniform vec3 u_Ld;" \
    "uniform vec3 u_Ls;"  \
    "uniform vec4 u_light_position;" \
    "uniform vec3 u_Ka;" \
    "uniform vec3 u_Kd;" \
    "uniform vec3 u_Ks;" \
    "uniform float u_material_shininess;" \
    "out vec3 phong_ads_color;" \
    "void main(void)"  \
    "{" \
    "if(u_lighting_enabled ==1 )" \
    "{" \
    "vec4 eye_coordinates = u_view_matrix * u_model_matrix * vPosition;" \
    "vec3 transformed_normals = normalize(mat3(u_view_matrix * u_model_matrix) * vNormal);" \
    "vec3 light_direction = normalize(vec3(u_light_position) - eye_coordinates.xyz);" \
    "float tn_dot_ld = max(dot(transformed_normals,light_direction),0.0);" \
    "vec3 ambient = u_La * u_Ka;" \
    "vec3 diffuse = u_Ld * u_Kd * tn_dot_ld;" \
    "vec3 reflection_vector = reflect(-light_direction,transformed_normals);" \
    "vec3 viewer_vector = normalize(-eye_coordinates.xyz);" \
    "vec3 specular = u_Ls * u_Ks * pow(max(dot(reflection_vector,viewer_vector),0.0),u_material_shininess);"  \
    "phong_ads_color = ambient + diffuse + specular ;" \
    "}" \
    "else" \
    "{"  \
    "phong_ads_color = vec3(1.0,1.0,1.0);" \
    "}" \
    "gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;" \
    "}";
    
    glShaderSource(vertexShaderObject,1,(const GLchar**)&vertexShaderSourceCode,NULL);
    
    //compile sahder
    glCompileShader(vertexShaderObject);
    GLint iInfoLogLength = 0;
    GLint iShaderCompiledStatus = 0;
    char *szInfoLog = NULL;
    
    glGetShaderiv(vertexShaderObject,GL_COMPILE_STATUS,&iShaderCompiledStatus);
    if(iShaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(vertexShaderObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
        if(iInfoLogLength > 0)
        {
            szInfoLog = (char *)malloc(iInfoLogLength);
            if (szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(vertexShaderObject,iInfoLogLength,&written,szInfoLog);
                fprintf(gpFile,"Vertex Shader Compilation Log: %s\n" ,szInfoLog);
                free(szInfoLog);
                [self release];
                [NSApp terminate:self];
            }
        }
    }
    
    //************Fregment shader**********
    fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    
    //provide source code to shader
    const GLchar *fragmentShaderSourceCode =
    "#version 410" \
    "\n" \
    "in vec3 phong_ads_color;" \
    "out vec4 FragColor;"  \
    "void main(void)" \
    "{" \
    "FragColor = vec4(phong_ads_color,1.0);" \
    "}" ;
    
    glShaderSource(fragmentShaderObject,1,(const char **)&fragmentShaderSourceCode,NULL);
    
    //compile shader
    glCompileShader(fragmentShaderObject);
    //re-initialize varible
    iInfoLogLength =0;
    iShaderCompiledStatus =0;
    szInfoLog = NULL;
    glGetShaderiv(fragmentShaderObject,GL_COMPILE_STATUS,&iShaderCompiledStatus);
    if(iShaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(fragmentShaderObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
        if(iInfoLogLength > 0)
        {
            szInfoLog = (char *)malloc(iInfoLogLength);
            if (szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(fragmentShaderObject,iInfoLogLength,&written,szInfoLog);
                fprintf(gpFile,"Fragement shader compilation Log:%s\n " ,szInfoLog);
                free(szInfoLog);
                [self release];
                [NSApp terminate:self];
            }
        }
    }
    //****SHADER PROGRAM **********
    //create
    perVertexShaderProgramObject  = glCreateProgram();
    
    //attach vertex shader to shader program
    glAttachShader(perVertexShaderProgramObject,vertexShaderObject);
    
    //attach fragment shader to shader program
    glAttachShader(perVertexShaderProgramObject,fragmentShaderObject);
    
    //pre-link binding of shader program object with vertex shader position attribute
    glBindAttribLocation(perVertexShaderProgramObject,VDG_ATTRIBUTE_VERTEX,"vPosition");
    glBindAttribLocation(perVertexShaderProgramObject,VDG_ATTRIBUTE_NORMAL, "vNormal");
    //link shader
    glLinkProgram(perVertexShaderProgramObject);
    GLint iShaderProgramLinkStatus =0 ;
    glGetProgramiv(perVertexShaderProgramObject,GL_LINK_STATUS,&iShaderProgramLinkStatus);
    if(iShaderProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(perVertexShaderProgramObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
        if(iInfoLogLength > 0)
        {
            szInfoLog = (char *)malloc(iInfoLogLength);
            if (szInfoLog != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(perVertexShaderProgramObject,iInfoLogLength,&written,szInfoLog);
                printf("Shader program link Log : %s \n",szInfoLog);
                free(szInfoLog);
                [self release];
                
            }
        }
    }
}
-(void) perFragmentShaderCode
{
    //*****VERTEX SHADER *************
    //create shader
    vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    
    //provide source code to shader
    const GLchar *vertexShaderSourceCode =
    "#version 410" \
    "\n" \
    "in vec4 vPosition;" \
    "in vec3 vNormal;"   \
    "uniform mat4 u_model_matrix;" \
    "uniform mat4 u_view_matrix;"  \
    "uniform mat4 u_projection_matrix;" \
    "uniform int u_lighting_enabled;" \
    "uniform vec4 u_light_position;" \
    "out vec3 transformed_normals;" \
    "out vec3 light_direction;" \
    "out vec3 viewer_vector;"  \
    "void main(void)" \
    "{" \
    "if(u_lighting_enabled == 1)" \
    "{" \
    "vec4 eye_coordinates = u_view_matrix * u_model_matrix * vPosition;" \
    "transformed_normals = mat3(u_view_matrix * u_model_matrix) * vNormal;" \
    "light_direction = vec3(u_light_position) - eye_coordinates.xyz;" \
    "viewer_vector = - eye_coordinates.xyz;"\
    "}" \
    "gl_Position=u_projection_matrix * u_view_matrix *u_model_matrix *vPosition;"
    "}";
    
    glShaderSource(vertexShaderObject,1,(const GLchar**)&vertexShaderSourceCode,NULL);
    
    //compile sahder
    glCompileShader(vertexShaderObject);
    GLint iInfoLogLength = 0;
    GLint iShaderCompiledStatus = 0;
    char *szInfoLog = NULL;
    
    glGetShaderiv(vertexShaderObject,GL_COMPILE_STATUS,&iShaderCompiledStatus);
    if(iShaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(vertexShaderObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
        if(iInfoLogLength > 0)
        {
            szInfoLog = (char *)malloc(iInfoLogLength);
            if (szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(vertexShaderObject,iInfoLogLength,&written,szInfoLog);
                fprintf(gpFile,"Vertex Shader Compilation Log: %s\n" ,szInfoLog);
                free(szInfoLog);
                [self release];
                [NSApp terminate:self];
            }
        }
    }
    
    //************Fregment shader**********
    fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    
    //provide source code to shader
    const GLchar *fragmentShaderSourceCode =
    "#version 410" \
    "\n" \
    "in vec3 transformed_normals;" \
    "in vec3 light_direction;" \
    "in vec3 viewer_vector;" \
    "out vec4 FragColor;" \
    "uniform vec3 u_La;" \
    "uniform vec3 u_Ld;" \
    "uniform vec3 u_Ls;" \
    "uniform vec3 u_Ka;" \
    "uniform vec3 u_Kd;" \
    "uniform vec3 u_Ks;" \
    "uniform float u_material_shininess;" \
    "uniform int u_lighting_enabled;"  \
    "void main(void)" \
    "{" \
    "vec3 phong_ads_color;" \
    "if(u_lighting_enabled == 1)" \
    "{" \
    "vec3 normalized_transformed_normals=normalize(transformed_normals);" \
    "vec3 normalized_light_direction = normalize(light_direction);" \
    "vec3 normalied_viewer_vector = normalize(viewer_vector);" \
    "vec3 ambient = u_La * u_Ka;" \
    "float tn_dot_ld = max(dot(normalized_transformed_normals,normalized_light_direction),0.0);" \
    "vec3 diffuse = u_Ld * u_Kd * tn_dot_ld;" \
    "vec3 reflection_vector = reflect(-normalized_light_direction,normalized_transformed_normals);" \
    "vec3 specular = u_Ls * u_Ks * pow(max(dot(reflection_vector,normalied_viewer_vector),0.0),u_material_shininess);" \
    "phong_ads_color = ambient + diffuse + specular;" \
    "}" \
    "else" \
    "{"  \
    "phong_ads_color = vec3(1.0,1.0,1.0);" \
    "}" \
    "FragColor = vec4(phong_ads_color ,1.0);" \
    "}" ;
    
    glShaderSource(fragmentShaderObject,1,(const char **)&fragmentShaderSourceCode,NULL);
    
    //compile shader
    glCompileShader(fragmentShaderObject);
    //re-initialize varible
    iInfoLogLength =0;
    iShaderCompiledStatus =0;
    szInfoLog = NULL;
    glGetShaderiv(fragmentShaderObject,GL_COMPILE_STATUS,&iShaderCompiledStatus);
    if(iShaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(fragmentShaderObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
        if(iInfoLogLength > 0)
        {
            szInfoLog = (char *)malloc(iInfoLogLength);
            if (szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(fragmentShaderObject,iInfoLogLength,&written,szInfoLog);
                fprintf(gpFile,"Fragement shader compilation Log:%s\n " ,szInfoLog);
                free(szInfoLog);
                [self release];
                [NSApp terminate:self];
            }
        }
    }
    //****SHADER PROGRAM **********
    //create
    perFragmentShaderProgramObject  = glCreateProgram();
    
    //attach vertex shader to shader program
    glAttachShader(perFragmentShaderProgramObject,vertexShaderObject);
    
    //attach fragment shader to shader program
    glAttachShader(perFragmentShaderProgramObject,fragmentShaderObject);
    
    //pre-link binding of shader program object with vertex shader position attribute
    glBindAttribLocation(perFragmentShaderProgramObject,VDG_ATTRIBUTE_VERTEX,"vPosition");
    glBindAttribLocation(perFragmentShaderProgramObject,VDG_ATTRIBUTE_NORMAL, "vNormal");
    //link shader
    glLinkProgram(perFragmentShaderProgramObject);
    GLint iShaderProgramLinkStatus =0 ;
    glGetProgramiv(perFragmentShaderProgramObject,GL_LINK_STATUS,&iShaderProgramLinkStatus);
    if(iShaderProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(perFragmentShaderProgramObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
        if(iInfoLogLength > 0)
        {
            szInfoLog = (char *)malloc(iInfoLogLength);
            if (szInfoLog != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(perFragmentShaderProgramObject,iInfoLogLength,&written,szInfoLog);
                printf("Shader program link Log : %s \n",szInfoLog);
                free(szInfoLog);
                [self release];
                
            }
        }
    }
    
}
-(void)drawRect:(NSRect)dirtyRect
{
    //code
    [self drawView];
}

-(void)drawView
{
    //code
    [[self openGLContext]makeCurrentContext];
    
    CGLLockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //start using OpenGL program object
    if(fKeyPressed == 1)
    glUseProgram(perFragmentShaderProgramObject);
    else if(vKeyPressed == 1)
        glUseProgram(perVertexShaderProgramObject);
    
    if (lKeyPressed == 1)
    {
        glUniform1i(lKeyPressedUniform,1);
        
        //setting light properties
        glUniform3fv(laUniform,1,light_ambient);
        glUniform3fv(ldUniform,1,light_diffuse);//diffuse light intensity of light
        glUniform3fv(lsUniform,1,light_specular);
        glUniform4fv(lightPositionUniform,1,light_position);
        
        //setting material properties
        glUniform3fv(kaUniform,1,material_ambient);
        glUniform3fv(kdUniform,1,material_diffuse);
        glUniform3fv(ksUniform,1,material_specular);
        glUniform1f(materialShininessUniform,material_shininess);
    }
    else
    {
        glUniform1i(lKeyPressedUniform,0);
    }
    
    //#####################Sphere Drawing #################
    //OpenGL Drawing
    //set modelview and modelviewprojection matrix to identity
    vmath::mat4 modelMatrix = vmath::mat4::identity();
    vmath::mat4 viewMatrix = vmath::mat4::identity();
  
    
    modelMatrix = modelMatrix * vmath::translate(0.0f,0.0f,-5.0f);
    
    glUniformMatrix4fv(modelMatrixUniform,1,GL_FALSE,modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform,1,GL_FALSE,viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1,GL_FALSE,perspectiveProjectionMatrix);
    
    
    glBindVertexArray(vao_sphere);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_element);
    glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);
    
    //stop using OpenGL progrma object
    glUseProgram(0);
    [self setUniformLocation];
    CGLFlushDrawable((CGLContextObj)[[self openGLContext]CGLContextObj]);
    CGLUnlockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
}


-(BOOL)acceptsFirstResponder
{
    //code
    [[self window]makeFirstResponder:self];
    return(YES);
}

-(void)keyDown:(NSEvent *)theEvent
{
    //code
    int key = (int)[[theEvent characters]characterAtIndex:0];
    switch(key)
    {
            case 27: //esc key  //fullscreen
            [[self window]toggleFullScreen:self];//repainting occurs   automatically
            break;
           case 'L':
           case 'l':
            lKeyPressed++;
            if(lKeyPressed > 1)
                lKeyPressed =0;
            break;
            
            case 'F':    //per fragment lighting
            case 'f':
            fKeyPressed = 1;
            vKeyPressed = 0;
            break;
        case 'V':
        case 'v':        //per vertex lighting
            vKeyPressed = 1;
            fKeyPressed = 0;
            break;
        case 'Q':
        case 'q':             //close program
            [self release];
            [NSApp terminate:self];
            break;
            
        default:
            break;
    }
}

-(void)mouseDown:(NSEvent *)theEvent
{
    //code
   
}

-(void)mouseDragged:(NSEvent *)theEvent
{
    //code
    
}

-(void)rightMouseDown:(NSEvent *)theEvent
{
    //code
    
}

-(void) dealloc
{
    if (vao_sphere)
    {
        glDeleteVertexArrays(1,&vao_sphere);
        vao_sphere=0;
    }
    
    //destroy vbo
    if(vbo_sphere_element)
    {
        glDeleteBuffers(1,&vbo_sphere_element);
        vbo_sphere_element=0;
    }
    if(vbo_sphere_normal)
    {
        glDeleteBuffers(1,&vbo_sphere_normal);
        vbo_sphere_normal=0;
    }
    if(vbo_sphere_position)
    {
        glDeleteBuffers(1,&vbo_sphere_position);
        vbo_sphere_position=0;
    }
    
    
    //detach vertex n fragment shader from shader program
    glDetachShader(perVertexShaderProgramObject,vertexShaderObject);
    glDetachShader(perVertexShaderProgramObject,fragmentShaderObject);
    
    glDetachShader(perFragmentShaderProgramObject,vertexShaderObject);
    glDetachShader(perFragmentShaderProgramObject,fragmentShaderObject);
    //delete vertex shader object
    glDeleteShader(vertexShaderObject);
    vertexShaderObject =0;
    
    glDeleteShader(fragmentShaderObject);
    fragmentShaderObject =0;
    
    glDeleteProgram(perVertexShaderProgramObject);
    perVertexShaderProgramObject =0 ;
    glDeleteProgram(perFragmentShaderProgramObject);
    perFragmentShaderProgramObject =0 ;
    
    
    CVDisplayLinkStop(displayLink);
    CVDisplayLinkRelease(displayLink);
    
    [super dealloc];
}

@end

CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink,const CVTimeStamp *pNow,
                               const CVTimeStamp *pOutputTime,CVOptionFlags flagsIn,
                               CVOptionFlags *pFlagsout,void *pDisplayLinkContext)
{
    CVReturn result = [(GLView *)pDisplayLinkContext getFrameForTime:pOutputTime];
    return result;
}



