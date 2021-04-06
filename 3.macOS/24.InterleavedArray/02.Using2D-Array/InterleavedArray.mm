//headers
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#import <QuartzCore/CVDisplayLink.h>

#import <OpenGL/gl3.h>
#import <OpenGL/gl3ext.h>
#import "vmath.h"
#define STEP_ANGLE 1.0f
enum
{
    VDG_ATTRIBUTE_VERTEX = 0,
    VDG_ATTRIBUTE_COLOR,
    VDG_ATTRIBUTE_NORMAL,
    VDG_ATTRIBUTE_TEXTURE0,
};

GLfloat light_ambient[] = {0.25f,0.25f,0.25f,1.0f};
GLfloat light_diffuse[] = {1.0f,1.0f,1.0f,1.0f};
GLfloat light_specular[] = {1.0f,1.0f,1.0f,1.0f};
GLfloat light_position[] = {100.0f,100.0f,100.0f,1.0f};

GLfloat material_ambient[]  = {0.0f ,0.0f,0.0f,1.0f};
GLfloat material_diffuse[] ={1.0f,1.0f,1.0f,1.0f};
GLfloat material_specular[] = {1.0f,1.0f,1.0f,1.0f};
GLfloat material_shininess = 128.0f;

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
    [window setTitle:@"macOS 3D Rotation"];
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
    
   
    GLuint vao_cube;
    GLuint vbo_cube;
    
    
   
    GLuint cube_texture;
    GLuint texture_sampler_uniform;
    


    GLint modelMatrixUniform,viewMatrixUniform,projectionMatrixUniform;
    GLint laUniform,lsUniform,ldUniform,lightPositionUniform;
    GLint kaUniform,kdUniform,ksUniform,materialShininessUniform;
    
    GLfloat angle;
    int lKeyPressedUniform;
    int lKeyPressed;
    
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
   
    
    //*****VERTEX SHADER *************
    //create shader
    vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    
    //provide source code to shader
    const GLchar *vertexShaderSourceCode =
    "#version 410" \
    "\n" \
    "in vec4 vPosition;" \
    "in vec3 vNormal;" \
    "in vec4 vColor;" \
    "out vec4 out_color;"
    "uniform mat4 u_model_matrix;" \
    "uniform mat4 u_view_matrix;" \
    "uniform mat4 u_projection_matrix;" \
    "uniform int u_lighting_enabled;" \
    "uniform vec4 u_light_position;" \
    "out vec3 transformed_normals;" \
    "out vec3 light_direction;" \
    "out vec3 viewer_vector;" \
    "in vec2 vTexture0_Coord;" \
    "out vec2 out_texture0_Coord;" \
    "void main(void)" \
    "{" \
    "if(u_lighting_enabled == 1)" \
    "{" \
    "vec4 eye_coordinates = u_view_matrix * u_model_matrix * vPosition;" \
    "transformed_normals = mat3(u_view_matrix * u_model_matrix) * vNormal;" \
    "light_direction = vec3(u_light_position) - eye_coordinates.xyz;" \
    "viewer_vector = - eye_coordinates.xyz;" \
    "}"
    "gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;" \
    "out_texture0_Coord = vTexture0_Coord;" \
    "out_color = vColor;"
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
    "in vec4 out_color;" \
    "out vec4 FragColor;" \
    "in vec2 out_texture0_Coord;" \
    "uniform sampler2D u_texture0_sampler;" \
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
    "vec3 tex = vec3(texture(u_texture0_sampler,out_texture0_Coord));" \
    "vec3 final_color = tex * phong_ads_color * vec3(out_color);" \
    "FragColor = vec4(final_color,1.0f);" \
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
    shaderProgramObject  = glCreateProgram();
    
    //attach vertex shader to shader program
    glAttachShader(shaderProgramObject,vertexShaderObject);
    
    //attach fragment shader to shader program
    glAttachShader(shaderProgramObject,fragmentShaderObject);
    
    //pre-link binding of shader program object with vertex shader position attribute
    glBindAttribLocation(shaderProgramObject,VDG_ATTRIBUTE_VERTEX,"vPosition");
    glBindAttribLocation(shaderProgramObject,VDG_ATTRIBUTE_TEXTURE0,"vTexture0_Coord");
    glBindAttribLocation(shaderProgramObject,VDG_ATTRIBUTE_NORMAL,"vNormal");
    glBindAttribLocation(shaderProgramObject,VDG_ATTRIBUTE_COLOR,"vColor");

    //link shader
    glLinkProgram(shaderProgramObject);
    GLint iShaderProgramLinkStatus =0 ;
    glGetProgramiv(shaderProgramObject,GL_LINK_STATUS,&iShaderProgramLinkStatus);
    if(iShaderProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(shaderProgramObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
        if(iInfoLogLength > 0)
        {
            szInfoLog = (char *)malloc(iInfoLogLength);
            if (szInfoLog != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(shaderProgramObject,iInfoLogLength,&written,szInfoLog);
                fprintf(gpFile,"Shader program link Log : %s \n",szInfoLog);
                free(szInfoLog);
                [self release];
                [NSApp terminate:self];
            }
        }
    }
    
     
    //get MVP Uniform location
    modelMatrixUniform= glGetUniformLocation(shaderProgramObject,"u_model_matrix");
    viewMatrixUniform = glGetUniformLocation(shaderProgramObject,"u_view_matrix");
    projectionMatrixUniform = glGetUniformLocation(shaderProgramObject,"u_projection_matrix");
    
    lKeyPressedUniform = glGetUniformLocation(shaderProgramObject,"u_lighting_enabled");
    laUniform = glGetUniformLocation(shaderProgramObject, "u_La");
    ldUniform = glGetUniformLocation(shaderProgramObject,"u_Ld");
    lsUniform = glGetUniformLocation(shaderProgramObject,"u_Ls");
    
    kaUniform = glGetUniformLocation(shaderProgramObject,"u_Ka");
    kdUniform = glGetUniformLocation(shaderProgramObject,"u_Kd");
    ksUniform = glGetUniformLocation(shaderProgramObject,"u_Ks");
    
    lightPositionUniform = glGetUniformLocation(shaderProgramObject,"u_light_position");
    materialShininessUniform = glGetUniformLocation(shaderProgramObject ,"u_material_shininess");

    texture_sampler_uniform = glGetUniformLocation(shaderProgramObject,"u_texture0_sampler");


  //load textures
  cube_texture    = [self loadTextureFromBMPFile:"marble.bmp"] ;  
    //***** Square data,vao,vbo initialization
    
    const GLfloat cubeVCNT[24][11] =
    {
        //front face (from right top in anticlockwise fashion)
        {1.0f,1.0f,1.0f,   1.0f,0.0f,0.0f,  0.0f,0.0f,1.0f,  1.0f,0.0f},
        {-1.0f,1.0f,1.0f,  1.0f,0.0f,0.0f,  0.0f,0.0f,1.0f,  0.0f,0.0f},
        {-1.0f,-1.0f,1.0f, 1.0f,0.0f,0.0f,  0.0f,0.0f,1.0f,  0.0f,1.0f},
        {1.0f,-1.0f,1.0f,  1.0f,0.0f,0.0f,  0.0f,0.0f,1.0f,  1.0f,1.0f},
        
        //right face
        {1.0f,1.0f,-1.0f,  1.0f,1.0f,0.0f,  1.0f,0.0f,0.0f,   1.0f,0.0f},
        {1.0f,1.0f,1.0f,   1.0f,1.0f,0.0f,  1.0f,0.0f,0.0f,   0.0f,0.0f},
        {1.0f,-1.0f,1.0f,  1.0f,1.0f,0.0f,  1.0f,0.0f,0.0f,   0.0f,1.0f},
        {1.0f,-1.0f,-1.0f, 1.0f,1.0f,0.0f,  1.0f,0.0f,0.0f,   1.0f,1.0f},
        
        //back face
        {-1.0f,1.0f,-1.0f, 1.0f,0.0f,1.0f,  0.0f,0.0f,-1.0f,  1.0f,0.0f},
        {1.0f,1.0f,-1.0f,  1.0f,0.0f,1.0f,  0.0f,0.0f,-1.0f,  0.0f,0.0f},
        {1.0f,-1.0f,-1.0f, 1.0f,0.0f,1.0f,  0.0f,0.0f,-1.0f,  0.0f,1.0f},
        {-1.0f,-1.0f,-1.0f,1.0f,0.0f,1.0f,  0.0f,0.0f,-1.0f,  1.0f,1.0f},
        
        //left face
        {-1.0f,1.0f,1.0f,  0.0f,1.0f,0.0f,  -1.0f,0.0,0.0f,   1.0f,0.0f},
        {-1.0f,1.0f,-1.0f, 0.0f,1.0f,0.0f,  -1.0f,0.0,0.0f,   0.0f,0.0f},
        {-1.0f,-1.0f,-1.0f,0.0f,1.0f,0.0f,  -1.0f,0.0,0.0f,   0.0f,1.0f},
        {-1.0f,-1.0f,1.0f, 0.0f,1.0f,0.0f,  -1.0f,0.0,0.0f,   1.0f,1.0f},
        
        //top face
        {1.0f,1.0f,-1.0f,  0.0f,1.0f,1.0f,  0.0f,1.0f,0.0f,  1.0f,0.0f},
        {-1.0f,1.0f,-1.0f, 0.0f,1.0f,1.0f,  0.0f,1.0f,0.0f,  0.0f,0.0f},
        {-1.0f,1.0f,1.0f,  0.0f,1.0f,1.0f,  0.0f,1.0f,0.0f,  0.0f,1.0f},
        {1.0f,1.0f,1.0f,   0.0f,1.0f,1.0f,  0.0f,1.0f,0.0f,  1.0f,1.0f},
        
        //bottom face
        {1.0f,-1.0f,1.0f,  0.0f,0.0f,1.0f,  0.0f,-1.0f,0.0f, 1.0f,0.0f},
        {-1.0f,-1.0f,1.0f, 0.0f,0.0f,1.0f,  0.0f,-1.0f,0.0f, 0.0f,0.0f},
        {-1.0f,-1.0f,-1.0f,0.0f,0.0f,1.0f,  0.0f,-1.0f,0.0f, 0.0f,1.0f},
        {1.0f,-1.0f,-1.0f , 0.0f,0.0f,1.0f,  0.0f,-1.0f,0.0f, 1.0f,1.0f},

    };
    

    
    //____________________________
    glGenVertexArrays(1,&vao_cube);
    glBindVertexArray(vao_cube);
    
    glGenBuffers(1,&vbo_cube);
    glBindBuffer(GL_ARRAY_BUFFER,vbo_cube);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(cubeVCNT),
                 cubeVCNT,
                 GL_STATIC_DRAW);
    
    glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          11*sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
    
    glVertexAttribPointer(VDG_ATTRIBUTE_COLOR,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          11*sizeof(float),
                          (void *) (3* sizeof(float)));
    glEnableVertexAttribArray(VDG_ATTRIBUTE_COLOR);
    
    glVertexAttribPointer(VDG_ATTRIBUTE_NORMAL,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          11*sizeof(float),
                          (void *) (6 * sizeof(float)));
    glEnableVertexAttribArray(VDG_ATTRIBUTE_NORMAL);
    
    glVertexAttribPointer(VDG_ATTRIBUTE_TEXTURE0,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          11 * sizeof(float),
                          (void *) (9 * sizeof(float)));
    glEnableVertexAttribArray(VDG_ATTRIBUTE_TEXTURE0);
    
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(0);
    
    
    glClearDepth(1.0f);
    
    //enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    //depth test to do
    glDepthFunc(GL_LEQUAL);
    //glEnable(GL_CULL_FACE);
 
   glEnable(GL_TEXTURE_2D);   
    
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

