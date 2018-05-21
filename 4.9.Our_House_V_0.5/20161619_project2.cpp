#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Shaders/LoadShaders.h"
GLuint h_ShaderProgram; // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

// include glm/*.hpp only if necessary
//#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.


#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f
#define NUMBER_OF_CAMERAS 9
#define CAM_TRANSLATION_SPEED 0.025f
#define CAM_ROTATION_SPEED 0.1f
int povUp = 0; //flag
int tiger_time = 0;
// ViewProjectionMatrix = ProjectionMatrix * ViewMatrix
glm::mat4 ViewProjectionMatrix[NUMBER_OF_CAMERAS], ViewMatrix[NUMBER_OF_CAMERAS], ProjectionMatrix[NUMBER_OF_CAMERAS];
// ModelViewProjectionMatrix = ProjectionMatrix * ViewMatrix * ModelMatrix
glm::mat4 ModelViewProjectionMatrix; // This one is sent to vertex shader when it is ready.
glm::mat4 ModelViewMatrix;

typedef enum {
	VIEW_WORLD, VIEW_SQUARE, VIEW_TIGER, VIEW_COW
} VIEW_MODE;

VIEW_MODE view_mode = VIEW_WORLD;

#include "Object_Definitions.h"

// Start of callback function definitions

typedef struct _CAMERA {
	glm::vec3 pos;
	glm::vec3 uaxis, vaxis, naxis;
	float fov_y, aspect_ratio, near_clip, far_clip;
	int move_status;
}CAMERA;
CAMERA camera[NUMBER_OF_CAMERAS];
int camera_selected;
typedef struct _VIEWPORT {
	int x, y, w, h;
}VIEWPORT;
VIEWPORT viewport[NUMBER_OF_CAMERAS];

typedef struct _CALLBACK_CONTEXT {
	int left_button_status, rotation_mode_cow, timestamp_cow;
	int prevx, prevy;
	float rotation_angle_cow;
} CALLBACK_CONTEXT;
CALLBACK_CONTEXT cc;

// Begin of callback supplement
#include "4.5.5.CallbackSupplement.h"
// End of callback supplement

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	for (int i = 0; i < NUMBER_OF_CAMERAS;i++)
		display_camera(i);

	glutSwapBuffers();
}

void set_ViewMatrix_from_camera_frame2(int cam_index) {
	ViewMatrix[cam_index][0] = glm::vec4(camera[cam_index].uaxis.x, camera[cam_index].vaxis.x, camera[cam_index].naxis.x, 0.0f);
	ViewMatrix[cam_index][1] = glm::vec4(camera[cam_index].uaxis.y, camera[cam_index].vaxis.y, camera[cam_index].naxis.y, 0.0f);
	ViewMatrix[cam_index][2] = glm::vec4(camera[cam_index].uaxis.z, camera[cam_index].vaxis.z, camera[cam_index].naxis.z, 0.0f);
	ViewMatrix[cam_index][3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	ViewMatrix[cam_index] = glm::translate(ViewMatrix[cam_index], -camera[cam_index].pos);
}

void set_ViewMatrix_from_camera_frame(int cam_index) {
	ViewMatrix[cam_index] = glm::mat4(1.0f);
	if (cam_index == 2)
	{
		ViewMatrix[2] = glm::lookAt(glm::vec3(120.0f, 90.0f, 1000.0f), glm::vec3(120.0f, 90.0f, 0.0f),
			glm::vec3(-10.0f, 0.0f, 0.0f));
		for (int i = 0; i<3; i++)
			printf("%5f %5f %5f\n", ViewMatrix[2][i].x, ViewMatrix[2][i].y, ViewMatrix[2][i].z);
		
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				switch (i)
				{
				case 0: printf("uaxis : %5f ", ViewMatrix[2][j].x);
					break;
				case 1: printf("vaxis : %5f ", ViewMatrix[2][j].y);
					break;
				case 2: printf("naxis : %5f", ViewMatrix[2][j].z);
					break;
				}
			}
				printf("\n");
		}

	}
	ViewMatrix[cam_index][0].x = camera[cam_index].uaxis.x;
	ViewMatrix[cam_index][0].y = camera[cam_index].vaxis.x;
	ViewMatrix[cam_index][0].z = camera[cam_index].naxis.x;

	ViewMatrix[cam_index][1].x = camera[cam_index].uaxis.y;
	ViewMatrix[cam_index][1].y = camera[cam_index].vaxis.y;
	ViewMatrix[cam_index][1].z = camera[cam_index].naxis.y;

	ViewMatrix[cam_index][2].x = camera[cam_index].uaxis.z;
	ViewMatrix[cam_index][2].y = camera[cam_index].vaxis.z;
	ViewMatrix[cam_index][2].z = camera[cam_index].naxis.z;
	
	
	ViewMatrix[cam_index] = glm::translate(ViewMatrix[cam_index], -camera[cam_index].pos);
	

}
void renew_cam_position_along_axis(int cam_index, float del, glm::vec3 trans_axis) {
	camera[cam_index].pos += CAM_TRANSLATION_SPEED * del*trans_axis;

	
}
void renew_cam_focus_view(int cam_index, float f)
{
	camera[cam_index].fov_y = f;
}
void renew_cam_orientation_rotation_around_axis(int cam_index, float angle, glm::vec3 rot_axis) {
	glm::mat3 RotationMatrix;

	RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0f), CAM_ROTATION_SPEED*TO_RADIAN*angle, rot_axis));

	camera[cam_index].uaxis = RotationMatrix * camera[cam_index].uaxis;
	camera[cam_index].vaxis = RotationMatrix * camera[cam_index].vaxis;
	camera[cam_index].naxis = RotationMatrix * camera[cam_index].naxis;
}


