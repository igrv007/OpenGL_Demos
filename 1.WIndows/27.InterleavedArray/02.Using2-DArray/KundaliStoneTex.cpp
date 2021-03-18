#include <windows.h>
#include <stdio.h>

#include <GL\glew.h>  //for GLSL extensions IMP: This line should be before gl.h

#include <gl\GL.h>

#include "vmath.h"
#include "GLError.h"
#include "texture.h"

#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"opengl32.lib")

#define WIN_WIDTH   800
#define WIN_HEIGHT  600
#define STEP_ANGLE  0.2f;
using namespace vmath;

enum
{
	VDG_ATTRIBUTE_VERTEX =0,
	VDG_ATTRIBUTE_COLOR,
	VDG_ATTRIBUTE_NORMAL,
	VDG_ATTRIBUTE_TEXTURE0,
};

//callback function prototype
LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);

//Global variable declarations
FILE *gpFile =NULL;

HWND ghwnd =NULL;
HDC ghdc=   NULL;
HGLRC ghrc =NULL;

DWORD dwStyle;
WINDOWPLACEMENT wpPrev{sizeof(WINDOWPLACEMENT)};

bool gbActiveWindow        =false;
bool gbEscapeKeyIsPressed  =false;
bool gbFullscreen          =false;

GLuint gVertexShaderObject;
GLuint gFragmentShaderObject;
GLuint gShaderProgramObject;

GLuint gVao_cube;

GLuint gVbo_cube;


GLfloat angle_cub=0.0f;

GLint modelMatrixUniform,viewMatrixUniform,projectionMatrixUniform;
GLint laUniform,lsUniform,ldUniform,lightPositionUniform;
GLint kaUniform,kdUniform,ksUniform,materialShininessUniform;

int lKeyPressedUniform;
int lKeyPressed;
    
mat4 gPerspectiveProjectionMatrix;

GLuint gTexture_sampler_uniform;
GLuint gTexture_Marble;
bool gbLight;

GLfloat light_ambient[] = {0.25f,0.25f,0.25f,1.0f};
GLfloat light_diffuse[] = {1.0f,1.0f,1.0f,1.0f};
GLfloat light_specular[] = {1.0f,1.0f,1.0f,1.0f};
GLfloat light_position[] = {100.0f,100.0f,100.0f,1.0f};

GLfloat material_ambient[]  = {0.0f ,0.0f,0.0f,1.0f};
GLfloat material_diffuse[] ={1.0f,1.0f,1.0f,1.0f};
GLfloat material_specular[] = {1.0f,1.0f,1.0f,1.0f};
GLfloat material_shininess = 128.0f;

