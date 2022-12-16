#include "simple_mesh.hpp"
#include "cube.hpp"
//#include <cstdio>

SimpleMeshData concatenate( SimpleMeshData aM, SimpleMeshData const& aN )
{
	aM.positions.insert( aM.positions.end(), aN.positions.begin(), aN.positions.end() );
	aM.colors.insert( aM.colors.end(), aN.colors.begin(), aN.colors.end() );
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


GLuint create_vao( SimpleMeshData const& aMeshData ) {

	GLuint positionVBO = 0;
	glGenBuffers( 1, &positionVBO );
	glBindBuffer( GL_ARRAY_BUFFER, positionVBO );
	glBufferData( GL_ARRAY_BUFFER, aMeshData.positions.size() * sizeof(Vec3f), aMeshData.positions.data(), GL_STATIC_DRAW );


	// Note: we can use sizeof(kPositions) because kPositions is defined as
	// C/C++ array. Never use sizeof() on pointers or on classes such as
	// std::vector!
	GLuint colorVBO = 0;
	glGenBuffers( 1, &colorVBO );
	glBindBuffer( GL_ARRAY_BUFFER, colorVBO );
	glBufferData( GL_ARRAY_BUFFER, aMeshData.colors.size() * sizeof(Vec3f), aMeshData.colors.data(), GL_STATIC_DRAW );


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


	glBindBuffer( GL_ARRAY_BUFFER, colorVBO );
	glVertexAttribPointer(1, // location = 1 in vertex shader
	3, GL_FLOAT, GL_FALSE, // 3 floats, not normalized to [0..1] (GL FALSE)
	0, // see above
	0 // see above
	);
	glEnableVertexAttribArray( 1 ); // Reset state


	glBindBuffer( GL_ARRAY_BUFFER, normalVBO );
	glVertexAttribPointer(2, // location = 2 in vertex shader
	3, GL_FLOAT, GL_FALSE, // 3 floats, not normalized to [0..1] (GL FALSE)
	0, // see above
	0 // see above
	);
	glEnableVertexAttribArray( 2 ); // Reset state

	glBindVertexArray( 0 );//poss
	glBindBuffer( GL_ARRAY_BUFFER, 0 );//poss

	glDeleteBuffers( 1, &colorVBO );
	glDeleteBuffers( 1, &positionVBO );
	glDeleteBuffers( 1, &normalVBO );

	return vao;
}



GLuint create_vaoM( SimpleMeshData * aMeshData , int number) {

	int c = number;
	//printf("posize =(%ld)\n", aMeshData[0].positions.size());
	//printf("locale =(%ld)\n", aMeshData[0].positions.data());
	//printf("\ncdl%d\n", c);

	GLuint positionVBO = 0;
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
	//printf("posdone");

	//Color
	GLuint colorVBO = 0;
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
	}

	//normal
	GLuint normalVBO = 0;
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
	printf("YAY");


	//index
	GLuint indexVBO = 0;
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
	printf("YAY");
	


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

	glBindBuffer( GL_ARRAY_BUFFER, colorVBO );
	glVertexAttribPointer(1, // location = 1 in vertex shader
	3, GL_FLOAT, GL_FALSE, // 3 floats, not normalized to [0..1] (GL FALSE)
	0, // see above
	0 // see above
	);
	glEnableVertexAttribArray( 1 ); // Reset state


	glBindBuffer( GL_ARRAY_BUFFER, normalVBO );
	glVertexAttribPointer(2, // location = 2 in vertex shader
	3, GL_FLOAT, GL_FALSE, // 3 floats, not normalized to [0..1] (GL FALSE)
	0, // see above
	0 // see above
	);
	glEnableVertexAttribArray( 2 ); // Reset state

	glBindVertexArray( 0 );//poss
	glBindBuffer( GL_ARRAY_BUFFER, 0 );//poss

	glDeleteBuffers( 1, &colorVBO );
	glDeleteBuffers( 1, &positionVBO );
	glDeleteBuffers( 1, &normalVBO );


	return vao;
}


SimpleMeshData make_cube(Vec3f aColor, Mat44f aPreTransform ) {
	std::vector<Vec3f> pos;
	std::vector<Vec3f> norm;
	Mat33f const N = mat44_to_mat33( transpose(invert(aPreTransform)) );

	int originSize = sizeof(kCubePositions)/sizeof(kCubePositions[0]);
	//float rightKCube[sizeof(kCubePositions)/sizeof(kCubePositions[0]) * 2];// = new float[kCubePositions.length];
	for (int i = 0; i < originSize; i+=3) {
		pos.emplace_back( Vec3f{ kCubePositions[i], kCubePositions[i+1], kCubePositions[i+2] } );
		norm.emplace_back( Vec3f{ kCubePositions[i], kCubePositions[i+1], kCubePositions[i+2] } );
		//norm.emplace_back( Vec3f{ 0.f, prevY, prevZ } );
	}

	for( auto& n : norm ) {
		//Vec4f p4{ n.x, n.y, n.z, 1.f };
		Vec3f t = N * n;
		//t /= t.w;
		n = t;
		//n = { 1, 1, 1};
	}

	for( auto& p : pos ) {
		Vec4f p4{ p.x, p.y, p.z, 1.f };
		Vec4f t = aPreTransform * p4;
		t /= t.w;
		p = Vec3f{ t.x, t.y, t.z };
	}
	std::vector col( pos.size(), aColor );
	return SimpleMeshData{ std::move(pos), std::move(col), std::move(norm) };
}


SimpleMeshData make_door(Vec3f aColor, Mat44f aPreTransform ) {
	std::vector<Vec3f> pos;
	std::vector<Vec3f> norm;
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
	std::vector col( pos.size(), aColor );
	return SimpleMeshData{ std::move(pos), std::move(col), std::move(norm) };
}