void timer_scene(int timestamp_scene) {

	tiger_time = (tiger_time + 1) % 500;
	tiger_data.cur_frame = timestamp_scene % N_TIGER_FRAMES;
	teapot_rotation_angle = (timestamp_scene % 360)*TO_RADIAN;
	tiger_data.rotation_angle = (timestamp_scene % 360)*TO_RADIAN;
	glutPostRedisplay();
	glutTimerFunc(100, timer_scene, (timestamp_scene + 1) % INT_MAX);
}

void mousepress(int button, int state, int x, int y) {
	if ((button == GLUT_LEFT_BUTTON)) {
		if (state == GLUT_DOWN) {
			cc.left_button_status = GLUT_DOWN;
			camera[camera_selected].move_status = 1;
			cc.prevx = x; cc.prevy = y;
		}
		else if (state == GLUT_UP) {
			cc.left_button_status = GLUT_UP;
			camera[camera_selected].move_status = 0;
		}
	}
}

void prepare_shader_program(void) {
	ShaderInfo shader_info[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
	{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
	{ GL_NONE, NULL }
	};

	h_ShaderProgram = LoadShaders(shader_info);
	glUseProgram(h_ShaderProgram);

	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram, "u_ModelViewProjectionMatrix");
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram, "u_primitive_color");
}

void motion(int x, int y) {
	float delx, dely;

	if (!camera[camera_selected].move_status) return;

	delx = (float)(x - cc.prevx); dely = (float)(cc.prevy - y);
	cc.prevx = x; cc.prevy = y;
	int current = camera[camera_selected].fov_y;
	int modifer_status = glutGetModifiers();

	switch (modifer_status) { // you may define the key combinations yourself.
	case GLUT_ACTIVE_SHIFT:
		renew_cam_position_along_axis(camera_selected, dely, camera[camera_selected].vaxis);
		renew_cam_position_along_axis(camera_selected, delx, camera[camera_selected].uaxis);
		break;
	case GLUT_ACTIVE_SHIFT | GLUT_ACTIVE_CTRL:
		renew_cam_orientation_rotation_around_axis(camera_selected, dely, -camera[camera_selected].uaxis);
		break;
	default:
		renew_cam_position_along_axis(camera_selected, dely, -camera[camera_selected].naxis);
		renew_cam_orientation_rotation_around_axis(camera_selected, delx, -camera[camera_selected].vaxis);
		break;
	}

	set_ViewMatrix_from_camera_frame(camera_selected);
	ViewProjectionMatrix[camera_selected] = ProjectionMatrix[camera_selected] * ViewMatrix[camera_selected];

	glutPostRedisplay();
}

