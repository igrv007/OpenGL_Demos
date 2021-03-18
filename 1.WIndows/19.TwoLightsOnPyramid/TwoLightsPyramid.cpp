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
#define STEP_ANGLE  0.2f

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

GLuint gNumElements;
GLuint gNumVertices;
float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_elements[2280];

GLuint gVao_pyramid;


GLuint gVbo_pyramid_position;
GLuint gVbo_pyramid_normal;
GLfloat angle_pyr=0.0f;


GLuint model_matrix_uniform,view_matrix_uniform,projection_matrix_uniform;
GLuint L_keyPressed_uniform;

GLuint La_uniform;
GLuint Ld_uniform;
GLuint Ls_uniform;
GLuint light_position_uniform;

GLuint La_uniform1;
GLuint Ld_uniform1;
GLuint Ls_uniform1;
GLuint light_position_uniform1;

GLuint Ka_uniform;
GLuint Kd_uniform;
GLuint Ks_uniform;

GLuint material_shininess_uniform;
mat4 gPerspectiveProjectionMatrix;

bool gbLight;
bool gbswitchLight=false;
/*GLfloat lightAmbient[]= {0.0f,0.0f,0.0f,1.0f};
GLfloat lightDiffuse[]= {1.0f,1.0f,1.0f,1.0f};
GLfloat lightSpecular[] = {1.0f,1.0f,1.0f,1.0f};
GLfloat lightPosition[] ={100.0f,100.0f,100.0f,1.0f};*/
GLfloat gfLight0Ambient[]  = { 0.0f,0.0f,0.0f,1.0f };
GLfloat gfLight0Diffuse[]  = { 1.0f,0.0f,0.0f,1.0f };
GLfloat gfLight0Specular[] = { 1.0f,0.0f,0.0f,1.0f };
GLfloat gfLight0Position[] = { 2.0f,1.0f,1.0f,0.0f };

//LIGHT1 (left side Blue Light)

GLfloat gfLight1Ambient[]  = { 0.0f,0.0f,0.0f,1.0f };
GLfloat gfLight1Diffuse[]  = { 0.0f,0.0f,1.0f,1.0f };
GLfloat gfLight1Specular[] = { 0.0f,0.0f,1.0f,1.0f };
GLfloat gfLight1Position[] = { -2.0f,1.0f,1.0f,0.0f };


GLfloat material_ambient[]={0.0f,0.0f,0.0f,1.0f};
GLfloat material_diffuse[]={1.0f,1.0f,1.0f,1.0f};
GLfloat material_specular[]={1.0f,1.0f,1.0f,1.0f};
GLfloat material_shininess=50.0f;


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
			TEXT("OpenGL Programmable Pipeline Per Fragment Phong Lighting"),
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

//#######################VERTEX SHADER ############################
//crete shader
gVertexShaderObject= glCreateShader(GL_VERTEX_SHADER);
check_gl_error();

