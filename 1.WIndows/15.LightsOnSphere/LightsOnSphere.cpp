#include <windows.h>
#include <stdio.h>

#include <gl/glew.h>

#include <gl/GL.h>

#include "vmath.h"
#include "GLError.h"

#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"opengl32.lib")

#define WIN_WIDTH   800
#define WIN_HEIGHT  600

using namespace vmath;

enum
{
	VDG_ATTRIBUTE_VERTEX=0,
	VDG_ATTRIBUTE_COLOR,
	VDG_ATTRIBUTE_NORMAL,
	VDG_ATTRIBUTE_TEXTURE0,
};

//WndProc
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
GLuint gVbo_cube_position;
GLuint gVbo_cube_normal;

GLuint gModelViewMatrixUniform,gProjectionMatrixUniform;
GLuint gLdUniform,gKdUniform,gLightPositionUniform;

GLuint gLKeyPressedUniform;
mat4 gPerspectiveProjectionMatrix;

GLfloat gAngle =0.0f;

bool gbAnimate;
bool gbLight;

//main()
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
       	    
       	    display();
       	    if(gbAnimate == true)
       	    	update();
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

	static bool bIsAKeyPressed =false;
	static bool bIsLKeyPressed =false;

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
                case 0x41:    //for "A" ot 'a'
                if(bIsAKeyPressed == false)
                {
                	gbAnimate =true;
                	bIsAKeyPressed =true;
                }
                else
                {
                	gbAnimate = false;
                	bIsAKeyPressed =false;
                }
                break;
                case 0x4C:
                if(bIsLKeyPressed == false)
                {
                	gbLight =true;
                	bIsLKeyPressed = true;
                }
                else
                {
                    gbLight =false;
                    bIsLKeyPressed = false;
                }
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


   //*******************VERTEX SHADER ********************
  //create shader
  gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

  //provide source code to shader
  const GLchar *vertexShaderSourceCode=
          "#version 430 core"  \
          "\n"   \
          "in vec4 vPosition;"  \
          "in vec3 vNormal;"  \
          "uniform mat4 u_model_view_matrix;" \
          "uniform mat4 u_projection_matrix;" \
          "uniform int u_LKeyPressed;" \
          "uniform vec3 u_Ld;"  \
          "uniform vec3 u_Kd;"  \
          "uniform vec3 u_light_position;" \
          "out vec3 diffuse_light;"   \
          "void main(void)"  \
          "{" \
          "if(u_LKeyPressed == 1)" \
          "{" \
          "vec4 eyeCoordinates = u_model_view_matrix * vPosition;" \
          "vec3 tnorm = normalize(mat3(u_model_view_matrix) * vNormal);"\
          "vec3 source_vector =normalize(vec3(u_light_position - eyeCoordinates)); " \
          "diffuse_light = u_Ld * u_Kd * max(dot(source_vector,tnorm),0.0);" \
          "}" \
          "gl_Position = u_projection_matrix * u_model_view_matrix * vPosition;" \
          "}";


          glShaderSource(gVertexShaderObject,
          	              1,                                      //No.of Strings
          	              (const char **)&vertexShaderSourceCode,  //sourc code
          	              NULL);    //NULL indicates that our strings are null terminated 
                                         //if strings are not null terminated then we will hav to provide a length of string


          //compile shader
          glCompileShader(gVertexShaderObject);
          check_gl_error();

          GLint iInfoLogLength =0;
          GLint iShaderCompilationStatus =0;
          char *szInfoLog  =NULL;

         //get the compilation status in parameterized variable
          glGetShaderiv(gVertexShaderObject,
          	            GL_COMPILE_STATUS,
          	            &iShaderCompilationStatus);
          check_gl_error();
          if(iShaderCompilationStatus == GL_FALSE)
          {
          	glGetShaderiv(gVertexShaderObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
          	if(iInfoLogLength >0)
          	{
          		szInfoLog = (char *)malloc(iInfoLogLength);
          		if(szInfoLog != NULL)
          		{
          			GLsizei written;
          			//get the error 
         		//S1: error characters length
         		//S2: number of characters actually written
         		//S3: pointer variable to hold the error string
          			glGetShaderInfoLog(gVertexShaderObject,
          				                iInfoLogLength,       //s1
          				                &written,            //s2
          				                szInfoLog);          //s3
          			check_gl_error();
          			fprintf(gpFile, "VertexShader compilation Log:%s\n",szInfoLog );
          			free(szInfoLog);
          			uninitialize();
          			exit(0);
          		}
          	}
          }

    
    //*******************FRAGMENT SHADER *****************
    //create shader
    gFragmentShaderObject = glCreateShader (GL_FRAGMENT_SHADER);
    check_gl_error();

    const GLchar *fragmentShaderSourceCode = 
       "#version 430 core" \
       "\n"  \
       "in vec3 diffuse_light;"  \
       "out vec4 FragColor;"  \
       "uniform int u_LKeyPressed;"  \
       "void main(void)"  \
       "{"  \
       "vec4 color;"  \
       "if(u_LKeyPressed == 1)"  \
       "{" \
       "color = vec4(diffuse_light,1.0);" \
       "}" \
       "else" \
       "{" \
       "color = vec4(1.0f,1.0f,1.0f,1.0f);"  \
       "}" \
       "FragColor = color;" \
       "}";


  //give above fragment shader source code to fragmentShaderObject
       glShaderSource(gFragmentShaderObject,
       	              1,    //No of Strings,
       	              (const GLchar **)&fragmentShaderSourceCode , //source code
       	              NULL);//NULL indicates that our strings are null terminated 
                                         //if strings are not null terminated then we will hav to provide a length of string

check_gl_error();
//compile shader
glCompileShader(gFragmentShaderObject);
check_gl_error();

iInfoLogLength =0;  //re - initialize the varible for good practise
iShaderCompilationStatus =0;   //re - initialize the varible for good practise
szInfoLog =NULL;


//get the compilation status in parameterized variable
glGetShaderiv(gFragmentShaderObject,GL_COMPILE_STATUS,&iShaderCompilationStatus);
check_gl_error();
if(iShaderCompilationStatus ==GL_FALSE)
{
	glGetShaderiv(gFragmentShaderObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
    check_gl_error();
	if(iInfoLogLength >0)
	{
		szInfoLog = (char * )malloc(iInfoLogLength);
		if(szInfoLog != NULL)
		{
			GLsizei written;
			//get the error 
 					//S1: error characters length
 					//S2: number of characters actually written
 					//S3: pointer variable to hold the error string
			glGetShaderInfoLog(gFragmentShaderObject,
				               iInfoLogLength,             //S1
				               &written,                   // S2
				               szInfoLog);                //S3
			check_gl_error();
			fprintf(gpFile, "Fragmenrt shader compilation Log:%s\n",szInfoLog );
			free(szInfoLog);
			uninitialize();
			exit(0);
		}
	}
}

//******************* SHADER PROGRAM****************
//create shader 
gShaderProgramObject = glCreateProgram();
check_gl_error();

//attach vertex shader to shader program
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
	                 VDG_ATTRIBUTE_NORMAL,
	                 "vNormal");
check_gl_error();
//link shader
glLinkProgram(gShaderProgramObject);
check_gl_error();
GLint iShaderProgramLinkStatus =0;
iInfoLogLength=0;
szInfoLog =NULL;

glGetProgramiv(gShaderProgramObject,
	          GL_LINK_STATUS,
	          &iShaderProgramLinkStatus);
check_gl_error();

if(iShaderProgramLinkStatus == GL_FALSE)
{
	glGetProgramiv(gShaderProgramObject ,GL_INFO_LOG_LENGTH,&iInfoLogLength);
	check_gl_error();
	if(iInfoLogLength >0)
	{
		szInfoLog =(char * )malloc(iInfoLogLength);
		if(szInfoLog != NULL)
		{
			GLsizei written;
			//get the error 
 					//S1: error characters length
 					//S2: number of characters actually written
 					//S3: pointer variable to hold the error string
			glGetShaderInfoLog(gShaderProgramObject,
				               iInfoLogLength,
				               &written,
				               szInfoLog);
			fprintf(gpFile, "Shader Program Link Log:%s\n",szInfoLog);
			free(szInfoLog);
			uninitialize();
			exit(0);
		}
	}
}

//get MVP uniform location
    //bind uniform location variable ,i.e here u_mvp_matrix, from shader source code to ours variable "
    //so that we can use our variable to refer the uniform location in gpu

gModelViewMatrixUniform =glGetUniformLocation(gShaderProgramObject,"u_model_view_matrix");
gProjectionMatrixUniform =glGetUniformLocation(gShaderProgramObject,
	"u_projection_matrix");

gLKeyPressedUniform = glGetUniformLocation(gShaderProgramObject,
	"u_LKeyPressed");
gLdUniform=glGetUniformLocation(gShaderProgramObject,"u_Ld");
gKdUniform=glGetUniformLocation(gShaderProgramObject,"u_Kd");

gLightPositionUniform = glGetUniformLocation(gShaderProgramObject,"u_light_position");

//***vertices ,colors,shader,attribs,vbo,vao initializations***

 GLfloat cubeVertices[]=
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

 for (int  i=0;i<72;i++)
 {
 	if(cubeVertices[i]<0.0f)
 		cubeVertices[i]=cubeVertices[i] + 0.25f;
 	else if(cubeVertices[i]>0.0f)
 		cubeVertices[i]=cubeVertices[i] - 0.25f;
 	else
 		cubeVertices[i] = cubeVertices[i];
 }

const GLfloat cubeNormals[]=
{
	//front face
	0.0f,0.0f,1.0f,
	0.0f,0.0f,1.0f,
	0.0f,0.0f,1.0f,
	0.0f,0.0f,1.0f,

    //right face
	1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
 
   //back face
    0.0f, 0.0f, -1.0f,
	0.0f, 0.0f, -1.0f,
	0.0f, 0.0f, -1.0f,
	0.0f, 0.0f, -1.0f,

    //left face
	-1.0f, 0.0f, 0.0f,
	-1.0f, 0.0f, 0.0f,
	-1.0f, 0.0f, 0.0f,
	-1.0f, 0.0f, 0.0f,

	//top face
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,

	//bottom face
	0.0f, -1.0f, 0.0f,
	0.0f, -1.0f, 0.0f,
	0.0f, -1.0f, 0.0f,
	0.0f, -1.0f, 0.0f

};

//cube code 
//###################start of VAO for cube###############
//i want to run following  instructions    letsay "x" no of instructions 
//so give me the key to run those instructions repeatedly.
glGenVertexArrays(1,&gVao_cube);
check_gl_error();

glBindVertexArray(gVao_cube);//get binded with the above key 
                             //following operations will be regarding this vao
                             //because OpenGL is a state machine
check_gl_error();

//#################start VBO for positon of Cube############

     glGenBuffers(1,&gVbo_cube_position);//i want to place the data in GPU so ask GPU to give me the buffer
                             //to store my vertex,normal,color,texture data . In return GPU will return 
                             //a key in "gVbo".
     check_gl_error();

     glBindBuffer(GL_ARRAY_BUFFER,gVbo_cube_position);//get binded with that key i.e gVbo
					check_gl_error();
     check_gl_error();


      //now following operations will affect this gVbo
						//Following function will send our data from CPU to GPU
    					//S1: send the data in gVbo area which comes under Array Buffer area in GPU
    					//S2: size of data =no.of elements * size of data type in array
						//S3: actual data
    					//S4: send the data at initialize (i.e STATIC_DRAW) ...another macro can be used  i.e GL_DYANAMIC_DRAW which states that push the data on GPU at runTime 


     glBufferData(GL_ARRAY_BUFFER,           //S1
     	         sizeof(cubeVertices),        //S2
     	         cubeVertices,                //S3
     	         GL_STATIC_DRAW);             //S4
     check_gl_error();

     //following function will attach our enum variable to the above data her square data
//S1.our enum variable name so that it will get attached to the above data which is in above vbo,our enum variable has already attached to the variable in shader,so indirectly here we are attaching that shader varible to the data we are passing to GPU in above steps
//S2.here we tells that vertex position is made up of x,y,z
//S3.data is of float type
//S4.is this interleaved data(i.e all vertex attributes(i.e color,position,normal,texture) are packed in one buffer)?GL_FALSE means NO
//S5.offset for interleaved data
//S6.address of interleaved data buffer
     glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX,
     	                   3,
     	                   GL_FLOAT,
     	                   GL_FALSE,
     	                   0,
     	                   NULL);
     check_gl_error();
    
    //get binded with our enum variable which is linked to "vPosition" variable from vertex shader code 
     glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
     check_gl_error();

     glBindBuffer(GL_ARRAY_BUFFER,0);
     check_gl_error();

