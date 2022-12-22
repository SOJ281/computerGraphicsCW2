#ifndef SIMPLE_MESH_HPP_C6B749D6_C83B_434C_9E58_F05FC27FEFC9
#define SIMPLE_MESH_HPP_C6B749D6_C83B_434C_9E58_F05FC27FEFC9

#include <glad.h>

#include <vector>
#include <cstdio>

#include "../vmlib/vec3.hpp"
#include "../vmlib/vec2.hpp"

#include "../vmlib/mat33.hpp"
#include "../vmlib/mat44.hpp"

struct SimpleMeshData
{
	std::vector<Vec3f> positions;
	std::vector<Vec3f> normals;
	std::vector<Vec2f> texCoords;
	std::vector<uint32_t> indices;
	//std::vector<std::uint32_t> indices;
	Vec3f centre;
};

SimpleMeshData concatenate( SimpleMeshData, SimpleMeshData const& );
SimpleMeshData make_change( SimpleMeshData, Mat44f );
std::vector<Vec3f> calcNorms(std::vector<Vec3f> pos);


GLuint create_vao( SimpleMeshData const& );
GLuint create_vaoM( SimpleMeshData* , int number);
SimpleMeshData make_cube(Vec3f aColor, Mat44f aPreTransform );
SimpleMeshData make_door(Vec3f aColor, Mat44f aPreTransform);
SimpleMeshData make_sphere(std::size_t aSubdivs, Mat44f aPreTransform );
SimpleMeshData invert_normals(SimpleMeshData aM);
SimpleMeshData make_frame(Vec3f aColor, Mat44f aPreTransform );
Vec3f getMean(SimpleMeshData data);


#endif // SIMPLE_MESH_HPP_C6B749D6_C83B_434C_9E58_F05FC27FEFC9
