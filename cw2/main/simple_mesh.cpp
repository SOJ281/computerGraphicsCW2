#include "simple_mesh.hpp"
#include "cube.hpp"
//#include <cstdio>

SimpleMeshData concatenate( SimpleMeshData aM, SimpleMeshData const& aN )
{
	aM.positions.insert( aM.positions.end(), aN.positions.begin(), aN.positions.end() );
	aM.normals.insert( aM.normals.end(), aN.normals.begin(), aN.normals.end() );
	//aM.indices.insert( aM.indices.end(), aN.indices.begin(), aN.indices.end() );
	return aM;
}


SimpleMeshData make_change( SimpleMeshData aM, Mat44f aPreTransform ) {
	SimpleMeshData newMesh = aM;

	for( auto& p : newMesh.positions ) {
		Vec4f p4{ p.x, p.y, p.z, 1.f };
		Vec4f t = aPreTransform * p4;
		t /= t.w;
		p = Vec3f{ t.x, t.y, t.z };
	}
	return newMesh;
}

SimpleMeshData invert_normals(SimpleMeshData aM) {
	SimpleMeshData newMesh = aM;

	for( auto& p : newMesh.normals ) {
		p = Vec3f{1,1,1} -p;
	}
	return newMesh;
}

GLuint create_vao( SimpleMeshData const& aMeshData ) {

	GLuint positionVBO = 0;
	glGenBuffers( 1, &positionVBO );
	glBindBuffer( GL_ARRAY_BUFFER, positionVBO );
	glBufferData( GL_ARRAY_BUFFER, aMeshData.positions.size() * sizeof(Vec3f), aMeshData.positions.data(), GL_STATIC_DRAW );

	GLuint normalVBO = 0;
	glGenBuffers( 1, &normalVBO );
	glBindBuffer( GL_ARRAY_BUFFER, normalVBO );
	glBufferData( GL_ARRAY_BUFFER, aMeshData.normals.size() * sizeof(Vec3f), aMeshData.normals.data(), GL_STATIC_DRAW );


	//VAO
	GLuint vao = 0;
	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao );
	
	glBindBuffer( GL_ARRAY_BUFFER, positionVBO );
	glVertexAttribPointer(0, // location = 0 in vertex shader
	3, GL_FLOAT, GL_FALSE, // 2 floats, not normalized to [0..1] (GL FALSE)
	0, // stride = 0 indicates that there is no padding between inputs
	0 // data starts at offset 0 in the VBO.
	);
	glEnableVertexAttribArray(0);


	glBindBuffer( GL_ARRAY_BUFFER, normalVBO );
	glVertexAttribPointer(1, // location = 2 in vertex shader
	3, GL_FLOAT, GL_FALSE, // 3 floats, not normalized to [0..1] (GL FALSE)
	0, // see above
	0 // see above
	);
	glEnableVertexAttribArray( 1 ); // Reset state

	glBindVertexArray( 0 );//poss
	glBindBuffer( GL_ARRAY_BUFFER, 0 );//poss

	glDeleteBuffers( 1, &positionVBO );
	glDeleteBuffers( 1, &normalVBO );

	return vao;
}