//#################End of VBO for position of cube###########
//#################Start of VBO for normal of cube###########
     glGenBuffers(1,&gVbo_cube_normal);//i want to place the data in GPU so ask GPU to give me the buffer
                             //to store my vertex,normal,color,texture data . In return GPU will return 
                             //a key in "gVbo".
     check_gl_error();

     glBindBuffer(GL_ARRAY_BUFFER,gVbo_cube_normal);
     check_gl_error();

       //now following operations will affect this gVbo
   //Following function will send our data from CPU to GPU
    //S1: send the data in gVbo area which comes under Array Buffer area in GPU
    //S2: size of data =no.of elements * size of data type in array
    //S3: actual data
    //S4: send the data at initialize (i.e STATIC_DRAW) ...another macro can be used  i.e GL_DYANAMIC_DRAW which states that push the data on GPU at runTime 
     glBufferData(GL_ARRAY_BUFFER,
     	         sizeof(cubeNormals),
     	         cubeNormals,
     	         GL_STATIC_DRAW);
     check_gl_error();

     //following function will attach our enum variable to the above data her square data
//S1.our enum variable name so that it will get attached to the above data which is in above vbo,our enum variable has already attached to the variable in shader,so indirectly here we are attaching that shader varible to the data we are passing to GPU in above steps
//S2.here we tells that vertex texture is made up of s & t
//S3.data is of float type
//S4.Data i not normalized take it as it is.
//S5.offset for interleaved data
//S6.address of interleaved data buffer
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
     check_gl_error();

