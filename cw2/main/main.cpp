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
#include "loadobj.hpp"
#include "materials.hpp"
#include <string>
#include <cstring>
#include <stb_image.h>
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

		struct AnimCtrl_
		{
			bool pausePlay, speedUp, slowDown;
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

	unsigned int loadTexture(const char *path);

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
	glEnable( GL_CULL_FACE );
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
	

	unsigned int ourSaviour = loadTexture("assets/markus.png");
	unsigned int ourBlank = loadTexture("assets/Solid_white.png");
	unsigned int ourVoid = loadTexture("assets/Solid_black.png");
	//unsigned int ourFish = loadTexture("assets/nong-v-wcMK9KKbmms-unsplash.jpg");
	//unsigned int ourFish = loadTexture("assets/fishTransparent.png");
	
	
	
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


	auto pillar = make_cylinder( true, 18, {.1f, 0, .1f}, make_scaling( .75f, 10.f, .75f ) * make_rotation_z( 3.141592f / 2.f ));

	auto lamp = make_cylinder( true, 18, {.1f, .1f, .1f}, make_scaling( .4f, 2.f, .4f ) * make_rotation_z( 3.141592f / 2.f ));
	lamp = invert_normals(lamp);

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
			transparent.emplace_back(make_change(lamp, make_translation( { 3.f - 6.f*l, 2.f, 15.f + 15.f * i }) ));
			transLocs.emplace_back(Vec3f{ 3.f - 6.f*l, 2.f, 15.f + 15.f * i });
			transPos.emplace_back(transPos[transPos.size()-1] + lamp.positions.size());
		}

	int windowCount = 4;
	auto stainWindow = make_cube( Vec3f{0.05f, 0.05f, 0.05f}, make_rotation_y( 3.141592f) * make_scaling( .1f, 5.f, 5.f ));//so 48 meter
	for (float i = 0; i < 2; i++)
		for (float l = 0; l < 2; l++) {
			Vec3f thisLoc = Vec3f{ 29.f - 58.f*l, 15.f, 21.f + 18.f * i };
			transparent.emplace_back(make_change(stainWindow, make_translation( thisLoc) ));
			transLocs.emplace_back(thisLoc);
			transPos.emplace_back(transPos[transPos.size()-1] + stainWindow.positions.size());
		}


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
	roof = make_change(roof, make_scaling( 28.f, 20.f, 50.f ) );
	roof = make_change(roof, make_translation( {0.f, 30.f, 55.f }) );
	chapel.emplace_back(roof);
	int roofCount = 1;

	auto plutonium = make_cube( Vec3f{0.05f, 0.05f, 0.05f}, make_scaling( 2.f, 2.f, 2.f ) * make_translation( {0.f, 5.f, 0.f }));//so 48 meter
	chapel.emplace_back(plutonium);
	int plutoniumCount = 1;


	auto shiny = make_cube( Vec3f{0.05f, 0.05f, 0.05f}, make_scaling( 2.f, 2.f, 2.f ));//so 48 meter
	chapel.emplace_back(make_change(shiny, make_translation( {0.f, 16.f, 0.f }) ));
	int shinyCount = 1;


	auto diffuseObject = make_cube( Vec3f{0.05f, 0.05f, 0.05f}, make_scaling( 2.f, 2.f, 2.f ));//so 48 meter
	chapel.emplace_back(make_change(diffuseObject, make_translation( {0.f, 22.f, 0.f }) ));
	int diffuseCount = 1;


	auto aquarium = make_cylinder( true, 18, {.5f, 0.5f, .5f}, make_scaling( 3.f, 10.f, 3.f ) * make_rotation_z( 3.141592f / 2.f ));
	//auto aquarium = make_cube( Vec3f{0.05f, 0.05f, 0.05f}, make_rotation_z( -3.141592f / 2.f ) * make_rotation_y( 3.141592f / 2.f ) * make_scaling( .1f, 6.f, 7.62f ));//so 48 meter
	chapel.emplace_back(make_change(aquarium, make_translation( {10.f, 5.f, 0.f }) ));
	int aquariumCount = 1;


	//auto window = make_cube( Vec3f{0.05f, 0.05f, 0.05f}, make_scaling( 2.f, 2.f, 2.f ));//so 48 meter
	//chapel.emplace_back(make_change(window, make_translation( {0.f, 0.f, 0.f }) ));
	//int windowCount = 1;
	
	//TEST DOOOR
	auto door = make_cube(Vec3f{ 0.05f, 0.05f, 0.05f }, make_scaling(2.f, 2.f, 2.f));
	chapel.emplace_back(make_change(door, make_translation( {-1.f, 2.f, 0.f} )));
	int doorCount = 1;

	int vertexCount = floor.positions.size() + frontWall.positions.size() * wallBits + sideWall.positions.size() * sideWallBits 
	+ pillar.positions.size() * rows*2;
	float sunXloc = -1.f;
	printf("\nNINJA\n");
	GLuint vao = create_vaoM(&chapel[0], 1 + wallBits + sideWallBits + rows*2+ pictures + backrooms 
	+ benchCount + roofCount + plutoniumCount + shinyCount + diffuseCount + aquariumCount + doorCount);
	//GLuint transparentVao = create_vaoM(&transparent[0], lampCount);
	GLuint transparentVao = create_vaoM(&transparent[0], transparent.size());
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
		Mat44f model2world = kIdentity44f;
		Mat44f Rx = make_rotation_x( state.camControl.theta );
		Mat44f Ry = make_rotation_y( state.camControl.phi );
		Mat44f world2camera = make_translation(state.camControl.cameraPos);
		//Mat44f world2camera = T*Rx*Ry;
		//Mat44f world2camera = make_translation( { 0.f, 0.f, -10.f } );

		Mat44f projection = make_perspective_projection(
			60.f * 3.1415926f / 180.f,
			fbwidth/float(fbheight),
			0.1f, 200.0f
		);
		projection = projection*Rx*Ry;

		//door = make_change( door, make_rotation_y(angle) );


		Mat44f projCameraWorld = projection * world2camera * model2world;
		Mat33f normalMatrix = mat44_to_mat33( transpose(invert(model2world)) );


		sunXloc += .01f;
		if (sunXloc > .99f)
			sunXloc = -1.f;
		Vec3f lightDir = normalize( Vec3f{ 0.f, 1.f, 1.f } );

		Vec3f lightPos = Vec3f{ 0.f, 1.f, 0.f };

		std::map<float, int> sorted;
		Vec3f camPosi = Vec3f {state.camControl.cameraPos.x, state.camControl.cameraPos.y, state.camControl.cameraPos.z };
        for (unsigned int i = 0; i < transLocs.size(); i++) {
            float distance = length(camPosi + transLocs[i]);
			//printf("Distance:%f", distance);
            sorted[distance] = i;
        }


		


	
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


		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		
		glBindVertexArray( vao );
		glUniform1f(glGetUniformLocation(prog.programId(), "material.opacity"), 1);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, NULL);

		setMaterial(stone, state.prog);
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
		glDrawArrays( GL_TRIANGLES, counter, picture.positions.size());
		counter += picture.positions.size();




		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, NULL);
		glDrawArrays( GL_TRIANGLES, counter, backroom.positions.size() * backrooms);
		counter += backroom.positions.size()*backrooms;
		

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
			setMaterial(cushion, state.prog);
			counter += benchShapes[0];
			glDrawArrays( GL_TRIANGLES, 0, counter);


			setMaterial(wood, state.prog);
			counter += benchShapes[1];
			glDrawArrays( GL_TRIANGLES, 0, counter);
			counter += benchShapes[2];
			glDrawArrays( GL_TRIANGLES, 0, counter);


			setMaterial(cushion, state.prog);
			counter += benchShapes[3];
			glDrawArrays( GL_TRIANGLES, 0, counter);


			setMaterial(wood, state.prog);
			for (int l = 4; l < benchShapes.size(); l++) {
				counter += benchShapes[l];
				glDrawArrays( GL_TRIANGLES, 0, counter);
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
		//glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, ourFish);
		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ourSaviour);
		glDrawArrays( GL_TRIANGLES, counter, aquarium.positions.size() * aquariumCount);
		counter += aquarium.positions.size() * aquariumCount;


		setMaterial(wood, state.prog);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, NULL);
		model2world = make_rotation_y(angle);
		projCameraWorld = projCameraWorld * model2world;
		glUniformMatrix4fv(glGetUniformLocation(prog.programId(), "uProjCameraWorld"), 1, GL_TRUE, projCameraWorld.v);
		glDrawArrays(GL_TRIANGLES, counter, door.positions.size());
		counter += door.positions.size() * doorCount;

		model2world = kIdentity44f;
		projCameraWorld = projection * world2camera * model2world;
		glUniformMatrix4fv(glGetUniformLocation(prog.programId(), "uProjCameraWorld"), 1, GL_TRUE, projCameraWorld.v);


		//counter += window.positions.size() * windowCount;
		//glDrawArrays( GL_TRIANGLES, 0, counter);
		//glDrawElements( GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, NULL);
		//glDrawElements( GL_TRIANGLES, vertexCount, GL_UNSIGNED_BYTE, NULL);
		//glDrawElements( GL_TRIANGLES, vertexCount, GL_UNSIGNED_SHORT, NULL);

		glBindVertexArray( 0 );

		glBindVertexArray( transparentVao );

		setMaterial(lampGlass, state.prog);
		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, NULL);
		glUniform1f(glGetUniformLocation(prog.programId(), "material.opacity"), 0.6);
		int transCounter = 0;

		for (std::map<float, int>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
			//printf("%d/", it->second);
			//glUniformMatrix4fv(glGetUniformLocation(prog.programId(), "uProjCameraWorld"), 1, GL_TRUE, (projCameraWorld * make_translation(it->second)).v );
			
			//glDrawArrays( GL_TRIANGLES, lamp.positions.size() * it->second, lamp.positions.size());
			glDrawArrays( GL_TRIANGLES, transPos[it->second], transparent[it->second].positions.size());
			//transCounter += lamp.positions.size();
		}


		//glColor3f(1.0, 1.0, 1.0);
		//glVertex3f(0, 5, 15);
		//glFlush();


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
					if (GLFW_PRESS == aAction)
						state->aniControl.speedUp = true;
					else if (GLFW_RELEASE == aAction)
						state->aniControl.speedUp = false;
				}
				if (GLFW_KEY_K == aKey)
				{
					if (GLFW_PRESS == aAction)
						state->aniControl.pausePlay = true;
					else if (GLFW_RELEASE == aAction)
						state->aniControl.pausePlay = false;
				}
				if (GLFW_KEY_L == aKey)
				{
					if (GLFW_PRESS == aAction)
						state->aniControl.slowDown = true;
					else if (GLFW_RELEASE == aAction)
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
		}
	}
	unsigned int loadTexture(char const * path) {
	
		assert( path );
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
