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
#include "jellyfish.hpp"
//#include "loadcustom.hpp"
#include "loadTexture.hpp"
#include "loadobj.hpp"
#include "materials.hpp"
#include <string>
#include <cstring>
#include <stb_image.h>
//#include <stb_image/stb_image.h>
#include <map>



namespace
{
	constexpr char const* kWindowTitle = "COMP3811 - Coursework 2";
	
	constexpr float kPi_ = 3.1415926f;

	//Camera state and constant values.
	constexpr float kMovementPerSecond_ = 10.f; // units per second
	constexpr float kMovementModNeg_ = 2.5f;
	constexpr float kMovementModPos_ = 5.f;
	constexpr float kMouseSensitivity_ = 0.01f; // radians per pixel

	//Animation speed modifiers
	constexpr float kAniModPos_ = 2.f;
	constexpr float kAniModNeg_ = 0.5f;

	struct State_
	{
		ShaderProgram* prog;
		ShaderProgram* moveProg;
		ShaderProgram* billProg;

		struct CamCtrl_
		{
			bool cameraActive;
			bool forward, backward, left, right, up, down;
			bool actionSpeedUp, actionSlowDown;

			float phi, theta;

			float lastX, lastY;
			// Camera position, direction faced and up vector.
			Vec3f cameraPos = { 0.f, -3.f, 10.f };
			Vec3f cameraFront = {0.f, 0.f, 1.f};
			Vec3f cameraUp = { 0.f, 1.f, 0.f };

		} camControl;
		int enterPressed = 1;

		//Animation control struct.
		struct AnimCtrl_
		{
			bool pause, speedUp, slowDown;
		}aniControl;
	};

	//Point light struct
	struct PointLight {
		Vec3f position;
		float constant;
		float linear;
		float quadratic;
		Vec3f ambient;
		Vec3f diffuse;
		Vec3f specular;
	};
	struct SpotLight {
		Vec3f position;
		Vec3f direction;
		float phi;
		
		float constant;
		float linear;
		float quadratic;
		
		Vec3f ambient;
		Vec3f diffuse;
		Vec3f specular;
	};


	void glfw_callback_error_( int, char const* );
	void glfw_callback_key_( GLFWwindow*, int, int, int, int );
	void glfw_callback_motion_(GLFWwindow*, double, double);
	void movementControl(State_ &state, float dt);
	
	float doorControl(State_& state, float angle, std::chrono::steady_clock::time_point &last, bool& increase);
	
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
	//glEnable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );
	//glDepthMask(GL_TRUE);
	glEnable(GL_BLEND);
	glDepthFunc(GL_LESS);
	glDepthRange(-1.0f, 1.0f);
	glClearColor( 0.f, 0.f, 0.1f, 1.0f );
	glEnable( GL_FRAMEBUFFER_SRGB );
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  


	OGL_CHECKPOINT_ALWAYS();

	// Get actual framebuffer size.
	// This can be different from the window size, as standard window
	// decorations (title bar, borders, ...) may be included in the window size
	// but not be part of the drawable surface area.
	int iwidth, iheight;
	glfwGetFramebufferSize( window, &iwidth, &iheight );

	glViewport( 0, 0, iwidth, iheight );

	// Load shader program
	ShaderProgram moveProg({
		{ GL_VERTEX_SHADER, "assets/movement.vert" },
		{ GL_FRAGMENT_SHADER, "assets/movement.frag" }
		});
	state.moveProg = &moveProg;

	ShaderProgram prog({
		{ GL_VERTEX_SHADER, "assets/default.vert" },
		{ GL_FRAGMENT_SHADER, "assets/default.frag" }
		});
	state.prog = &prog;

	ShaderProgram billProg({
		{ GL_VERTEX_SHADER, "assets/billboard.vert" },
		{ GL_FRAGMENT_SHADER, "assets/billboard.frag" }
		});
	state.billProg = &billProg;

	// Animation state
	auto last = Clock::now();
	float angle = 0.f;
	float headAngle = 0.f;
	float legAngle = 0.f;
	float movementAngle = 0.f;

	auto aniStop = Clock::now();
	float doorAngle = 0.f;
	bool increase = true;

	// Other initialization & loading
	OGL_CHECKPOINT_ALWAYS();

	unsigned int ourSaviour = loadTexture("assets/markus.png");
	unsigned int ourCross = loadTexture("assets/Cross.png");
	unsigned int eyeball = loadTexture("assets/eyeBall.png");
	unsigned int clayN = loadTexture("assets/woolly-mammoth-skeleton-obj/ClayNormal.jpg");
	unsigned int clayC = loadTexture("assets/woolly-mammoth-skeleton-obj/ClayColor.jpg");
	
	
	
	std::vector<SimpleMeshData> chapel;
	std::vector<SimpleMeshData> transparent;
	std::vector<Vec3f> transLocs;
	std::vector<int> transPos;
	transPos.emplace_back(0);

	auto floor = make_cube( Vec3f{0.07f, 0.07f, 0.07f}, make_scaling( 200.f, 2.f, 200.f ) * make_translation( { 0.f, -1.f, 0.f }));
	chapel.emplace_back(floor);