//#################end of VBO of Normal of Cube##############
 glBindVertexArray(0);
 check_gl_error();

 //#########END of VAO of cube##############################

 glShadeModel(GL_SMOOTH);
 check_gl_error();

 glEnable(GL_DEPTH_TEST); //enable depth test in the state machine
 check_gl_error();

 glDepthFunc(GL_LEQUAL);        //Specify the depth comparison function .GL_LEQUAL:paases if incoming z value is less than
                              //or equal to stored z value
 check_gl_error();

 glClearDepth(1.0f);           //specifies the clear value for depth buffer
 check_gl_error();

//glEnable(GL_CULL_FACE);
check_gl_error();

glClearColor(0.0f,0.0f,0.0f,0.0f);
check_gl_error();

gPerspectiveProjectionMatrix = mat4::identity();

gbAnimate =false;
gbLight = false;

resize(WIN_WIDTH,WIN_HEIGHT);
}

void display(void)
{
	//code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     check_gl_error();
	//start using OpenGL program object 
	glUseProgram(gShaderProgramObject);
    check_gl_error();
	if(gbLight == true)
	{
		glUniform1i(gLKeyPressedUniform,1);
		check_gl_error();

		glUniform3f(gLdUniform,1.0f,1.0f,1.0f);
		glUniform3f(gKdUniform,0.5f,0.5f,0.5f);

		float lightPosition[] = {0.0f,0.0f,2.0f,1.0f};
		glUniform4fv(gLightPositionUniform,1,(GLfloat *)lightPosition);
	}
	else
    {
    	glUniform1i(gLKeyPressedUniform,0);
    }

    //OpenGL Drawing
    //set all matrices to identity
    mat4 modelMatrix = mat4 :: identity();
    mat4 modelViewMatrix = mat4 ::identity();
    mat4 rotationMatrix = mat4 :: identity();

   // apply z axis translation to go deep into the screen by -5.0,
	// so that triangle with same fullscreen co-ordinates, but due to above translation will look small
	modelMatrix = translate(0.0f,0.0f,-5.0f);

	//all axes rotation 
	rotationMatrix = rotate(gAngle,gAngle,gAngle);

	//multiply rotation  matrix and model matrix to get modelViewMatrix
	modelViewMatrix = modelMatrix * rotationMatrix; //ORDER is IMPORTANT

	// pass modelview matrix to the vertex shader in 'u_model_view_matrix' shader variable
	// whose position value we already calculated in initialize() by using glGetUniformLocation()
	glUniformMatrix4fv(gModelViewMatrixUniform,
		               1,  //no of Matrices
		               GL_FALSE,
		               modelViewMatrix);

	// pass projection matrix to the vertex shader in 'u_projection_matrix' shader variable
	// whose position value we already calculated in initialize() by using glGetUniformLocation()
	glUniformMatrix4fv(gProjectionMatrixUniform,
		               1,
		               GL_FALSE,
		               gPerspectiveProjectionMatrix);

	// bind vao
	glBindVertexArray(gVao_cube);

	//draw cube
	glDrawArrays(GL_TRIANGLE_FAN,0,4);
	glDrawArrays(GL_TRIANGLE_FAN,4,4);
	glDrawArrays(GL_TRIANGLE_FAN,8,4);
	glDrawArrays(GL_TRIANGLE_FAN,12,4);
	glDrawArrays(GL_TRIANGLE_FAN,16,4);
	glDrawArrays(GL_TRIANGLE_FAN,20,4);

	//unbind vao
	glBindVertexArray(0);


	//stop using OpenGL progrma object
	glUseProgram(0);

	SwapBuffers(ghdc);

}

void resize(int width,int height)
{
	if(height ==0)
		height=1;
	if(width ==0)
		width=1;
	glViewport(0,0,(GLsizei)width,(GLsizei)height);

	gPerspectiveProjectionMatrix= perspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

}
void update(void)
{
	//code
	gAngle = gAngle + 0.1f;
	if(gAngle >= 360.0f)
		gAngle =gAngle -360.0f;

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
		if(gVbo_cube_position)
	{
		glDeleteBuffers(1,&gVbo_cube_position);
		gVbo_cube_position=0;
	}
	if(gVbo_cube_normal)
	{
		glDeleteBuffers(1,&gVbo_cube_normal);
		gVbo_cube_normal=0;
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



