//main
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpszCmdLine,int iCmdShow)
{
	//function prototype
	void initialize(void);
	void uninitialize(void);
	void display(void);
    void update(void);

	//variable declarations
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] =TEXT("RTRAssignment");
	bool bDone =false;

   //code
	//create log file 
    if(fopen_s(&gpFile,"Log.txt","w") != 0)
    {
    	MessageBox(NULL,TEXT("failed To create log file...\n"),TEXT("ERROR"),MB_OK);
    	exit(0);

    }
    else
    {
    	fprintf(gpFile, "Log File is Created successfully..\n");
    }

     
	//initializing members of struct WNDCLASSEX
	wndclass.cbSize =sizeof(WNDCLASSEX);
	wndclass.style  =CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra=0;
	wndclass.cbWndExtra=0;
	wndclass.hInstance=hInstance;
	wndclass.hbrBackground =(HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.hIcon =LoadIcon(NULL,IDI_APPLICATION);
	wndclass.hIconSm=LoadIcon(NULL,IDI_APPLICATION);
	wndclass.hCursor=LoadCursor(NULL,IDC_ARROW);
	wndclass.lpfnWndProc=WndProc;
	wndclass.lpszMenuName=NULL;
	wndclass.lpszClassName=szAppName;

  //Register class
	if(!RegisterClassEx(&wndclass))
	{
		MessageBox(NULL,TEXT("RegisterClass Failed"),TEXT("ERROR"),MB_OK);
			fclose(gpFile);
            gpFile=NULL;
			exit(0);
	}

	UINT uiWidth = GetSystemMetrics(SM_CXSCREEN);
	UINT uiHeight= GetSystemMetrics(SM_CYSCREEN);

	UINT uiXPos = (uiWidth/2)-(WIN_WIDTH/2);
	UINT uiYPos = (uiHeight/2) -(WIN_HEIGHT/2);

	//create window
	hwnd =CreateWindow(szAppName,
			TEXT("OpenGL Programmable Pipeline Kundali and Texture"),
			WS_OVERLAPPEDWINDOW ,
			uiXPos,
			uiYPos,
			WIN_WIDTH,
			WIN_HEIGHT,
			NULL,
			NULL,
			hInstance,
			NULL);


    if(!hwnd)
		{
			MessageBox(NULL,TEXT("CreateWindow failed"),TEXT("ERROR"),MB_OK);
            fclose(gpFile);
            gpFile=NULL;
			exit(0);
		}

   ghwnd =hwnd;

   ShowWindow(hwnd,iCmdShow);
   SetForegroundWindow(hwnd);
   SetFocus(hwnd);

   //initialize
   initialize();

   while(bDone == false)
   {
   	 //Remark: PeekMessage is NOT a blooking function 
   	 if(PeekMessage(&msg,          //pointer to structure for window message
   	 	          (HWND)NULL,      //handle to a window
   	 	          0,               //First message in queue
   	 	          0,               //last message in queue (select all messages)
   	 	          PM_REMOVE))      //Remove message from queue after processing from PeekMessage
       {
       	  if(msg.message ==WM_QUIT)   //if current message is quit message then exit game loop
             bDone=true;              //causes game loop to exit
          else
          {
          	TranslateMessage(&msg);  //Translate virtual key message into character message
          	DispatchMessage(&msg);  //Dispatches message to Window procedure
          }
       }

       else                                     //if there is no message queue then do rendering
       { 
       	   //rendering function 
       	    update();
       	    display();
       	if(gbActiveWindow ==true)               //if window is foreground (receiving keystrokes)
       	 { 
       		if(gbEscapeKeyIsPressed ==true)     //if escape key is pressed
       	      bDone=true;
       	 }
       }
   
   }
 uninitialize();

  return((int)msg.wParam);
}