	//auto frontWall = make_cube( Vec3f{1, 1, 1}, make_scaling( 5.f, 10.f, 0.5f ) * make_translation( { 3.5f, 5.f, 2.25f }));
	auto frontWall = make_cube( Vec3f{.02f, .02f, .02f}, make_scaling( 13.f, 15.f, 1.f )); //so 10 meter
	int wallBits = 2;
	for (float i = 0; i < wallBits; i++)
		chapel.emplace_back(make_change(frontWall, make_translation( { 17.f - 34.f*i, 15.f, 5.f }) ));


	//auto sideWall = make_cube( Vec3f{.02f, .02f, .02f}, make_scaling( 1.f, 15.f, 24.f ));//so 48 meter
	auto sideWall = make_cube( Vec3f{.02f, .02f, .02f}, make_scaling( 1.f, 5.f, 24.f ));//so 48 meter
	auto sideMidWall = make_cube( Vec3f{.02f, .02f, .02f}, make_scaling( 1.f, 5.f, 5.f ));//so 48 meter
	int sideWallBits = 10;
	for (float i = 0; i < 2; i++) {
		for (float l = 0; l < 2; l++)
			chapel.emplace_back(make_change(sideWall, make_translation( { 29.f - 58.f*i, 5.f + 20.f*l, 30.f }) ));
		for (float l = 0; l < 3; l++)
			chapel.emplace_back(make_change(sideMidWall, make_translation( { 29.f - 58.f*i, 15.f, 11.f + l * 19.f }) ));
	}


	auto pillar = make_cylinder( true, 18, {.1f, 0, .1f}, make_scaling( .75f, 20.f, .75f ) * make_rotation_z( 3.141592f / 2.f ));

	auto lamp = make_cylinder( true, 18, {.1f, .1f, .1f}, make_scaling( .4f, 2.f, .4f ) * make_rotation_z( 3.141592f / 2.f ));
	std::vector<PointLight> PointLightData;
	int rows = 3;
	for (float i = 0; i < rows; i++)
		for (float l = 0; l < 2; l++) {
			chapel.emplace_back(make_change(pillar, make_translation( { 5.f - 10.f*l, 0.f, 15.f + 15.f * i }) ));
			//chapel.emplace_back(make_cylinder( true, 18, Vec3f{1, 0,0 1}, make_translation( { 22.f - 44.f*i, 0.f, 29.f })));
		
			PointLight p1;
			p1.position = Vec3f{ 3.f - 6.f*l, 11.f, 15.f + 15.f * i };
			p1.constant = 1.f;
			p1.linear = 0.1f;
			p1.quadratic = 0.016f;
			p1.ambient = Vec3f{ .5f, .15f, .05f };
			p1.diffuse = Vec3f{ .5f, .4f, .05f };
			p1.specular = Vec3f{ .5f, .4f, .05f };
			PointLightData.emplace_back(p1);
			transparent.emplace_back(make_change(lamp, make_translation( { 3.5f - 7.f*l, 10.f, 15.f + 15.f * i }) ));
			transLocs.emplace_back(Vec3f{ 3.f - 6.f*l, 2.f, 15.f + 15.f * i });
			transPos.emplace_back(transPos[transPos.size()-1] + lamp.positions.size());
		}

	std::vector<SpotLight> spotLightData;
	SpotLight p1;
	p1.position = Vec3f{ 0.f, 5.f, 86.6f };
	p1.direction = Vec3f{ 1.f, 1.f, 0.f };
	p1.phi = 0.94f;
	p1.constant = .5f;
	p1.linear = 0.00003f;
	p1.quadratic = 0.000004f;
	p1.ambient = Vec3f{ -.5f, -.5f, .0f };
	p1.diffuse = Vec3f{ -.9f, -.7f, -.05f };
	p1.specular = Vec3f{ -.5f, -.4f, -.05f };
	spotLightData.emplace_back(p1);

	//4 Windows
	//auto stainWindow = make_cube( Vec3f{0.05f, 0.05f, 0.05f}, make_rotation_y( 3.141592f) * make_scaling( .1f, 5.f, 4.5f ));//so 48 meter
	auto stainWindow = make_frame( Vec3f{0.05f, 0.05f, 0.05f}, make_rotation_y( 3.141592f/2) * make_scaling( 5.f, 5.f, 4.5f ));//so 48 meter
	for (float i = 0; i < 2; i++)
		for (float l = 0; l < 2; l++) {
			Vec3f thisLoc = Vec3f{ 29.f - 58.f*l, 15.f, 20.5f + 19.f * i };
			transparent.emplace_back(make_change(stainWindow, make_translation( thisLoc) ));
			transLocs.emplace_back(thisLoc);
			transPos.emplace_back(transPos[transPos.size()-1] + stainWindow.positions.size());
		}



	auto picture = make_cube( Vec3f{0.05f, 0.05f, 0.05f}, make_rotation_z( -3.141592f / 2.f ) * make_rotation_y( 3.141592f / 2.f ) * make_scaling( .1f, 6.f, 7.62f ));//so 48 meter
	chapel.emplace_back(make_change(picture, make_translation( { 0.f, 9.f, 108.8f }) ));

	auto backroom = make_partial_cylinder( false, 6, 1, make_rotation_y( -3.141592f / 3.f )* make_rotation_z( 3.141592f / 2.f ));
	chapel.emplace_back(make_change(backroom, make_translation( { 0.f, 0.f, 86.8f })*  make_scaling( 54.f, 40.f, 40.f ) ));
	//chapel.emplace_back(backroom);
	int backrooms = 1;
	auto backRoof = make_cone(6, make_rotation_y( -3.141592f / 3.f )* make_rotation_z( 3.141592f / 2.f ));
	chapel.emplace_back(make_change(backRoof, make_translation( { 0.f, 40.f, 86.8f })*  make_scaling( 54.f, 40.f, 40.f ) ));
	//chapel.emplace_back(backroom);


