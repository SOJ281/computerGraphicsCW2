#include "cylinder.hpp"

//#include "../vmlib/mat33.hpp"

SimpleMeshData make_cylinder( bool aCapped, std::size_t aSubdivs, Vec3f aColor, Mat44f aPreTransform ) {
	std::vector<Vec3f> pos;
	std::vector<Vec3f> norm;
	std::vector<Vec2f> tex;
	Mat33f const N = mat44_to_mat33( transpose(invert(aPreTransform)) );

	float prevY = (float)cos( 0.f );
	float prevZ = (float)sin( 0.f );

	for( std::size_t i = 0; i < aSubdivs; ++i ) {
		float const angle = (i+1) / float(aSubdivs) * 2.f * 3.1415926f;

		float y = (float)cos( angle );
		float z = (float)sin( angle );

		// Two triangles (= 3*2 positions) create one segment of the cylinder’s shell.
		pos.emplace_back( Vec3f{ 0.f, prevY, prevZ } );
		pos.emplace_back( Vec3f{ 0.f, y, z } );
		pos.emplace_back( Vec3f{ 1.f, prevY, prevZ } );

		norm.emplace_back( Vec3f{ 0.f, prevY, prevZ } );
		norm.emplace_back( Vec3f{ 0.f, y, z } );
		norm.emplace_back( Vec3f{ 0.f, prevY, prevZ } );

		tex.emplace_back( Vec2f{ 0.f, (float)((i)/(float)aSubdivs)} );
		tex.emplace_back( Vec2f{ 0.f, (float)((i+1)/(float)aSubdivs)} );
		tex.emplace_back( Vec2f{ 1.f, (float)((i)/(float)aSubdivs)} );



		pos.emplace_back( Vec3f{ 0.f, y, z } );
		pos.emplace_back( Vec3f{ 1.f, y, z } );
		pos.emplace_back( Vec3f{ 1.f, prevY, prevZ } );

		norm.emplace_back( Vec3f{ 0.f, y, z } );
		norm.emplace_back( Vec3f{ 0.f, y, z } );
		norm.emplace_back( Vec3f{ 0.f, prevY, prevZ } );

		tex.emplace_back( Vec2f{ 0.f, (float)((i+1)/(float)aSubdivs)} );
		tex.emplace_back( Vec2f{ 1.f, (float)((i+1)/(float)aSubdivs)} );
		tex.emplace_back( Vec2f{ 1.f, (float)((i)/(float)aSubdivs)} );


		if (aCapped) {
			pos.emplace_back( Vec3f{ 0.f, y, z } );
			pos.emplace_back( Vec3f{ 0.f, 0, 0 } );
			pos.emplace_back( Vec3f{ 0.f, prevY, prevZ } );

			norm.emplace_back( Vec3f{ -1.f, 0, 0 } );
			norm.emplace_back( Vec3f{ -1.f, 0, 0 } );
			norm.emplace_back( Vec3f{ -1.f, 0, 0 } );

			tex.emplace_back( Vec2f{ 0.f, (float)((i+1)/(float)aSubdivs)} );
			tex.emplace_back( Vec2f{ 0.f, 0.f} );
			tex.emplace_back( Vec2f{ 0.f, (float)((i)/(float)aSubdivs)} );



			pos.emplace_back( Vec3f{ 1.f, y, z } );
			pos.emplace_back( Vec3f{ 1.f, 0, 0 } );
			pos.emplace_back( Vec3f{ 1.f, prevY, prevZ } );

			norm.emplace_back( Vec3f{ 1.f, 0, 0 } );
			norm.emplace_back( Vec3f{ 1.f, 0, 0 } );
			norm.emplace_back( Vec3f{ 1.f, 0, 0 } );

			tex.emplace_back( Vec2f{ 1.f, (float)((i+1)/(float)aSubdivs)} );
			tex.emplace_back( Vec2f{ 1.f, 0.f} );
			tex.emplace_back( Vec2f{ 1.f, (float)((i)/(float)aSubdivs)});
		}

		prevY = y;
		prevZ = z;
	}
	printf("pos.positions.size() = %ld\n", pos.size());
	printf("tex.positions.size() = %ld", tex.size());
	//Vec3f operator*( Mat33f const& aLeft, Vec3f const& aRight )
	/**/
	//norm = calcNorms(pos);
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
	//for( std::size_t i = 0; i < aSubdivs; ++i ) {
	return SimpleMeshData{ std::move(pos), std::move(norm), std::move(tex) };

}

