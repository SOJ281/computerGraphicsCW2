#include "loadobj.hpp"

#include <rapidobj/rapidobj.hpp>

#include "../support/error.hpp"

SimpleMeshData load_wavefront_obj( char const* aPath )
{
	auto result = rapidobj::ParseFile( aPath );
	if( result.error )
		throw Error( "Unable to load OBJ file ’%s’: %s", aPath, result.error.code.message().c_str() );
	
	// OBJ files can define faces that are not triangles. However, OpenGL will only render triangles (and lines
	// and points), so we must triangulate any faces that are not already triangles. Fortunately, rapidobj can do
	// this for us.

	rapidobj::Triangulate( result );

	// Convert the OBJ data into a SimpleMeshData structure. For now, we simply turn the object into a triangle
	// soup, ignoring the indexing information that the OBJ file contains

	SimpleMeshData ret; 

	for( auto const& shape : result.shapes ) {
		for( std::size_t i = 0; i < shape.mesh.indices.size(); ++i ) {
			auto const& idx = shape.mesh.indices[i];
			//ret.indices.emplace_back((unsigned int)idx.position_index);
			ret.indices.emplace_back((unsigned int)i);

			//ret.texCoords.emplace_back((unsigned int)idx.position_index);
			//ret.normals.emplace_back((unsigned int)idx.position_index);
			//int position_index;
   			// int texcoord_index;
    		//int normal_index;
			ret.texCoords.emplace_back( Vec2f{
				result.attributes.texcoords[idx.texcoord_index*2+0],
				result.attributes.texcoords[idx.texcoord_index*2+1]} );
			ret.normals.emplace_back( Vec3f{
				result.attributes.normals[idx.normal_index*3+0],
				result.attributes.normals[idx.normal_index*3+1], 
				result.attributes.normals[idx.normal_index*3+2]} );
			ret.positions.emplace_back( Vec3f{
				result.attributes.positions[idx.position_index*3+0],
				result.attributes.positions[idx.position_index*3+1], 
				result.attributes.positions[idx.position_index*3+2]} );
			auto const& mat = result.materials[shape.mesh.material_ids[i/3]];
			// Just replicate the material ambient color for each vertex...
			ret.colors.emplace_back( Vec3f{
				mat.ambient[0],
				mat.ambient[1],
				mat.ambient[2]} );		
		}	
	}
	return ret;
}

