#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>


//headers for XServer
#include <X11/Xlib.h>  //analogous to windows.h
#include <X11/Xutil.h>  //for visuals
#include <X11/XKBlib.h> //XkbKeycodeTokeysym
#include <X11/keysym.h>
#include<GL/glew.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include "vmath.h"

#define WIN_WIDTH  800
#define WIN_HEIGHT 600

using namespace vmath;

enum
{
  VDG_ATTRIBUTE_VERTEX =0,
  VDG_ATTRIBUTE_COLOR,
  VDG_ATTRIBUTE_NORMAL,
  VDG_ATTRIBUTE_TEXTURE,
};


//global variabrl declarations
FILE *gpFile =NULL;

Display *gpDisplay=NULL;
XVisualInfo *gpXVisualInfo =NULL;
Colormap gColormap;
Window gWindow;

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display *,GLXFBConfig,GLXContext,Bool,const int *);

glXCreateContextAttribsARBProc glXCreateContextAttribsARB =NULL;

GLXFBConfig gGLXFBConfig;
GLXContext gGLXContext; //parallel to  HGLRC

bool gbFullscreen =false;

//OpenGL shader Related variables
GLuint gVertexShaderObject;
GLuint gFragmentShaderObject;
GLuint gShaderProgramObject;

GLuint gVao;
GLuint gVbo;
GLuint gMVPUniform;

mat4 gPerspectiveProjectionMatrix;

//main
int main(int argc,char *argv[])
{
     //function prototype
     void CreateWindow(void);
     void ToggleFullscreen(void);
     void initialize(void);
     void resize(int,int);
     void display(void);
     void uninitialize(void);


   //code
  //create log file
 gpFile =fopen("Log.txt","w");
 
 if(gpFile == NULL)
  {
    printf("failed to create log file\n");
    exit(0);
  }
  else
    fprintf(gpFile,"Log file is Created successfully\n");


  //create window
  CreateWindow();

  //intialize
   initialize();

  //message loop
  //variable declarations
  XEvent event; //parallel to 'MSG' Structeure
  KeySym keysym;
  int winWidth;
  int winHeight;
  bool bDone =false;

  while (bDone == false)
   {
     while(XPending(gpDisplay))
      {
           XNextEvent(gpDisplay,&event);  //parallel to GetMessage()
           switch(event.type)//parallel to iMsg
           {
             case MapNotify:  //parallel to WM_CREATE
                  break;
             case KeyPress: //parallel to WM_KEYDOWN 
                  keysym =XkbKeycodeToKeysym(gpDisplay,event.xkey.keycode,0,0);
                  switch(keysym)
                 {
                    case XK_Escape:     
                         bDone =true;
                         break;
                    case XK_F:
                    case XK_f:
                          if(gbFullscreen == false)
                           {
                             ToggleFullscreen();
                             gbFullscreen=true;
                           }
                           else
                           {
                               ToggleFullscreen();
                               gbFullscreen= false;
                           }
                         break;
                    default:
                         break;
                  }
                 break;
            case ButtonPress:
                     switch(event.xbutton.button)
                      {
                         case 1://left button
                           break;
                         case 2://middle button
                           break;
                          case 3://right button
                            break;
                          default:
                            break;
                       }
                   break;
             case MotionNotify:
                   break;
             case ConfigureNotify://parallel to WM_SIZE
                   
               winWidth=event.xconfigure.width;
               winHeight=event.xconfigure.height;

               resize(winWidth,winHeight);
               break;
             case Expose:
                 break;
             case DestroyNotify:
                 break;
             case 33: //close button
                 bDone=true;
                 break;
             default:
                 break;
         }
       }
   display();
    }
     uninitialize();
     return(0);
 }