//WNDPROC
LRESULT CALLBACK WndProc(HWND hwnd,UINT iMsg,WPARAM wParam,LPARAM lParam)
{
	//function prototype
    void resize(int,int);
    void ToggleFullscreen(void);
	void uninitialize(void);

	//variable deaclarations
	static WORD xMouse = NULL;
	static WORD yMouse =NULL;

	switch(iMsg)
	{
		case WM_ACTIVATE:
			if(HIWORD(wParam) == 0)               //if 0,the window is active
				gbActiveWindow =true;
			else
				gbActiveWindow =false;           //if non-zero the window is not active
		break;
		case WM_ERASEBKGND:
			return(0);
		case WM_SIZE:
			resize(LOWORD(lParam),HIWORD(lParam));
		break;
		case WM_KEYDOWN:
			switch(wParam)
			{
				case VK_ESCAPE:
					if(gbEscapeKeyIsPressed == false)
						gbEscapeKeyIsPressed=true;
					
 				break;
                case 0x46:  //for f or F
                 if(gbFullscreen ==false)
                 {
                 	ToggleFullscreen();
                 	gbFullscreen=true;

                 }
                 else
                 {
                 	ToggleFullscreen();
                 	gbFullscreen =false;
                 }
                break;
                case 0x4c:    //for "L" or 'l'
                  lKeyPressed++;
                  if(lKeyPressed > 1)
                    lKeyPressed =0;
                break;
                default: 
                break;
			}
		break;
		case WM_CLOSE:
            uninitialize();
		break;
		case WM_DESTROY:
		PostQuitMessage(0);
		break;
		default:
		break;

	}
	return(DefWindowProc(hwnd,iMsg,wParam,lParam));
}
void ToggleFullscreen(void)
{


	//varible declarations
	MONITORINFO mi;

	//code
	if(gbFullscreen == false)
	{
		dwStyle =GetWindowLong(ghwnd,GWL_STYLE);
		if(dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi = { sizeof(MONITORINFO)};
			if(GetWindowPlacement(ghwnd,&wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd,MONITORINFOF_PRIMARY),&mi))
			{
				//if wpPrev is successfully filled with current window placement and 
				//if mi is successfully filled with primary monitor info then 
				//S1:Remove WS_OVERLAPPEDWINDOW style
				//S2: Set window position by aligning left-top point's XY coordinates to 
				//monitors left-top coordinates and setting window width and height to
				//monitors width and height (effectively making window full screen)
				//SWP_NOZORDER:Dont change th Z-Order
				//SWP_FRAMECHANGED :Forces recalculation of non-client area
				SetWindowLong(ghwnd,GWL_STYLE,dwStyle & ~WS_OVERLAPPEDWINDOW);    //S1
				SetWindowPos(ghwnd,HWND_TOP,mi.rcMonitor.left,mi.rcMonitor.top,    //S2
				            mi.rcMonitor.right - mi.rcMonitor.left,
                            mi.rcMonitor.bottom-mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
					
			}
			//make cursor disappear
			ShowCursor(FALSE);
		}

	}
	else
	{
       //screen is full screen so,toggle to previously saved dimension
		//S1: Add WS_OVERLAPPEDWINDOW to window style via SetWindowLong
		//S2: Set window placement to stored previous placement in wpPrev via SetWindowPlacement
		//S3: Force the placement effects of SetWindowPlacement by call to SetWindowPos With
		//   SWP_NOMOVE :Don't change left top point (i.e ignore third and fourth parameter)
		//   SWP_NOSIZE :Don't change dimensions of window (i.e ignore fifth and sixth parameter)
		//   SWP_NOZORDER:Don't change Z-order of the window and its children
		//   SWP_NOOWNERZORDER: Don't change Z-order of owner of the window(refered by ghwnd)
		//   SWP_FRAMECHANGED :Force recalculation of non-client area.
		//S4: Make Sure Cursor is visible
        SetWindowLong(ghwnd,GWL_STYLE,dwStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(ghwnd,&wpPrev);
        SetWindowPos(ghwnd,HWND_TOP,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

        ShowCursor(TRUE);
	}

}

void initialize(void)
{
	//function prototype
	void uninitialize(void);
	void resize(int,int);
	int LoadTextures(GLuint *, TCHAR[]);


	PIXELFORMATDESCRIPTOR pfd;  //Information strucutre describing the format
	int iPixelFormatindex =-1;  //Index to Pixel format structure in the internal tables


	//fill pfd by zeros
	ZeroMemory((void *)&pfd,sizeof(PIXELFORMATDESCRIPTOR));

	//Initialize PIXELFORMATDESCRIPTOR with desired values of its attribute
	//pfd.nSize : Allows Microsoft to maintainmultiple versions of structure
	//pfd.nVersion :version infomation
	//pfd.dwFlags
	//       PFD_DRAW_TO_WINDOW :Indiacates real time rendering ie.draw on window
	//       PFD_SUPPORT_OPENGL : Support for OpenGL rendering 
	//       PFD_DOUBLEBUFFER :  use DoubleBuffer
	//pfd.iPixelType: Type Of pixel format to be chosen 
	//pfd.cColorBits :Specifies color depth in bits 
	//pfd.cRedBits:   Specifiies bit depth for red color
	//pfd.cGreenBits: Specifiies bit depth for green color
	//pfd.cBlueBits:  Specifiies bit depth for blue color
	//pfd.cAlphabits: Specifiies bit depth for alpha component
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion =1;    
    pfd.dwFlags =PFD_DRAW_TO_WINDOW |PFD_SUPPORT_OPENGL |PFD_DOUBLEBUFFER;
    pfd.iPixelType =PFD_TYPE_RGBA;
    pfd.cColorBits =32;
    pfd.cRedBits   =8;
    pfd.cGreenBits =8;
    pfd.cBlueBits  =8;
    pfd.cAlphaBits =8;
    pfd.cDepthBits =32;

    ghdc =GetDC(ghwnd);

    //choose a pixel format which is best mathches with that o f'pfd'
    iPixelFormatindex =ChoosePixelFormat(ghdc,&pfd);
    if(iPixelFormatindex == 0)
    {
    	ReleaseDC(ghwnd,ghdc);
    	ghdc=NULL;
    }

    //set the pixel format choosen above
    if(SetPixelFormat(ghdc,iPixelFormatindex,&pfd) == false)
    {
    	ReleaseDC(ghwnd,ghdc);
    	ghdc=NULL;
    }

    //create OpenGL rendering context
    ghrc = wglCreateContext(ghdc);
    if(ghrc == NULL)
    {
    	ReleaseDC(ghwnd,ghdc);
    	ghdc=NULL;
    }

    //make the  rendering context created above as  current hdc
    if(wglMakeCurrent(ghdc,ghrc) ==false)
    {
    	wglDeleteContext(ghrc);
    	ghrc =NULL;
    	ReleaseDC(ghwnd,ghdc);
    	ghdc=NULL;

    }

      //GLEW Initialization code for GLSL (IMP: It must be here. Means after  creating OpenGL Context but before using any OpenGL Function )
    GLenum glew_error = glewInit();
    if(glew_error != GLEW_OK)
    {
    	wglDeleteContext(ghrc);
    	ghrc=NULL;
    	ReleaseDC(ghwnd,ghdc);
    	ghdc=NULL;
    }
  fprintf(gpFile, "%s\n",glGetString(GL_VERSION));
  fprintf(gpFile, "%s\n",glGetString(GL_SHADING_LANGUAGE_VERSION) );

//*************VERTEX SHADER*****************8
//create shader

gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
check_gl_error();

const GLchar *vertexShaderSourceCode=
  "#version 430 core"  \
  "\n"  \
  "in vec4 vPosition;"  \
  "in vec3 vNormal;"\
   "in vec4 vColor;"    \
  "out vec4 out_color;"  \
  "uniform mat4 u_model_matrix;" \
  "uniform mat4 u_view_matrix;"  \
  "uniform mat4 u_projection_matrix;" \
  "uniform int u_lighting_enabled;" \
  "uniform vec4 u_light_position;" \
  "out vec3 transformed_normals;" \
  "out vec3 light_direction;" \
  "out vec3 viewer_vector;"  \
  "in vec2 vTexture0_coord;"    \
  "out vec2 out_texture0_coord;"  \
  "void main(void)"   \
  "{"  \
   "if(u_lighting_enabled == 1)" \
   "{" \
   "vec4 eye_coordinates = u_view_matrix * u_model_matrix * vPosition;" \
   "transformed_normals = mat3(u_view_matrix * u_model_matrix) * vNormal;" \
   "light_direction = vec3(u_light_position) - eye_coordinates.xyz;" \
   "viewer_vector = - eye_coordinates.xyz;"\
   "}" \
   "gl_Position=u_projection_matrix * u_view_matrix *u_model_matrix *vPosition;" \
  "out_texture0_coord =vTexture0_coord;"\
   "out_color =vColor;"\
  "}";

  //give this source code to shader object
  glShaderSource(gVertexShaderObject,
        	            1,                                   //No.of Strings
        	            (const char **)&vertexShaderSourceCode,//source code
        	            NULL); //NULL indicates that our strings are null terminated 
                                         //if strings are not null terminated then we will hav to provide a length of string
check_gl_error();
//compile shader
glCompileShader(gVertexShaderObject);
check_gl_error();

GLint iInfoLogLength =0;             //length of error string 
GLint iShaderCompilationStatus =0;   //parametarized return value of compilation status
char *szInfoLog =NULL;        //pointer variable to hold the error in a string

//get the shader compilation status of vertex shader object
glGetShaderiv(gVertexShaderObject,GL_COMPILE_STATUS,&iShaderCompilationStatus);
check_gl_error();
if(iShaderCompilationStatus == GL_FALSE)
{
	glGetShaderiv(gVertexShaderObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
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
			glGetShaderInfoLog(gVertexShaderObject,
				               iInfoLogLength,           //S1
				               &written,                 //S2
				               szInfoLog);               //S3
        check_gl_error();
        fprintf(gpFile, "Vertex Shader Compilation Log:%s\n",szInfoLog );
        free(szInfoLog);
        uninitialize();
        exit(0);
		}
	}
}

//**********************FRAGMENT SHADER********************
//create shader
gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
check_gl_error();

//Provide source code to fragment shader
GLchar *fragmentShaderSourceCode=
      "#version 430 core"  \
      "\n"   \
       "in vec3 transformed_normals;" \
        "in vec3 light_direction;" \
        "in vec3 viewer_vector;" \
        "in vec4 out_color;"   \
        "out vec4 FragColor;" \
        "uniform vec3 u_La;" \
        "uniform vec3 u_Ld;" \
        "uniform vec3 u_Ls;" \
        "uniform vec3 u_Ka;" \
        "uniform vec3 u_Kd;" \
        "uniform vec3 u_Ks;" \
        "uniform float u_material_shininess;" \
        "uniform int u_lighting_enabled;"  \
      "in vec2 out_texture0_coord;"   \
      "uniform sampler2D u_texture0_sampler;" \
      "void main(void)"    \
      "{"    \
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
    "vec3 tex = vec3(texture(u_texture0_sampler,out_texture0_coord));" \
    "vec3 final_color = tex * phong_ads_color * vec3(out_color);" \
    "FragColor = vec4(final_color,1.0f);"      
    "}";

      //give above fragment shader source code to fragmentShaderObject
      glShaderSource(gFragmentShaderObject,
          	             1,         //NO of Strings
          	             (const char **)&fragmentShaderSourceCode,//source code 
          	             NULL);//NULL indicates that our strings are null terminated 
                                         //if strings are not null terminated then we will hav to provide a length of string
          check_gl_error();
  //compile shader
  glCompileShader(gFragmentShaderObject);
  check_gl_error();

  iInfoLogLength=0;                 //re-initialize the variables for good programming practice
  iShaderCompilationStatus=0;         //re-initialize the variables for good programming practice
  szInfoLog=NULL;

  //get the compilation status in parameterized varible
  glGetShaderiv(gFragmentShaderObject,GL_COMPILE_STATUS,&iShaderCompilationStatus);
  check_gl_error();
  if(iShaderCompilationStatus == GL_FALSE)
  {
  	glGetShaderiv(gFragmentShaderObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
  	check_gl_error();
  	if(iInfoLogLength > 0)
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
               fprintf(gpFile, "Fragment shader compilation status:%s\n",szInfoLog );
               free(szInfoLog);
               uninitialize();
               exit(0);
  		}
  	}
  }

//****************************SHADER PROGRAM********************
 //create program
  gShaderProgramObject=glCreateProgram();
  check_gl_error();

  //attach vertex shader  to shader program
  glAttachShader(gShaderProgramObject,gVertexShaderObject);
  check_gl_error();
  //attach fragment shader to shader program
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
                         VDG_ATTRIBUTE_COLOR,
                         "vColor");
   glBindAttribLocation(gShaderProgramObject,
   	                     VDG_ATTRIBUTE_TEXTURE0,
   	                     "vTexture0_coord");
    check_gl_error();

 //link program
    glLinkProgram(gShaderProgramObject);
    check_gl_error();

    GLint iShaderProgramLinkStatus=0;
    iInfoLogLength=0;
    szInfoLog=NULL;

    glGetProgramiv(gShaderProgramObject,
    	            GL_LINK_STATUS,
    	            &iShaderProgramLinkStatus);
    check_gl_error();
    if(iShaderProgramLinkStatus ==GL_FALSE)
    {
    	glGetProgramiv(gFragmentShaderObject,
    		            GL_INFO_LOG_LENGTH,
    		            &iInfoLogLength);
    	if(iInfoLogLength>0)
    	{
    		szInfoLog=(char *)malloc(iInfoLogLength);
    		if(szInfoLog != NULL)
    		{
    			GLsizei written;
    			//get the error 
         		//S1: error characters length
         		//S2: number of characters actually written
         		//S3: pointer variable to hold the error string
                glGetProgramInfoLog(gShaderProgramObject,
                	                 iInfoLogLength,   //S1
                	                 &written,          //S2
                	                 szInfoLog);         //S3
                check_gl_error();
                free(szInfoLog);
                uninitialize();
                exit(0);

    		}
    	}
    }