void initialize_camera(void);

void keyboard(unsigned char key, int x, int y) {
	if (key == 27) { // ESC key
		glutLeaveMainLoop(); // incur destuction callback for cleanups.
		return;
	}
	switch (key) {
	case '0':
	case '1':
		if (cc.left_button_status == GLUT_UP) {
			camera[camera_selected].move_status = 0;
			camera_selected = key - '0';
			glutPostRedisplay();
		}
		break;
	case 'c':
		
		cc.rotation_mode_cow = 1 - cc.rotation_mode_cow;
		if (cc.rotation_mode_cow)
			glutTimerFunc(100, timer_scene, 0);
		
		break;
	
	case 'i':
		initialize_camera();
		ViewProjectionMatrix[0] = ProjectionMatrix[0] * ViewMatrix[0];
		ViewProjectionMatrix[1] = ProjectionMatrix[1] * ViewMatrix[1];
		glutPostRedisplay();
		break;

	case 'w':
		if (camera[camera_selected].fov_y >= 10.0f)
		{
			camera[camera_selected].fov_y -= 10.0f;
			ProjectionMatrix[camera_selected] = glm::perspective(camera[camera_selected].fov_y*TO_RADIAN, camera[camera_selected].aspect_ratio, camera[camera_selected].near_clip, camera[camera_selected].far_clip);
			ViewProjectionMatrix[camera_selected] = ProjectionMatrix[camera_selected] * ViewMatrix[camera_selected];
			glutPostRedisplay();
		}
		break;
	case 's':
		if (camera[camera_selected].fov_y + 10.0f <= camera[camera_selected].far_clip)
		{
			camera[camera_selected].fov_y += 10.0f;

			ProjectionMatrix[camera_selected] = glm::perspective(camera[camera_selected].fov_y*TO_RADIAN, camera[camera_selected].aspect_ratio, camera[camera_selected].near_clip, camera[camera_selected].far_clip);
			ViewProjectionMatrix[camera_selected] = ProjectionMatrix[camera_selected] * ViewMatrix[camera_selected];
			glutPostRedisplay();
		}
		break;
	case 't':
		if (view_mode == VIEW_TIGER)
		{
			view_mode = VIEW_WORLD;
		}
		else
		{
			view_mode = VIEW_TIGER;
		}
		if (view_mode == VIEW_TIGER)
			display_tiger(0);
		glutPostRedisplay();

		break;
	}
	
	glutPostRedisplay();
}

