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
    [window setTitle:@"macOS Smiley 3D Rotation"];
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
    
    GLuint vao_triangle;
    GLuint vao_square;
    GLuint vbo_postion;
    GLuint vbo_texture;
    
    GLuint smiley_texture;
    //GLuint cube_texture;
    GLuint texture_sampler_uniform;
    GLuint mvpUniform;
    
    GLfloat angle;
    
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
    "in vec2 vTexture0_Coord;" \
    "out vec2 out_texture0_Coord;" \
    "uniform mat4 u_mvp_matrix;" \
    "void main(void)" \
    "{" \
    "gl_Position = u_mvp_matrix * vPosition;" \
    "out_texture0_Coord = vTexture0_Coord;" \
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
    "in vec2 out_texture0_Coord;" \
    "uniform sampler2D u_texture0_sampler;" \
    "out vec4 FragColor;" \
    "void main(void)" \
    "{" \
    "vec3 tex = vec3(texture(u_texture0_sampler,out_texture0_Coord));"
    "FragColor = vec4(tex,1.0f);" \
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
    mvpUniform = glGetUniformLocation(shaderProgramObject,"u_mvp_matrix");
    texture_sampler_uniform = glGetUniformLocation(shaderProgramObject,"u_texture0_sampler");


  //load textures
  smiley_texture = [self loadTextureFromBMPFile:"Smiley.bmp"];
  
    //*** vao,vab etc inittializations
  // **** triangle data ,vao vbo initialization
    const GLfloat pyramidVertices[] =
    {
        //front face
        0.0f,1.0f,0.0f,//apex
        -1.0f,-1.0f,1.0f,//left-bottom
        1.0f,-1.0,1.0f, //right bottom
        
        //right face
        0.0f,1.0f,0.0f,
        1.0f,-1.0f,1.0f,
        1.0f,-1.0f,-1.0f,
        
        //back face
        0.0f,1.0f,0.0f,
        1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        
        //left face
        0.0f,1.0f,0.0f,
        -1.0f,-1.0,-1.0f,
        -1.0f,-1.0f,1.0f
    };
    const GLfloat pyramidTexCoords[] =
    {
        
        0.5f,1.0f,//front top
        0.0f,0.0f,//front right
        1.0f,0.0f, //front left
        
        0.5f,1.0f,//front top
        0.0f,0.0f,//front right
        1.0f,0.0f, //front left
        
        0.5f,1.0f,//front top
        0.0f,0.0f,//front right
        1.0f,0.0f, //front left
        
        0.5f,1.0f,//front top
        0.0f,0.0f,//front right
        1.0f,0.0f, //front left
        
    };
    
    glGenVertexArrays(1,&vao_triangle);
    glBindVertexArray(vao_triangle);
    //____________________
    glGenBuffers(1,&vbo_postion);
    glBindBuffer(GL_ARRAY_BUFFER,vbo_postion);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(pyramidVertices),
                 pyramidVertices,
                 GL_STATIC_DRAW);
    
    glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX,
                        3,
                        GL_FLOAT,
                        GL_FALSE,
                        0,
                          NULL);
    glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
    
    glBindBuffer(GL_ARRAY_BUFFER,0);
    //__________________________
    glGenBuffers(1,&vbo_texture);
    glBindBuffer(GL_ARRAY_BUFFER,vbo_texture);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(pyramidTexCoords),
                 pyramidTexCoords,
                 GL_STATIC_DRAW);
    
    glVertexAttribPointer(VDG_ATTRIBUTE_TEXTURE0,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          0,
                          NULL);
    glEnableVertexAttribArray(VDG_ATTRIBUTE_TEXTURE0);
    
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(0);
    
    //***** Square data,vao,vbo initialization
    
    const GLfloat cubeVertices[] =
    {
        //front face (from right top in anticlockwise fashion)
        1.0f,1.0f,1.0f,
        -1.0f,1.0f,1.0f,
        -1.0f,-1.0f,1.0f,
        1.0f,-1.0f,1.0f,
        
        //right face
        1.0f,1.0f,-1.0f,
        1.0f,1.0f,1.0f,
        1.0f,-1.0f,1.0f,
        1.0f,-1.0f,-1.0f,
        
        //back face
        -1.0f,1.0f,-1.0f,
        1.0f,1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        
        //left face
        -1.0f,1.0f,1.0f,
        -1.0f,1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,1.0f,
        
        //top face
        1.0f,1.0f,-1.0f,
        -1.0f,1.0f,-1.0f,
        -1.0f,1.0f,1.0f,
        1.0f,1.0f,1.0f,
        
        //bottom face
        1.0f,-1.0f,1.0f,
        -1.0f,-1.0f,1.0f,
        -1.0f,-1.0f,-1.0f,
        1.0f,-1.0f,-1.0f

    };
    
    const GLfloat cubeTexCoords[] =
    {
        //In macOS it takes mirror image.so provide tex coordinates according to that.
        1.0f,0.0f,
        0.0f,0.0f,
        0.0f,1.0f,
        1.0f,1.0f,


        1.0f,0.0f,
        0.0f,0.0f,
        0.0f,1.0f,
        1.0f,1.0f,


        1.0f,0.0f,
        0.0f,0.0f,
        0.0f,1.0f,
        1.0f,1.0f,


        1.0f,0.0f,
        0.0f,0.0f,
        0.0f,1.0f,
        1.0f,1.0f,


        1.0f,0.0f,
        0.0f,0.0f,
        0.0f,1.0f,
        1.0f,1.0f,


        1.0f,0.0f,
        0.0f,0.0f,
        0.0f,1.0f,
        1.0f,1.0f,
    };
    
    
    //____________________________
    glGenVertexArrays(1,&vao_square);
    glBindVertexArray(vao_square);
    
    glGenBuffers(1,&vbo_postion);
    glBindBuffer(GL_ARRAY_BUFFER,vbo_postion);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(cubeVertices),
                 cubeVertices,
                 GL_STATIC_DRAW);
    
    glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          0,
                          NULL);
    glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
    
    glBindBuffer(GL_ARRAY_BUFFER,0);
    //______________________________
    
    glGenBuffers(1,&vbo_texture);
    glBindBuffer(GL_ARRAY_BUFFER,vbo_texture);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(cubeTexCoords),
                 cubeTexCoords,
                 GL_STATIC_DRAW);
    
    glVertexAttribPointer(VDG_ATTRIBUTE_TEXTURE0,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          0,
                          NULL);
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
    
    
    //#####################Triangle Drawing #################
    //OpenGL Drawing
    //set modelview and modelviewprojection matrix to identity
    vmath::mat4 modelViewMatrix = vmath::mat4::identity();
    vmath::mat4 translationMatrix = vmath::mat4::identity();
    vmath::mat4 rotationMatrix = vmath::mat4::identity();
    vmath::mat4 modelViewProjectionMatrix = vmath::mat4::identity();
    
    //multiply modelView to orthographic matrix to get modelViewProjection Matrix
    translationMatrix = modelViewMatrix * vmath::translate(-1.0f,0.0f,-4.0f);
    modelViewMatrix  = modelViewMatrix * translationMatrix;
    rotationMatrix = modelViewMatrix * vmath::rotate(angle,0.0f,1.0f,0.0f);
    modelViewMatrix = modelViewMatrix * rotationMatrix;
    modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix; //order is imp
    
    //pass above modelViewProjection Matrix to vertex shader in u_mvp_matrix shader varible
    //whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
    glUniformMatrix4fv(mvpUniform,1,GL_FALSE,modelViewProjectionMatrix);

    glBindTexture(GL_TEXTURE_2D,smiley_texture);
    
    //bind vao
    glBindVertexArray(vao_triangle);
    
    //draw triangle
    glDrawArrays(GL_TRIANGLES,0,12); //3 with its x,y,z
    
    //unbind vao
    glBindVertexArray(0);
    
    //############################Square Drawing #####################
    //OpenGL Drawing
    //set modelview and modelviewprojection matrix to identity
    modelViewMatrix = vmath::mat4::identity();
    translationMatrix = vmath::mat4::identity();
    rotationMatrix = vmath::mat4::identity();
    vmath::mat4 scaleMatrix = vmath::mat4::identity();
    modelViewProjectionMatrix = vmath::mat4::identity();
    
    //multiply modelView to orthographic matrix to get modelViewProjection Matrix
    translationMatrix = modelViewMatrix * vmath::translate(1.0f,0.0f,-4.0f);
    modelViewMatrix  = modelViewMatrix * translationMatrix;
    
    rotationMatrix = modelViewMatrix * vmath::rotate(angle,1.0f,0.0f,0.0f);
     rotationMatrix = rotationMatrix * vmath::rotate(angle,0.0f,1.0f,0.0f);
     rotationMatrix = rotationMatrix * vmath::rotate(angle,0.0f,0.0f,1.0f);
    //modelViewMatrix = modelViewMatrix * rotationMatrix;
    //modelViewMatrix = modelViewMatrix * vmath::rotate(angle,1.0f,1.0f,1.0f);
    
    scaleMatrix = rotationMatrix * vmath::scale(0.75f,0.75f,0.75f);
    modelViewMatrix = modelViewMatrix *scaleMatrix;
    modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix; //order is imp
    
    //pass above modelViewProjection Matrix to vertex shader in u_mvp_matrix shader varible
    //whose position value we already calculated in initWithFrame() by using glGetUniformLocation()
    glUniformMatrix4fv(mvpUniform,1,GL_FALSE,modelViewProjectionMatrix);
    
    glBindTexture(GL_TEXTURE_2D,smiley_texture);
    //bind vao
    glBindVertexArray(vao_square);
    
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
    if (vao_triangle)
    {
        glDeleteVertexArrays(1,&vao_triangle);
        vao_triangle=0;
    }
    if (vao_square)
    {
        glDeleteVertexArrays(1,&vao_square);
        vao_square=0;
    }
    //destroy vbo
    if(vbo_texture)
    {
        glDeleteBuffers(1,&vbo_texture);
        vbo_texture=0;
    }
    if(vbo_postion)
    {
        glDeleteBuffers(1,&vbo_postion);
        vbo_postion=0;
    }
    if(smiley_texture)
    {
        glDeleteTextures(1,&smiley_texture);
        smiley_texture=0;
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