-(GLuint)loadTextureFromBMPFile:(const char *)texFileName
{
    NSBundle *mainBundle = [NSBundle mainBundle];
    NSString *appDirName = [mainBundle bundlePath];
    NSString *parentDirPath = [appDirName stringByDeletingLastPathComponent];
    NSString *textureFileNameWithPath = [NSString stringWithFormat:@"%@/%s",parentDirPath,texFileName];

    NSImage *bmpImage = [[NSImage alloc]initWithContentsOfFile:textureFileNameWithPath];
    if(!bmpImage)
    {
        NSLog(@"can't find %@",textureFileNameWithPath);
        return(0);
    }

    CGImageRef cgImage = [bmpImage CGImageForProposedRect:nil context:nil hints:nil];

    int w = (int)CGImageGetWidth(cgImage);
    int h = (int)CGImageGetHeight(cgImage);

 CFDataRef imageData = CGDataProviderCopyData(CGImageGetDataProvider(cgImage));
 void *pixels = (void *)CFDataGetBytePtr(imageData);

 GLuint bmpTexture ;
 glGenTextures(1,&bmpTexture);

 glPixelStorei(GL_UNPACK_ALIGNMENT,1); //set 1 rather than default 4 ,for better performance
 glBindTexture(GL_TEXTURE_2D,bmpTexture);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);

 glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_RGBA,
               w,
               h,
               0,
               GL_RGBA,
               GL_UNSIGNED_BYTE,
               pixels);

 //Create mipmaps for this texture for better image quality
 glGenerateMipmap(GL_TEXTURE_2D);

 CFRelease(imageData);
 return(bmpTexture);

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
    glUseProgram(shaderProgramObject);
    
    //############################Square Drawing #####################
       
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
    
    
    vmath::mat4 modelMatrix = vmath::mat4::identity();
    vmath::mat4 viewMatrix = vmath::mat4::identity();
  
    
    modelMatrix = modelMatrix * vmath::translate(0.0f,0.0f,-7.0f);
    modelMatrix = modelMatrix * vmath::rotate(angle,angle,angle);
    glUniformMatrix4fv(modelMatrixUniform,1,GL_FALSE,modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform,1,GL_FALSE,viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1,GL_FALSE,perspectiveProjectionMatrix);
    
    glBindTexture(GL_TEXTURE_2D,cube_texture);
    //bind vao
    glBindVertexArray(vao_cube);
    
    //draw triangle
    glDrawArrays(GL_TRIANGLE_FAN,0,4); //3 with its x,y,z
    glDrawArrays(GL_TRIANGLE_FAN,4,4);
    glDrawArrays(GL_TRIANGLE_FAN,8,4);
    glDrawArrays(GL_TRIANGLE_FAN,12,4);
    glDrawArrays(GL_TRIANGLE_FAN,16,4);
    glDrawArrays(GL_TRIANGLE_FAN,20,4);
    
    //unbind vao
    glBindVertexArray(0);
    
    //################################################
    //stop using OpenGL progrma object
    glUseProgram(0);
    [self spin];
    CGLFlushDrawable((CGLContextObj)[[self openGLContext]CGLContextObj]);
    CGLUnlockContext((CGLContextObj)[[self openGLContext]CGLContextObj]);
}