void reshape(int width, int height) {
	camera[0].aspect_ratio = (float)width / height;
	viewport[0].x = (int)(0.50f*width); viewport[0].y = (int)(0.10f*height);
	viewport[0].w = (int)(0.8f*width); viewport[0].h = (int)(1.0f*height);
	ProjectionMatrix[0] = glm::perspective(camera[0].fov_y*TO_RADIAN, camera[0].aspect_ratio, camera[0].near_clip, camera[0].far_clip);
	ViewProjectionMatrix[0] = ProjectionMatrix[0] * ViewMatrix[0];

	camera[1].aspect_ratio = camera[0].aspect_ratio; // for the time being ...
	viewport[1].x = (int)(0.00f*width); viewport[1].y = (int)(0.00f*height);
	viewport[1].w = (int)(0.30f*width); viewport[1].h = (int)(0.30*height);
	ProjectionMatrix[1] = glm::perspective(camera[1].fov_y*TO_RADIAN, camera[1].aspect_ratio, camera[1].near_clip, camera[1].far_clip);
	ViewProjectionMatrix[1] = ProjectionMatrix[1] * ViewMatrix[1];


	camera[2].aspect_ratio = camera[0].aspect_ratio; // for the time being ...
	viewport[2].x = (int)(0.00f*width); viewport[2].y = (int)(0.25f*height);
	viewport[2].w = (int)(0.30f*width); viewport[2].h = (int)(0.30*height);
	ProjectionMatrix[2] = glm::perspective(camera[2].fov_y*TO_RADIAN, camera[2].aspect_ratio, camera[2].near_clip, camera[2].far_clip);
	ViewProjectionMatrix[2] = ProjectionMatrix[2] * ViewMatrix[2];


	camera[3].aspect_ratio = camera[0].aspect_ratio; // for the time being ...
	viewport[3].x = (int)(0.00f*width); viewport[3].y = (int)(0.50f*height);
	viewport[3].w = (int)(0.30f*width); viewport[3].h = (int)(0.30*height);
	ProjectionMatrix[3] = glm::perspective(15.0f*TO_RADIAN, camera[3].aspect_ratio, 1.0f, 10000.0f);
	ViewProjectionMatrix[3] = ProjectionMatrix[3] * ViewMatrix[3];


	camera[4].aspect_ratio = camera[0].aspect_ratio; // for the time being ...
	viewport[4].x = (int)(0.00f*width); viewport[4].y = (int)(0.70f*height);
	viewport[4].w = (int)(0.30f*width); viewport[4].h = (int)(0.30*height);
	ProjectionMatrix[4] = glm::perspective(15.0f*TO_RADIAN, camera[4].aspect_ratio, 1.0f, 10000.0f);
	ViewProjectionMatrix[4] = ProjectionMatrix[4] * ViewMatrix[4];


	camera[5].aspect_ratio = camera[0].aspect_ratio; // for the time being ...
	viewport[5].x = (int)(0.00f*width); viewport[5].y = (int)(0.3f*height);
	viewport[5].w = (int)(0.30f*width); viewport[5].h = (int)(0.30*height);
	ProjectionMatrix[5] = glm::perspective(15.0f*TO_RADIAN, camera[5].aspect_ratio, 1.0f, 10000.0f);
	ViewProjectionMatrix[5] = ProjectionMatrix[5] * ViewMatrix[5];

	glutPostRedisplay();
}

void cleanup(void) {/*
	glDeleteVertexArrays(1, &points_VAO);
	glDeleteBuffers(1, &points_VBO);

	glDeleteVertexArrays(1, &axes_VAO);
	glDeleteBuffers(1, &axes_VBO);

	glDeleteVertexArrays(N_OBJECTS, object_VAO);
	glDeleteBuffers(N_OBJECTS, object_VBO);*/
}
// End of callback function definitions