GLuint create_vaoM( SimpleMeshData * aMeshData , int number) {
	GLuint positionVBO = 0;
	GLuint colorVBO = 0;
	GLuint normalVBO = 0;
	GLuint textureVBO = 0;
	GLuint indexVBO = 0;
	GLuint vao = 0;

		int c = number;
		//printf("posize =(%ld)\n", aMeshData[0].positions.size());
		//printf("locale =(%ld)\n", aMeshData[0].positions.data());
		//printf("\ncdl%d\n", c);

		glGenBuffers( 1, &positionVBO );
		glBindBuffer( GL_ARRAY_BUFFER, positionVBO );
		int pMax = 0;
		for (int i = 0; i < c; i++) {
			pMax += aMeshData[i].positions.size()* sizeof(Vec3f);
			//printf("pMax = %ld\n", pMax);
		}
		glBufferData(GL_ARRAY_BUFFER, pMax, 0, GL_STATIC_DRAW);
		int pSize = 0;
		for (int i = 0; i < c; i++) {
			//printf("pSize%d = %d\n", i, pSize);
			//printf("locale =(%f)\n", aMeshData[0].positions.data()->x);
			//printf("size =(%ld)\n", aMeshData[i].positions.size()* sizeof(Vec3f));
			glBufferSubData(GL_ARRAY_BUFFER, pSize, aMeshData[i].positions.size()* sizeof(Vec3f), aMeshData[i].positions.data());
			pSize += aMeshData[i].positions.size()* sizeof(Vec3f);
			//printf("posit");
		}
		printf("\n\npmax = %d\n", pMax);
		//printf("posdone");

/*
		//Color
		glGenBuffers( 1, &colorVBO );
		glBindBuffer( GL_ARRAY_BUFFER, colorVBO );
		int cMax = 0;
		for (int i = 0; i < c; i++) {
			cMax += aMeshData[i].colors.size()* sizeof(Vec3f);
		}
		glBufferData(GL_ARRAY_BUFFER, cMax, 0, GL_STATIC_DRAW);
		int cSize = 0;
		for (int i = 0; i < c; i++) {
			//printf("cSize%d = %d\n", i, cSize);
			glBufferSubData(GL_ARRAY_BUFFER, cSize, aMeshData[i].colors.size()* sizeof(Vec3f), aMeshData[i].colors.data());
			cSize += aMeshData[i].colors.size()* sizeof(Vec3f);
		}*/

		//normal
		glGenBuffers( 1, &normalVBO );
		glBindBuffer( GL_ARRAY_BUFFER, normalVBO );
		int nMax = 0;
		for (int i = 0; i < c; i++) {
			nMax += aMeshData[i].normals.size()* sizeof(Vec3f);
		}
		glBufferData(GL_ARRAY_BUFFER, nMax, 0, GL_STATIC_DRAW);
		int nSize = 0;
		for (int i = 0; i < c; i++) {
			glBufferSubData(GL_ARRAY_BUFFER, nSize, aMeshData[i].normals.size()* sizeof(Vec3f), aMeshData[i].normals.data());
			nSize += aMeshData[i].normals.size()* sizeof(Vec3f);
		}

		//textures
		glGenBuffers( 1, &textureVBO );
		glBindBuffer( GL_ARRAY_BUFFER, textureVBO );

		int tMax = 0;
		for (int i = 0; i < c; i++) {
			tMax += aMeshData[i].texCoords.size()* sizeof(Vec2f);
		}
		printf("tex = %d", tMax);
		glBufferData(GL_ARRAY_BUFFER, tMax, 0, GL_STATIC_DRAW);
		int tSize = 0;
		for (int i = 0; i < c; i++) {
			glBufferSubData(GL_ARRAY_BUFFER, tSize, aMeshData[i].texCoords.size()*sizeof(Vec2f), aMeshData[i].texCoords.data());
			tSize += aMeshData[i].texCoords.size()*sizeof(Vec2f);
		}


		//index
		glGenBuffers( 1, &indexVBO );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexVBO );

		int iMax = 0;
		for (int i = 0; i < c; i++) {
			iMax += aMeshData[i].indices.size() *sizeof(unsigned int);
		}
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, iMax, 0, GL_STATIC_DRAW);
		int iSize = 0;
		for (int i = 0; i < c; i++) {
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, iSize, aMeshData[i].indices.size()*sizeof(unsigned int), aMeshData[i].indices.data());
			iSize += aMeshData[i].indices.size()*sizeof(unsigned int);
		}

		
		


		//VAO
		glGenVertexArrays( 1, &vao );
		glBindVertexArray( vao );
		glBindBuffer( GL_ARRAY_BUFFER, positionVBO );
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		//glBindBuffer( GL_ARRAY_BUFFER, colorVBO );
		//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		//glEnableVertexAttribArray( 1 ); // Reset state


		glBindBuffer( GL_ARRAY_BUFFER, normalVBO );
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray( 1 ); // Reset state


		glBindBuffer( GL_ARRAY_BUFFER, textureVBO );
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray( 2 ); // Reset state

		glBindVertexArray( 0 );//poss
		glBindBuffer( GL_ARRAY_BUFFER, 0 );//poss

		glDeleteBuffers( 1, &colorVBO );
		glDeleteBuffers( 1, &positionVBO );
		glDeleteBuffers( 1, &normalVBO );
		glDeleteBuffers( 1, &textureVBO );


		return vao;
	}