void CreateWindow(void)
{ 
    //function prototype
         void uninitialize(void);
  
    //variable declarations
     XSetWindowAttributes winAttribs;
     GLXFBConfig *pGLXFBConfigs =NULL;
     GLXFBConfig bestGLXFBConfig;
     XVisualInfo *pTempXVisualInfo =NULL;
    int iNumFBConfigs =0;
    int styleMask;
    int i;


    static int frameBufferAttributes[] = {
           GLX_X_RENDERABLE,True,
           GLX_DRAWABLE_TYPE,GLX_WINDOW_BIT,
           GLX_RENDER_TYPE,GLX_RGBA_BIT,
           GLX_X_VISUAL_TYPE,GLX_TRUE_COLOR,
           GLX_RED_SIZE,    8,
           GLX_GREEN_SIZE,  8,
           GLX_BLUE_SIZE,   8,
           GLX_ALPHA_SIZE,  8,
           GLX_DEPTH_SIZE,  24,
           GLX_STENCIL_SIZE,8,
           GLX_DOUBLEBUFFER,True,
         //GLX_SAMPLEBUFFER,1,
         //GLX_SAMPLES ,    4,
           None}; //array must be terminated by 0

      //code
        gpDisplay =XOpenDisplay(NULL);
        if(gpDisplay == NULL)
        {
          printf("ERROR: gpDisplay failed\n");
          uninitialize();
          exit(1);
        }
  //getnew framebuffer config that meets our attrib requirements
  
        pGLXFBConfigs = glXChooseFBConfig(gpDisplay,DefaultScreen(gpDisplay),frameBufferAttributes,&iNumFBConfigs);
         if(pGLXFBConfigs ==NULL)
       {
              printf("ERROR FB config failed\n");
              uninitialize();
              exit(1);
       }
     printf(" %d Matching FB config Found\n",iNumFBConfigs);
   
     //pick that FB config/visual with the most samples per pixel
    int bestFramebufferconfig=-1,worstFramebufferConfig=-1,bestNumberOfSamples=-1,worstNumberOfSamples=999;
  
      for(i=0;i<iNumFBConfigs;i++)
      {
         pTempXVisualInfo=glXGetVisualFromFBConfig(gpDisplay,pGLXFBConfigs[i]);
         if(pTempXVisualInfo)
          {
             int sampleBuffer,samples;
             glXGetFBConfigAttrib(gpDisplay,pGLXFBConfigs[i],GLX_SAMPLE_BUFFERS, &sampleBuffer);
             glXGetFBConfigAttrib(gpDisplay,pGLXFBConfigs[i],GLX_SAMPLES,&samples);
            printf("matching FrameBuffer Config = %d : VisualID =0x%lu: SAMPLE_BUFFERS =%d :SAMPLES =%d\n",i,pTempXVisualInfo->visualid,sampleBuffer,samples);
    
         if(bestFramebufferconfig <0 || sampleBuffer && samples >bestNumberOfSamples)
         {
               bestFramebufferconfig=i;
               bestNumberOfSamples=samples;
         }
        if(worstFramebufferConfig < 0 || !sampleBuffer || samples < worstNumberOfSamples)
         {
               worstFramebufferConfig = i;
               worstNumberOfSamples =samples;
         }
         
      }
        XFree(pTempXVisualInfo);
      }
      bestGLXFBConfig = pGLXFBConfigs[bestFramebufferconfig];
      //set global GLXFBConfig
      gGLXFBConfig = bestGLXFBConfig;

     //be sure to free FB COnfig list allocated by glXChooseFBConfig()
     
       XFree(pGLXFBConfigs);
   
       gpXVisualInfo = glXGetVisualFromFBConfig(gpDisplay,bestGLXFBConfig);
        printf("choosen Visual ID =0x%lu\n",gpXVisualInfo->visualid);
          
       //setting window'attribs
         winAttribs.border_pixel=0;
         winAttribs.background_pixmap=0;
         winAttribs.colormap =XCreateColormap(gpDisplay,RootWindow(gpDisplay,gpXVisualInfo->screen),//you can give defalut screen as well
         
          gpXVisualInfo->visual,
          AllocNone);//for "movable"memory allocation
         
        winAttribs.event_mask =StructureNotifyMask | KeyPressMask | ButtonPressMask | ExposureMask | VisibilityChangeMask | PointerMotionMask;

       styleMask= CWBorderPixel | CWEventMask | CWColormap;
 
       gColormap = winAttribs.colormap;
 
       gWindow=XCreateWindow(gpDisplay,                           RootWindow(gpDisplay,gpXVisualInfo->screen),
                             0,
                             0,
                             WIN_WIDTH,
                             WIN_HEIGHT,
                             0,//border width
                             gpXVisualInfo->depth,
                             InputOutput,//class (type)of your window
                             gpXVisualInfo->visual,
                             styleMask,
                             &winAttribs);
            if(!gWindow)
           {
             printf("failure in window creation\n");
             uninitialize();             
              exit(1);
           }

     XStoreName(gpDisplay,gWindow,"OpenGL Window Programmable Pipeline Perspective Triangle");

    Atom windowManagerDelete=XInternAtom(gpDisplay,"WM_WINDOW_DELETE",True);
       
    XSetWMProtocols(gpDisplay,gWindow,&windowManagerDelete,1);
    XMapWindow(gpDisplay,gWindow);
}