-(void)spin
{
    angle = angle + STEP_ANGLE;
    if (angle >= 360.0f)
    angle = angle - 360.0f;
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
            case 27: //esc key
            [self release];
            [NSApp terminate:self];
            break;
            case 'L':
           case 'l':
            lKeyPressed++;
            if(lKeyPressed > 1)
                lKeyPressed =0;
            break;
            case 'F':
            case 'f':
            [[self window]toggleFullScreen:self];//repainting occurs   automatically
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
    //code
    //destroy vao
   
    if (vao_cube)
    {
        glDeleteVertexArrays(1,&vao_cube);
        vao_cube=0;
    }
    //destroy vbo
    if(vbo_cube)
    {
        glDeleteBuffers(1,&vbo_cube);
        vbo_cube=0;
    }
    
    if(cube_texture)
    {
        glDeleteTextures(1,&cube_texture);
        cube_texture =0 ;
    }
    
    //detach vertex n fragment shader from shader program
    glDetachShader(shaderProgramObject,vertexShaderObject);
    glDetachShader(shaderProgramObject,fragmentShaderObject);
    
    //delete vertex shader object
    glDeleteShader(vertexShaderObject);
    vertexShaderObject =0;
    
    glDeleteShader(fragmentShaderObject);
    fragmentShaderObject =0;
    
    glDeleteProgram(shaderProgramObject);
    shaderProgramObject =0 ;
    
    
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



