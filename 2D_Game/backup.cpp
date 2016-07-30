#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string.h>
#include <math.h>

using namespace std;
glm::mat4 MVP;
glm::mat4 VP;

double x_cur,y_cur;

int pass;

double flag = 0,keyboard_movement = 0,mouse_movement = 0,power_movement = 0,k_pos_x,k_pos_y,e_pos_x,e_pos_y;
struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;

	GLenum PrimitiveMode;
	GLenum FillMode;
	int NumVertices;
};
typedef struct VAO VAO;

vector<VAO*>arr_obs;
vector<glm::vec3>obst;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;


GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
			0,                  // attribute 0. Vertices
			3,                  // size (x,y,z)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
			1,                  // attribute 1. Color
			3,                  // size (r,g,b)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;



/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.

	if (action == GLFW_RELEASE) {
		switch (key) {
			
			case GLFW_KEY_W:
				keyboard_movement = 0;
				break;
				case GLFW_KEY_D:
				power_movement = 0;
				break;
			case GLFW_KEY_S:
				keyboard_movement = 0;
				break;
			case GLFW_KEY_A:
				power_movement = 0;
				break;
			case GLFW_KEY_SPACE:
				flag = 1
				;
				break;
			
			default:
				break;
		}
	}
	else if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_ESCAPE:
				quit(window);
				break;
			case GLFW_KEY_W:
				keyboard_movement = 1;
				break;
			case GLFW_KEY_D:
				power_movement = 1;
				break;
			case GLFW_KEY_S:
				keyboard_movement = -1;
				break;
			case GLFW_KEY_A:
				power_movement = -1;
				break;
			case GLFW_KEY_SPACE:
				flag = 0;
				break;
			default:
				break;
		}
	}
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
			quit(window);
			break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			if (action == GLFW_PRESS){
				mouse_movement = 1;
				flag = 0;
			}
			if(action == GLFW_RELEASE){
				mouse_movement = 0;
				flag = 1;
			}
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			if (action == GLFW_RELEASE) {
				flag = 1;
			}
			if (action == GLFW_PRESS) {
				flag = 0;
			}
			break;
		default:
			break;
	}
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;
	/* With Retina display on Mac OS X, GLFW's FramebufferSize
	   is different from WindowSize */
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	// Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

	// Ortho projection for 2D views
	Matrices.projection = glm::ortho(0.0f, 96.0f, 0.0f, 54.0f, 0.1f, 500.0f);
}

VAO *rectangle, *circle, *ball, *block , *bar[40], *key, *ex;

