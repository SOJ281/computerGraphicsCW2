#include <glad.h>
#include <GLFW/glfw3.h>

#include <typeinfo>
#include <stdexcept>

#include <cstdio>
#include <cstdlib>

#include "../support/error.hpp"
#include "../support/program.hpp"
#include "../support/checkpoint.hpp"
#include "../support/debug_output.hpp"

#include "../vmlib/vec4.hpp"
#include "../vmlib/mat44.hpp"
#include "../vmlib/mat33.hpp"

#include "defaults.hpp"
#include "cylinder.hpp"
//#include "loadcustom.hpp"
#include "loadobj.hpp"
#include <string>
#include <cstring>
#include <stb_image.h>


namespace
{
	constexpr char const* kWindowTitle = "COMP3811 - Coursework 2";
	
	constexpr float kPi_ = 3.1415926f;

	//Camera state and constant values.
	constexpr float kMovementPerSecond_ = 10.f; // units per second
	constexpr float kMovementModNeg_ = 2.5f;
	constexpr float kMovementModPos_ = 5.f;
	constexpr float kMouseSensitivity_ = 0.01f; // radians per pixel

	
	struct State_
	{
		ShaderProgram* prog;

		struct CamCtrl_
		{
			bool cameraActive;
			bool forward, backward, left, right, up, down;
			bool actionSpeedUp, actionSlowDown;

			float phi, theta;

			float lastX, lastY;
			// Camera position, direction faced and up vector.
			Vec3f cameraPos = { 0.f, -3.f, -3.f };
			Vec3f cameraFront = {0.f, 0.f, 1.f};
			Vec3f cameraUp = { 0.f, 1.f, 0.f };

		} camControl;
	};

	//Point light struct
	struct PointLight {
		Vec3f position;
		float constant;
		float linear;
		float quadratic;
		Vec3f ambient;
		Vec3f diffuse;
	};
	struct Material {
		Vec3f ambient;
		Vec3f diffuse;
		Vec3f specular;
		Vec3f emissive;   
		float shininess;
	}; 

	unsigned int loadTexture(const char *path);
	void setMaterial(Material material);

	void glfw_callback_error_( int, char const* );

	void glfw_callback_key_( GLFWwindow*, int, int, int, int );
	void glfw_callback_motion_(GLFWwindow*, double, double);

	struct GLFWCleanupHelper
	{
		~GLFWCleanupHelper();
	};
	struct GLFWWindowDeleter
	{
		~GLFWWindowDeleter();
		GLFWwindow* window;
	};
}

int main() try
{
	// Initialize GLFW
	if( GLFW_TRUE != glfwInit() )
	{
		char const* msg = nullptr;
		int ecode = glfwGetError( &msg );
		throw Error( "glfwInit() failed with '%s' (%d)", msg, ecode );
	}

	// Ensure that we call glfwTerminate() at the end of the program.
	GLFWCleanupHelper cleanupHelper;

	// Configure GLFW and create window
	glfwSetErrorCallback( &glfw_callback_error_ );

	glfwWindowHint( GLFW_SRGB_CAPABLE, GLFW_TRUE );
	glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );

	//glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
	glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

	glfwWindowHint( GLFW_DEPTH_BITS, 24 );

#	if !defined(NDEBUG)
	// When building in debug mode, request an OpenGL debug context. This
	// enables additional debugging features. However, this can carry extra
	// overheads. We therefore do not do this for release builds.
	glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE );
#	endif // ~ !NDEBUG

	GLFWwindow* window = glfwCreateWindow(
		1280,
		720,
		kWindowTitle,
		nullptr, nullptr
	);

	if( !window )
	{
		char const* msg = nullptr;
		int ecode = glfwGetError( &msg );
		throw Error( "glfwCreateWindow() failed with '%s' (%d)", msg, ecode );
	}

	GLFWWindowDeleter windowDeleter{ window };


	// Set up event handling
	State_ state{};

	glfwSetWindowUserPointer(window, &state);

	// Set up event handling
	glfwSetKeyCallback( window, &glfw_callback_key_ );
	glfwSetCursorPosCallback(window, &glfw_callback_motion_);

	// Set up drawing stuff
	glfwMakeContextCurrent( window );
	glfwSwapInterval( 1 ); // V-Sync is on.

	// Initialize GLAD
	// This will load the OpenGL API. We mustn't make any OpenGL calls before this!
	if( !gladLoadGLLoader( (GLADloadproc)&glfwGetProcAddress ) )
		throw Error( "gladLoaDGLLoader() failed - cannot load GL API!" );

	std::printf( "RENDERER %s\n", glGetString( GL_RENDERER ) );
	std::printf( "VENDOR %s\n", glGetString( GL_VENDOR ) );
	std::printf( "VERSION %s\n", glGetString( GL_VERSION ) );
	std::printf( "SHADING_LANGUAGE_VERSION %s\n", glGetString( GL_SHADING_LANGUAGE_VERSION ) );

	// Ddebug output