	std::vector<SimpleMeshData> parts;
	auto benches = load_wavefront_obj( "assets/chair-y-good-obj/chair-y-good.obj" );
	auto benchShapes = getDimensions( "assets/chair-y-good-obj/chair-y-good.obj" );
	benches = make_change(benches, make_scaling( 5.f, 1.f, 1.f ) );
	benches = make_change(benches, make_rotation_y( 3.141592f ) );
	benches = make_change(benches, make_translation( {-8.f, 0.f, 15.f }) );
	int benchCount = 12;
	////printf("benches ", benches.positions.size());
	for (float i = 0; i < 6; i++)
		for (float l = 0; l < 2; l++)
			chapel.emplace_back(make_change(benches, make_translation( {29.f * l, 0.f, i*6.f }) ));

	

	auto roof = make_partial_cylinder( true, 124, 62,  make_scaling( 1.f, 1.f, 1.f ));
	roof = make_change(roof, make_rotation_x( -3.141592f / 2.f )* make_rotation_z( 3.141592f / 2.f ) );
	roof = make_change(roof, make_scaling( 28.f, 20.f, 50.f ) );
	roof = make_change(roof, make_translation( {0.f, 30.f, 55.f }) );
	chapel.emplace_back(roof);
	int roofCount = 1;

	auto plutonium = make_cube( Vec3f{0.05f, 0.05f, 0.05f}, make_scaling( 2.f, 2.f, 2.f ) * make_translation( {0.f, 8.f, 0.f }));//so 48 meter
	chapel.emplace_back(plutonium);
	int plutoniumCount = 1;


	//auto shiny = make_sphere( 46, make_scaling( 2.f, 2.f, 2.f ));//so 48 meter
	auto shiny = make_cylinder( true, 16, Vec3f{0.05f, 0.05f, 0.05f}, make_scaling( 2.f, 2.f, 2.f ));//so 48 meter
	chapel.emplace_back(make_change(shiny, make_translation( {0.f, 22.f, 0.f }) ));
	int shinyCount = 1;


	auto diffuseObject = make_cube( Vec3f{0.05f, 0.05f, 0.05f}, make_scaling( 2.f, 2.f, 2.f ));//so 48 meter
	chapel.emplace_back(make_change(diffuseObject, make_translation( {0.f, 28.f, 0.f }) ));
	int diffuseCount = 1;


	auto aquarium = make_cylinder( true, 18, {.5f, 0.5f, .5f}, make_scaling( 3.f, 10.f, 3.f ) * make_rotation_z( 3.141592f / 2.f ));
	chapel.emplace_back(make_change(aquarium, make_translation( {-25.f, 5.f, 86.f }) ));
	int aquariumCount = 1;


	auto mammoth = load_wavefront_obj( "assets/woolly-mammoth-skeleton-obj/woolly-mammoth-skeleton.obj" );
	auto mammothShapes = getDimensions( "assets/woolly-mammoth-skeleton-obj/woolly-mammoth-skeleton.obj" );
	//printf("mammothShapes = %ld\n", mammothShapes.size());
	mammoth = make_change(mammoth, make_scaling( 0.007f, 0.007f, 0.007f ));
	float lowest = 5.f;
	for (int i = 0; i < (int)mammoth.positions.size(); i++)
		if (lowest > mammoth.positions[i].y)
			lowest = mammoth.positions[i].y;
	mammoth = (make_change(mammoth,  make_rotation_y( -3.141592f / 2.f )));
	chapel.emplace_back(make_change(mammoth, make_translation( {25.f, -lowest-.01f, 84.f })));

	auto door = make_door(Vec3f{ 0.05f, 0.05f, 0.05f }, make_scaling(8.f, 10.f, 0.5f));
	chapel.emplace_back(make_change(door, make_translation( {-4.f, 0.f, 5.f} )));
	int doorCount = 1;

	//printf("Chapel size=%ld", chapel.size());

	float sunXloc = -1.f;
	GLuint vao = create_vaoM(&chapel[0], chapel.size());
	GLuint transparentVao = create_vaoM(&transparent[0], transparent.size());


	std::vector<SimpleMeshData> movingObjects;
	auto jelly = make_jellyfish( 16, make_translation( Vec3f{ 0.f, 10.f, -20.f} ));//so 48 meter
	movingObjects.emplace_back(jelly.data);
	GLuint movingVao = create_vaoM(&movingObjects[0], 1);


	auto sprite = make_frame( Vec3f{0.05f, 0.05f, 0.05f}, make_rotation_y( 3.141592f/2));//so 48 meter
	//printf("sprite=%f", getMean(sprite));
	std::vector<SimpleMeshData> congregants;
	std::vector<Vec3f> centers;
	//congregants.emplace_back(sprite);
	congregants.emplace_back(sprite);
	centers.emplace_back(getMean(sprite));
	for (float i = 0; i < 12; i++) {
		auto sp = make_frame( Vec3f{0.05f, 0.05f, 0.05f}, make_rotation_y( 3.141592f/2));//so 48 meter
		sp = make_change(sp, make_translation( {29.f * ((int)i%2) -8.f, 6.f, i*3.f - 12.f} ));
		//printf("centers=%f", getMean(sp).x);

		congregants.emplace_back(sp);
		centers.emplace_back(getMean(sp));
		////printf("centers=%f", centers[i]);
	}

