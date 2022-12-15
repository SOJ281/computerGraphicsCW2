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
#include "loadcustom.hpp"

namespace
{
	constexpr char const* kWindowTitle = "COMP3811 - Coursework 2";
	
	constexpr float kPi_ = 3.1415926f;

	//Camera state and constant values.
	constexpr float kMovementPerSecond_ = 5.f; // units per second
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
			float radius;

			float lastX, lastY;
			float X, Y, Z;
		} camControl;
	};

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
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

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
	state.camControl.radius = 10.f;
	state.camControl.Y = -3.f;

	// Animation state
	auto last = Clock::now();
	float angle = 0.f;

	// Other initialization & loading
	OGL_CHECKPOINT_ALWAYS();
	
	// TODO: 

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
			besties.emplace_back(make_cylinder( 
				true, 18, Vec3f{1, 0, 1}, make_scaling( .5f, 5.f, .5f ) * make_translation( { 4.f - l*8.f, 0.f, 9.f + i*5.f }) * make_rotation_z( 3.141592f / 2.f )));

	//for (auto idx :bestie.positions )
	//	printf("INDICES =(%f,%f,%f)", idx.x,idx.y,idx.z);
			//besties.emplace_back(bestie);
	//auto aMeshData = make_cylinder( true, 18, {1, 0, 1}, make_rotation_z( 3.141592f / 2.f )* make_scaling( 8.f, 2.f, 2.f ));
	//besties.emplace_back(aMeshData);

	//auto aMeshData = make_div_cylinder( true, 18, {1, 0, 1}, make_scaling( 5.f, 1.f, 1.f ), 16);
	//besties.emplace_back(testCylinder);
	//besties.emplace_back(aMeshData);
	//GLuint vao = create_vao(aMeshData);
	GLuint vao = create_vaoM(&besties[0],11);
	printf("floor =(%ld)", floor.positions.size());
	printf("pillar =(%ld)", pillar.positions.size());


	int vertexCount = bestie.positions.size() *friends * friends + floor.positions.size() +pillar.positions.size() * rows*2;
	float sunXloc = -1.f;

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

		float speedChange = 0;
		if (state.camControl.actionSpeedUp)
			speedChange = kMovementModPos_;
		else if (state.camControl.actionSlowDown)
			speedChange = -kMovementModNeg_;
		else
			speedChange = 0;
		// Update camera state based on keyboard input.
		if (state.camControl.forward) {
			state.camControl.radius -= (kMovementPerSecond_ + speedChange) * dt;
			state.camControl.Z += (kMovementPerSecond_ + speedChange) * dt;
		} else if (state.camControl.backward) {
			state.camControl.radius += (kMovementPerSecond_ + speedChange) * dt;
			state.camControl.Z -= (kMovementPerSecond_ + speedChange) * dt;
		}

		//Left right
		if (state.camControl.left) {
			state.camControl.radius -= (kMovementPerSecond_ + speedChange) * dt;
			state.camControl.X += (kMovementPerSecond_ + speedChange) * dt;
		} else if (state.camControl.right) {
			state.camControl.radius += (kMovementPerSecond_ + speedChange) * dt;
			state.camControl.X -= (kMovementPerSecond_ + speedChange) * dt;
		}

		//Up down
		if (state.camControl.up) {
			state.camControl.radius -= (kMovementPerSecond_ + speedChange) * dt;
			state.camControl.Y += (kMovementPerSecond_ + speedChange) * dt;
		}else if (state.camControl.down) {
			state.camControl.radius += (kMovementPerSecond_ + speedChange) * dt;
			state.camControl.Y -= (kMovementPerSecond_ + speedChange) * dt;
		}

		if (state.camControl.radius <= 0.1f)
			state.camControl.radius = 0.1f;

		//TODO: define and compute projCameraWorld matrix
		Mat44f model2world = make_rotation_y(0);

		Mat44f Rx = make_rotation_x( state.camControl.theta );
		Mat44f Ry = make_rotation_y( state.camControl.phi );
		Mat44f T = make_translation( { 0.f, 0.f, -state.camControl.radius } );
		Mat44f moving = make_translation( { state.camControl.X, state.camControl.Y, state.camControl.Z } );
		Mat44f world2camera = moving;
		//Mat44f world2camera = T*Rx*Ry;
		//Mat44f world2camera = make_translation( { 0.f, 0.f, -10.f } );

		Mat44f projection = make_perspective_projection(
			60.f * 3.1415926f / 180.f,
			fbwidth/float(fbheight),
			0.1f, 100.0f
		);
		projection = projection*Rx*Ry;

		Mat44f projCameraWorld = projection * world2camera * model2world;
		Mat33f normalMatrix = mat44_to_mat33( transpose(invert(model2world)) );

		sunXloc += .01f;
		if (sunXloc > .99f)
			sunXloc = -1.f;
		Vec3f lightDir = normalize( Vec3f{ 1.f, 1.f, 0.5f } );

		Vec3f lightPos = Vec3f{ 0.f, 1.f, 0.f };

		struct PointLight {
			Vec3f position;
			float constant;
			float linear;
			float quadratic;
			Vec3f ambient;
			Vec3f diffuse;
		};

		std::vector<PointLight> PointLightData;
		
		PointLight p1;
		p1.position = Vec3f{ 0, 1.f, 0.f };
		p1.constant = 5.f;
		p1.linear = 0.3f;
		p1.quadratic = 0.32f;
		p1.ambient = Vec3f{ 0.f, 1.f, 0.f };
		p1.diffuse = Vec3f{ 0.f, 1.f, 0.f };
		PointLightData.emplace_back(p1);

	
		// Draw scene
		OGL_CHECKPOINT_DEBUG();

		//TODO: draw frame
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glUseProgram( prog.programId() );


		//glUniform3fv( 2, 1, &lightDir.x ); //lightDir uLightDir
		glUniform3fv(glGetUniformLocation(prog.programId(), "uLightDir"), 1, &lightDir.x ); //lightDir uLightDir
		//glUniform3f( 3, 0.f, 0.f, 0.f ); //lightDiffuse
		glUniform3f( glGetUniformLocation(prog.programId(), "uLightDiffuse"), .1f, 0.3f, .1f ); //lightDiffuse
		//glUniform3f( 4, 0.05f, 0.05f, 0.05f ); //uSceneAmbient
		glUniform3f( glGetUniformLocation(prog.programId(), "uSceneAmbient"), 0.05f, 0.05f, 0.05f ); //uSceneAmbient

		//glUniform3fv( 10, &pl.position.x ); //uSceneAmbient
		// point light 1
        //lightingShader.setVec3("pointLights[0].position", pointLightPositions[0]);
		glUniform3fv(glGetUniformLocation(prog.programId(), "pointLights[0].position"), 1, &PointLightData[0].position.x ); //lightDir uLightDir
		glUniform1f(glGetUniformLocation(prog.programId(), "pointLights[0].constant"), PointLightData[0].constant ); //lightDir uLightDir
		glUniform1f(glGetUniformLocation(prog.programId(), "pointLights[0].linear"), PointLightData[0].linear ); //lightDir uLightDir
		glUniform1f(glGetUniformLocation(prog.programId(), "pointLights[0].quadratic"), PointLightData[0].quadratic ); //lightDir uLightDir
		glUniform3fv(glGetUniformLocation(prog.programId(), "pointLights[0].ambient"), 1, &PointLightData[0].ambient.x ); //lightDir uLightDir
		glUniform3fv(glGetUniformLocation(prog.programId(), "pointLights[0].diffuse"), 1, &PointLightData[0].diffuse.x ); //lightDir uLightDir


		//glUniform3f(5, 0.f, 6.f, 0.f  ); //uSceneAmbient

		//glUniformMatrix4fv( 0, 1, GL_TRUE, projCameraWorld.v );
		glUniformMatrix4fv(glGetUniformLocation(prog.programId(), "uProjCameraWorld"), 1, GL_TRUE, projCameraWorld.v );
		//glUniformMatrix3fv(1,1, GL_TRUE, normalMatrix.v);
		glUniformMatrix3fv(glGetUniformLocation(prog.programId(), "uNormalMatrix"),1, GL_TRUE, normalMatrix.v);

		
		//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		
		glBindVertexArray( vao );
		glDrawArrays( GL_TRIANGLES, 0, vertexCount);
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
				
				//Speed modifiers
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

				state->camControl.phi += dx * kMouseSensitivity_;

				state->camControl.theta += dy * kMouseSensitivity_;
				if (state->camControl.theta > kPi_ / 2.f)
					state->camControl.theta = kPi_ / 2.f;
				else if (state->camControl.theta < -kPi_ / 2.f)
					state->camControl.theta = -kPi_ / 2.f;
			}

			state->camControl.lastX = float(aX);
			state->camControl.lastY = float(aY);
		}
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