void prepare_callbacks(void) {
	cc.left_button_status = GLUT_UP;
	cc.rotation_mode_cow = 1;
	cc.timestamp_cow = 0;
	cc.rotation_angle_cow = 0.0f;

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mousepress);
	glutMotionFunc(motion);
	glutReshapeFunc(reshape);
	glutTimerFunc(100, timer_scene, 0);
	glutCloseFunc(cleanup);
}
void set(void)
{

	ViewMatrix[2] = glm::lookAt(glm::vec3(120.0f, 90.0f, 1000.0f), glm::vec3(120.0f, 90.0f, 0.0f),
		glm::vec3(-10.0f, 0.0f, 0.0f));
}
void initialize_camera(void) {
	// called only once when the OpenGL system is initialized!!!
	// only ViewMatrix[*] are set up in this function.
	// ProjectionMatrix will be initialized in the reshape callback function when the window pops up.

	// Camera 0/
	/*
	camera[0].pos = glm::vec3(100.0f, 300.0f, 100.0f);
	camera[0].uaxis = glm::vec3(1.0f, 0.0f, 0.0f);
	camera[0].vaxis = glm::vec3(0.0f, 0.0f, 1.0f);
	camera[0].naxis = glm::vec3(0.0f, 1.0f, 0.0f);
	*/
	
	
	camera[0].pos = glm::vec3(100.0f, 300.0f, 100.0f);
	camera[0].uaxis = glm::vec3(1.0f, 0.0f, 0.0f);
	camera[0].vaxis = glm::vec3(0.0f, 0.0f, 1.0f);
	camera[0].naxis = glm::vec3(0.0f, 1.0f, 0.0f);

	camera[0].move_status = 0;
	camera[0].fov_y = 100.0f;

	camera[0].aspect_ratio = 1.0f; // will be set when the viewing window pops up.
	camera[0].near_clip = 0.01f;
	camera[0].far_clip = 500.0f;
	set_ViewMatrix_from_camera_frame(0);

	//Camera 1  안고쳐도됨
	camera[1].pos = glm::vec3(120.0f, 400.0f,0.0f);
	camera[1].uaxis = glm::vec3(0.7f, 0.3f, 0.0f);
	camera[1].vaxis = glm::vec3(0.0f, 0.0f, 1.0f);
	camera[1].naxis = glm::vec3(0.0f, 1.0f, 0.0f);

	camera[1].move_status = 0;
	camera[1].fov_y = 50.0f;
	camera[1].aspect_ratio = 1.0f; // will be set when the viewing window pops up.
	camera[1].near_clip = 0.01f;
	camera[1].far_clip = 500.0f;

	set_ViewMatrix_from_camera_frame(1);

	//ViewMatrix[3] = glm::lookAt(glm::vec3(120.0f, 90.0f, 1000.0f), glm::vec3(120.0f, 90.0f, 0.0f),
		//glm::vec3(-10.0f, 0.0f, 0.0f));
	//Camera  2
	camera[2].pos = glm::vec3(0.0f, 500.0f, 00.0f);
	camera[2].uaxis = glm::vec3(0.0f, 1.0f, 0.0f);
	camera[2].vaxis = glm::vec3(-1.0f, 0.0f, 0.0f);
	camera[2].naxis = glm::vec3(0.0f, 0.0f, 1.0f);

	camera[2].move_status = 0;
	camera[2].fov_y = 50.0f;
	camera[2].aspect_ratio = 1.0f; // will be set when the viewing window pops up.
	camera[2].near_clip = 0.01f;
	camera[2].far_clip = 500.0f;
	set_ViewMatrix_from_camera_frame(2);
	

	//Camera 3  측면 


	camera[3].pos = glm::vec3(100.0f, 300.0f, 100.0f);
	camera[3].uaxis = glm::vec3(1.0f, 0.0f, 0.0f);
	camera[3].vaxis = glm::vec3(0.0f, 0.0f, 1.0f);
	camera[3].naxis = glm::vec3(0.0f, 1.0f, 0.0f);

	camera[3].move_status = 0;
	camera[3].fov_y = 100.0f;
	camera[3].aspect_ratio = 1.0f; // will be set when the viewing window pops up.
	camera[3].near_clip = 0.01f;
	camera[3].far_clip = 500.0f;

	set_ViewMatrix_from_camera_frame(3);


	//Camera 3
	
	camera[4].pos = glm::vec3(100.0f, 300.0f, 100.0f);
	camera[4].uaxis = glm::vec3(1.0f, 0.0f, 0.0f);
	camera[4].vaxis = glm::vec3(0.0f, 0.0f, 1.0f);
	camera[4].naxis = glm::vec3(0.0f, 1.0f, 0.0f);
	
	//set();
	camera[4].move_status = 0;
	camera[4].fov_y = 100.0f;
	camera[4].aspect_ratio = 1.0f; // will be set when the viewing window pops up.
	camera[4].near_clip = 0.01f;
	camera[4].far_clip = 500.0f;
	
	set_ViewMatrix_from_camera_frame(4);
	

	camera[5].pos = glm::vec3(100.0f, 300.0f, 100.0f);
	camera[5].uaxis = glm::vec3(1.0f, 0.0f, 0.0f);
	camera[5].vaxis = glm::vec3(0.0f, 0.0f, 1.0f);
	camera[5].naxis = glm::vec3(0.0f, 1.0f, 0.0f);

	//set();
	camera[5].move_status = 0;
	camera[5].fov_y = 100.0f;
	camera[5].aspect_ratio = 1.0f; // will be set when the viewing window pops up.
	camera[5].near_clip = 0.01f;
	camera[5].far_clip = 500.0f;

	set_ViewMatrix_from_camera_frame(5);
	camera_selected = 0;

}