#	if !defined(NDEBUG)
	setup_gl_debug_output();
#	endif // ~ !NDEBUG

	// Global GL state
	OGL_CHECKPOINT_ALWAYS();

	// Global GL setup goes here
	glEnable(GL_FRAMEBUFFER_SRGB);
	//glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.f, 0.f, 0.1f, 0.0f);


	OGL_CHECKPOINT_ALWAYS();

	// Get actual framebuffer size.
	// This can be different from the window size, as standard window
	// decorations (title bar, borders, ...) may be included in the window size
	// but not be part of the drawable surface area.
	int iwidth, iheight;
	glfwGetFramebufferSize( window, &iwidth, &iheight );

	glViewport( 0, 0, iwidth, iheight );

	// Load shader program
	ShaderProgram prog({
		{ GL_VERTEX_SHADER, "assets/default.vert" },
		{ GL_FRAGMENT_SHADER, "assets/default.frag" }
		});
	state.prog = &prog;

	// Animation state
	auto last = Clock::now();
	float angle = 0.f;

	// Other initialization & loading
	OGL_CHECKPOINT_ALWAYS();
	
	// TODO: 

	/*
	std::vector<SimpleMeshData> besties;
	auto bestie = load_simple_binary_mesh( "assets/Armadillo.comp3811bin" );
	auto bestieIndex = load_simple_binary_mesh_index( "assets/Armadillo.comp3811bin" );
	int friends = 2;
	for (float i = 0; i < friends; i++)
		for (float l = 0; l < friends; l++)
			besties.emplace_back(make_change(bestie, make_translation( { -5.f + 10*i, 0.f, -5.f + 10*l }) ));
	for (int i = 0; i < bestie.positions.size(); i++) {
		if (bestie.positions[i].x != bestieIndex.positions[bestieIndex.indices[i]].x || bestie.positions[i].y != bestieIndex.positions[bestieIndex.indices[i]].y 
		 || bestie.positions[i].z != bestieIndex.positions[bestieIndex.indices[i]].z)
			printf("value =(%f,%f,%f)\n", bestie.positions[i].x,bestie.positions[i].y,bestie.positions[i].z);
	}

	//auto floor = make_cube({1, 0, 1}, make_scaling( 200.f, 2.f, 200.f ) * make_translation( { -100.f, -2.f, -100.f }));
	auto floor = make_cube( Vec3f{1, 1, 1}, make_scaling( 200.f, 2.f, 200.f ) * make_translation( { 0.f, -1.f, 0.f }));
	besties.emplace_back(floor);

	auto pillar = make_cylinder( true, 18, {1, 0, 1}, make_scaling( 5.f, 1.f, 1.f ));
	int rows = 3;
	for (float i = 0; i < rows; i++)
		for (float l = 0; l < 2; l++)
			besties.emplace_back(make_change(pillar, make_translation({ 4.f - l * 8.f, 0.f, 9.f + i * 5.f }) * make_rotation_z(3.141592f / 2.f)));

	GLuint vao = create_vaoM(&besties[0],11);
	printf("floor =(%ld)", floor.positions.size());
	printf("pillar =(%ld)", pillar.positions.size());
	int vertexCount = bestie.positions.size() *friends * friends + floor.positions.size() +pillar.positions.size() * rows*2;*/

	/*
	Vec3f ambient;
	Vec3f diffuse;
	Vec3f specular;
	Vec3f emissive;   
	float shininess;
	*/
	//Random guess
	Material stone = {
		Vec3f{.01f, .01f, .01f},
		Vec3f{.4f, .4f, .4f},
		Vec3f{0.f, 0.f, 0.f},
		Vec3f{0.0f, 0.05f, 0.05f},
		.4f
	};
	Material vantaBlack = {
		Vec3f{0.f, 0.f, 0.f},
		Vec3f{0.f, 0.f, 0.f},
		Vec3f{0.f, 0.f, 0.f},
		Vec3f{0.0f, 0.0f, 0.0f},
		.4f
	};

	//For coursework objective
	Material uranium = {
		Vec3f{.01f, .01f, .01f},
		Vec3f{0.f, 0.f, 0.f},
		Vec3f{0.f, 0.f, 0.f},
		Vec3f{1.f, .5f, 0.3f},
		.4f
	};
	Material shinyShiny = {
		Vec3f{.01f, .01f, .01f},
		Vec3f{0.5f, 0.5f, 0.f},
		Vec3f{1.f, 1.f, 0.f},
		Vec3f{0.f, 0.f, 0.f},
		.4f
	};
	Material noPun = {
		Vec3f{.01f, .01f, .01f},
		Vec3f{1.f, 1.f, 0.f},
		Vec3f{0.f, 0.f, 0.f},
		Vec3f{0.f, 0.f, 0.f},
		.4f
	};
	//From Here http://devernay.free.fr/cours/opengl/materials.html
	Material brass = {
		Vec3f{0.329412f, 0.223529f, 0.027451f},
		Vec3f{0.780392f, 0.568627f, 0.113725f},
		Vec3f{0.992157f, 0.941176f, 0.807843f},
		Vec3f{0.05f, 0.05f, 0.05f}, //Most objects don't emit light
		0.21794872f
	};
	//From MTL files:
	Material cushion = {
		Vec3f{0.f, 0.f, 0.f},
		Vec3f{0.203922f, 0.305882f, 0.556863f},
		Vec3f{0.009961f, 0.009961f, 0.009961f},
		Vec3f{0.f, 0.f, 0.f},
		.4f
	};

	Material wood = {
		Vec3f{0.f, 0.f, 0.f},
		Vec3f{0.356863f, 0.223529f, 0.019608f},
		Vec3f{0.009961f, 0.009961, 0.009961f},
		Vec3f{0.f, 0.f, 0.f},
		.4f
	};

	unsigned int ourSaviour = loadTexture("assets/markus.png");
	unsigned int ourBlank = loadTexture("assets/Solid_white.png");
	unsigned int ourVoid = loadTexture("assets/Solid_black.png");
	


	std::vector<SimpleMeshData> chapel;

	auto floor = make_cube( Vec3f{0.07f, 0.07f, 0.07f}, make_scaling( 200.f, 2.f, 200.f ) * make_translation( { 0.f, -1.f, 0.f }));
	chapel.emplace_back(floor);

	//auto frontWall = make_cube( Vec3f{1, 1, 1}, make_scaling( 5.f, 10.f, 0.5f ) * make_translation( { 3.5f, 5.f, 2.25f }));
	auto frontWall = make_cube( Vec3f{.02f, .02f, .02f}, make_scaling( 13.f, 15.f, 1.f )); //so 10 meter
	int wallBits = 2;
	for (float i = 0; i < wallBits; i++)
		chapel.emplace_back(make_change(frontWall, make_translation( { 17.f - 34.f*i, 15.f, 5.f }) ));


	auto sideWall = make_cube( Vec3f{.02f, .02f, .02f}, make_scaling( 1.f, 15.f, 24.f ));//so 48 meter
	int sideWallBits = 2;
	for (float i = 0; i < sideWallBits; i++)
		chapel.emplace_back(make_change(sideWall, make_translation( { 29.f - 58.f*i, 15.f, 30.f }) ));


	auto pillar = make_cylinder( true, 18, {.1f, 0, .1f}, make_scaling( .75f, 10.f, .75f ) * make_rotation_z( 3.141592f / 2.f ));
	auto lamp = make_cylinder( true, 18, {.1f, .1f, .1f}, make_scaling( .1f, 5.f, .1f ) * make_rotation_z( 3.141592f / 2.f ));
	//lamp = invert_normals(lamp);
	struct PointLight {
			Vec3f position;
			float constant;
			float linear;
			float quadratic;
			Vec3f ambient;
			Vec3f diffuse;
			Vec3f specular;
	};
	std::vector<PointLight> PointLightData;
	int rows = 3;
	int lampCount = 6;
	for (float i = 0; i < rows; i++)
		for (float l = 0; l < 2; l++) {
			chapel.emplace_back(make_change(pillar, make_translation( { 5.f - 10.f*l, 0.f, 15.f + 15.f * i }) ));
			//chapel.emplace_back(make_cylinder( true, 18, Vec3f{1, 0, 1}, make_translation( { 22.f - 44.f*i, 0.f, 29.f })));
		
			PointLight p1;
			p1.position = Vec3f{ 3.f - 6.f*l, 4.f, 15.f + 15.f * i };
			p1.constant = 1.f;
			p1.linear = 0.3f;
			p1.quadratic = 0.032f;
			p1.ambient = Vec3f{ .5f, .15f, .05f };
			p1.diffuse = Vec3f{ .5f, .4f, .05f };
			p1.specular = Vec3f{ .5f, .4f, .05f };
			PointLightData.emplace_back(p1);
			chapel.emplace_back(make_change(lamp, make_translation( { 3.f - 6.f*l, 4.f, 17.f + 15.f * i }) ));
		}

	/*
	unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);
    glBindVertexArray(transparentVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);*/


	auto picture = make_cube( Vec3f{0.05f, 0.05f, 0.05f}, make_rotation_z( -3.141592f / 2.f ) * make_rotation_y( 3.141592f / 2.f ) * make_scaling( .1f, 6.f, 7.62f ));//so 48 meter
	chapel.emplace_back(make_change(picture, make_translation( { 0.f, 9.f, 108.8f }) ));
	int pictures = 1;

	auto backroom = make_partial_building( true, 6, 1, {.02f, .02f, .02f},  make_rotation_y( -3.141592f / 3.f )* make_rotation_z( 3.141592f / 2.f ));
	chapel.emplace_back(make_change(backroom, make_translation( { 0.f, 0.f, 86.8f })*  make_scaling( 60.f, 40.f, 40.f ) ));
	//chapel.emplace_back(backroom);
	int backrooms = 1;


	std::vector<SimpleMeshData> parts;
	auto benches = load_wavefront_obj( "assets/chair-y-good-obj/chair-y-good.obj" );
	auto benchShapes = getDimensions( "assets/chair-y-good-obj/chair-y-good.obj" );
	benches = make_change(benches, make_scaling( 5.f, 1.f, 1.f ) );
	benches = make_change(benches, make_rotation_y( 3.141592f ) );
	benches = make_change(benches, make_translation( {-8.f, 0.f, 15.f }) );
	int benchCount = 12;
	//printf("benches ", benches.positions.size());
	for (float i = 0; i < 6; i++)
		for (float l = 0; l < 2; l++)
			chapel.emplace_back(make_change(benches, make_translation( {29.f * l, 0.f, i*6.f }) ));

	

	auto roof = make_partial_cylinder( true, 16, 8, {.02f, .02f, .02f},  make_scaling( 1.f, 1.f, 1.f ));
	roof = make_change(roof, make_rotation_x( -3.141592f / 2.f )* make_rotation_z( 3.141592f / 2.f ) );
	roof = make_change(roof, make_scaling( 27.f, 20.f, 50.f ) );
	roof = make_change(roof, make_translation( {0.f, 30.f, 55.f }) );
	chapel.emplace_back(roof);
	int roofCount = 1;

	auto plutonium = make_cube( Vec3f{0.05f, 0.05f, 0.05f}, make_scaling( 2.f, 2.f, 2.f ));//so 48 meter
	chapel.emplace_back(plutonium);
	int plutoniumCount = 1;


	int vertexCount = floor.positions.size() + frontWall.positions.size() * wallBits + sideWall.positions.size() * sideWallBits 
	+ pillar.positions.size() * rows*2;
	float sunXloc = -1.f;
	printf("\nNINJA\n");
	GLuint vao = create_vaoM(&chapel[0], 1 + wallBits + sideWallBits + rows*2 + lampCount + pictures + backrooms + benchCount + roofCount + plutoniumCount);
	printf("\nNINJA2\n");


	OGL_CHECKPOINT_ALWAYS();

	// Main loop
	while( !glfwWindowShouldClose( window ) )
	{
		// Let GLFW process events
		glfwPollEvents();
		
		// Check if window was resized.
		float fbwidth, fbheight;
		{
			int nwidth, nheight;
			glfwGetFramebufferSize( window, &nwidth, &nheight );

			fbwidth = float(nwidth);
			fbheight = float(nheight);

			if( 0 == nwidth || 0 == nheight )
			{
				// Window minimized? Pause until it is unminimized.
				// This is a bit of a hack.
				do
				{
					glfwWaitEvents();
					glfwGetFramebufferSize( window, &nwidth, &nheight );
				} while( 0 == nwidth || 0 == nheight );
			}

			glViewport( 0, 0, nwidth, nheight );
		}

		// Update state of camera
		// TODO: Make movement based on current camera orientation.
		auto const now = Clock::now();
		float dt = std::chrono::duration_cast<Secondsf>(now - last).count();
		last = now;
		angle += dt * kPi_ * 0.3f;
		if (angle >= 2.f * kPi_)
			angle -= 2.f * kPi_;



		//Camera speed decision.
		float speedChange = 0;
		if (state.camControl.actionSpeedUp)
			speedChange = kMovementModPos_;
		else if (state.camControl.actionSlowDown)
			speedChange = -kMovementModNeg_;
		else
			speedChange = 0;
		// Update camera state based on keyboard input.
		//Forward, backward
		if (state.camControl.forward) {
			state.camControl.cameraPos += state.camControl.cameraFront * (kMovementPerSecond_ + speedChange) * dt;
		} else if (state.camControl.backward) {
			state.camControl.cameraPos -= state.camControl.cameraFront *  (kMovementPerSecond_ + speedChange) * dt;
		}
		//Left, right
		if (state.camControl.left) {
			state.camControl.cameraPos -= normalize(cross(state.camControl.cameraFront, state.camControl.cameraUp)) * (kMovementPerSecond_ + speedChange) * dt;
		} else if (state.camControl.right) {
			state.camControl.cameraPos += normalize(cross(state.camControl.cameraFront, state.camControl.cameraUp)) * (kMovementPerSecond_ + speedChange) * dt;
		}
		//Up, down
		if (state.camControl.up) {
			state.camControl.cameraPos.y += (kMovementPerSecond_ + speedChange) * dt;
		}else if (state.camControl.down) {
			state.camControl.cameraPos.y -= (kMovementPerSecond_ + speedChange) * dt;
		}


		//TODO: define and compute projCameraWorld matrix
		Mat44f model2world = make_rotation_y(0);
		Mat44f Rx = make_rotation_x( state.camControl.theta );
		Mat44f Ry = make_rotation_y( state.camControl.phi );
		Mat44f world2camera = make_translation(state.camControl.cameraPos);
		//Mat44f world2camera = T*Rx*Ry;
		//Mat44f world2camera = make_translation( { 0.f, 0.f, -10.f } );

		Mat44f projection = make_perspective_projection(
			60.f * 3.1415926f / 180.f,
			fbwidth/float(fbheight),
			0.1f, 100.0f
		);
		projection = projection*Rx*Ry;

		//door = make_change( door, make_rotation_y(angle) );


		Mat44f projCameraWorld = projection * world2camera * model2world;
		Mat33f normalMatrix = mat44_to_mat33( transpose(invert(model2world)) );
		/*
		for (int i = 0; i< 4; i++)
			printf("projection: (%f, %f, %f, %f)\n", projection.v[i*4],projection.v[i*4+1],projection.v[i*4+2],projection.v[i*4+3]);
		printf("\n");
		for (int i = 0; i< 4; i++)
			printf("projCameraWorld: (%f, %f, %f, %f)\n", projCameraWorld.v[i*4],projCameraWorld.v[i*4+1],projCameraWorld.v[i*4+2],projCameraWorld.v[i*4+3]);

		for (int i = 0; i< 3; i++)
			printf("normalMatrix: (%f, %f, %f)\n", projCameraWorld.v[i*3],projCameraWorld.v[i*3+1],projCameraWorld.v[i*3+2]);
		*/


		sunXloc += .01f;
		if (sunXloc > .99f)
			sunXloc = -1.f;
		Vec3f lightDir = normalize( Vec3f{ 0.f, .5f, -1.f } );

		Vec3f lightPos = Vec3f{ 0.f, 1.f, 0.f };

		


	
		// Draw scene
		OGL_CHECKPOINT_DEBUG();

		//Draw frame
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glUseProgram( prog.programId() );



		glUniform1i(glGetUniformLocation(prog.programId(), "texture.diffuse"), 0); //lightDir uLightDir
		glUniform1i(glGetUniformLocation(prog.programId(), "texture.specular"), 0); //lightDir uLightDir


		glUniform3fv(glGetUniformLocation(prog.programId(), "uLightDir"), 1, &lightDir.x ); //lightDir uLightDir
		glUniform3f( glGetUniformLocation(prog.programId(), "uLightDiffuse"), .6f, .6f, .6f ); //lightDiffuse
		glUniform3f( glGetUniformLocation(prog.programId(), "uSceneAmbient"), 0.1f, 0.1f, 0.1f ); //uSceneAmbient
		//printf("normalMatrix: (%f, %f, %f)\n", state.camControl.cameraPos.x, state.camControl.cameraPos.y, state.camControl.cameraPos.z);
		glUniform3f( glGetUniformLocation(prog.programId(), "viewPos"), state.camControl.cameraPos.x, state.camControl.cameraPos.y, state.camControl.cameraPos.z ); //uSceneAmbient

		
		#include <string>
		for (int i = 0; i < 6; i++) {

			std::string number = std::to_string(i);

			glUniform3fv(glGetUniformLocation(prog.programId(), ("pointLights["+number+"].position").c_str()), 1, &PointLightData[i].position.x ); //lightDir uLightDir
			glUniform1f(glGetUniformLocation(prog.programId(), ("pointLights["+number+"].constant").c_str()), PointLightData[i].constant ); //lightDir uLightDir
			glUniform1f(glGetUniformLocation(prog.programId(), ("pointLights["+number+"].linear").c_str()), PointLightData[i].linear ); //lightDir uLightDir
			glUniform1f(glGetUniformLocation(prog.programId(), ("pointLights["+number+"].quadratic").c_str()), PointLightData[i].quadratic ); //lightDir uLightDir
			glUniform3fv(glGetUniformLocation(prog.programId(), ("pointLights["+number+"].ambient").c_str()), 1, &PointLightData[i].ambient.x ); //lightDir uLightDir
			glUniform3fv(glGetUniformLocation(prog.programId(), ("pointLights["+number+"].diffuse").c_str()), 1, &PointLightData[i].diffuse.x ); //lightDir uLightDir
			glUniform3fv(glGetUniformLocation(prog.programId(), ("pointLights["+number+"].specular").c_str()), 1, &PointLightData[i].specular.x ); //lightDir uLightDir
		}
		//scottscott681
		//gazmaz99

		glUniformMatrix4fv(glGetUniformLocation(prog.programId(), "uProjCameraWorld"), 1, GL_TRUE, projCameraWorld.v );
		glUniformMatrix3fv(glGetUniformLocation(prog.programId(), "uNormalMatrix"),1, GL_TRUE, normalMatrix.v);
		Mat44f blankMatrix = make_translation( { 1.f, 1.f, 1.f } );
		glUniformMatrix3fv(glGetUniformLocation(prog.programId(), "transformation"),1, GL_TRUE, blankMatrix.v);

		
		//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		
		glBindVertexArray( vao );

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, NULL);
/*
		glUniform3fv(glGetUniformLocation(prog.programId(), "material.ambient"), 1, &vantaBlack.ambient.x );
		glUniform3fv(glGetUniformLocation(prog.programId(), "material.diffuse"), 1, &vantaBlack.diffuse.x );
		glUniform3fv(glGetUniformLocation(prog.programId(), "material.specular"), 1, &vantaBlack.specular.x );
		glUniform1f(glGetUniformLocation(prog.programId(), "material.shininess"), vantaBlack.shininess);
*/

		glUniform3fv(glGetUniformLocation(prog.programId(), "material.ambient"), 1, &stone.ambient.x );
		glUniform3fv(glGetUniformLocation(prog.programId(), "material.diffuse"), 1, &stone.diffuse.x );
		glUniform3fv(glGetUniformLocation(prog.programId(), "material.specular"), 1, &stone.specular.x );
		glUniform3fv(glGetUniformLocation(prog.programId(), "material.emissive"), 1, &vantaBlack.emissive.x );
		glUniform1f(glGetUniformLocation(prog.programId(), "material.shininess"), stone.shininess);
		int counter = floor.positions.size();
		glDrawArrays( GL_TRIANGLES, 0, counter);
		


		counter += frontWall.positions.size() * wallBits;
		glDrawArrays( GL_TRIANGLES, 0, counter);

		counter += sideWall.positions.size() * sideWallBits;
		glDrawArrays( GL_TRIANGLES, 0, counter);

		glUniform3fv(glGetUniformLocation(prog.programId(), "material.ambient"), 1, &brass.ambient.x );
		glUniform3fv(glGetUniformLocation(prog.programId(), "material.diffuse"), 1, &brass.diffuse.x );
		glUniform3fv(glGetUniformLocation(prog.programId(), "material.specular"), 1, &brass.specular.x );
		glUniform1f(glGetUniformLocation(prog.programId(), "material.shininess"),brass.shininess);
		counter += pillar.positions.size() * rows*2;
		glDrawArrays( GL_TRIANGLES, 0, counter);


		counter += lamp.positions.size() * lampCount;
		glDrawArrays( GL_TRIANGLES, 0, counter);


		//counter += door.positions.size();
		//glDrawArrays( GL_TRIANGLES, 0, counter);

		//counter += backSwall.positions.size()*backSwallBits;
		//glDrawArrays( GL_TRIANGLES, 0, counter);


		glUniform3fv(glGetUniformLocation(prog.programId(), "material.ambient"), 1, &stone.ambient.x );
		glUniform3fv(glGetUniformLocation(prog.programId(), "material.diffuse"), 1, &stone.diffuse.x );
		glUniform3fv(glGetUniformLocation(prog.programId(), "material.specular"), 1, &stone.specular.x );
		glUniform1f(glGetUniformLocation(prog.programId(), "material.shininess"), stone.shininess);
		

		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ourSaviour);
		counter += picture.positions.size();
		glDrawArrays( GL_TRIANGLES, 0, counter);




		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, NULL);
		counter += backroom.positions.size()*backrooms;
		glDrawArrays( GL_TRIANGLES, 0, counter);

		

		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ourVoid);
		//counter += benches.positions.size()*benchCount;
		//glDrawArrays( GL_TRIANGLES, 0, counter);
		//printf("\nbencSize = %ld", benches.positions.size());
		int bCoun = 0;
		for (int l = 0; l < benchShapes.size(); l++) {
			bCoun += benchShapes[l];
		}
		//printf("\nbCoun = %d", bCoun);


		for (int i = 0; i < benchCount; i++) {
			glUniform3fv(glGetUniformLocation(prog.programId(), "material.ambient"), 1, &cushion.ambient.x );
			glUniform3fv(glGetUniformLocation(prog.programId(), "material.diffuse"), 1, &cushion.diffuse.x );
			glUniform3fv(glGetUniformLocation(prog.programId(), "material.specular"), 1, &cushion.specular.x );
			glUniform1f(glGetUniformLocation(prog.programId(), "material.shininess"), cushion.shininess);
			counter += benchShapes[0];
			glDrawArrays( GL_TRIANGLES, 0, counter);

			glUniform3fv(glGetUniformLocation(prog.programId(), "material.ambient"), 1, &wood.ambient.x );
			glUniform3fv(glGetUniformLocation(prog.programId(), "material.diffuse"), 1, &wood.diffuse.x );
			glUniform3fv(glGetUniformLocation(prog.programId(), "material.specular"), 1, &wood.specular.x );
			glUniform1f(glGetUniformLocation(prog.programId(), "material.shininess"), wood.shininess);
			counter += benchShapes[1];
			glDrawArrays( GL_TRIANGLES, 0, counter);
			counter += benchShapes[2];
			glDrawArrays( GL_TRIANGLES, 0, counter);

			glUniform3fv(glGetUniformLocation(prog.programId(), "material.ambient"), 1, &cushion.ambient.x );
			glUniform3fv(glGetUniformLocation(prog.programId(), "material.diffuse"), 1, &cushion.diffuse.x );
			glUniform3fv(glGetUniformLocation(prog.programId(), "material.specular"), 1, &cushion.specular.x );
			glUniform1f(glGetUniformLocation(prog.programId(), "material.shininess"), cushion.shininess);
			counter += benchShapes[3];
			glDrawArrays( GL_TRIANGLES, 0, counter);


			glUniform3fv(glGetUniformLocation(prog.programId(), "material.ambient"), 1, &wood.ambient.x );
			glUniform3fv(glGetUniformLocation(prog.programId(), "material.diffuse"), 1, &wood.diffuse.x );
			glUniform3fv(glGetUniformLocation(prog.programId(), "material.specular"), 1, &wood.specular.x );
			glUniform1f(glGetUniformLocation(prog.programId(), "material.shininess"), wood.shininess);

			for (int l = 4; l < benchShapes.size()-2; l++) {
				counter += benchShapes[l];
				glDrawArrays( GL_TRIANGLES, 0, counter);
			}
			glUniform3fv(glGetUniformLocation(prog.programId(), "material.ambient"), 1, &wood.ambient.x );
			glUniform3fv(glGetUniformLocation(prog.programId(), "material.diffuse"), 1, &wood.diffuse.x );
			glUniform3fv(glGetUniformLocation(prog.programId(), "material.specular"), 1, &wood.specular.x );
			glUniform1f(glGetUniformLocation(prog.programId(), "material.shininess"), wood.shininess);
			counter += benchShapes[6];
			glDrawArrays( GL_TRIANGLES, 0, counter);
			counter += benchShapes[7];
			glDrawArrays( GL_TRIANGLES, 0, counter);


		}
		
		counter += roof.positions.size() * roofCount;
		glDrawArrays( GL_TRIANGLES, 0, counter);


		glUniform3fv(glGetUniformLocation(prog.programId(), "material.ambient"), 1, &uranium.ambient.x );
		glUniform3fv(glGetUniformLocation(prog.programId(), "material.diffuse"), 1, &uranium.diffuse.x );
		glUniform3fv(glGetUniformLocation(prog.programId(), "material.specular"), 1, &uranium.specular.x );
		glUniform3fv(glGetUniformLocation(prog.programId(), "material.emissive"), 1, &uranium.emissive.x );
		glUniform1f(glGetUniformLocation(prog.programId(), "material.shininess"), uranium.shininess);

		counter += plutonium.positions.size() * plutoniumCount;
		glDrawArrays( GL_TRIANGLES, 0, counter);

		//glDrawElements( GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, NULL);
		//glDrawElements( GL_TRIANGLES, vertexCount, GL_UNSIGNED_BYTE, NULL);
		//glDrawElements( GL_TRIANGLES, vertexCount, GL_UNSIGNED_SHORT, NULL);


		glBindVertexArray( 0 );
		glUseProgram( 0 );

		OGL_CHECKPOINT_DEBUG();

		// Display results
		glfwSwapBuffers( window );
	}

	// Cleanup.
	state.prog = nullptr;
	//TODO: additional cleanup
	
	return 0;
}
catch( std::exception const& eErr )
{
	std::fprintf( stderr, "Top-level Exception (%s):\n", typeid(eErr).name() );
	std::fprintf( stderr, "%s\n", eErr.what() );
	std::fprintf( stderr, "Bye.\n" );
	return 1;
}