SimpleMeshData make_partial_building( bool aCapped, std::size_t aSubdivs, std::size_t cutOff, Vec3f aColor, Mat44f aPreTransform ) {
	std::vector<Vec3f> pos;
	std::vector<Vec3f> norm;
	std::vector<Vec2f> tex;
	printf("PARTIAL\n");
	Mat33f const N = mat44_to_mat33( transpose(invert(aPreTransform)) );

	float prevY = (float)cos( 0.f );
	float prevZ = (float)sin( 0.f );

	for( std::size_t i = 0; i < aSubdivs-cutOff; ++i ) {
		float const angle = (i+1) / float(aSubdivs) * 2.f * 3.1415926f;

		float y = (float)cos( angle );
		float z = (float)sin( angle );

		// 2x Two triangles (= 3*2 positions) create one segment of the cylinder’s shell.
		//interior
		pos.emplace_back( Vec3f{ 0.f, prevY, prevZ } );
		pos.emplace_back( Vec3f{ 0.f, y, z } );
		pos.emplace_back( Vec3f{ 1.f, prevY, prevZ } );

		norm.emplace_back( Vec3f{ 0.f, 1.f-prevY, 1.f-prevZ } );
		norm.emplace_back( Vec3f{ 0.f, 1.f-y, 1.f-z } );
		norm.emplace_back( Vec3f{ 0.f, 1.f-prevY, 1.f-prevZ } );

		tex.emplace_back( Vec2f{ 0.f, (float)((i)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 0.f, (float)((i+1)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 1.f, (float)((i)/aSubdivs)} );


		pos.emplace_back( Vec3f{ 0.f, y, z } );
		pos.emplace_back( Vec3f{ 1.f, y, z } );
		pos.emplace_back( Vec3f{ 1.f, prevY, prevZ } );

		norm.emplace_back( Vec3f{ 0.f, 1.f-y, 1.f-z } );
		norm.emplace_back( Vec3f{ 0.f, 1.f-y, 1.f-z } );
		norm.emplace_back( Vec3f{ 0.f, 1-prevY, 1-prevZ } );

		tex.emplace_back( Vec2f{ 0.f, (float)((i+1)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 1.f, (float)((i+1)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 1.f, (float)((i)/aSubdivs)} );

		//exterior
		pos.emplace_back( Vec3f{ 0.f, prevY*1.1f, prevZ*1.1f } );
		pos.emplace_back( Vec3f{ 0.f, y*1.1f, z*1.1f } );
		pos.emplace_back( Vec3f{ 1.f, prevY*1.1f, prevZ*1.1f } );

		norm.emplace_back( Vec3f{ 0.f, prevY, prevZ } );
		norm.emplace_back( Vec3f{ 0.f, y, z } );
		norm.emplace_back( Vec3f{ 0.f, prevY, prevZ } );

		tex.emplace_back( Vec2f{ 0.f, (float)((i)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 0.f, (float)((i+1)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 1.f, (float)((i)/aSubdivs)} );


		pos.emplace_back( Vec3f{ 0.f, y*1.1f, z*1.1f } );
		pos.emplace_back( Vec3f{ 1.f, y*1.1f, z*1.1f } );
		pos.emplace_back( Vec3f{ 1.f, prevY*1.1f, prevZ*1.1f } );

		norm.emplace_back( Vec3f{ 0.f, y, z } );
		norm.emplace_back( Vec3f{ 0.f, y, z } );
		norm.emplace_back( Vec3f{ 0.f, prevY, prevZ } );

		tex.emplace_back( Vec2f{ 0.f, (float)((i+1)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 1.f, (float)((i+1)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 1.f, (float)((i)/aSubdivs)} );
		

		//Link interior and exterior
		pos.emplace_back( Vec3f{ 0.f, prevY*1.1f, prevZ*1.1f } );
		pos.emplace_back( Vec3f{ 0.f, prevY, prevZ } );
		pos.emplace_back( Vec3f{ 1.f, prevY*1.1f, prevZ*1.1f } );

		norm.emplace_back( Vec3f{ 0.f, prevY, prevZ } );
		norm.emplace_back( Vec3f{ 0.f, y, z } );
		norm.emplace_back( Vec3f{ 0.f, prevY, prevZ } );

		tex.emplace_back( Vec2f{ 0.f, (float)((i)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 0.f, (float)((i+1)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 1.f, (float)((i)/aSubdivs)} );


		pos.emplace_back( Vec3f{ 1.f, prevY, prevZ } );
		pos.emplace_back( Vec3f{ 0.f, prevY, prevZ } );
		pos.emplace_back( Vec3f{ 1.f, prevY*1.1f, prevZ*1.1f } );

		norm.emplace_back( Vec3f{ 0.f, prevY, prevZ } );
		norm.emplace_back( Vec3f{ 0.f, y, z } );
		norm.emplace_back( Vec3f{ 0.f, prevY, prevZ } );

		tex.emplace_back( Vec2f{ 0.f, (float)((i)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 0.f, (float)((i+1)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 1.f, (float)((i)/aSubdivs)} );



		if (aCapped) {
			//interior
			pos.emplace_back( Vec3f{ 1.f, y, z } );
			pos.emplace_back( Vec3f{ 2.f, 0, 0 } );
			pos.emplace_back( Vec3f{ 1.f, prevY, prevZ } );

			norm.emplace_back( Vec3f{ 1.f, 1-y, 1-z } );
			norm.emplace_back( Vec3f{ 1.f, 0, 0 } );
			norm.emplace_back( Vec3f{ 1.f, 1-prevY, 1-prevZ } );

			tex.emplace_back( Vec2f{ 1.f, (float)((i+1)/aSubdivs)} );
			tex.emplace_back( Vec2f{ 1.f, 0.f} );
			tex.emplace_back( Vec2f{ 1.f, (float)((i)/aSubdivs)});

			//exterior
			pos.emplace_back( Vec3f{ 1.f, y*1.1f, z*1.1f } );
			pos.emplace_back( Vec3f{ 2.f*1.1f, 0, 0 } );
			pos.emplace_back( Vec3f{ 1.f, prevY*1.1f, prevZ*1.1f } );

			norm.emplace_back( Vec3f{ 1.f, y*1.1f, z*1.1f  } );
			norm.emplace_back( Vec3f{ 2.f*1.1f, 0, 0 } );
			norm.emplace_back( Vec3f{ 1.f, prevY*1.1f, prevZ*1.1f } );

			tex.emplace_back( Vec2f{ 1.f, (float)((i+1)/aSubdivs)} );
			tex.emplace_back( Vec2f{ 1.f, 0.f} );
			tex.emplace_back( Vec2f{ 1.f, (float)((i)/aSubdivs)});

			


			//interior
			pos.emplace_back( Vec3f{ 1.f, prevY*1.1f, prevZ*1.1f } );
			pos.emplace_back( Vec3f{ 2.f, 0, 0  } );
			pos.emplace_back( Vec3f{ 1.f, prevY, prevZ } );

			norm.emplace_back( Vec3f{ 1.f, 1-y, 1-z } );
			norm.emplace_back( Vec3f{ 1.f, 0, 0 } );
			norm.emplace_back( Vec3f{ 1.f, 1-prevY, 1-prevZ } );

			tex.emplace_back( Vec2f{ 1.f, (float)((i+1)/aSubdivs)} );
			tex.emplace_back( Vec2f{ 1.f, 0.f} );
			tex.emplace_back( Vec2f{ 1.f, (float)((i)/aSubdivs)});

			//exterior
			pos.emplace_back( Vec3f{ 2.f,  0, 0 } );
			pos.emplace_back( Vec3f{ 2.2f, 0, 0 } );
			pos.emplace_back( Vec3f{ 1.f, prevY*1.1f, prevZ*1.1f } );

			norm.emplace_back( Vec3f{ 1.f, y*1.1f, z*1.1f  } );
			norm.emplace_back( Vec3f{ 2.2f, 0, 0 } );
			norm.emplace_back( Vec3f{ 1.f, prevY*1.1f, prevZ*1.1f } );

			tex.emplace_back( Vec2f{ 1.f, (float)((i+1)/aSubdivs)} );
			tex.emplace_back( Vec2f{ 1.f, 0.f} );
			tex.emplace_back( Vec2f{ 1.f, (float)((i)/aSubdivs)});
		}

		prevY = y;
		prevZ = z;
	}
	//prevY = std::cos( 0.f );
	//prevZ = std::sin( 0.f );

	//float y = std::cos( angle );
	//float z = std::sin( angle );
	if (aCapped) {
		//Link interior and exterior
		pos.emplace_back( Vec3f{ 0.f, prevY*1.1f, prevZ*1.1f } );
		pos.emplace_back( Vec3f{ 0.f, prevY, prevZ } );
		pos.emplace_back( Vec3f{ 1.f, prevY*1.1f, prevZ*1.1f } );

		norm.emplace_back( Vec3f{ 0.f, prevY, prevZ } );
		norm.emplace_back( Vec3f{ 0.f, prevY, prevY } );
		norm.emplace_back( Vec3f{ 0.f, prevY, prevZ } );

		tex.emplace_back( Vec2f{ 0.f, (float)((aSubdivs)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 0.f, (float)((aSubdivs+1)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 1.f, (float)((aSubdivs)/aSubdivs)} );


		pos.emplace_back( Vec3f{ 1.f, prevY, prevZ } );
		pos.emplace_back( Vec3f{ 0.f, prevY, prevZ } );
		pos.emplace_back( Vec3f{ 1.f, prevY*1.1f, prevZ*1.1f } );

		norm.emplace_back( Vec3f{ 0.f, prevY, prevZ } );
		norm.emplace_back( Vec3f{ 0.f, prevY, prevY } );
		norm.emplace_back( Vec3f{ 0.f, prevY, prevZ } );

		tex.emplace_back( Vec2f{ 0.f, (float)((aSubdivs)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 0.f, (float)((aSubdivs+1)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 1.f, (float)((aSubdivs)/aSubdivs)} );
	}

	norm = calcNorms(pos);


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
	return SimpleMeshData{ std::move(pos), std::move(norm), std::move(tex) };
}


SimpleMeshData make_cone( bool aCapped, std::size_t aSubdivs,Vec3f aColor, Mat44f aPreTransform ) {
	std::vector<Vec3f> pos;
	std::vector<Vec3f> norm;
	std::vector<Vec2f> tex;
	printf("PARTIAL\n");
	Mat33f const N = mat44_to_mat33( transpose(invert(aPreTransform)) );

	float prevY = (float)cos( 0.f );
	float prevZ = (float)sin( 0.f );

	for( std::size_t i = 0; i < aSubdivs; ++i ) {
		float const angle = (i+1) / float(aSubdivs) * 2.f * 3.1415926f;

		float y = (float)cos( angle );
		float z = (float)sin( angle );


		//interior
		pos.emplace_back( Vec3f{ 0.f, y, z } );
		pos.emplace_back( Vec3f{ 2.f, 0, 0 } );
		pos.emplace_back( Vec3f{ 0.f, prevY, prevZ } );

		//norm.emplace_back( Vec3f{ 0.f, 1-y, 1-z } );
		//norm.emplace_back( Vec3f{ 0.f, 0, 0 } );
		//norm.emplace_back( Vec3f{ 0.f, 1-prevY, 1-prevZ } );

		tex.emplace_back( Vec2f{ 1.f, (float)((i+1)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 1.f, 0.f} );
		tex.emplace_back( Vec2f{ 1.f, (float)((i)/aSubdivs)});

		//interior
		pos.emplace_back( Vec3f{ 0.f, prevY*1.1f, prevZ*1.1f } );
		pos.emplace_back( Vec3f{ 1.f, 0, 0  } );
		pos.emplace_back( Vec3f{ 0.f, prevY, prevZ } );

		//norm.emplace_back( Vec3f{ 1.f, 1-y, 1-z } );
		//norm.emplace_back( Vec3f{ 1.f, 0, 0 } );
		//norm.emplace_back( Vec3f{ 1.f, 1-prevY, 1-prevZ } );

		tex.emplace_back( Vec2f{ 0.f, (float)((i+1)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 0.f, 0.f} );
		tex.emplace_back( Vec2f{ 0.f, (float)((i)/aSubdivs)});




		//exterior
		pos.emplace_back( Vec3f{ 0.f, y*1.1f, z*1.1f } );
		pos.emplace_back( Vec3f{ 1.f*1.1f, 0, 0 } );
		pos.emplace_back( Vec3f{ 0.f, prevY*1.1f, prevZ*1.1f } );

		//norm.emplace_back( Vec3f{ 0.f, y*1.1f, z*1.1f  } );
		//norm.emplace_back( Vec3f{ 1.f*1.1f, 0, 0 } );
		//norm.emplace_back( Vec3f{ 0.f, prevY*1.1f, prevZ*1.1f } );

		tex.emplace_back( Vec2f{ 1.f, (float)((i+1)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 1.f, 0.f} );
		tex.emplace_back( Vec2f{ 1.f, (float)((i)/aSubdivs)});

		//exterior
		pos.emplace_back( Vec3f{ 1.f,  0, 0 } );
		pos.emplace_back( Vec3f{ 1.1f, 0, 0 } );
		pos.emplace_back( Vec3f{ 0.f, prevY*1.1f, prevZ*1.1f } );

		//norm.emplace_back( Vec3f{ 1.f, y*1.1f, z*1.1f  } );
		//norm.emplace_back( Vec3f{ 2.2f, 0, 0 } );
		//norm.emplace_back( Vec3f{ 1.f, prevY*1.1f, prevZ*1.1f } );

		tex.emplace_back( Vec2f{ 1.f, (float)((i+1)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 1.f, 0.f} );
		tex.emplace_back( Vec2f{ 1.f, (float)((i)/aSubdivs)});


		//Bottom

		pos.emplace_back( Vec3f{ 0.f, y*1.1f, z*1.1f } );
		pos.emplace_back( Vec3f{ 0.f, y*1.1f, prevZ*1.1f } );
		pos.emplace_back( Vec3f{ 0.f, prevY*1.1f, prevZ*1.1f } );

		tex.emplace_back( Vec2f{ 1.f, (float)((i+1)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 1.f, 0.f} );
		tex.emplace_back( Vec2f{ 1.f, (float)((i)/aSubdivs)});

		
		pos.emplace_back( Vec3f{ 0.f, prevY*1.1f, prevZ*1.1f } );
		pos.emplace_back( Vec3f{ 0.f, prevY*1.1f, z*1.1f } );
		pos.emplace_back( Vec3f{ 0.f, y*1.1f, z*1.1f } );

		tex.emplace_back( Vec2f{ 1.f, (float)((i+1)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 1.f, 0.f} );
		tex.emplace_back( Vec2f{ 1.f, (float)((i)/aSubdivs)});



		prevY = y;
		prevZ = z;
	}


	norm = calcNorms(pos);


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
	return SimpleMeshData{ std::move(pos), std::move(norm), std::move(tex) };
}


SimpleMeshData make_partial_cylinder( bool aCapped, std::size_t aSubdivs, std::size_t cutOff, Vec3f aColor, Mat44f aPreTransform ) {
	std::vector<Vec3f> pos;
	std::vector<Vec3f> norm;
	std::vector<Vec2f> tex;
	printf("PARTIAL\n");
	Mat33f const N = mat44_to_mat33( transpose(invert(aPreTransform)) );

	float prevY = (float)cos( 0.f );
	float prevZ = (float)sin( 0.f );

	for( std::size_t i = 0; i < aSubdivs-cutOff; ++i ) {
		float const angle = (i+1) / float(aSubdivs) * 2.f * 3.1415926f;

		float y = (float)cos( angle );
		float z = (float)sin( angle );

		// 2x Two triangles (= 3*2 positions) create one segment of the cylinder’s shell.
		//interior
		pos.emplace_back( Vec3f{ 0.f, prevY, prevZ } );
		pos.emplace_back( Vec3f{ 0.f, y, z } );
		pos.emplace_back( Vec3f{ 1.f, prevY, prevZ } );

		norm.emplace_back( Vec3f{ 0.f, 1.f-prevY, 1.f-prevZ } );
		norm.emplace_back( Vec3f{ 0.f, 1.f-y, 1.f-z } );
		norm.emplace_back( Vec3f{ 0.f, 1.f-prevY, 1.f-prevZ } );

		tex.emplace_back( Vec2f{ 0.f, (float)((i)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 0.f, (float)((i+1)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 1.f, (float)((i)/aSubdivs)} );


		pos.emplace_back( Vec3f{ 0.f, y, z } );
		pos.emplace_back( Vec3f{ 1.f, y, z } );
		pos.emplace_back( Vec3f{ 1.f, prevY, prevZ } );

		norm.emplace_back( Vec3f{ 0.f, 1.f-y, 1.f-z } );
		norm.emplace_back( Vec3f{ 0.f, 1.f-y, 1.f-z } );
		norm.emplace_back( Vec3f{ 0.f, 1-prevY, 1-prevZ } );

		tex.emplace_back( Vec2f{ 0.f, (float)((i+1)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 1.f, (float)((i+1)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 1.f, (float)((i)/aSubdivs)} );


		//exterior
		pos.emplace_back( Vec3f{ 0.f, prevY*1.1f, prevZ*1.1f } );
		pos.emplace_back( Vec3f{ 0.f, y*1.1f, z*1.1f } );
		pos.emplace_back( Vec3f{ 1.f, prevY*1.1f, prevZ*1.1f } );

		norm.emplace_back( Vec3f{ 0.f, prevY, prevZ } );
		norm.emplace_back( Vec3f{ 0.f, y, z } );
		norm.emplace_back( Vec3f{ 0.f, prevY, prevZ } );

		tex.emplace_back( Vec2f{ 0.f, (float)((i)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 0.f, (float)((i+1)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 1.f, (float)((i)/aSubdivs)} );


		pos.emplace_back( Vec3f{ 0.f, y*1.1f, z*1.1f } );
		pos.emplace_back( Vec3f{ 1.f, y*1.1f, z*1.1f } );
		pos.emplace_back( Vec3f{ 1.f, prevY*1.1f, prevZ*1.1f } );

		norm.emplace_back( Vec3f{ 0.f, y, z } );
		norm.emplace_back( Vec3f{ 0.f, y, z } );
		norm.emplace_back( Vec3f{ 0.f, prevY, prevZ } );

		tex.emplace_back( Vec2f{ 0.f, (float)((i+1)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 1.f, (float)((i+1)/aSubdivs)} );
		tex.emplace_back( Vec2f{ 1.f, (float)((i)/aSubdivs)} );
		




		if (aCapped) {
			//interior
			pos.emplace_back( Vec3f{ 1.f, y, z } );
			pos.emplace_back( Vec3f{ 1.f, 0, 0 } );
			pos.emplace_back( Vec3f{ 1.f, prevY, prevZ } );

			norm.emplace_back( Vec3f{ 1.f, 1-y, 1-z } );
			norm.emplace_back( Vec3f{ 1.f, 0, 0 } );
			norm.emplace_back( Vec3f{ 1.f, 1-prevY, 1-prevZ } );

			tex.emplace_back( Vec2f{ 1.f, (float)((i+1)/aSubdivs)} );
			tex.emplace_back( Vec2f{ 1.f, 0.f} );
			tex.emplace_back( Vec2f{ 1.f, (float)((i)/aSubdivs)});

			//exterior
			pos.emplace_back( Vec3f{ 1.f, y*1.1f, z*1.1f } );
			pos.emplace_back( Vec3f{ 1.1f, 0, 0 } );
			pos.emplace_back( Vec3f{ 1.f, prevY*1.1f, prevZ*1.1f } );

			norm.emplace_back( Vec3f{ 1.f, y*1.1f, z*1.1f  } );
			norm.emplace_back( Vec3f{ 2.f*1.1f, 0, 0 } );
			norm.emplace_back( Vec3f{ 1.f, prevY*1.1f, prevZ*1.1f } );

			tex.emplace_back( Vec2f{ 1.f, (float)((i+1)/aSubdivs)} );
			tex.emplace_back( Vec2f{ 1.f, 0.f} );
			tex.emplace_back( Vec2f{ 1.f, (float)((i)/aSubdivs)});

		}

		prevY = y;
		prevZ = z;
	}
	//norm = calcNorms(pos);



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
	return SimpleMeshData{ std::move(pos), std::move(norm), std::move(tex) };
}


SimpleMeshData make_div_cylinder( bool aCapped, std::size_t aSubdivs, Vec3f aColor, Mat44f aPreTransform , std::size_t lSubdivs) {
	std::vector<Vec3f> pos;
	std::vector<Vec3f> norm;
	std::vector<Vec2f> tex;

	float prevY = (float)cos( 0.f );
	float prevZ = (float)sin( 0.f );
	Mat33f const N = mat44_to_mat33( transpose(invert(aPreTransform)) );


	for( std::size_t i = 0; i < aSubdivs; ++i ) {
		float const angle = (i+1) / float(aSubdivs) * 2.f * 3.1415926f;

		float y = (float)cos( angle );
		float z = (float)sin( angle );

		// Two triangles (= 3*2 positions) create one segment of the cylinder’s shell.

		float prevdp = 0.f;
		for( float i = 1; i <= lSubdivs; ++i ) {
			float dp = i/(lSubdivs);
			printf("%6.4lf,", dp);
			//printf("%6.4lf,", prevdp);
			pos.emplace_back( Vec3f{ prevdp, prevY, prevZ } );
			pos.emplace_back( Vec3f{ prevdp, y, z } );
			pos.emplace_back( Vec3f{ dp, prevY, prevZ } );

			norm.emplace_back( Vec3f{ prevdp, prevY, prevZ } );
			norm.emplace_back( Vec3f{ prevdp, y, z } );
			norm.emplace_back( Vec3f{ dp, prevY, prevZ } );

			tex.emplace_back( Vec2f{ prevdp, (float)((i)/(float)aSubdivs)} );
			tex.emplace_back( Vec2f{ prevdp, (float)((i+1)/(float)aSubdivs)} );
			tex.emplace_back( Vec2f{ prevdp, (float)((i)/(float)aSubdivs)} );





			pos.emplace_back( Vec3f{ prevdp, y, z } );
			pos.emplace_back( Vec3f{ dp, y, z } );
			pos.emplace_back( Vec3f{ dp, prevY, prevZ } );

			norm.emplace_back( Vec3f{ prevdp, y, z } );
			norm.emplace_back( Vec3f{ dp, y, z } );
			norm.emplace_back( Vec3f{ dp, prevY, prevZ } );

			tex.emplace_back( Vec2f{ prevdp, (float)((i+1)/(float)aSubdivs)} );
			tex.emplace_back( Vec2f{ dp, (float)((i+1)/(float)aSubdivs)} );
			tex.emplace_back( Vec2f{ dp, (float)((i)/(float)aSubdivs)} );
			prevdp = dp;
			if (aCapped) {
				pos.emplace_back( Vec3f{ prevdp, y, z } );
				pos.emplace_back( Vec3f{ prevdp, 0, 0 } );
				pos.emplace_back( Vec3f{ prevdp, prevY, prevZ } );

				norm.emplace_back( Vec3f{ -1.f, 0, 0 } );
				norm.emplace_back( Vec3f{ -1.f, 0, 0 } );
				norm.emplace_back( Vec3f{ -1.f, 0, 0 } );

				tex.emplace_back( Vec2f{ 0.f, (float)((i+1)/(float)aSubdivs)} );
				tex.emplace_back( Vec2f{ 0.f, 0.f} );
				tex.emplace_back( Vec2f{ 0.f, (float)((i)/(float)aSubdivs)} );



				pos.emplace_back( Vec3f{ prevdp, y, z } );
				pos.emplace_back( Vec3f{ prevdp, 0, 0 } );
				pos.emplace_back( Vec3f{ prevdp, prevY, prevZ } );

				norm.emplace_back( Vec3f{ 1.f, 0, 0 } );
				norm.emplace_back( Vec3f{ 1.f, 0, 0 } );
				norm.emplace_back( Vec3f{ 1.f, 0, 0 } );

				tex.emplace_back( Vec2f{ 1.f, (float)((i+1)/(float)aSubdivs)} );
				tex.emplace_back( Vec2f{ 1.f, 0.f} );
				tex.emplace_back( Vec2f{ 1.f, (float)((i)/(float)aSubdivs)});
			}
		}
		
		if (aCapped) {
			pos.emplace_back( Vec3f{ 0.f, y, z } );
			pos.emplace_back( Vec3f{ 0.f, 0, 0 } );
			pos.emplace_back( Vec3f{ 0.f, prevY, prevZ } );

			norm.emplace_back( Vec3f{ -1.f, 0, 0 } );
			norm.emplace_back( Vec3f{ -1.f, 0, 0 } );
			norm.emplace_back( Vec3f{ -1.f, 0, 0 } );

			tex.emplace_back( Vec2f{ 0.f, (float)((i+1)/(float)aSubdivs)} );
			tex.emplace_back( Vec2f{ 0.f, 0.f} );
			tex.emplace_back( Vec2f{ 0.f, (float)((i)/(float)aSubdivs)} );



			pos.emplace_back( Vec3f{ 1.f, y, z } );
			pos.emplace_back( Vec3f{ 1.f, 0, 0 } );
			pos.emplace_back( Vec3f{ 1.f, prevY, prevZ } );

			norm.emplace_back( Vec3f{ 1.f, 0, 0 } );
			norm.emplace_back( Vec3f{ 1.f, 0, 0 } );
			norm.emplace_back( Vec3f{ 1.f, 0, 0 } );

			tex.emplace_back( Vec2f{ 1.f, (float)((i+1)/(float)aSubdivs)} );
			tex.emplace_back( Vec2f{ 1.f, 0.f} );
			tex.emplace_back( Vec2f{ 1.f, (float)((i)/(float)aSubdivs)});
		}

		prevY = y;
		prevZ = z;
	}
	//norm = calcNorms(pos);
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
	return SimpleMeshData{ std::move(pos), std::move(norm), std::move(tex) };
}

SimpleMeshData make_bent_cylinder( bool aCapped, std::size_t aSubdivs, Vec3f aColor, Mat44f aPreTransform , std::size_t lSubdivs) {
	std::vector<Vec3f> pos;

	float prevY = (float)cos( 0.f );
	float prevZ = (float)sin( 0.f );

	for( std::size_t i = 0; i < aSubdivs; ++i ) {
		float const angle = (i+1) / float(aSubdivs) * 2.f * 3.1415926f;

		float y = (float)cos( angle );
		float z = (float)sin( angle );

		// Two triangles (= 3*2 positions) create one segment of the cylinder’s shell.

		float prevX = (float)cos( 0.f );
		float pY = (float)sin( 0.f );
		for( float i = 1; i <= lSubdivs; ++i ) {
			float ang = i/float(lSubdivs) * 3.1415926f;
			float x = (float)cos( ang );
			float ny = (float)sin( ang );
			printf("%6.4lf,", x);
			//printf("%6.4lf,", prevdp);
			pos.emplace_back( Vec3f{ prevX, prevY + pY, prevZ } );
			pos.emplace_back( Vec3f{ prevX, y + ny, z } );
			pos.emplace_back( Vec3f{ x, prevY + pY, prevZ } );

			pos.emplace_back( Vec3f{ prevX, y + ny, z } );
			pos.emplace_back( Vec3f{ x, y + ny, z } );
			pos.emplace_back( Vec3f{ x, prevY + pY, prevZ } );
			prevX = x;
			pY = ny;
		}
		/*
		if (aCapped) {
			pos.emplace_back( Vec3f{ 0.f, y, z } );
			pos.emplace_back( Vec3f{ 0.f, 0, 0 } );
			pos.emplace_back( Vec3f{ 0.f, prevY, prevZ } );

			pos.emplace_back( Vec3f{ 1.f, y, z } );
			pos.emplace_back( Vec3f{ 1.f, 0, 0 } );
			pos.emplace_back( Vec3f{ 1.f, prevY, prevZ } );
		}*/

		prevY = y;
		prevZ = z;
	}

	for( auto& p : pos ) {
		Vec4f p4{ p.x, p.y, p.z, 1.f };
		Vec4f t = aPreTransform * p4;
		t /= t.w;
		p = Vec3f{ t.x, t.y, t.z };
	}
	std::vector col( pos.size(), aColor );
	return SimpleMeshData{ std::move(pos), std::move(col) };

}