//get MVP uniform location
    //bind uniform location variable ,i.e here u_mvp_matrix, from shader source code to ours variable "
    //so that we can use our variable to refer the uniform location in gpu
   //get MVP Uniform location
    modelMatrixUniform= glGetUniformLocation(gShaderProgramObject,"u_model_matrix");
    viewMatrixUniform = glGetUniformLocation(gShaderProgramObject,"u_view_matrix");
    projectionMatrixUniform = glGetUniformLocation(gShaderProgramObject,"u_projection_matrix");
    
    lKeyPressedUniform = glGetUniformLocation(gShaderProgramObject,"u_lighting_enabled");
    laUniform = glGetUniformLocation(gShaderProgramObject, "u_La");
    ldUniform = glGetUniformLocation(gShaderProgramObject,"u_Ld");
    lsUniform = glGetUniformLocation(gShaderProgramObject,"u_Ls");
    
    kaUniform = glGetUniformLocation(gShaderProgramObject,"u_Ka");
    kdUniform = glGetUniformLocation(gShaderProgramObject,"u_Kd");
    ksUniform = glGetUniformLocation(gShaderProgramObject,"u_Ks");
    
    lightPositionUniform = glGetUniformLocation(gShaderProgramObject,"u_light_position");
    materialShininessUniform = glGetUniformLocation(gShaderProgramObject ,"u_material_shininess");

    check_gl_error();
    gTexture_sampler_uniform = glGetUniformLocation(gShaderProgramObject,"u_texture0_sampler");

        //// *** vertices,colors,shaders,attribs,vbo,vao,initializations ***
 