/*
char * converter(std::string str) {
	int n = str.length();
	char arr[n + 1]; 
	for (int x = 0; x < sizeof(arr); x++) { 
		arr[x] = str[x]; 
		cout << arr[x]; 
	} 
	return arr;
}
*/

//GLFW key inputs for movement and camera rotation.
namespace
{
	void glfw_callback_error_( int aErrNum, char const* aErrDesc )
	{
		std::fprintf( stderr, "GLFW error: %s (%d)\n", aErrDesc, aErrNum );
	}

	void glfw_callback_key_( GLFWwindow* aWindow, int aKey, int, int aAction, int )
	{
		if( GLFW_KEY_ESCAPE == aKey && GLFW_PRESS == aAction )
		{
			glfwSetWindowShouldClose( aWindow, GLFW_TRUE );
			return;
		}
		if( auto* state = static_cast<State_*>(glfwGetWindowUserPointer( aWindow )) )
		{
			// R-key reloads shaders.
			if( GLFW_KEY_R == aKey && GLFW_PRESS == aAction )
			{
				if( state->prog )
				{
					try
					{
						state->prog->reload();
						std::fprintf( stderr, "Shaders reloaded and recompiled.\n" );
					}
					catch( std::exception const& eErr )
					{
						std::fprintf( stderr, "Error when reloading shader:\n" );
						std::fprintf( stderr, "%s\n", eErr.what() );
						std::fprintf( stderr, "Keeping old shader.\n" );
					}
				}
			}

			// Space toggles camera
			if( GLFW_KEY_SPACE == aKey && GLFW_PRESS == aAction )
			{
				state->camControl.cameraActive = !state->camControl.cameraActive;

				if( state->camControl.cameraActive )
					glfwSetInputMode( aWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN );
				else
					glfwSetInputMode( aWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
			}

			// Camera controls if camera is active
			if (state->camControl.cameraActive)
			{
				//Forward back left and right
				if (GLFW_KEY_W == aKey)
				{
					if (GLFW_PRESS == aAction)
						state->camControl.forward = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.forward = false;
				}
				else if (GLFW_KEY_S == aKey)
				{
					if (GLFW_PRESS == aAction)
						state->camControl.backward = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.backward = false;
				}
				if (GLFW_KEY_A == aKey)
				{
					if (GLFW_PRESS == aAction)
						state->camControl.left = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.left = false;
				}
				else if (GLFW_KEY_D == aKey)
				{
					if (GLFW_PRESS == aAction)
						state->camControl.right = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.right = false;
				}

				//Up and down (EQ)
				if (GLFW_KEY_E == aKey)
				{
					if (GLFW_PRESS == aAction)
						state->camControl.up = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.up = false;
				}
				else if (GLFW_KEY_Q == aKey)
				{
					if (GLFW_PRESS == aAction)
						state->camControl.down = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.down = false;
				}
				
				//Speed modifiers Lshift and Lctrl
				if (GLFW_KEY_LEFT_SHIFT == aKey)
				{
					if (GLFW_PRESS == aAction)
						state->camControl.actionSpeedUp= true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.actionSpeedUp = false;
				}
				else if (GLFW_KEY_LEFT_CONTROL == aKey)
				{
					if (GLFW_PRESS == aAction)
						state->camControl.actionSlowDown = true;
					else if (GLFW_RELEASE == aAction)
						state->camControl.actionSlowDown = false;
				}
			}

			// If camera is not active then stop movement
			else
			{
				state->camControl.forward = false;
				state->camControl.backward = false;
				state->camControl.left = false;
				state->camControl.right = false;
				state->camControl.actionSpeedUp = false;
				state->camControl.actionSlowDown = false;
			}
		}
	}

	//Camera rotation function.
	void glfw_callback_motion_(GLFWwindow* aWindow, double aX, double aY)
	{
		if (auto* state = static_cast<State_*>(glfwGetWindowUserPointer(aWindow)))
		{
			if (state->camControl.cameraActive)
			{
				auto const dx = float(aX - state->camControl.lastX);
				auto const dy = float(aY - state->camControl.lastY);


				//Yaw
				state->camControl.phi += dx * kMouseSensitivity_;
				//Pitch
				state->camControl.theta += dy * kMouseSensitivity_;
				if (state->camControl.theta > kPi_ / 2.f)
					state->camControl.theta = kPi_ / 2.f;
				else if (state->camControl.theta < -kPi_ / 2.f)
					state->camControl.theta = -kPi_ / 2.f;

				//Update the camera direction based on yaw and pitch.
				Vec3f front = { -(sinf(state->camControl.phi) * cosf(state->camControl.theta)),
					sinf(state->camControl.theta),
					cosf(state->camControl.phi)* cosf(state->camControl.theta)
				};
				//Normalize direction.
				state->camControl.cameraFront = normalize(front);
			}

			state->camControl.lastX = float(aX);
			state->camControl.lastY = float(aY);
		}
	}
	unsigned int loadTexture(char const * path) {
	
		unsigned int textureID;
		glGenTextures(1, &textureID);

		int width, height, nrComponents;
		unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
		GLenum format = 0;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);

		return textureID;
	}
	/*
	void setMaterial(Material material, ShaderProgram prog) {
		glUniform3fv(glGetUniformLocation(prog.programId(), "material.ambient"), 1, &material.ambient.x );
		glUniform3fv(glGetUniformLocation(prog.programId(), "material.diffuse"), 1, &material.diffuse.x );
		glUniform3fv(glGetUniformLocation(prog.programId(), "material.specular"), 1, &material.specular.x );
		glUniform1f(glGetUniformLocation(prog.programId(), "material.shininess"), material.shininess);
	}*/
}


namespace
{
	GLFWCleanupHelper::~GLFWCleanupHelper()
	{
		glfwTerminate();
	}

	GLFWWindowDeleter::~GLFWWindowDeleter()
	{
		if( window )
			glfwDestroyWindow( window );
	}
}