// Creates the rectangle object used in this sample code
void createRectangle ()
{
	// GL3 accepts only Triangles. Quads are not supported
	 GLfloat vertex_buffer_data [] = {
		0,0.5,0, // vertex 1
		6,0.5,0, // vertex 2
		6,-0.5,0, // vertex 3

		0,-0.5,0, // vertex 3
		6,-0.5,0, // vertex 4
		0,0.5,0  // vertex 1
	};
 
	 GLfloat color_buffer_data [] = {
		1,1,0, // color 1
		1,1,0, // color 2
		1,1,0, // color 3

		1,1,0, // color 3
		1,1,0, // color 4
		1,1,0  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

VAO* createblock(float *color)
{
	// GL3 accepts only Triangles. Quads are not supported
	VAO *block;
	 GLfloat vertex_buffer_data [] = {
		
		-0.5,0.5,0, // vertex 1
		0.5,0.5,0, // vertex 2
		0.5,-0.5,0, // vertex 3

		-0.5,0.5,0, // vertex 1
		-0.5,-0.5,0, // vertex 2
		0.5,-0.5,0, // vertex 3

		
	};
 
	 GLfloat color_buffer_data [] = {
		color[0],color[1],color[2],
		color[0],color[1],color[2],
		color[0],color[1],color[2],

		color[0],color[1],color[2],
		color[0],color[1],color[2],
		color[0],color[1],color[2]
		
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	block = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
	return block;
}




void createball(int numberOfSides,int x,int y,int z,float radius){
	int numberOfVertices = numberOfSides + 2;

	GLfloat twicePi = 2.0f * M_PI;
	

	GLfloat circleVerticesX[numberOfVertices];
	GLfloat circleVerticesY[numberOfVertices];
	GLfloat circleVerticesZ[numberOfVertices];

	circleVerticesX[0] = x;
	circleVerticesY[0] = y;
	circleVerticesZ[0] = z;

	for ( int i = 1; i < numberOfVertices; i++ )
	{
		circleVerticesX[i] = x + ( radius * cos( i *  twicePi / numberOfSides ) );
		circleVerticesY[i] = y + (radius * sin( i * twicePi / numberOfSides ) );
		circleVerticesZ[i] = z;
	}

	GLfloat vertex_buffer_data[( numberOfVertices ) * 3];
	GLfloat color_buffer_data[(numberOfVertices) * 3];

	for ( int i = 0; i < numberOfVertices; i++ )
	{
		vertex_buffer_data[i * 3] = circleVerticesX[i];
		vertex_buffer_data[( i * 3 ) + 1] = circleVerticesY[i];
		vertex_buffer_data[( i * 3 ) + 2] = circleVerticesZ[i];
		color_buffer_data[(i*3)] = 1;
		color_buffer_data[(i*3)+1] = 1;
		color_buffer_data[(i*3)+2] = 1;

	}

	ball = create3DObject(GL_TRIANGLE_FAN, numberOfVertices, vertex_buffer_data, color_buffer_data, GL_FILL);

}
void createkey(){
	int numberOfSides = 10000;
	int numberOfVertices = numberOfSides + 2;

	GLfloat twicePi = 2.0f * M_PI;
	

	GLfloat circleVerticesX[numberOfVertices];
	GLfloat circleVerticesY[numberOfVertices];
	GLfloat circleVerticesZ[numberOfVertices];

	circleVerticesX[0] = 0;
	circleVerticesY[0] = 0;
	circleVerticesZ[0] = 0;

	for ( int i = 1; i < numberOfVertices; i++ )
	{
		circleVerticesX[i] = ( 1 * cos( i *  twicePi / numberOfSides ) );
		circleVerticesY[i] = ( 1* sin( i * twicePi / numberOfSides ) );
		circleVerticesZ[i] = 0;
	}

	GLfloat vertex_buffer_data[( numberOfVertices ) * 3];
	GLfloat color_buffer_data[(numberOfVertices) * 3];

	for ( int i = 0; i < numberOfVertices; i++ )
	{
		vertex_buffer_data[i * 3] = circleVerticesX[i];
		vertex_buffer_data[( i * 3 ) + 1] = circleVerticesY[i];
		vertex_buffer_data[( i * 3 ) + 2] = circleVerticesZ[i];
		color_buffer_data[(i*3)] = 1;
		color_buffer_data[(i*3)+1] = 0;
		color_buffer_data[(i*3)+2] = 1;

	}

	key = create3DObject(GL_TRIANGLE_FAN, numberOfVertices, vertex_buffer_data, color_buffer_data, GL_FILL);

}
void createexit(){
	int numberOfSides = 10000;
	int numberOfVertices = numberOfSides + 2;

	GLfloat twicePi = 2.0f * M_PI;
	

	GLfloat circleVerticesX[numberOfVertices];
	GLfloat circleVerticesY[numberOfVertices];
	GLfloat circleVerticesZ[numberOfVertices];

	circleVerticesX[0] = 0;
	circleVerticesY[0] = 0;
	circleVerticesZ[0] = 0;

	for ( int i = 1; i < numberOfVertices; i++ )
	{
		circleVerticesX[i] = ( 2 * cos( i *  twicePi / numberOfSides ) );
		circleVerticesY[i] = ( 2* sin( i * twicePi / numberOfSides ) );
		circleVerticesZ[i] = 0;
	}

	GLfloat vertex_buffer_data[( numberOfVertices ) * 3];
	GLfloat color_buffer_data[(numberOfVertices) * 3];

	for ( int i = 0; i < numberOfVertices; i++ )
	{
		vertex_buffer_data[i * 3] = circleVerticesX[i];
		vertex_buffer_data[( i * 3 ) + 1] = circleVerticesY[i];
		vertex_buffer_data[( i * 3 ) + 2] = circleVerticesZ[i];
		color_buffer_data[(i*3)] = 0;
		color_buffer_data[(i*3)+1] = 0.5;
		color_buffer_data[(i*3)+2] = 0.5;

	}

	ex = create3DObject(GL_TRIANGLE_FAN, numberOfVertices, vertex_buffer_data, color_buffer_data, GL_FILL);

}




void createCircle (int numberOfSides,int x,int y,int z,int radius){
	int numberOfVertices = numberOfSides + 2;

	GLfloat twicePi = 2.0f * M_PI;
	

	GLfloat circleVerticesX[numberOfVertices];
	GLfloat circleVerticesY[numberOfVertices];
	GLfloat circleVerticesZ[numberOfVertices];

	circleVerticesX[0] = x;
	circleVerticesY[0] = y;
	circleVerticesZ[0] = z;

	for ( int i = 1; i < numberOfVertices; i++ )
	{
		circleVerticesX[i] = x + ( radius * cos( i *  twicePi / numberOfSides ) );
		circleVerticesY[i] = y + (radius * sin( i * twicePi / numberOfSides ) );
		circleVerticesZ[i] = z;
	}

	GLfloat vertex_buffer_data[( numberOfVertices ) * 3];
	GLfloat color_buffer_data[(numberOfVertices) * 3];

	for ( int i = 0; i < numberOfVertices; i++ )
	{
		vertex_buffer_data[i * 3] = circleVerticesX[i];
		vertex_buffer_data[( i * 3 ) + 1] = circleVerticesY[i];
		vertex_buffer_data[( i * 3 ) + 2] = circleVerticesZ[i];
		color_buffer_data[(i*3)] = 1;
		color_buffer_data[(i*3)+1] = .2;
		color_buffer_data[(i*3)+2] = 0;

	}

	circle = create3DObject(GL_TRIANGLE_FAN, numberOfVertices, vertex_buffer_data, color_buffer_data, GL_FILL);

}



float camera_rotation_angle = 90;
double v_x,v_y,a_x = 0.015 ,a_y = 0.03,v = 1;
double x_proj,y_proj;

double x_curm;
double y_curm;

float rectangle_rotation;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use the loaded shader program
	// Don't change unless you know what you are doing
	glUseProgram (programID);

	// Eye - Location of camera. Don't change unless you are sure!!
	glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
	// Target - Where is the camera looking at.  Don't change unless you are sure!!
	glm::vec3 target (0, 0, 0);
	// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
	glm::vec3 up (0, 1, 0);

	//Compute Camera matrix (view)
	//Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
	//  Don't change unless you are sure!!
	Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	//  Don't change unless you are sure!!
	glm::mat4 VP = Matrices.projection * Matrices.view;

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)
	//  Don't change unless you are sure!!
	glm::mat4 MVP;	// MVP = Projection * View * Model

	// Load identity to model matrix
	//Matrices.model = glm::mat4(1.0f);

	/* Render your scene */

	///glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef
	///glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
	///glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
	///Matrices.model *= triangleTransform; 
	///MVP = VP * Matrices.model; // MVP = p * V * M

	//  Don't change unless you are sure!!
	///glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix
//	draw3DObject(triangle);

	// Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
	// glPopMatrix ();
	///rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
	for( int i=0;i < arr_obs.size();i++){
		Matrices.model = glm::mat4(1.0f);
		glm::mat4 translateRectangle = glm::translate (obst[i]);
		Matrices.model *=  (translateRectangle );
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(arr_obs[i]);
	}

	

	
	x_curm= x_cur/10.0f;
	y_curm= 54.0f - y_cur/10.0f;
	if (mouse_movement == 1){
		rectangle_rotation = atan2(y_curm-5,x_curm-5);
	}
	if (keyboard_movement == 1 or keyboard_movement == -1){
		rectangle_rotation += 0.01 * keyboard_movement;
	}
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateRectangle = glm::translate (glm::vec3(5, 5, 0));
	glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation), glm::vec3(0,0,1)); 
	Matrices.model *=  (translateRectangle * rotateRectangle);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);


	// draw3DObject draws the VAO given to it using current MVP matrix
	draw3DObject(rectangle);
	// Increment angles
	///float increments = 1;

	//camera_rotation_angle++; // Simulating camera rotation
	///triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
	
	if (mouse_movement == 1){
			v = 10*((x_curm/96)/6+(y_curm/54)/5.5);
		}
	if (power_movement == 1 or power_movement == -1){
			v += 0.025*power_movement; 
		}
	int len = v*10 - 10 ;
	if ( v >= 2.2){
		v = 2.2;
	}
	if ( v < 0){
		v = 0;
	}
	
	if ( len > 13){
		len = 12;
	}
	if ( len < 1 ){
		len = 1;
	}
	

	if (flag == 0){
		Matrices.model = glm::mat4(1.0f);

		
		//glm::mat4 translateball = glm::translate (glm::vec3(6*cos(rectangle_rotation), 6*sin(rectangle_rotation), 0));
		//Matrices.model *=  (translateball );
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			//Matrices.model *=  (translateRectangle );
		draw3DObject(ball);
		/*if (mouse_movement == 1){
			v = 10*((x_curm/96)/6+(y_curm/54)/5.5);
		}
		if (power_movement == 1 or power_movement == -1){
			v += 0.5*power_movement; 
		}*/
		
		v_x = v*cos(rectangle_rotation);
		v_y = v*sin(rectangle_rotation); 
		x_proj = 6*cos(rectangle_rotation);
		y_proj = 6*sin(rectangle_rotation);

	}
	if (flag == 1) {
		Matrices.model = glm::mat4(1.0f);

		
		glm::mat4 translateball = glm::translate (glm::vec3(x_proj, y_proj, 0));
		Matrices.model *=  (translateball );
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		
		draw3DObject(ball);
		
		v_y -= a_y; 
		v_x -= v_x*a_x;

		
		x_proj += v_x;
		y_proj += v_y;

		
	
		if (x_proj > 90.5) {
			v_x = - 0.7 * v_x;
			x_proj = 90.5;
		}
		if (x_proj < -4.5 ){
			v_x = -0.7 *v_x;
			x_proj = -4.5;
		}
		if((y_proj > 41.5) ){
			v_y = -0.8 * v_y;
			y_proj = 41.5;
		}
		if((y_proj < -4.5)){
			v_y = -0.8*v_y;
			y_proj = -4.5;
		}
		double R = 0.8535533;
		double K = 1.2071067;
		for (int i = 0; i < obst.size();i++)
		{ 
			double x2 = obst[i][0]-5, y2 = obst[i][1] -5 ;
			printf("%lf : %lf ,  %lf : %lf\n",x_proj,x2,y_proj,y2 );
			double d = sqrt(pow((x_proj-x2),2) + pow((y_proj-y2),2));
			if( d < K) {
				cout<<"colloid";
				
				
				if(y_proj > y2 + R - 0.6 )
				{
					if(v_y>0){
						v_y = -0.8*v_y;
						y_proj = y2 - K;
					}
					else{
						v_y = -0.8 * v_y;
						y_proj = y2 + K;
					}
					cout<<"up";
					
				}
				else if (x_proj < x2 )
				{
					if(v_x < 0){
						x_proj = x2 + K;
						v_x = -0.8 *v_x;

					}
					else{
						v_x = -0.8*v_x;
						x_proj = x2 - K;
					}
					cout<<"left";
				}
				else if(x_proj > x2 + R )
				{
					if(v_x > 0){
						x_proj = x2 - K;
						v_x = -0.8*v_x;
					}
					else{
						v_x = -0.8 *v_x;
						x_proj = x2 + K;
					}
					cout<<"right";

				}
				else  
				{	
					if(v_y < 0){
						y_proj = y2 + K;
						v_y = -0.8*v_y;
					}
					else
					{
						v_y = -0.8*v_y;
						y_proj = y2 - K;
					}
					cout<<"down";
				}
				
				break;

			}

		}

	}
	if ( sqrt( pow((x_proj-k_pos_x+5),2)+pow((y_proj-k_pos_y+5),2) ) < 1.8 ){
		pass = 1;
	}  
	if (pass == 0){
		Matrices.model = glm::mat4(1.0f);
		glm::mat4 translatekey = glm::translate (glm::vec3(k_pos_x,k_pos_y,0));
		Matrices.model *=  (translatekey);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(key);
	}
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translatekey = glm::translate (glm::vec3(e_pos_x,e_pos_y,0));
	Matrices.model *=  (translatekey);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(ex);
	if (pass == 1)
	{
		if ( sqrt( pow((x_proj-e_pos_x+5),2)+pow((y_proj-e_pos_y+5),2) ) < 2.6)
		{
			pass = 0;
		}
	}


	
	//printf(", flag = %d \n ", flag);
	Matrices.model = glm::mat4(1.0f);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(circle); 	

	
	float k=0.2;
	for (int i=0;i < 2*len;i++ ){
		Matrices.model = glm::mat4(1.0f);
		glm::mat4 translateTriangle = glm::translate (glm::vec3(4.5+i+k,49.5, 0));
		Matrices.model *=  (translateTriangle);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(bar[i]);
		k +=0.2;
	}
	
	
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
	GLFWwindow* window; // window desciptor/handle

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval( 1 );

	/* --- register callbacks with GLFW --- */

	/* Register function to handle window resizes */
	/* With Retina display on Mac OS X GLFW's FramebufferSize
	   is different from WindowSize */
	glfwSetFramebufferSizeCallback(window, reshapeWindow);
	glfwSetWindowSizeCallback(window, reshapeWindow);

	/* Register function to handle window close */
	glfwSetWindowCloseCallback(window, quit);

	/* Register function to handle keyboard input */
	glfwSetKeyCallback(window, keyboard);      // general keyboard input
	glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

	/* Register function to handle mouse click */
	glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

	return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	/* Objects should be created before any other gl function and shaders */
	// Create the models