	GLuint congregantsVao = create_vaoM(&congregants[0], 12);


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
		auto const now = Clock::now();
		float dt = std::chrono::duration_cast<Secondsf>(now - last).count();
		last = now;
		angle += dt * kPi_ * 0.3f;
		if (angle >= 2.f * kPi_)
			angle -= 2.f * kPi_;

		headAngle += dt * kPi_ * 0.15f;
		if (headAngle >= kPi_)
			headAngle -= kPi_;

		legAngle += dt * kPi_ * 0.05f;
		if (legAngle >= kPi_/3.f)
			legAngle -= kPi_/3.f;


		movementAngle += dt * kPi_ * 0.05f;
		if (movementAngle >= 2*kPi_)
			movementAngle -= 2*kPi_;


		spotLightData[0].direction = Vec3f{ .5f + 2*movementAngle/(kPi_) , 1.f, 0.f };

 
		
		movementControl(state, dt);

		//TODO: define and compute projCameraWorld matrix
		Mat44f model2world = kIdentity44f;
		Mat44f Rx = make_rotation_x( state.camControl.theta );
		Mat44f Ry = make_rotation_y( state.camControl.phi );
		Mat44f world2camera = make_translation(state.camControl.cameraPos);


		Mat44f projection = make_perspective_projection(
			60.f * 3.1415926f / 180.f,
			fbwidth/float(fbheight),
			0.1f, 200.0f
		);
		projection = projection*Rx*Ry;


		Mat44f projCameraWorld = projection * world2camera * model2world;
		Mat33f normalMatrix = mat44_to_mat33( transpose(invert(model2world)) );


		sunXloc += .01f;
		if (sunXloc > .99f)
			sunXloc = -1.f;
		Vec3f lightDir = normalize( Vec3f{ 0.f, 0.5f, -1.f } );


		//Animation stuff
		Vec3f noHinge = Vec3f{0.f, 0.f, 0.f };

		

		
		std::map<float, int> sorted;
		Vec3f camPosi = Vec3f {state.camControl.cameraPos.x, state.camControl.cameraPos.y, state.camControl.cameraPos.z };
		////printf("campos: (%f, %f, %f)\n", state.camControl.cameraPos.x, state.camControl.cameraPos.y, state.camControl.cameraPos.z);
		////printf("campos: (%f, %f, %f)\n", transLocs[0].x, transLocs[0].y, transLocs[0].z);
		
        for (unsigned int i = 0; i < transLocs.size(); i++) {
            float distance = length(camPosi + transLocs[i]);
			if (sorted.find(distance) != sorted.end()) //if two values that are the same exist
				distance += 0.00001f;
            sorted[distance] = i;
        }
		


		


	
		// Draw scene
		OGL_CHECKPOINT_DEBUG();

		//Draw frame
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		glUseProgram( prog.programId() );

		glUniform1f(glGetUniformLocation(prog.programId(), "controls.animating"), 0 );
		glUniformMatrix4fv(glGetUniformLocation(prog.programId(), "rotation"),1, GL_TRUE, kIdentity44f.v);
		glUniformMatrix4fv(glGetUniformLocation(prog.programId(), "rotateMat"),1, GL_TRUE, kIdentity44f.v);
		glUniformMatrix4fv(glGetUniformLocation(prog.programId(), "scaleMat"),1, GL_TRUE, kIdentity44f.v);


		glUniform1i(glGetUniformLocation(prog.programId(), "texture.diffuse"), 1); //lightDir uLightDir
		glUniform1i(glGetUniformLocation(prog.programId(), "texture.specular"), 1); //lightDir uLightDir


		glUniform3fv(glGetUniformLocation(prog.programId(), "uLightDir"), 1, &lightDir.x ); //lightDir uLightDir
		glUniform3f( glGetUniformLocation(prog.programId(), "uLightDiffuse"), .2f, .2f, .2f ); //lightDiffuse
		glUniform3f( glGetUniformLocation(prog.programId(), "uSceneAmbient"), 0.1f, 0.1f, 0.1f ); //uSceneAmbient
		glUniform3f( glGetUniformLocation(prog.programId(), "uSpecular"), .5f, .4f, .05f ); //uSceneAmbient
		////printf("normalMatrix: (%f, %f, %f)\n", state.camControl.cameraPos.x, state.camControl.cameraPos.y, state.camControl.cameraPos.z);
		glUniform3f(glGetUniformLocation(prog.programId(), "viewPos"), state.camControl.cameraPos.x, state.camControl.cameraPos.y, state.camControl.cameraPos.z ); //uSceneAmbient

