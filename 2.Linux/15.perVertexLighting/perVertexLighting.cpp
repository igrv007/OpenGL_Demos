#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>


//headers for XServer
#include <X11/Xlib.h>  //analogous to windows.h
#include <X11/Xutil.h>  //for visuals
#include <X11/XKBlib.h> //XkbKeycodeTokeysym
#include <X11/keysym.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glx.h>


#include "vmath.h"
#include "GLError.h"
#include "Sphere.h"

#define WIN_WIDTH  800
#define WIN_HEIGHT 600
#define STEP_ANGLE 0.5f

using namespace vmath;

enum
{
  VDG_ATTRIBUTE_VERTEX =0,
  VDG_ATTRIBUTE_COLOR,
  VDG_ATTRIBUTE_NORMAL,
  VDG_ATTRIBUTE_TEXTURE0,
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

GLuint gNumElements;
GLuint gNumVertices;
float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_elements[2280];


GLuint gVao_sphere;
GLuint gVbo_sphere_position;
GLuint gVbo_sphere_normal;
GLuint gVbo_sphere_element;


GLuint model_matrix_uniform,view_matrix_uniform,projection_matrix_uniform;
GLuint L_keyPressed_uniform;

GLuint La_uniform;
GLuint Ld_uniform;
GLuint Ls_uniform;
GLuint light_position_uniform;

GLuint Ka_uniform;
GLuint Kd_uniform;
GLuint Ks_uniform;

GLuint material_shininess_uniform;
mat4 gPerspectiveProjectionMatrix;

bool gbLight;

GLfloat lightAmbient[]= {0.0f,0.0f,0.0f,1.0f};
GLfloat lightDiffuse[]= {1.0f,1.0f,1.0f,1.0f};
GLfloat lightSpecular[] = {1.0f,1.0f,1.0f,1.0f};
GLfloat lightPosition[] ={100.0f,100.0f,100.0f,1.0f};

GLfloat material_ambient[]={0.0f,0.0f,0.0f,1.0f};
GLfloat material_diffuse[]={1.0f,1.0f,1.0f,1.0f};
GLfloat material_specular[]={1.0f,1.0f,1.0f,1.0f};
GLfloat material_shininess=50.0f;


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
     void update(void);

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
  static bool bIsAKeyPressed =false;
  static bool bIsLKeyPressed = false;


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
                      case XK_L:
                      case XK_l:
                      if(bIsLKeyPressed == false)
                      {
                      	gbLight = true;
                      	bIsLKeyPressed = true;
                      }
                      else
                      {
                      	gbLight = false;
                      	bIsLKeyPressed = false;
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

     XStoreName(gpDisplay,gWindow,"OpenGL Window Programmable Pipeline -Per Vertex Lighting");

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
     int LoadGLTexture(GLuint * ,const char*);
   
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
  fprintf(gpFile, "%s\n",glGetString(GL_SHADING_LANGUAGE_VERSION) );

//************************VERTEX SHADER****************
//create shader
 
 gVertexShaderObject =glCreateShader(GL_VERTEX_SHADER);
 check_gl_error();

 const GLchar *vertexShaderSourceCode =
         "#version 430 core" \
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
      "}" ;

  
 //give this source code to shader object
 
 glShaderSource(gVertexShaderObject,
                   1,                                       //No.Of strings
                   (const char **)&vertexShaderSourceCode,
                   NULL); //NULL indicates that our strings are null terminated 
                                         //if strings are not null terminated then we will hav to provide a length of string

 check_gl_error();

 //compile shader
 glCompileShader(gVertexShaderObject);
 check_gl_error();
 
 GLint iInfoLogLength =0; //length of error string
 GLint iShaderCompilationStatus =0 ;  //parametarized return value of compilation status
 char *szInfoLog= NULL;     //pointer variable to hold the error in a string

 //get the shader compilation status of vertex shader object


 glGetShaderiv(gVertexShaderObject,GL_COMPILE_STATUS,&iShaderCompilationStatus);
 check_gl_error();
 if(iShaderCompilationStatus ==GL_FALSE)
 { 
  //get the length of error in a variable iInfoLogLength
  glGetShaderiv(gVertexShaderObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
  check_gl_error();
  if(iInfoLogLength >0)
  {
    szInfoLog = (char *)malloc(iInfoLogLength);
    if(szInfoLog !=NULL)
    {
      GLsizei written;
           
           //get the error 
            //S1: error characters length
            //S2: number of characters actually written
            //S3: pointer variable to hold the error string
           glGetShaderInfoLog(gVertexShaderObject,
                              iInfoLogLength,         //S1
                              &written,              //S2
                              szInfoLog);            //S3

           check_gl_error();
           fprintf(gpFile, "Vertex Shader Compilation Log:%s\n",szInfoLog );
           free(szInfoLog);
           uninitialize();
           exit(0);
    }
  }
 }

//**************************FRAGMENT SHADER******************
 //create shader
 gFragmentShaderObject=glCreateShader(GL_FRAGMENT_SHADER);
 check_gl_error();

 //provide a source code to Fragment shader
 const GLchar *fragmentShaderSourceCode=
         "#version 430 core"
         "\n"   \
         "in vec3 phong_ads_color;" \
         "out vec4 FragColor;" \
         "void main(void)" \
         "{" \
         "FragColor = vec4(phong_ads_color,1.0);" \
         "}" ;

          //give above fragment shader source code to gFragmentShaderObject
          glShaderSource(gFragmentShaderObject,
                           1,                     //NO.of Strings
                           (const char **)&fragmentShaderSourceCode,  //source code address
                           NULL);//NULL indicates that our strings are null terminated 
                                         //if strings are not null terminated then we will hav to provide a length of string
  //compile shader
          glCompileShader(gFragmentShaderObject);
          check_gl_error();

          iInfoLogLength=0;      //re-initialize the variables for good programming practice
          iShaderCompilationStatus=0;  //re-initialize the variables for good programming practice

          szInfoLog= NULL;
//get the compilation status in parameterized variable
glGetShaderiv(gFragmentShaderObject,GL_COMPILE_STATUS,&iShaderCompilationStatus);
check_gl_error();

if(iShaderCompilationStatus == GL_FALSE)
{
  glGetShaderiv(gFragmentShaderObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
  check_gl_error();
  if(iInfoLogLength >0)
  {
    szInfoLog=(char *)malloc(iInfoLogLength);
    if(szInfoLog != NULL)
     {    
    GLsizei written;
    //get the error 
    //S1: error characters length
    //S2: number of characters actually written
    //S3: pointer variable to hold the error string
      
       glGetShaderInfoLog(gFragmentShaderObject,
                        iInfoLogLength,          //S1
                        &written,               //S2
                        szInfoLog);             //S3
       check_gl_error();
       fprintf(gpFile, "Fragment Shader Compilation Log:%s\n",szInfoLog);
       free(szInfoLog);
       uninitialize();
       exit(0);
      }
  }
}

//**************SHADER PROGRAM*****************
//create program
gShaderProgramObject = glCreateProgram();
check_gl_error();

//attach vertex shader program to shader program
glAttachShader(gShaderProgramObject,gVertexShaderObject);
check_gl_error();
//attach fragment shader program to shader program
glAttachShader(gShaderProgramObject,gFragmentShaderObject);
check_gl_error();
// //pre-link binding of shader program object  with vertex shader position
     //bind our attribute i.e VDG_ATTRIBUTE_VERTEX to "vPosition" variable in vertex shader source code
//also bind our enum  color attribute to "vColor" in fragment shader source code
glBindAttribLocation(gShaderProgramObject,
                   VDG_ATTRIBUTE_VERTEX,
                   "vPosition");
check_gl_error();

glBindAttribLocation(gShaderProgramObject,
                   VDG_ATTRIBUTE_NORMAL,
                   "vNormal");
check_gl_error();


//link shader
glLinkProgram(gShaderProgramObject);
check_gl_error();

GLint iShaderProgramLinkStatus =0;
iInfoLogLength=0;
szInfoLog=NULL;


//get the linking error status in parameterized variable
glGetProgramiv(gShaderProgramObject,
              GL_LINK_STATUS, 
              &iShaderProgramLinkStatus);
check_gl_error();

if(iShaderProgramLinkStatus == GL_FALSE)
{
  glGetProgramiv(gShaderProgramObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
  check_gl_error();
  if(iInfoLogLength >0)
  {
    szInfoLog=(char *)malloc(iInfoLogLength);
    if(szInfoLog !=NULL)
    {
      GLsizei written;

      //get the error 
            //S1: error characters length
            //S2: number of characters actually written
            //S3: pointer variable to hold the error string
      glGetProgramInfoLog(gShaderProgramObject,
                        iInfoLogLength,     //S1
                        &written,          //S2
                        szInfoLog);       //S3
            check_gl_error();
            fprintf(gpFile, "Shader Program Compilation Log:%s\n", szInfoLog);
            free(szInfoLog);
            uninitialize();
            exit(0);
    }
  }
}

//get MVP uniform location
    //bind uniform location variable ,i.e here u_mvp_matrix, from shader source code to ours variable "
    //so that we can use our variable to refer the uniform location in gpu 
    model_matrix_uniform = glGetUniformLocation(gShaderProgramObject,"u_model_matrix");
    check_gl_error();
    view_matrix_uniform = glGetUniformLocation(gShaderProgramObject,"u_view_matrix");
    check_gl_error();
    projection_matrix_uniform = glGetUniformLocation(gShaderProgramObject,"u_projection_matrix");
   
   //L or l key is pressed or not
    L_keyPressed_uniform = glGetUniformLocation(gShaderProgramObject,"u_lighting_enabled");

    //ambient color intensity of light
    La_uniform = glGetUniformLocation(gShaderProgramObject,"u_La");
    check_gl_error();

    Ld_uniform = glGetUniformLocation(gShaderProgramObject,"u_Ld");
    check_gl_error();

    Ls_uniform = glGetUniformLocation(gShaderProgramObject,"u_Ls");
    check_gl_error();

    light_position_uniform = glGetUniformLocation(gShaderProgramObject,"u_light_position");
    check_gl_error();

    //ambient reflective color intensity of material 
    Ka_uniform  = glGetUniformLocation(gShaderProgramObject,"u_Ka");
    check_gl_error();

    Kd_uniform = glGetUniformLocation(gShaderProgramObject,"u_Kd");
    check_gl_error();

    Ks_uniform = glGetUniformLocation(gShaderProgramObject,"u_Ks");
    check_gl_error();

    //shininess of material (value is conventionally between 1 to 200)
    material_shininess_uniform = glGetUniformLocation(gShaderProgramObject,"u_material_shininess");


    //// *** vertices,colors,shaders,attribs,vbo,vao,initializations ***
   getSphereVertexData(sphere_vertices,sphere_normals,sphere_textures,sphere_elements);

   gNumVertices = getNumberOfSphereVertices();
   gNumElements = getNumberOfSphereElements();

   //vao
    glGenVertexArrays(1,&gVao_sphere);
    check_gl_error();

    glBindVertexArray(gVao_sphere);
    check_gl_error();


//position vbo
     glGenBuffers(1,&gVbo_sphere_position);
     check_gl_error();

     glBindBuffer(GL_ARRAY_BUFFER,gVbo_sphere_position);
     check_gl_error();

     glBufferData(GL_ARRAY_BUFFER,
     	           sizeof(sphere_vertices),
     	           sphere_vertices,
     	           GL_STATIC_DRAW);
     check_gl_error();

     glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX,3,GL_FLOAT,GL_FALSE,0,NULL);
     check_gl_error();

     glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
     glBindBuffer(GL_ARRAY_BUFFER,0);

     //normal vbo 
     glGenBuffers(1,&gVbo_sphere_normal);
     check_gl_error();

     glBindBuffer(GL_ARRAY_BUFFER,gVbo_sphere_normal);
     check_gl_error();

     glBufferData(GL_ARRAY_BUFFER,
     	           sizeof(sphere_normals),
     	           sphere_normals,
     	           GL_STATIC_DRAW);
     check_gl_error();

     glVertexAttribPointer(VDG_ATTRIBUTE_NORMAL,
     	                      3,
     	                      GL_FLOAT,
     	                      GL_FALSE,
     	                      0,
     	                      NULL);
     check_gl_error();

     glEnableVertexAttribArray(VDG_ATTRIBUTE_NORMAL);
     check_gl_error();

     glBindBuffer(GL_ARRAY_BUFFER,0);

     //element vbo
     glGenBuffers(1,&gVbo_sphere_element);
     check_gl_error();
     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,gVbo_sphere_element);
     check_gl_error();

     glBufferData(GL_ELEMENT_ARRAY_BUFFER,
     	           sizeof(sphere_elements),
     	           sphere_elements,
     	           GL_STATIC_DRAW);
     check_gl_error();

     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
     check_gl_error();

     glBindVertexArray(0);

     check_gl_error();


  

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
   glClearColor(0.0f,0.0f,0.0f,0.0f);


    //set orthographicMatrix to identity matrix
    gPerspectiveProjectionMatrix = mat4::identity();

   
   gbLight=0;

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
  gPerspectiveProjectionMatrix=perspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);


}
void display(void)
{

  //code
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    //start using OpenGL progrma object
  glUseProgram(gShaderProgramObject);
  check_gl_error();

if(gbLight == true)
{
	//set "u_lighting_enabled" uniform
	glUniform1i(L_keyPressed_uniform,1);

	//setting light's properties
	glUniform3fv(La_uniform,1,lightAmbient);
	glUniform3fv(Ld_uniform,1,lightDiffuse);
	glUniform3fv(Ls_uniform,1,lightSpecular);
	glUniform4fv(light_position_uniform,1,lightPosition);

	//setting material's properties
	glUniform3fv(Ka_uniform,1,material_ambient);
	glUniform3fv(Kd_uniform,1,material_diffuse);
	glUniform3fv(Ks_uniform,1,material_specular);

	glUniform1f(material_shininess_uniform,material_shininess);

}
else
{
	glUniform1i(L_keyPressed_uniform,0);
	check_gl_error();
}
  
  

  mat4 viewMatrix= mat4::identity();
  mat4 modelMatrix = mat4 :: identity();
  

  
//************************SPHERE *******************8
//OpenGL drawing 
   //set modelView & ModelViewPorjection matrices to identity
   //mat4 is 4x4 matr
   modelMatrix = translate(0.0f,0.0f,-2.0f);

   glUniformMatrix4fv(model_matrix_uniform,1,GL_FALSE,modelMatrix);
   glUniformMatrix4fv(view_matrix_uniform,1,GL_FALSE,viewMatrix);
   glUniformMatrix4fv(projection_matrix_uniform,1,GL_FALSE,gPerspectiveProjectionMatrix);

//bind vao
   glBindVertexArray(gVao_sphere);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,gVbo_sphere_element);
   glDrawElements(GL_TRIANGLES,gNumElements,GL_UNSIGNED_SHORT,0);

   glBindVertexArray(0);

glUseProgram(0);
 
check_gl_error();
  glXSwapBuffers(gpDisplay,gWindow);
}


void uninitialize(void)
{
  //code
   //releasing OpenGL related and XWindow related objects
  //destroy vao
  if(gVao_sphere)
	{
		glDeleteVertexArrays(1,&gVao_sphere);
		gVao_sphere=0;
	}
		if(gVbo_sphere_position)
	{
		glDeleteBuffers(1,&gVbo_sphere_position);
		gVbo_sphere_position=0;
	}
	if(gVbo_sphere_normal)
	{
		glDeleteBuffers(1,&gVbo_sphere_normal);
		gVbo_sphere_normal=0;
	}
	if(gVbo_sphere_element)
	{
		glDeleteBuffers(1,&gVbo_sphere_element);
		gVbo_sphere_element=0;
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


           