//provide source code toshader
const GLchar *vertexShaderSourceCode =  
       "#version 430 core" \
       "\n" \
       "in vec4 vPosition;" \
       "in vec3 vNormal;"   \
       "uniform mat4 u_model_matrix;" \
       "uniform mat4 u_view_matrix;"  \
       "uniform mat4 u_projection_matrix;" \
       "uniform int u_lighting_enabled;" \
       "uniform vec4 u_light_position;" \
       "uniform vec4 u_light_position1;" \
       "out vec3 transformed_normals;" \
       "out vec3 light_direction;" \
       "out vec3 light_direction1;" \
       "out vec3 viewer_vector;"  \
       "void main(void)" \
       "{" \
       "if(u_lighting_enabled == 1)" \
       "{" \
       "vec4 eye_coordinates = u_view_matrix * u_model_matrix * vPosition;" \
       "transformed_normals = mat3(u_view_matrix * u_model_matrix) * vNormal;" \
       "light_direction = vec3(u_light_position) - eye_coordinates.xyz;" \
       "light_direction1 = vec3(u_light_position1) - eye_coordinates.xyz;" \
       "viewer_vector = - eye_coordinates.xyz;"\
       "}" \
       "gl_Position=u_projection_matrix * u_view_matrix *u_model_matrix *vPosition;"
       "}";
       

      glShaderSource(gVertexShaderObject,1,(const GLchar **)&vertexShaderSourceCode,NULL);
       check_gl_error();
      //compile shader
      glCompileShader(gVertexShaderObject);
      check_gl_error();

      GLint iInfoLogLength =0;
      GLint iShaderCompiledStatus =0;
      char *szInfoLog =NULL;

      glGetShaderiv(gVertexShaderObject,GL_COMPILE_STATUS,&iShaderCompiledStatus);
      check_gl_error();
      if(iShaderCompiledStatus == GL_FALSE)
      {
      	glGetShaderiv(gVertexShaderObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
         check_gl_error();
         if(iInfoLogLength >0)
         {
         	szInfoLog = (char *)malloc(iInfoLogLength);
         	if(szInfoLog != NULL)
         	{
         		GLsizei  written;
         		glGetShaderInfoLog(gVertexShaderObject,iInfoLogLength,&written,szInfoLog);
         		fprintf(gpFile, "Vertex Shader Compilation Log:%s\n",szInfoLog );
         		free(szInfoLog);
         		uninitialize();
         		exit(0);
         	}
         }
     }

     //Fragment shader
     //create shader
     gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
     check_gl_error();
    
    //provide source code to shader
     const GLchar *fragmentShaderSourceCode = 
        "#version 430 core" \
        "\n" \
        "in vec3 transformed_normals;" \
        "in vec3 light_direction;" \
        "in vec3 light_direction1;"
        "in vec3 viewer_vector;" \
        "out vec4 FragColor;" \
        "uniform vec3 u_La;" \
        "uniform vec3 u_Ld;" \
        "uniform vec3 u_Ls;" \
        "uniform vec3 u_La1;" \
        "uniform vec3 u_Ld1;" \
        "uniform vec3 u_Ls1;" \
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
        "vec3 normalized_light_direction1 = normalize(light_direction1);" \
        "vec3 ambient1 = u_La1 * u_Ka;" \
        "tn_dot_ld = max(dot(normalized_transformed_normals,normalized_light_direction1),0.0);" \
        "vec3 diffuse1 = u_Ld1 * u_Kd * tn_dot_ld;" \
        "reflection_vector = reflect(-normalized_light_direction1,normalized_transformed_normals);" \
        "vec3 specular1 = u_Ls1 * u_Ks * pow(max(dot(reflection_vector,normalied_viewer_vector),0.0),u_material_shininess);" \
        "phong_ads_color = ambient+ambient1 + diffuse+diffuse1 + specular+specular1;" \
        "}" \
        "else" \
        "{"  \
        "phong_ads_color = vec3(1.0,1.0,1.0);" \
        "}" \
        "FragColor = vec4(phong_ads_color ,1.0);" \
        "}" ;

     glShaderSource(gFragmentShaderObject,
     	              1,
     	              (const char **)&fragmentShaderSourceCode,
     	              NULL);
     check_gl_error();
     glCompileShader(gFragmentShaderObject);
     check_gl_error();

     iShaderCompiledStatus =0;
     iInfoLogLength=0;
     szInfoLog = NULL;

      glGetShaderiv(gFragmentShaderObject,GL_COMPILE_STATUS,&iShaderCompiledStatus);
      check_gl_error();
      if(iShaderCompiledStatus == GL_FALSE)
      {
      	glGetShaderiv(gFragmentShaderObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
         check_gl_error();
         if(iInfoLogLength >0)
         {
         	szInfoLog = (char *)malloc(iInfoLogLength);
         	if(szInfoLog != NULL)
         	{
         		GLsizei  written;
         		glGetShaderInfoLog(gFragmentShaderObject,iInfoLogLength,&written,szInfoLog);
         		fprintf(gpFile, "Fragment Shader Compilation Log:%s\n",szInfoLog );
         		free(szInfoLog);
         		uninitialize();
         		exit(0);
         	}
         }
     }


//******************SHADER PROGRAM***************
//create shader
 gShaderProgramObject = glCreateProgram();

 //attach vertex shader to shader program
  glAttachShader(gShaderProgramObject,gVertexShaderObject);
  check_gl_error();

  //attach fragment shader to shader program
  glAttachShader(gShaderProgramObject,gFragmentShaderObject);
  check_gl_error();

  //pre -link binding of shader program object with vertex shader position
  //attribute

  glBindAttribLocation(gShaderProgramObject,VDG_ATTRIBUTE_VERTEX,"vPosition");
  check_gl_error();
  glBindAttribLocation(gShaderProgramObject,VDG_ATTRIBUTE_NORMAL,"vNormal");
  check_gl_error();

  //link shader
  glLinkProgram(gShaderProgramObject);
  GLint iShaderProgramLinkStatus = 0;
  glGetProgramiv(gShaderProgramObject,GL_LINK_STATUS,&iShaderProgramLinkStatus);
  if(iShaderProgramLinkStatus == GL_FALSE)
  {
  	glGetProgramiv(gShaderProgramObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
  	if(iInfoLogLength >0)
  	{
  		szInfoLog =(char *)malloc(iInfoLogLength);
  		if(szInfoLog != NULL)
  		{
           GLsizei written;
           glGetProgramInfoLog(gShaderProgramObject,
           	                   iInfoLogLength,
           	                     &written,
           	                     szInfoLog);
           fprintf(gpFile, "Shader Program Link Log:%s\n", szInfoLog);
           free(szInfoLog);
           uninitialize();
           exit(0);
  		}
  	}
  } 

  //get uniform locations
  model_matrix_uniform = glGetUniformLocation(gShaderProgramObject,"u_model_matrix");
  check_gl_error();
  view_matrix_uniform = glGetUniformLocation(gShaderProgramObject,"u_view_matrix");
  check_gl_error();
  projection_matrix_uniform = glGetUniformLocation(gShaderProgramObject,"u_projection_matrix");
   check_gl_error();

   //L or l key is pressed or not
   L_keyPressed_uniform =glGetUniformLocation(gShaderProgramObject,"u_lighting_enabled");

   //ambient color intensity of light
   La_uniform = glGetUniformLocation(gShaderProgramObject,"u_La");
   check_gl_error();

   //diffuse color intensity of light
   Ld_uniform = glGetUniformLocation(gShaderProgramObject,"u_Ld");
   check_gl_error();

   //specular color intensity of light 
   Ls_uniform = glGetUniformLocation(gShaderProgramObject,"u_Ls");
   check_gl_error();

    //ambient color intensity of light
   La_uniform1 = glGetUniformLocation(gShaderProgramObject,"u_La1");
   check_gl_error();

   //diffuse color intensity of light
   Ld_uniform1 = glGetUniformLocation(gShaderProgramObject,"u_Ld1");
   check_gl_error();

   //specular color intensity of light 
   Ls_uniform1 = glGetUniformLocation(gShaderProgramObject,"u_Ls1");
   check_gl_error();


   light_position_uniform = glGetUniformLocation(gShaderProgramObject,"u_light_position");
   check_gl_error();

    light_position_uniform1 = glGetUniformLocation(gShaderProgramObject,"u_light_position1");
   check_gl_error();

   //ambient reflective color intensity of material 
   Ka_uniform = glGetUniformLocation(gShaderProgramObject,"u_Ka");
   check_gl_error();

   //diffuse reflective color intensity of material 
   Kd_uniform = glGetUniformLocation(gShaderProgramObject,"u_Kd");
   check_gl_error();

   //specular reflective color intensity of material
   Ks_uniform = glGetUniformLocation(gShaderProgramObject,"u_Ks");
   check_gl_error();

   //shininess of material (value is conventionally between 1 to 200)
   material_shininess_uniform = glGetUniformLocation(gShaderProgramObject,"u_material_shininess");

   //**vertices ,colors ,shader attribs ,vbo,vao,initializations **
   const GLfloat pyramidVertices[]=
{   
  //front face 
  0.0f,1.0f,0.0f,    //apex
  -1.0f,-1.0f,1.0f,  //left bottom
  1.0f,-1.0f,1.0f,   //right bottom

  //right face
  0.0f,1.0f,0.0f,    //apex
  1.0f,-1.0f,1.0f,
  1.0f,-1.0f,-1.0f,

  //back fce
  0.0f,1.0f,0.0f,
  1.0f,-1.0f,-1.0f,
  -1.0f,-1.0f,-1.0f,

  //left face
  0.0f,1.0f,0.0f,
  -1.0f,-1.0f,-1.0f,
  -1.0f,-1.0f,1.0f
};

const GLfloat pyramidNormal[]=
{
  //front face
  0.0f,0.0f,1.0f,
    0.0f,0.0f,1.0f,
    0.0f,0.0f,1.0f,

    //right face
    1.0f,0.0f,0.0f,
    1.0f,0.0f,0.0f,
    1.0f,0.0f,0.0f,

    //back face
    0.0f,0.0f,-1.0f,
    0.0f,1.0f,-1.0f,
    0.0f,0.0f,-1.0f,

    //left face
    -1.0f,0.0f,0.0f,
    -1.0f,0.0f,0.0f,
    -1.0f,0.0f,0.0f
};

//i want to run following  instructions    letsay "x" no of instructions 
//so give me the key to run those instructions repeatedly.
glGenVertexArrays(1,&gVao_pyramid);

{
    check_gl_error();

  glBindVertexArray(gVao_pyramid);//get binded with the above key 
                             //following operations will be regarding this vao
                             //because OpenGL is a state machine

  check_gl_error();
      
    {
        glGenBuffers(1,&gVbo_pyramid_position);  //i want to place the data in GPU so ask GPU to give me the buffer
                             //to store my vertex,normal,color,texture data . In return GPU will return 
                             //a key in "gVbo".
              {
          check_gl_error();

          glBindBuffer(GL_ARRAY_BUFFER,gVbo_pyramid_position);//get binded with that key i.e gVbo
          check_gl_error();
                  {

                      //now following operations will affect this gVbo
            //Following function will send our data from CPU to GPU
              //S1: send the data in gVbo area which comes under Array Buffer area in GPU
              //S2: size of data =no.of elements * size of data type in array
            //S3: actual data
              //S4: send the data at initialize (i.e STATIC_DRAW) ...another macro can be used  i.e GL_DYANAMIC_DRAW which states that push the data on GPU at runTime 

            glBufferData(GL_ARRAY_BUFFER,
                       sizeof(pyramidVertices),
                       pyramidVertices,
                       GL_STATIC_DRAW);

            check_gl_error();
                       //following function will attach our enum variable to the above data her square data
//S1.our enum variable name so that it will get attached to the above data which is in above vbo,our enum variable has already attached to the variable in shader,so indirectly here we are attaching that shader varible to the data we are passing to GPU in above steps
//S2.here we tells that vertex position is made up of x,y,z
//S3.data is of float type
//S4.is this interleaved data(i.e all vertex attributes(i.e color,position,normal,texture) are packed in one buffer)?GL_FALSE means NO
//S5.offset for interleaved data
//S6.address of interleaved data buffer
            glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX,   //S1
                                  3,                     //S2
                                  GL_FLOAT,               //S3
                                  GL_FALSE,              //S4
                                  0,                     //S5
                                  NULL);                //S6

            check_gl_error();

                       
//get binded with our enum variable which is linked to "vPosition" variable from vertex shader code
            glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);

            check_gl_error();
            //unbind vbo_pyramid_position
            glBindBuffer(GL_ARRAY_BUFFER,0);
            check_gl_error();
          }
        }
               
        glGenBuffers(1,&gVbo_pyramid_normal);//i want to place the data in GPU so ask GPU to give me the buffer
                             //to store my vertex,normal,color,texture data . In return GPU will return 
                             //a key in "gVbo".
        {
          check_gl_error();

          glBindBuffer(GL_ARRAY_BUFFER,gVbo_pyramid_normal);//get binded with that key i.e gVbo

          check_gl_error();
          {
                    //now following operations will affect this gVbo
   //Following function will send our data from CPU to GPU
    //S1: send the data in gVbo area which comes under Array Buffer area in GPU
    //S2: size of data =no.of elements * size of data type in array
    //S3: actual data
    //S4: send the data at initialize (i.e STATIC_DRAW) ...another macro can be used  i.e GL_DYANAMIC_DRAW which states that push the data on GPU at runTime 

          glBufferData(GL_ARRAY_BUFFER,
                      sizeof(pyramidNormal),
                      pyramidNormal,
                      GL_STATIC_DRAW);
                   check_gl_error();
                   //following function will attach our enum variable to the above data her square data
//S1.our enum variable name so that it will get attached to the above data which is in above vbo,our enum variable has already attached to the variable in shader,so indirectly here we are attaching that shader varible to the data we are passing to GPU in above steps
//S2.here we tells that vertex color is made up of rgb
//S3.data is of float type
//S4.Data i not normalized take it as it is.
//S5.offset for interleaved data
//S6.address of interleaved data buffer
                    glVertexAttribPointer(VDG_ATTRIBUTE_NORMAL,      //S1
                                         3,                      //S2
                                         GL_FLOAT,              //S3
                                         GL_FALSE,             //S4
                                         0,                    //S5
                                         NULL);                //S6

                    check_gl_error();

                    glEnableVertexAttribArray(VDG_ATTRIBUTE_NORMAL);
                    check_gl_error();

                    glBindBuffer(GL_ARRAY_BUFFER,0);
                    check_gl_error();


          }
        }
    }
}
glBindVertexArray(0);
check_gl_error();


     glShadeModel(GL_SMOOTH);
     glClearDepth(1.0f);
     glEnable(GL_DEPTH_TEST);
     glDepthFunc(GL_LEQUAL);
     glEnable(GL_CULL_FACE);


    glClearColor(0.0f,0.0f,0.0f,0.0f); //black

    gPerspectiveProjectionMatrix = mat4::identity();

    gbLight = false;

    //resize
     resize(WIN_WIDTH,WIN_HEIGHT);

}