//###############End of Pyramid Related Vao and Vbo#####################

//###############Start of Cube related Vao and Vbo#########################

const GLfloat cubeVCNT[24][11]=
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



 glGenVertexArrays(1,&gVao_cube);
    glBindVertexArray(gVao_cube);
    
    glGenBuffers(1,&gVbo_cube);
    glBindBuffer(GL_ARRAY_BUFFER,gVbo_cube);
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

glShadeModel(GL_SMOOTH);   //set the shading mdel

  glEnable(GL_DEPTH_TEST);    //enable depth test in the state machine
  glDepthFunc(GL_LEQUAL);      //Specify the depth comparison function .GL_LEQUAL:paases if incoming z value is less than
                              //or equal to stored z value
  glClearDepth(1.0f);     //specifies the clear value for depth buffer
  glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
  
  glEnable(GL_CULL_FACE);

  LoadTextures(&gTexture_Marble,MAKEINTRESOURCE(IDBITMAP_MARBLE));
  
  glEnable(GL_TEXTURE_2D);//enable texture mapping

  //set background color to which it will display even if it will empty.
  glClearColor(0.0f,0.0f,0.0f,0.0f);///black

  //set orthographicMatrix to identity matrix
  gPerspectiveProjectionMatrix=mat4:: identity();
  resize(WIN_WIDTH,WIN_HEIGHT);
 
}

