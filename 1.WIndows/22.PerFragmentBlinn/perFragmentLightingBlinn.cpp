#include <windows.h>
#include <stdio.h>

#include <gl/glew.h>

#include <gl/GL.h>

#include "vmath.h"
#include "GLError.h"
#include "Sphere.h"



#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"Sphere.lib")


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


//main()
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpszCmdLine,int iCmdShow)
{
	//function prototype
	void initialize(void);
	void uninitialize(void);
	void display(void);
    

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
        "vec3 vector_addition= light_direction + viewer_vector;" \
        "vec3 half_vector = normalize(vector_addition * (1/sqrt(vector_addition)));"\
        "vec3 normalized_transformed_normals=normalize(transformed_normals);" \
        "vec3 normalized_light_direction = normalize(light_direction);" \
        "vec3 ambient = u_La * u_Ka;" \
        "float tn_dot_ld = max(dot(normalized_transformed_normals,normalized_light_direction),0.0);" \
        "vec3 diffuse = u_Ld * u_Kd * tn_dot_ld;" \
        "vec3 specular = u_Ls * u_Ks * pow(max(dot(half_vector,normalized_transformed_normals),0.0),u_material_shininess);" \
        "phong_ads_color = ambient + diffuse + specular;" \
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

   light_position_uniform = glGetUniformLocation(gShaderProgramObject,"u_light_position");
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

	SwapBuffers(ghdc);

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
         
      






