void initialize_OpenGL(void) {
	initialize_camera();
	glEnable(GL_DEPTH_TEST); // Default state

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glClearColor(0.12f, 0.18f, 0.12f, 1.0f);



	if (0) {
		ViewMatrix[0] = glm::lookAt(glm::vec3(120.0f, 90.0f, 1000.0f), glm::vec3(120.0f, 90.0f, 0.0f),
			glm::vec3(-10.0f, 0.0f, 0.0f));
	}
	if (1) {
		ViewMatrix[6] = glm::lookAt(glm::vec3(800.0f, 90.0f, 25.0f), glm::vec3(0.0f, 90.0f, 25.0f),
			glm::vec3(0.0f, 0.0f, 1.0f));

		ViewMatrix[4] = glm::lookAt(glm::vec3(120.0f, 90.0f, 1000.0f), glm::vec3(120.0f, 90.0f, 0.0f),
			glm::vec3(-10.0f, 0.0f, 0.0f));


		ViewMatrix[5] = glm::lookAt(glm::vec3(600.0f, 600.0f, 200.0f), glm::vec3(125.0f, 80.0f, 25.0f),
			glm::vec3(0.0f, 0.0f, 1.0f));
	}

	if (1) {

		camera[0].pos = glm::vec3(200.0f, 300.0f, 100.0f);
		camera[0].uaxis = glm::vec3(1.0f, 0.0f, 0.0f);
		camera[0].vaxis = glm::vec3(0.0f, 0.0f, 1.0f);
		camera[0].naxis = glm::vec3(0.0f, 1.0f, 0.0f);

		
		set_ViewMatrix_from_camera_frame(0);
		
		/*
		if (0) {
			ViewMatrix[2] = glm::lookAt(glm::vec3(120.0f, 90.0f, 1000.0f), glm::vec3(120.0f, 90.0f, 0.0f),
				glm::vec3(-10.0f, 0.0f, 0.0f));
		}
		if (0) {
			ViewMatrix[2] = glm::lookAt(glm::vec3(800.0f, 90.0f, 25.0f), glm::vec3(0.0f, 90.0f, 25.0f),
				glm::vec3(0.0f, 0.0f, 1.0f));
		}

		if (1) {
		//	ViewMatrix[2] = glm::lookAt(glm::vec3(600.0f, 600.0f, 200.0f), glm::vec3(125.0f, 80.0f, 25.0f),
			//	glm::vec3(0.0f, 0.0f, 1.0f));
		}
		//ViewMatrix[0]= glm::lookAt(glm::vec3(600.0f, 600.0f, 200.0f), glm::vec3(125.0f, 80.0f, 25.0f),
			//glm::vec3(0.0f, 0.0f, 1.0f));
	*/
	}

	glClearColor(0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);

	ViewMatrix[3] = glm::lookAt(glm::vec3(800.0f, 90.0f, 25.0f), glm::vec3(0.0f, 90.0f, 25.0f),
		glm::vec3(0.0f, 0.0f, 1.0f));

	ViewMatrix[4] = glm::lookAt(glm::vec3(120.0f, 90.0f, 1000.0f), glm::vec3(120.0f, 90.0f, 0.0f),
		glm::vec3(-10.0f, 0.0f, 0.0f));


	ViewMatrix[5] = glm::lookAt(glm::vec3(600.0f, 600.0f, 200.0f), glm::vec3(125.0f, 80.0f, 25.0f),
		glm::vec3(0.0f, 0.0f, 1.0f));
	//[3] = glm::lookAt(glm::vec3(120.0f, 90.0f, 1000.0f), glm::vec3(120.0f, 90.0f, 0.0f),
	//	glm::vec3(-10.0f, 0.0f, 0.0f));
}

void prepare_scene(void) {
	define_axes();
	define_static_objects();
	define_animated_tiger();
	define_animated_teapot();
}

void initialize_renderer(void) {
	prepare_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	prepare_scene();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = GL_TRUE;

	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "*********************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "*********************************************************\n\n");
}

void print_message(const char * m) {
	fprintf(stdout, "%s\n\n", m);
}

void greetings(char *program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 1
void main(int argc, char *argv[]) { 
	char program_name[256] = "Sogang CSE4170 Our_House_GLSL_V_0.5";
	char messages[N_MESSAGE_LINES][256] = { "    - Keys used: 'c', 'f', 'd', 'ESC'" };

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(1200, 800);
	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}