//	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	createRectangle ();
	createball(10000,5,5,0,0.5);
	createCircle (10000,5,5,0,3);

	float color[3];
	color[1] = 1;
	color[2] = 0;
	color[3] = 0;
	int i;
	for ( i=0;i < 40 ; i++) {
		bar[i]=createblock(color);

	}
	

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor (0.0f, 0.0f, 0.0f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

void createMap(){
	string line;
	float cl[3];
	cl[0]=0;
	cl[1]=1;
	cl[2]=0.5;
	ifstream file;
	file.open("1.txt");
	int y=54;
	while (getline(file,line)&&y>=0){
		int x=0;
		while(x<line.length() && x <= 96 ){
			switch(line[x]){
				case'x':
					obst.push_back(glm::vec3(float(x)+0.5,y-0.5,0.0f));
					arr_obs.push_back(createblock(cl));
					break;
				case'k':
					k_pos_x = x+0.5;
					k_pos_y = y-0.5;
					createkey();
					pass = 0;
					break;
				case'e':
					e_pos_x = x+0.5;
					e_pos_y = y-0.5;
					createexit();
					break;
				default:
					break;
			}
			x++;
		}
		y--;
	}
}
/*void checkcollision(){

	for (int i = 0; i < obst.size();i++){ 
		if ((x_proj + 0.5  == obst[i][0] - 0.5 ) && ((y_proj - 0.5 <= obst[i][0] + 0.5) || (y_proj + 0.5 >= obst[i][1] - 0.5)) ){
			v_x = -0.7*v_x;
		}
	}

		
}*/
	

int main (int argc, char** argv)
{
	int width = 960;
	int height = 540;

	GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

	double last_update_time = glfwGetTime(), current_time;
	createMap();

	/* Draw in loop */
	while (!glfwWindowShouldClose(window)) {

		// OpenGL Draw commands
		glfwGetCursorPos(window, &x_cur, &y_cur);
		draw();


		// Swap Frame Buffer in double buffering
		glfwSwapBuffers(window);

		// Poll for Keyboard and mouse events
		glfwPollEvents();
		//updateRectangle();


		// Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
		current_time = glfwGetTime(); // Time in seconds
		if ((current_time - last_update_time) >= 0.0001) { // atleast 0.5s elapsed since last frame
			// do something every 0.5 seconds ..
			last_update_time = current_time;
		}
	}

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