int LoadTextures(GLuint *texture,TCHAR imageResourceId[])
{
  //variable declarations
  HBITMAP hBitmap;
  BITMAP bmp;
  int iStatus=FALSE;

  //code
  glGenTextures(1,texture); //1 image
  hBitmap=(HBITMAP)LoadImage(GetModuleHandle(NULL),imageResourceId,IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION);
  if(hBitmap)//if bitmap exists
  {
    iStatus = TRUE;
    GetObject(hBitmap,sizeof(bmp),&bmp);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);//set 1 rather than default 4, for better performance
    glBindTexture(GL_TEXTURE_2D,*texture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,
      0,   //internal image format
      GL_RGB,
      bmp.bmWidth,
      bmp.bmHeight,
      0,  //external image format
      GL_BGR,
      GL_UNSIGNED_BYTE,
      bmp.bmBits);

    //create mipmaps for this texture for better image quality
    glGenerateMipmap(GL_TEXTURE_2D);

    DeleteObject(hBitmap); //delete unwanted bitmap handle
  }
  return(iStatus);

}


void display(void)
{
	//code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


	//star using OpenGL program object 
	glUseProgram(gShaderProgramObject);

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
    
	
  //****************************SQUARE************************
  ////OpenGL drawing 
	 //set modelView & ModelViewPorjection matrices to identity
	 //mat4 is 4x4 matrix
   vmath::mat4 modelMatrix = vmath::mat4::identity();
    vmath::mat4 viewMatrix = vmath::mat4::identity();
  
    
    modelMatrix = modelMatrix * vmath::translate(0.0f,0.0f,-7.0f);
    modelMatrix = modelMatrix * vmath::rotate(angle_cub,angle_cub,angle_cub);
    glUniformMatrix4fv(modelMatrixUniform,1,GL_FALSE,modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform,1,GL_FALSE,viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform,1,GL_FALSE,gPerspectiveProjectionMatrix);
        check_gl_error();

    //bind with cube texture
    glActiveTexture(GL_TEXTURE0); //0th texture(corresponds to VDG_ATTRIBUTE_TEXUTRE0)
    glBindTexture(GL_TEXTURE_2D,gTexture_Marble);
    glUniform1i(gTexture_sampler_uniform,0);//0 th sampler enable as we have only one texture sampler in fragment shader


    ////*****************bind vao_cube*********************
  
    glBindVertexArray(gVao_cube);

    check_gl_error();

    glDrawArrays(GL_TRIANGLE_FAN,0,4);   //seond parameter is a offset 
    check_gl_error();
    
    glDrawArrays(GL_TRIANGLE_FAN,4,4);   //start from 4 th index after taking 4 vertices
    check_gl_error();

    glDrawArrays(GL_TRIANGLE_FAN,8,4);    //start from 8 th index after taking 4 vertices 
    check_gl_error();

    glDrawArrays(GL_TRIANGLE_FAN,12,4);   //start from 12 th index after taking 4 vertices    
    check_gl_error();

    glDrawArrays(GL_TRIANGLE_FAN,16,4);   //start from 16 th index after taking 4 vertices 
    check_gl_error();

    glDrawArrays(GL_TRIANGLE_FAN,20,4);  //start from 20 th index after taking 4 vertices 
    check_gl_error();

    glBindVertexArray(0);
    check_gl_error();

    glUseProgram(0);

    SwapBuffers(ghdc);
}


 void update(void)
 {
 	

 	angle_cub=angle_cub +STEP_ANGLE;
 	if(angle_cub >=360.0f)
 		angle_cub=angle_cub-360.0f;
 }
