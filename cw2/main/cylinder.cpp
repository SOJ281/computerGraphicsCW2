#include "cylinder.hpp"

//#include "../vmlib/mat33.hpp"

SimpleMeshData make_cylinder( bool aCapped, std::size_t aSubdivs, Vec3f aColor, Mat44f aPreTransform ) {
	std::vector<Vec3f> pos;
	std::vector<Vec3f> norm;
	Mat33f const N = mat44_to_mat33( transpose(invert(aPreTransform)) );

	float prevY = std::cos( 0.f );
	float prevZ = std::sin( 0.f );

	for( std::size_t i = 0; i < aSubdivs; ++i ) {
		float const angle = (i+1) / float(aSubdivs) * 2.f * 3.1415926f;

		float y = std::cos( angle );
		float z = std::sin( angle );

		// Two triangles (= 3*2 positions) create one segment of the cylinder’s shell.
		pos.emplace_back( Vec3f{ 0.f, prevY, prevZ } );
		pos.emplace_back( Vec3f{ 0.f, y, z } );
		pos.emplace_back( Vec3f{ 1.f, prevY, prevZ } );

		norm.emplace_back( Vec3f{ 0.f, prevY, prevZ } );
		norm.emplace_back( Vec3f{ 0.f, y, z } );
		norm.emplace_back( Vec3f{ 0.f, prevY, prevZ } );


		pos.emplace_back( Vec3f{ 0.f, y, z } );
		pos.emplace_back( Vec3f{ 1.f, y, z } );
		pos.emplace_back( Vec3f{ 1.f, prevY, prevZ } );

		norm.emplace_back( Vec3f{ 0.f, y, z } );
		norm.emplace_back( Vec3f{ 0.f, y, z } );
		norm.emplace_back( Vec3f{ 0.f, prevY, prevZ } );

		if (aCapped) {
			pos.emplace_back( Vec3f{ 0.f, y, z } );
			pos.emplace_back( Vec3f{ 0.f, 0, 0 } );
			pos.emplace_back( Vec3f{ 0.f, prevY, prevZ } );

			norm.emplace_back( Vec3f{ -1.f, 0, 0 } );
			norm.emplace_back( Vec3f{ -1.f, 0, 0 } );
			norm.emplace_back( Vec3f{ -1.f, 0, 0 } );

			pos.emplace_back( Vec3f{ 1.f, y, z } );
			pos.emplace_back( Vec3f{ 1.f, 0, 0 } );
			pos.emplace_back( Vec3f{ 1.f, prevY, prevZ } );

			norm.emplace_back( Vec3f{ 1.f, 0, 0 } );
			norm.emplace_back( Vec3f{ 1.f, 0, 0 } );
			norm.emplace_back( Vec3f{ 1.f, 0, 0 } );
		}

		prevY = y;
		prevZ = z;
	}
	//Vec3f operator*( Mat33f const& aLeft, Vec3f const& aRight )
	/**/
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
	return SimpleMeshData{ std::move(pos), std::move(col), std::move(norm) };

}

SimpleMeshData make_div_cylinder( bool aCapped, std::size_t aSubdivs, Vec3f aColor, Mat44f aPreTransform , std::size_t lSubdivs) {
	std::vector<Vec3f> pos;

	float prevY = std::cos( 0.f );
	float prevZ = std::sin( 0.f );

	for( std::size_t i = 0; i < aSubdivs; ++i ) {
		float const angle = (i+1) / float(aSubdivs) * 2.f * 3.1415926f;

		float y = std::cos( angle );
		float z = std::sin( angle );

		// Two triangles (= 3*2 positions) create one segment of the cylinder’s shell.

		float prevdp = 0.f;
		for( float i = 1; i <= lSubdivs; ++i ) {
			float dp = i/(lSubdivs);
			printf("%6.4lf,", dp);
			//printf("%6.4lf,", prevdp);
			pos.emplace_back( Vec3f{ prevdp, prevY, prevZ } );
			pos.emplace_back( Vec3f{ prevdp, y, z } );
			pos.emplace_back( Vec3f{ dp, prevY, prevZ } );

			pos.emplace_back( Vec3f{ prevdp, y, z } );
			pos.emplace_back( Vec3f{ dp, y, z } );
			pos.emplace_back( Vec3f{ dp, prevY, prevZ } );
			prevdp = dp;
		}

		if (aCapped) {
			pos.emplace_back( Vec3f{ 1.f, y, z } );
			pos.emplace_back( Vec3f{ 1.f, 0, 0 } );
			pos.emplace_back( Vec3f{ 1.f, prevY, prevZ } );

			
			pos.emplace_back( Vec3f{ 0.f, y, z } );
			pos.emplace_back( Vec3f{ 0.f, 0, 0 } );
			pos.emplace_back( Vec3f{ 0.f, prevY, prevZ } );
		}

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

SimpleMeshData make_bent_cylinder( bool aCapped, std::size_t aSubdivs, Vec3f aColor, Mat44f aPreTransform , std::size_t lSubdivs) {
	std::vector<Vec3f> pos;

	float prevY = std::cos( 0.f );
	float prevZ = std::sin( 0.f );

	for( std::size_t i = 0; i < aSubdivs; ++i ) {
		float const angle = (i+1) / float(aSubdivs) * 2.f * 3.1415926f;

		float y = std::cos( angle );
		float z = std::sin( angle );

		// Two triangles (= 3*2 positions) create one segment of the cylinder’s shell.

		float prevX = std::cos( 0.f );
		float pY = std::sin( 0.f );
		for( float i = 1; i <= lSubdivs; ++i ) {
			float ang = i/float(lSubdivs) * 3.1415926f;
			float x = std::cos( ang );
			float ny = std::sin( ang );
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