SimpleMeshData make_cube(Vec3f aColor, Mat44f aPreTransform ) {
	std::vector<Vec3f> pos;
	std::vector<Vec3f> norm;
	std::vector<Vec2f> tex;
	Mat33f const N = mat44_to_mat33( transpose(invert(aPreTransform)) );

	int originSize = sizeof(kCubePositions)/sizeof(kCubePositions[0]);
	//float rightKCube[sizeof(kCubePositions)/sizeof(kCubePositions[0]) * 2];// = new float[kCubePositions.length];
	for (int i = 0; i < originSize; i+=3) {
		pos.emplace_back( Vec3f{ kCubePositions[i], kCubePositions[i+1], kCubePositions[i+2] } );
		//norm.emplace_back( Vec3f{ kCubePositions[i], kCubePositions[i+1], kCubePositions[i+2] } );
		//norm.emplace_back( Vec3f{ 0.f, prevY, prevZ } );
	}
	for (int i = 0; i < pos.size(); i+=3) {
		Vec3f U = pos[i+1] - pos[i];
		Vec3f V = pos[i+2] - pos[i];
		float normX = U.y*V.z - U.z*V.y;
		float normY = U.z*V.x - U.x*V.z;
		float normZ = U.x*V.y - U.y*V.x;
		norm.emplace_back( Vec3f{ normX, normY, normZ } );
		norm.emplace_back( Vec3f{ normX, normY, normZ } );
		norm.emplace_back( Vec3f{ normX, normY, normZ } );
	}


	for (int i = 0; i < 6; i++) {
		tex.emplace_back( Vec2f{ 0.f, 0.f} );
		tex.emplace_back( Vec2f{ 1.f, 0.f} );
		tex.emplace_back( Vec2f{ 1.f, 1.f} );

		tex.emplace_back( Vec2f{ 0.f, 0.f} );
		tex.emplace_back( Vec2f{ 1.f, 1.f} );
		tex.emplace_back( Vec2f{ 0.f, 1.f} );
	}
	printf("\nCube:\n");
	printf("pos.positions.size() = %ld\n", pos.size());
	printf("tex.positions.size() = %ld\n", tex.size());
	printf("norm.positions.size() = %ld\n", norm.size());

/*
	for( auto& n : norm ) {
		//Vec4f p4{ n.x, n.y, n.z, 1.f };
		Vec3f t = N * n;
		//t /= t.w;
		n = t;
		//n = { 1, 1, 1};
	}
*/
	for( auto& p : pos ) {
		Vec4f p4{ p.x, p.y, p.z, 1.f };
		Vec4f t = aPreTransform * p4;
		t /= t.w;
		p = Vec3f{ t.x, t.y, t.z };
	}
	std::vector col( pos.size(), aColor );
	return SimpleMeshData{ std::move(pos), std::move(norm), std::move(tex) };
}
/*
SimpleMeshData make_roof(Vec3f aColor, Mat44f aPreTransform ) {
	std::vector<Vec3f> pos;
	std::vector<Vec3f> norm;
	std::vector<Vec2f> tex;
	Mat33f const N = mat44_to_mat33( transpose(invert(aPreTransform)) );

	int originSize = sizeof(kCubePositions)/sizeof(kCubePositions[0]);

	for (int i = 0; i < originSize; i+=3) {
		pos.emplace_back( Vec3f{ 1, 0, 1 } );
		pos.emplace_back( Vec3f{ -1, 0, 1 } );
		pos.emplace_back( Vec3f{ 0, 1, 0 } );

		pos.emplace_back( Vec3f{ 1, 0, 1 } );
		pos.emplace_back( Vec3f{ 1, 0, -1 } );
		pos.emplace_back( Vec3f{ 0, 1, 0 } );
		
		//norm.emplace_back( Vec3f{ 0.f, prevY, prevZ } );
	}

	for (int i = 0; i < 6; i++) {
		tex.emplace_back( Vec2f{ 0.f, 0.f} );
		tex.emplace_back( Vec2f{ 1.f, 0.f} );
		tex.emplace_back( Vec2f{ 1.f, 1.f} );

		tex.emplace_back( Vec2f{ 0.f, 0.f} );
		tex.emplace_back( Vec2f{ 1.f, 1.f} );
		tex.emplace_back( Vec2f{ 0.f, 1.f} );
	}
	printf("\nCube:\n");
	printf("pos.positions.size() = %ld\n", pos.size());
	printf("tex.positions.size() = %ld\n", tex.size());

	for( auto& n : norm ) {
		Vec3f t = N * n;
		n = t;
	}

	for( auto& p : pos ) {
		Vec4f p4{ p.x, p.y, p.z, 1.f };
		Vec4f t = aPreTransform * p4;
		t /= t.w;
		p = Vec3f{ t.x, t.y, t.z };
	}
	std::vector col( pos.size(), aColor );
	return SimpleMeshData{ std::move(pos), std::move(norm), std::move(tex) };
}
*/

SimpleMeshData make_door(Vec3f aColor, Mat44f aPreTransform ) {
	std::vector<Vec3f> pos;
	std::vector<Vec3f> norm;
	std::vector<Vec2f> tex;
	Mat33f const N = mat44_to_mat33( transpose(invert(aPreTransform)) );

	int originSize = sizeof(kCubePositions)/sizeof(kCubePositions[0]);
	//float rightKCube[sizeof(kCubePositions)/sizeof(kCubePositions[0]) * 2];// = new float[kCubePositions.length];
	for (int i = 0; i < originSize; i+=3) {
		pos.emplace_back( Vec3f{ kCubePositions[i], kCubePositions[i+1], kCubePositions[i+2] } );
		norm.emplace_back( Vec3f{ kCubePositions[i], kCubePositions[i+1], kCubePositions[i+2] } );
	}

	for( auto& n : norm ) {
		Vec3f t = N * n;
		n = t;
	}

	for( auto& p : pos ) {
		Vec4f p4{ p.x, p.y, p.z, 1.f };
		Vec4f t = aPreTransform * p4;
		t /= t.w;
		p = Vec3f{ t.x, t.y, t.z };
	}
	return SimpleMeshData{ std::move(pos), std::move(norm) };
}