		//glUniform3f( glGetUniformLocation(prog.programId(), "rotateDoor"), make_rotation_x( 0 ) );


		
		//lolo += 1;
		//if (lolo >= 30)
		//	lolo -= 30;
		//spotLightData[0].phi = lolo;
		for (int i = 0; i < 1; i++) {
			std::string number = std::to_string(i);
			glUniform3fv(glGetUniformLocation(prog.programId(), ("spotLights["+number+"].position").c_str()), 1, &spotLightData[i].position.x ); //lightDir uLightDir
			glUniform3fv(glGetUniformLocation(prog.programId(), ("spotLights["+number+"].direction").c_str()), 1, &spotLightData[i].direction.x ); //lightDir uLightDir
			glUniform1f(glGetUniformLocation(prog.programId(), ("spotLights["+number+"].phi").c_str()), spotLightData[i].phi ); //lightDir uLightDir
			glUniform1f(glGetUniformLocation(prog.programId(), ("spotLights["+number+"].constant").c_str()), spotLightData[i].constant ); //lightDir uLightDir
			glUniform1f(glGetUniformLocation(prog.programId(), ("spotLights["+number+"].linear").c_str()), spotLightData[i].linear ); //lightDir uLightDir
			glUniform1f(glGetUniformLocation(prog.programId(), ("spotLights["+number+"].quadratic").c_str()), spotLightData[i].quadratic ); //lightDir uLightDir
			glUniform3fv(glGetUniformLocation(prog.programId(), ("spotLights["+number+"].ambient").c_str()), 1, &spotLightData[i].ambient.x ); //lightDir uLightDir
			glUniform3fv(glGetUniformLocation(prog.programId(), ("spotLights["+number+"].diffuse").c_str()), 1, &spotLightData[i].diffuse.x ); //lightDir uLightDir
			glUniform3fv(glGetUniformLocation(prog.programId(), ("spotLights["+number+"].specular").c_str()), 1, &spotLightData[i].specular.x ); //lightDir uLightDir
		}

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
		glUniformMatrix4fv(glGetUniformLocation(prog.programId(), "rotation"),1, GL_TRUE, make_rotation_x( 0 ).v);
		glUniform3fv(glGetUniformLocation(prog.programId(), "point"),1, &noHinge.x);


		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		
		glBindVertexArray( vao );
		glUniform1f(glGetUniformLocation(prog.programId(), "material.opacity"), 1);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);

		setMaterial(hardStone, state.prog);
		int counter = 0;
		glDrawArrays( GL_TRIANGLES, counter, floor.positions.size());
		counter += floor.positions.size();


		glDrawArrays( GL_TRIANGLES, counter, frontWall.positions.size() * wallBits);
		counter += frontWall.positions.size() * wallBits;

		glDrawArrays( GL_TRIANGLES, counter, sideWall.positions.size() * sideWallBits);
		counter += sideWall.positions.size() * sideWallBits;

		setMaterial(brass, state.prog);
		glDrawArrays( GL_TRIANGLES, counter, pillar.positions.size() * rows * 2);
		counter += pillar.positions.size() * rows * 2;


		
		setMaterial(stone, state.prog);
		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ourSaviour);
		glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, ourCross);
		glDrawArrays( GL_TRIANGLES, counter, picture.positions.size());
		counter += picture.positions.size();




		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
		glDrawArrays( GL_TRIANGLES, counter, backroom.positions.size() * backrooms);
		counter += backroom.positions.size()*backrooms;


		glDrawArrays( GL_TRIANGLES, counter, backRoof.positions.size());
		counter += backRoof.positions.size();
		

		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
		//counter += benches.positions.size()*benchCount;
		//glDrawArrays( GL_TRIANGLES, 0, counter);
		////printf("\nbencSize = %ld", benches.positions.size());
		int bCoun = 0;
		for (int l = 0; l < (int)benchShapes.size(); l++) {
			bCoun += benchShapes[l];
		}
		////printf("\nbCoun = %d", bCoun);


		for (int i = 0; i < benchCount; i++) {
			setMaterial(cushion, state.prog);
			glDrawArrays( GL_TRIANGLES, counter, benchShapes[0]);
			counter += benchShapes[0];


			setMaterial(wood, state.prog);
			glDrawArrays( GL_TRIANGLES, counter, benchShapes[1]);
			counter += benchShapes[1];
			glDrawArrays( GL_TRIANGLES, counter, benchShapes[2]);
			counter += benchShapes[2];

			setMaterial(cushion, state.prog);
			glDrawArrays( GL_TRIANGLES, counter, benchShapes[3]);
			counter += benchShapes[3];


			setMaterial(wood, state.prog);
			for (int l = 4; l < (int)benchShapes.size(); l++) {
				glDrawArrays( GL_TRIANGLES, counter, benchShapes[l]);
				counter += benchShapes[l];
			}
		}
		
		glDrawArrays( GL_TRIANGLES, counter, roof.positions.size()* roofCount);
		counter += roof.positions.size() * roofCount;

		setMaterial(uranium, state.prog);
		glDrawArrays( GL_TRIANGLES, counter, plutonium.positions.size()* plutoniumCount);
		counter += plutonium.positions.size() * plutoniumCount;


		setMaterial(shinyShiny, state.prog);
		glDrawArrays( GL_TRIANGLES, counter, shiny.positions.size()* shinyCount);
		counter += shiny.positions.size() * shinyCount;


		setMaterial(highDiffuse, state.prog);
		glDrawArrays( GL_TRIANGLES, counter, diffuseObject.positions.size() * diffuseCount);
		counter += diffuseObject.positions.size() * diffuseCount;




		setMaterial(brass, state.prog);
		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ourSaviour);
		glDrawArrays( GL_TRIANGLES, counter, aquarium.positions.size() * aquariumCount);
		counter += aquarium.positions.size() * aquariumCount;


		setMaterial(pureWhite, state.prog);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, clayN);

		glDrawArrays( GL_TRIANGLES, counter, mammothShapes[0]);
		counter += mammothShapes[0];

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, clayC);
		glDrawArrays( GL_TRIANGLES, counter, mammothShapes[1]);
		counter += mammothShapes[1];


		setMaterial(wood, state.prog);
		//Calculates the angle of rotation for the door
		doorAngle = doorControl(state, doorAngle, aniStop, increase);
		model2world = make_translation({ -4.f, 0.f, 5.f }) * make_rotation_y(doorAngle) * make_translation({ 4.f, 0.f, -5.f });
		projCameraWorld = projCameraWorld * model2world;
		glUniformMatrix4fv(glGetUniformLocation(prog.programId(), "uProjCameraWorld"), 1, GL_TRUE, projCameraWorld.v);
		glUniformMatrix4fv(glGetUniformLocation(prog.programId(), "rotateMat"), 1, GL_TRUE, make_rotation_y(doorAngle).v);
		glDrawArrays(GL_TRIANGLES, counter, door.positions.size());
		counter += door.positions.size() * doorCount;

		model2world = kIdentity44f;
		projCameraWorld = projection * world2camera * model2world;
		glUniformMatrix4fv(glGetUniformLocation(prog.programId(), "uProjCameraWorld"), 1, GL_TRUE, projCameraWorld.v);
		glUniformMatrix4fv(glGetUniformLocation(prog.programId(), "rotateMat"), 1, GL_TRUE, kIdentity44f.v);





		//Second shader for the jellyfish
		glUseProgram( 0 );
		glUseProgram( moveProg.programId() );

		glUniform3fv(glGetUniformLocation(moveProg.programId(), "uLightDir"), 1, &lightDir.x ); //lightDir uLightDir
		glUniform3f( glGetUniformLocation(moveProg.programId(), "uLightDiffuse"), .6f, .6f, .6f ); //lightDiffuse
		glUniform3f( glGetUniformLocation(moveProg.programId(), "uSceneAmbient"), 0.1f, 0.1f, 0.1f ); //uSceneAmbient
		glUniform3f( glGetUniformLocation(moveProg.programId(), "uSpecular"), 0.5f, 0.5f, 0.5f ); //uSceneAmbient
		glUniform3f(glGetUniformLocation(moveProg.programId(), "viewPos"), state.camControl.cameraPos.x, state.camControl.cameraPos.y, state.camControl.cameraPos.z ); //uSceneAmbient

		glUniformMatrix4fv(glGetUniformLocation(moveProg.programId(), "uProjCameraWorld"), 1, GL_TRUE, projCameraWorld.v );
		glUniformMatrix3fv(glGetUniformLocation(moveProg.programId(), "uNormalMatrix"),1, GL_TRUE, normalMatrix.v);


		glUniformMatrix4fv(glGetUniformLocation(moveProg.programId(), "rotation"),1, GL_TRUE, kIdentity44f.v);

		setMaterial(brass, state.moveProg);
		glUniform1f(glGetUniformLocation(moveProg.programId(), "material.opacity"), 1);


		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray( movingVao );
		int currentMLoc = 0;

		glUniformMatrix4fv(glGetUniformLocation(moveProg.programId(), "rotateMat"),1, GL_TRUE, make_rotation_y(movementAngle).v);
		Vec3f chapelMid = Vec3f{0.f, 0.f, 65.f };
		glUniform3fv(glGetUniformLocation(moveProg.programId(), "point"),1, &chapelMid.x);
		glUniform1f(glGetUniformLocation(moveProg.programId(), "material.opacity"), 1);
		glUniformMatrix4fv(glGetUniformLocation(moveProg.programId(), "scaleMat"),1, GL_TRUE, kIdentity44f.v);
		glUniformMatrix4fv(glGetUniformLocation(moveProg.programId(), "rotationLegX"),1, GL_TRUE, kIdentity44f.v);
		glUniformMatrix4fv(glGetUniformLocation(moveProg.programId(), "rotationLegZ"),1, GL_TRUE, kIdentity44f.v);

		Vec3f bounce = Vec3f{0.f, 6.f *(float)sin(headAngle) + 7.f, 0.f };
		glUniform3fv(glGetUniformLocation(moveProg.programId(), "translateV"),1, &bounce.x);


		glUniform3fv(glGetUniformLocation(moveProg.programId(), "centre"),1, &jelly.centreData.head.x);
		////printf("projection: (%f, %f, %f)\n", jelly.centreData.head.x, jelly.centreData.head.y, jelly.centreData.head.z);


		//head
		glUniformMatrix4fv(glGetUniformLocation(moveProg.programId(), "scaleMat"),1, GL_TRUE, make_scaling((sin(headAngle)+1), (sin(headAngle)+1), (sin(headAngle)+1)).v);
		glDrawArrays( GL_TRIANGLES, currentMLoc, jelly.centreData.headCount);
		currentMLoc += jelly.centreData.headCount;
		glUniformMatrix4fv(glGetUniformLocation(moveProg.programId(), "scaleMat"),1, GL_TRUE, kIdentity44f.v);

		//glDrawArrays( GL_TRIANGLES, currentMLoc, jelly.centreData.bodyCount);
		currentMLoc += jelly.centreData.bodyCount;


		for (int i = 0; i < jelly.centreData.legNo; i++) {
		//for (int i = 0; i < 1; i++) {
			//float dzdx = (jelly.centreData.legs[i].z - jelly.centreData.head.z) / (jelly.centreData.legs[i].x - jelly.centreData.head.x);
			//float lX = legAngle * (jelly.centreData.legs[i].x - jelly.centreData.head.x)/6;
			//float lZ = legAngle * (jelly.centreData.legs[i].z - jelly.centreData.head.z)/6;
			//Vec3f currentPointBy = jelly.centreData.legTop[i];
			glUniform3fv(glGetUniformLocation(moveProg.programId(), "legCentre"),1, &jelly.centreData.legs[i].x);
			//glUniform3fv(glGetUniformLocation(moveProg.programId(), "point"),1, &jelly.centreData.legTop[i].x);
			for (int l = 0; l < 6; l++) {
				glUniform1f(glGetUniformLocation(moveProg.programId(), "legSeg"), l);
				//glUniformMatrix4fv(glGetUniformLocation(moveProg.programId(), "rotationLegX"),1, GL_TRUE, make_rotation_z(lX).v);
				//glUniformMatrix4fv(glGetUniformLocation(moveProg.programId(), "rotationLegZ"),1, GL_TRUE, make_rotation_x(-lZ).v);
				glDrawArrays( GL_TRIANGLES, currentMLoc, jelly.centreData.segment);
				currentMLoc += jelly.centreData.segment;
			}
			//currentMLoc += jelly.centreData.legCount;
		}
		glUniformMatrix4fv(glGetUniformLocation(moveProg.programId(), "rotationLegX"),1, GL_TRUE, kIdentity44f.v);
		glUniformMatrix4fv(glGetUniformLocation(moveProg.programId(), "rotationLegZ"),1, GL_TRUE, kIdentity44f.v);



		Vec3f blankTranslation = Vec3f{0.f, 0.f, 0.f };
		glUniform3fv(glGetUniformLocation(moveProg.programId(), "translateV"),1, &(blankTranslation).x);


		glUniformMatrix4fv(glGetUniformLocation(moveProg.programId(), "rotation"),1, GL_TRUE, kIdentity44f.v);
		glUniformMatrix4fv(glGetUniformLocation(moveProg.programId(), "rotateMat"),1, GL_TRUE, kIdentity44f.v);
		glBindVertexArray( 0 );


		glUseProgram( 0 );

		glUseProgram( billProg.programId() );
		glBindVertexArray( congregantsVao );
		glUniformMatrix4fv(glGetUniformLocation(billProg.programId(), "uProjCameraWorld"), 1, GL_TRUE, projCameraWorld.v );
		glUniform1i(glGetUniformLocation(billProg.programId(), "sprite"), 1); //lightDir uLightDir
		glUniform3fv(glGetUniformLocation(billProg.programId(), "viewPos"),1,  &state.camControl.cameraPos.x ); //uSceneAmbient

		int spSize = sprite.positions.size();
		for (int i = 0; i < 1; i++) {

			glActiveTexture(GL_TEXTURE1);
        	glBindTexture(GL_TEXTURE_2D, eyeball);
			
			glUniform3fv(glGetUniformLocation(billProg.programId(), "center"),1,  &centers[i].x); //uSceneAmbient
			glDrawArrays( GL_TRIANGLES, spSize*i, spSize);
			//printf("RCS=%f,%f,%f\n", centers[i].x, centers[i].y, centers[i].z);
			////printf("state.camControl.cameraPos=%f,%f,%f\n", state.camControl.cameraPos.x, state.camControl.cameraPos.y, state.camControl.cameraPos.z);
		}

		glBindVertexArray( 0 );



		glBindVertexArray( transparentVao );
		glUseProgram( 0 );
		glUseProgram( prog.programId() );


		for (std::map<float, int>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
			if (transparent[it->second].positions.size() == lamp.positions.size()) {
				setMaterial(lampGlass, state.prog);
				glActiveTexture(GL_TEXTURE0);
        		glBindTexture(GL_TEXTURE_2D, ourCross);
			} else {
				setMaterial(stainedWindow, state.prog);
				glActiveTexture(GL_TEXTURE0);
        		glBindTexture(GL_TEXTURE_2D, ourCross);
			}
			glDrawArrays( GL_TRIANGLES, transPos[it->second], transparent[it->second].positions.size());

		}


		glUseProgram( 0 );

		OGL_CHECKPOINT_DEBUG();

		// Display results
		glfwSwapBuffers( window );
	}

	// Cleanup.
	state.prog = nullptr;
	state.moveProg = nullptr;
	
	return 0;
}
catch( std::exception const& eErr )
{
	std::fprintf( stderr, "Top-level Exception (%s):\n", typeid(eErr).name() );
	std::fprintf( stderr, "%s\n", eErr.what() );
	std::fprintf( stderr, "Bye.\n" );
	return 1;
}


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
		if (auto* state = static_cast<State_*>(glfwGetWindowUserPointer(aWindow)))
		{
			// R-key reloads shaders.
			if (GLFW_KEY_R == aKey && GLFW_PRESS == aAction)
			{
				if (state->prog)
				{
					try
					{
						state->prog->reload();
						std::fprintf(stderr, "Shaders reloaded and recompiled.\n");
					}
					catch (std::exception const& eErr)
					{
						std::fprintf(stderr, "Error when reloading shader:\n");
						std::fprintf(stderr, "%s\n", eErr.what());
						std::fprintf(stderr, "Keeping old shader.\n");
					}
				}
			}

			// Space toggles camera
			if (GLFW_KEY_SPACE == aKey && GLFW_PRESS == aAction)
			{
				state->camControl.cameraActive = !state->camControl.cameraActive;

				if (state->camControl.cameraActive)
					glfwSetInputMode(aWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
				else
					glfwSetInputMode(aWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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
						state->camControl.actionSpeedUp = true;
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
				if (GLFW_KEY_ENTER == aKey && state->enterPressed != 1) {
					state->enterPressed = 1;
					GLint windowWidth, windowHeight;
					glfwGetWindowSize(aWindow, &windowWidth, &windowHeight);
					uint8_t* pixels = new uint8_t[windowWidth * windowHeight * 3];
					glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels);
					createTexture(windowWidth, windowHeight, pixels);
				} else if (GLFW_KEY_ENTER != aKey) {
					state->enterPressed = 0;
				}
			}

			// If camera is not active then stop movement
			else
			{
				state->camControl.forward = false;
				state->camControl.backward = false;
				state->camControl.left = false;
				state->camControl.right = false;
				state->camControl.up = false;
				state->camControl.down = false;
				state->camControl.actionSpeedUp = false;
				state->camControl.actionSlowDown = false;
			}

			//Animation controls.
			if (state->camControl.cameraActive)
			{
				//Speedup, pause/play, slowdown
				if (GLFW_KEY_J == aKey)
				{
					if (GLFW_PRESS == aAction && state->aniControl.speedUp == false)
						state->aniControl.speedUp = true;
					else if (GLFW_PRESS == aAction && state->aniControl.speedUp == true)
						state->aniControl.speedUp = false;
				}
				if (GLFW_KEY_K == aKey)
				{
					if (GLFW_PRESS == aAction && state->aniControl.pause == false)
						state->aniControl.pause = true;
					else if (GLFW_PRESS == aAction && state->aniControl.pause == true)
						state->aniControl.pause = false;
				}
				if (GLFW_KEY_L == aKey)
				{
					if (GLFW_PRESS == aAction && state->aniControl.slowDown == false)
						state->aniControl.slowDown = true;
					else if (GLFW_PRESS == aAction && state->aniControl.slowDown == true)
						state->aniControl.slowDown = false;
				}
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

			if (state->camControl.cameraActive) {
				GLint windowWidth, windowHeight;
				glfwGetWindowSize(aWindow, &windowWidth, &windowHeight);
				if ( aX < 10 || aX > windowWidth - 10 ) { 
					state->camControl.lastX = windowWidth/2;   
					state->camControl.lastY = int(aY);  
					glfwSetCursorPos(aWindow, windowWidth/2, int(aY));
				} else if (aY < 10 || aY > windowHeight - 10) {
					state->camControl.lastX = float(aX);
					state->camControl.lastY = windowHeight/2;
					glfwSetCursorPos(aWindow, int(aX),windowHeight/2);
				}
			}
		}
	}

	void movementControl(State_ &state, float dt){
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
		}
		else if (state.camControl.backward) {
			state.camControl.cameraPos -= state.camControl.cameraFront * (kMovementPerSecond_ + speedChange) * dt;
		}
		//Left, right
		if (state.camControl.left) {
			state.camControl.cameraPos -= normalize(cross(state.camControl.cameraFront, state.camControl.cameraUp)) * (kMovementPerSecond_ + speedChange) * dt;
		}
		else if (state.camControl.right) {
			state.camControl.cameraPos += normalize(cross(state.camControl.cameraFront, state.camControl.cameraUp)) * (kMovementPerSecond_ + speedChange) * dt;
		}
		//Up, down
		if (state.camControl.up) {
			state.camControl.cameraPos.y += (kMovementPerSecond_ + speedChange) * dt;
		}
		else if (state.camControl.down) {
			state.camControl.cameraPos.y -= (kMovementPerSecond_ + speedChange) * dt;
		}
	}

	//Function for door control.
	float doorControl(State_& state, float angle, std::chrono::steady_clock::time_point& last, bool& increase)
	{
		auto now = Clock::now();
		//Rotate door around door frame by 90 degrees.
		float aniDt = std::chrono::duration_cast<Secondsf>(now - last).count();
		float aniSpeedMod = 1.f;
		//Speed modifiers
		if (state.aniControl.speedUp == true)
			aniSpeedMod = kAniModPos_;
		else if (state.aniControl.slowDown == true)
			aniSpeedMod = kAniModNeg_;
		else
			aniSpeedMod = 1.f;

		//Checks whether to pause the animation.
		if (state.aniControl.pause == false)
		{
			last = now;
			//Calculates whether the door is in closing or opening state.
			if (angle > kPi_ / 2 && increase == true)
				increase = false;
			else if (angle < 0 && increase == false)
				increase = true;
			//Rotates positive if opening, negative if closing.
			switch (increase)
			{
			case true:
				angle += aniDt * kPi_ * 0.3f * aniSpeedMod;
				break;
			case false:
				angle -= aniDt * kPi_ * 0.3f * aniSpeedMod;
			}
		}
		else
		{
			last = now;
		}
		return angle;
	}
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