void ToggleFullscreen(void)
{
   //code
    Atom wm_state = XInternAtom(gpDisplay,"_NET_WM_STATE",False);
//normal window style
  
    XEvent event;
   memset(&event,0,sizeof(XEvent));
     
    event.type=ClientMessage;
    event.xclient.window=gWindow;
    event.xclient.message_type=wm_state;
    event.xclient.format=32;
    event.xclient.data.l[0]=gbFullscreen ? 0: 1;
  
    Atom fullscreen =XInternAtom(gpDisplay,"_NET_WM_STATE_FULLSCREEN",False);
 
    event.xclient.data.l[1]=fullscreen;

    XSendEvent(gpDisplay,RootWindow(gpDisplay,gpXVisualInfo->screen),
               False,//do not send this message to Sibiling WIndow
               StructureNotifyMask,//resizing mask
               &event);

}

void initialize(void)
{

    //function declarations
     void uninitialize(void);
     void resize(int,int);
   
     //code 
     //create a new GL context 4.5 for rendering 
      
     glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((GLubyte *)"glXCreateContextAttribsARB");

     GLint attribs[] ={ 
                     GLX_CONTEXT_MAJOR_VERSION_ARB,4,
                     GLX_CONTEXT_MINOR_VERSION_ARB,5,
                     GLX_CONTEXT_PROFILE_MASK_ARB,
                     GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                     0};//ARRAY must be terminated by 0

        gGLXContext = glXCreateContextAttribsARB(gpDisplay,gGLXFBConfig,0,True,attribs);

        if(!gGLXContext) //fallback to old style 2.x context
      {
         //when a cntext version below 3.0 is requested, implementations will return
         //the newest context version compatible with OpenGl vsrsion less than varsion 3.0

        GLint attribs[] = {
                 GLX_CONTEXT_MAJOR_VERSION_ARB,1,
                 GLX_CONTEXT_MINOR_VERSION_ARB,0,
                 0};
        printf("failed to create GLX 4.5 context .Hence using old style \n");
       gGLXContext= glXCreateContextAttribsARB(gpDisplay,gGLXFBConfig,0,True,attribs);
     }
    else
      {
        printf("OpenGL COntext 4.5 is Created \n");
      }
    
  if(!glXIsDirect(gpDisplay,gGLXContext))
    {
         printf("indirect GLX Rendering Context Obtained\n");
    }
   else
    {
       printf("Direct GLX Rendering COntext Obtaind \n");
    }
   glXMakeCurrent(gpDisplay,gWindow,gGLXContext);
  
  GLenum glew_error = glewInit();
  if(glew_error !=GLEW_OK)
  {
    glXDestroyContext(gpDisplay,gGLXContext);
  }
  fprintf(gpFile, "%s\n",glGetString(GL_VERSION));
    fprintf(gpFile, "%s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));
    

    //**** VERTEX SHADER *****
    //create shader
    gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    //provide source code to shader
    const GLchar *vertexShaderSourceCode=
         "#version 430 core" \
         "\n"     \
         "in vec4 vPosition;" \
         "uniform mat4 u_mvp_matrix;" \
         "void main(void)"  \
         "{" \
         "gl_Position = u_mvp_matrix * vPosition;" \
         "}";



     //give the above source code to shader object
      glShaderSource(gVertexShaderObject,   
                      1,                   //No.of strings
                      (const char **)&vertexShaderSourceCode,
                      NULL);             //NULL indicates that our strings are null terminated 
                                         //if strings are not null terminated then we will hav to provide a length of string

      //compile shader
      glCompileShader(gVertexShaderObject);
      GLint iInfoLogLength  = 0;        //length of error string 
      GLint iShaderCompiledStatus =0;   //parametarized return value of compilation status
      char *szInfoLog =NULL;            //pointer variable to hold the error in a string
      

      //get the compilation status of vertexShaderObject
      glGetShaderiv(gVertexShaderObject,GL_COMPILE_STATUS,&iShaderCompiledStatus);
      if(iShaderCompiledStatus ==GL_FALSE)    //i.e there is a compilation error
      {
        //get the length of error in a variable iInfoLogLength
         glGetShaderiv(gVertexShaderObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
         if(iInfoLogLength >0)
         {
          szInfoLog  =(char *)malloc(iInfoLogLength);
          if(szInfoLog != NULL)
          {
            GLsizei written;
            
            //get the error 
            //S1: error characters length
            //S2: number of characters actually written
            //S3: pointer variable to hold the error string
            glGetShaderInfoLog(gVertexShaderObject,
                             iInfoLogLength,       //S1
                             &written,             //S2
                             szInfoLog);           //S3
            fprintf(gpFile, "Vertex Shader Compilation Log : %s\n", szInfoLog);
              free(szInfoLog);
              uninitialize();
              exit(0);
          }
         }
      }

      // ***** FRAGMENT SHADER *****
      //create shader
      gFragmentShaderObject =glCreateShader(GL_FRAGMENT_SHADER);

      //provide source code to shader
      const GLchar *fragmentShaderSourceCode=
      "#version 430 core" \
      "\n" \
      "out vec4 FragColor;"\
      "void main(void)"  \
      "{" \
      "FragColor =vec4(1.0,1.0,1.0,1.0);" \
      "}";

     //give above fragment shader programm to FrgmentShaderObject
      glShaderSource(gFragmentShaderObject,
                     1,                     //no.of strings
                     (const char **)&fragmentShaderSourceCode,
                      NULL);                 //NULL indicates that our strings are null terminated 
                                         //if strings are not null terminated then we will hav to provide a length of string
     //compile shader 
      glCompileShader(gFragmentShaderObject);
       iInfoLogLength =0;          //re-initialize the variables for good programming practice
       iShaderCompiledStatus =0;   //re-initialize the variables for good programming practice
     //get the compilation status in parameterized variable
     glGetShaderiv(gFragmentShaderObject,GL_COMPILE_STATUS,&iShaderCompiledStatus);
     if(iShaderCompiledStatus ==GL_FALSE)    //i.e there is a compilation error
     {
      //get the error length in parameterized return value
      glGetShaderiv(gFragmentShaderObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
      if(iInfoLogLength > 0)
      {
        szInfoLog =(char *)malloc(iInfoLogLength);
        if(szInfoLog !=NULL)
        {
          GLsizei written;
          
          //get the error 
            //S1: error characters length
            //S2: number of characters actually written
            //S3: pointer variable to hold the error string
            glGetShaderInfoLog(gFragmentShaderObject,
                              iInfoLogLength,        //S1
                              &written,              //S2
                              szInfoLog);            //S3

            fprintf(gpFile, "Fragment Shader Compilation Log :%s\n", szInfoLog );
            free(szInfoLog);
            uninitialize();
            exit(0);
        }
      }
     }

     // ****** SHADER PROGRAM ******
     //Create  shader program
     gShaderProgramObject =glCreateProgram();

     //attach vertex shader to shader program
     glAttachShader(gShaderProgramObject,gVertexShaderObject);

     //attach fragment shader to shader program
     glAttachShader(gShaderProgramObject,gFragmentShaderObject);

     //pre-link binding of shader program object  with vertex shader position
     //bind our attribute i.e VDG_ATTRIBUTE_VERTEX to "vPosition" variable in vertex shader source code
     glBindAttribLocation(gShaderProgramObject,
                        VDG_ATTRIBUTE_VERTEX,
                        "vPosition");

     //link shader
     glLinkProgram(gShaderProgramObject);
     GLint iShaderProgramLinkStatus =0;
     //get the linking error status in parameterized variable
     glGetProgramiv(gShaderProgramObject,GL_LINK_STATUS,&iShaderProgramLinkStatus);
    
    if(iShaderProgramLinkStatus ==GL_FALSE) //i.e there is a linking error
    {
      //get the length of error string in a parameterized variable
      glGetProgramiv(gShaderProgramObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
      if(iInfoLogLength > 0)
      {
        szInfoLog = (char *)malloc(iInfoLogLength);
        if(szInfoLog != NULL)
        {
          GLsizei written;

          //get the error 
            //S1: error characters length
            //S2: number of characters actually written
            //S3: pointer variable to hold the error string
            glGetProgramInfoLog(gShaderProgramObject,
                              iInfoLogLength,          //S1
                              &written,                //S2
                              szInfoLog);              //S3
            fprintf(gpFile, "Shader Program Link  Log :%s\n",szInfoLog );
            free(szInfoLog);
            uninitialize();
            exit(0);
        }
      }
    } 

    //get MVP uniform location
    //bind uniform location variable ,i.e here u_mvp_matrix, from shader source code to ours variable "
    //so that we can use our variable to refer the uniform location in gpu 
    gMVPUniform =glGetUniformLocation(gShaderProgramObject,"u_mvp_matrix");

    // *** vertices,colors,shaders,attribs,vbo,vao,initializations ***
    const GLfloat triangleVertices[] =
    {
      0.0f,1.0f,0.0f,  //Apex
      -1.0f,-1.0f,0.0f, //left-bottom
      1.0f,-1.0f,0.0f //right-bottom

    };

    glGenVertexArrays(1,    //No. of vao
                    &gVao) ;//i want to run following  instructions letsay "x" no of instructions 
                             //so give me the key to run those instructions repeatedly.
    glBindVertexArray(gVao);//get binded with the above key 
                             //following operations will be regarding this vao
                             //because OpenGL is a state machine

    glGenBuffers(1,&gVbo);   //i want to place the data in GPU so ask GPU to give me the buffer
                             //to store my vertex,normal,color,texture data . In return GPU will return 
                             //a key in "gVbo".
    glBindBuffer(GL_ARRAY_BUFFER,gVbo);//get binded with that key i.e gVbo
                                         //now following operations will affect this gVbo
   
   //Following function will send our data from CPU to GPU
    //S1: send the data in gVbo area which comes under Array Buffer area in GPU
    //S2: size of data =no.of elements * size of data type in array
    //S3: actual data
    //S4: send the data at initialize (i.e STATIC_DRAW) ...another macro can be used  i.e GL_DYANAMIC_DRAW which states that push the data on GPU at runTime 
    glBufferData(GL_ARRAY_BUFFER,    //S1
                sizeof(triangleVertices),  //S2
                triangleVertices,           //S3
                GL_STATIC_DRAW);             //S4
       //following function will attach our enum variable to the above data her square data
//S1.our enum variable name so that it will get attached to the above data which is in above vbo,our enum variable has already attached to the variable in shader,so indirectly here we are attaching that shader varible to the data we are passing to GPU in above steps
//S2.here we tells that vertex color is made up of rgb
//S3.data is of float type
//S4.Data i not normalized take it as it is.
//S5.offset for interleaved data
//S6.address of interleaved data buffer
   glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX,   //S1
                          3,                      //S2
                          GL_FLOAT,              //S3
                          GL_FALSE,               //S4
                          0,                      //S5
                          NULL);                  //S6
  

  //get binded with our enum variable which is linked to "vPosition" variable from vertex shader code
   glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);

  //Unbind the GL_ARRAY_BUFFER by passing "0" as second parameter
   glBindBuffer(GL_ARRAY_BUFFER,0);
   
   //Unbind the vao 
   glBindVertexArray(0);

  //code
  glShadeModel(GL_SMOOTH);
  //SET DEPTH BUFFER
  glClearDepth(1.0f);
  //enable depth testing
  glEnable(GL_DEPTH_TEST);
  //depth test to do
  glDepthFunc(GL_LEQUAL);
  //set really nice perspective calculations 
  glHint(GL_PERSPECTIVE_CORRECTION_HINT ,GL_NICEST);
  //we will always cull back faces for better performance
   glEnable(GL_CULL_FACE);

   //SET BACKGROUND  CLEARING COLOR
   glClearColor(0.0f,0.0f,1.0f,0.0f);

    //set orthographicMatrix to identity matrix
    gPerspectiveProjectionMatrix = mat4::identity();

   //resize
   resize(WIN_WIDTH,WIN_HEIGHT);
}
void resize(int width, int height)
{

  //code
    if(height ==0 )
       height =1;
    glViewport(0,0,(GLsizei)width,(GLsizei)height);

   
  //glPerspective
  gPerspectiveProjectionMatrix=perspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,150.0f);


}
void display(void)
{

  //code
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
     //start using OpenGL Program object 
  glUseProgram(gShaderProgramObject);

  //OpenGL Drawing 
  //set modelView & modelViewProjection matrices to identity
  mat4 modelViewMatrix =mat4::identity();             //mat4 is 4x4 matrix
  mat4 modelViewProjectionMatrix = mat4::identity();
   mat4 translationMatrix =mat4::identity();

     translationMatrix = modelViewMatrix * vmath::translate(0.0f,0.0f,-6.0f);
     modelViewMatrix=modelViewMatrix*translationMatrix;
  //multiply the modelView and orthographic matrix to get modelViewProjection matrix
  modelViewProjectionMatrix = gPerspectiveProjectionMatrix * modelViewMatrix ; //ORDER IS IMPORTANT
    
    //pass above modelviewprojection matrix to the vertex shader in 'u_mvp_matrix' shader variable
    //whose position value we already calculated  by using glGetUniformLocation()
    glUniformMatrix4fv(gMVPUniform,1,GL_FALSE,modelViewProjectionMatrix);

    //*** bind vao ****
    glBindVertexArray(gVao);

    //here we are telling shaders to draw triangle using the data we had pushed in initiliazation
     glDrawArrays(GL_TRIANGLES,0,3); //3(each with its x,y,z) vertices in triangle vertices array

     //****unbind vao***
     glBindVertexArray(0);

     //stop using OpenGL program object
     glUseProgram(0);
  glXSwapBuffers(gpDisplay,gWindow);
}

void uninitialize(void)
{
  //code
   //releasing OpenGL related and XWindow related objects
   //destroy vao
  if(gVao)
  {
    glDeleteVertexArrays(1,&gVao);
    gVao=0;
  }

  //destroy vbo
  if(gVbo)
  {
    glDeleteBuffers(1,&gVbo);
    gVbo=0;
  }

  //detach vertex shader from shader program object
  glDetachShader(gShaderProgramObject,gVertexShaderObject);

  //detach fragment shader from shader program object
  glDetachShader(gShaderProgramObject,gFragmentShaderObject);

  //delete vertex shader object
  glDeleteShader(gVertexShaderObject);
  gVertexShaderObject=0;

  //deleta fragmetn shade object
  glDeleteShader(gFragmentShaderObject);
  gFragmentShaderObject=0;

  //delete shadeer program object
  glDeleteProgram(gShaderProgramObject);
  gShaderProgramObject=0;

  //unlink shader program
  glUseProgram(0);

   GLXContext currentContext=glXGetCurrentContext();
   if(currentContext !=NULL && currentContext == gGLXContext)
   {
     glXMakeCurrent(gpDisplay,0,0);
   } 
  if(gGLXContext)
   {
     glXDestroyContext(gpDisplay,gGLXContext);
   }
  if(gWindow)
   {
     XDestroyWindow(gpDisplay,gWindow);
    }
  if(gColormap)
  {
     XFreeColormap(gpDisplay,gColormap);
   }
  if(gpXVisualInfo)
  {
     free(gpXVisualInfo);
     gpXVisualInfo=NULL;
  }
   if(gpDisplay)
  {
    XCloseDisplay(gpDisplay);
    gpDisplay =NULL;
  }
 if(gpFile)
  {
    fprintf(gpFile,"Log File is successfully Closed.\n");
     fclose(gpFile);
      gpFile=NULL;
  }
}


           