void display(void)
{
	//code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	check_gl_error();

	glUseProgram(gShaderProgramObject);
	check_gl_error();

	if(gbLight == true)
	{
		//set "u_lighting_enabled" uniform
		glUniform1i(L_keyPressed_uniform,1);

		//setting light's properties
    //if(gbswitchLight == false)
     glUniform3fv(La_uniform,1,gfLight0Ambient);
    	glUniform3fv(Ld_uniform,1,gfLight0Diffuse);
    	glUniform3fv(Ls_uniform,1,gfLight0Specular);
    	glUniform4fv(light_position_uniform,1,gfLight0Position);
    

     glUniform3fv(La_uniform1,1,gfLight1Ambient);
      glUniform3fv(Ld_uniform1,1,gfLight1Diffuse);
      glUniform3fv(Ls_uniform1,1,gfLight1Specular);
      glUniform4fv(light_position_uniform1,1,gfLight1Position);
    

		//setting material's properties
		glUniform3fv(Ka_uniform,1,material_ambient);
		glUniform3fv(Kd_uniform,1,material_diffuse);
		glUniform3fv(Ks_uniform,1,material_specular);

		glUniform1f(material_shininess_uniform,material_shininess);

	}
	else
	{
		//set "u_lighting_enabled" uniform
		glUniform1i(L_keyPressed_uniform,0);
		check_gl_error();

	}
	//OpenGL drawing
	//set all matrices to identity
	mat4 modelMatrix = mat4:: identity();
	mat4 viewMatrix = mat4::identity();

	//apply z axix translation to go deep into the screen by -2.0,
	//so that triangle with same fullscreen co-ordinates,but due to above translation will look small
	modelMatrix = translate(0.0f,0.0f,-6.0f);
  modelMatrix = modelMatrix * vmath::rotate(angle_pyr,0.0f,1.0f,0.0f);

	glUniformMatrix4fv(model_matrix_uniform,1,GL_FALSE,modelMatrix);
	glUniformMatrix4fv(view_matrix_uniform,1,GL_FALSE,viewMatrix);
	glUniformMatrix4fv(projection_matrix_uniform,1,GL_FALSE,gPerspectiveProjectionMatrix);

	//bind vao 
	glBindVertexArray(gVao_pyramid);

	glDrawArrays(GL_TRIANGLES,0,12);

	glBindVertexArray(0);




	glUseProgram(0);
//gbswitchLight =!gbswitchLight;

	SwapBuffers(ghdc);

}
 void update(void)
 {
  angle_pyr=angle_pyr + STEP_ANGLE;
  if(angle_pyr >=360.0f)
    angle_pyr=angle_pyr-360.0f;
 }

void resize(int width, int height)
{
	//code
	if(height == 0)
		height =1;
	if(width == 0)
		width =1;
	glViewport(0,0,(GLsizei)width,(GLsizei)height);

	gPerspectiveProjectionMatrix = perspective(45.0f,(GLfloat)width /(GLfloat)height,0.1f,100.0f);
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
	 if(gVao_pyramid)
  {
    glDeleteVertexArrays(1,&gVao_pyramid);
    gVao_pyramid=0;
  }
      

  //destroy vbo
  if(gVbo_pyramid_position)
  {
    glDeleteBuffers(1,&gVbo_pyramid_position);
    gVbo_pyramid_position=0;
  }
  if(gVbo_pyramid_normal)
  {
    glDeleteBuffers(1,&gVbo_pyramid_normal);
    gVbo_pyramid_normal=0;
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
         
      