void resize(int width,int height)
{
	if(height ==0)
		height=1;
	glViewport(0,0,(GLsizei)width,(GLsizei)height);

	//glPerspective
	gPerspectiveProjectionMatrix=perspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

}
void uninitialize(void)
{
	if(gbFullscreen == true)
	{
		dwStyle =GetWindowLong(ghwnd,GWL_STYLE);
		SetWindowLong(ghwnd,GWL_STYLE,dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd,&wpPrev);
		SetWindowPos(ghwnd,HWND_TOP,0,0,0,0,SWP_NOMOVE |SWP_NOSIZE|SWP_NOZORDER |SWP_NOOWNERZORDER |SWP_FRAMECHANGED);
        ShowCursor(TRUE);

	}

	//destroy vao
	if(gVao_cube)
	{
		glDeleteVertexArrays(1,&gVao_cube);
		gVao_cube=0;
	}
  
	if(gVbo_cube)
	{
		glDeleteBuffers(1,&gVbo_cube);
		gVbo_cube=0;
	}

  if(gTexture_Marble)
  {
    glDeleteTextures(1,&gTexture_Marble);
    gTexture_Marble=0;
  }
 
    //detach vertex shader from shader program object
    glDetachShader(gShaderProgramObject,gVertexShaderObject);

    //detach fragment shader from shader program object
    glDetachShader(gShaderProgramObject,gFragmentShaderObject);

    //delete  vertex shader object
    glDeleteShader(gVertexShaderObject);
    gVertexShaderObject=0;

    //delelte frgment shader object
    glDeleteShader(gFragmentShaderObject);
    gFragmentShaderObject=0;

    //delete shader progrma object
    glDeleteProgram(gShaderProgramObject);
    gShaderProgramObject=0;

    //unlink shader program
    glUseProgram(0);

    //deselect the rendering context
    wglDeleteContext(ghrc);
    ghrc=0;

    //delete the devise context
    ReleaseDC(ghwnd,ghdc);
    ghdc=0;

    if(gpFile)
    {
    	fprintf(gpFile, "Log File is closed successfully\n" );
    	fclose(gpFile);
    	gpFile=0;
    } 

    
}
















