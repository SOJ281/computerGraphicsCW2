
#include "jellyfish.hpp"
#include "cube.hpp"

JellyModel make_jellyfish(std::size_t legs, Mat44f aPreTransform ) {
    JellyModel model;
    SimpleMeshData fullBody = make_cylinder( true, 18, {.5f, 0.5f, .5f}, make_rotation_z( 3.141592f / 2.f ));
    fullBody = make_change(fullBody, make_scaling( 1.5f, 3.f, 1.5f ));
    fullBody = make_change(fullBody, make_translation( {0.f, -3.f, 0.f }));
    fullBody = make_change(fullBody, aPreTransform);
    model.centreData.body = getMean(fullBody);
    model.centreData.bodyCount = fullBody.positions.size();

    SimpleMeshData head = make_dome(16, aPreTransform );
    model.centreData.head = getMean(head);
    model.centreData.headCount = head.positions.size();

    fullBody = concatenate( head , fullBody );


    for (int i = 0; i < (int)legs; i++) {
        float const angle = (i+1) / float(legs) * 2.f * 3.1415926f;
		float z = (float)cos(angle);
        float x = (float)sin(angle);
        SimpleMeshData leg = createLeg(aPreTransform);
        leg = make_change(leg, make_scaling( .05f, 7.f, .05f ));
        leg = make_change(leg, make_translation( {x, -9.5f, z }));
        leg = make_change(leg, aPreTransform);
        model.centreData.legs.emplace_back(getMean(leg));
        model.centreData.legTop.emplace_back( Vec3f {x, -12.f, z });
        model.centreData.legCount = leg.positions.size();
        fullBody = concatenate( fullBody, leg );
    }
    model.centreData.legNo = legs;
    model.centreData.segment = make_cylinder( true, 18, {.5f, 0.5f, .5f}, make_rotation_z( 3.141592f / 2.f )).positions.size();

    model.data = fullBody;
    //model.centreData = fullBody;

    return model;
}

SimpleMeshData createLeg(Mat44f aPreTransform) {
    SimpleMeshData leg = make_cylinder( true, 18, {.5f, 0.5f, .5f}, make_rotation_z( 3.141592f / 2.f ));

    for (float i = 6; i > 0 ; i--) {
        SimpleMeshData segment = make_cylinder( true, 18, {.5f, 0.5f, .5f}, make_rotation_z( 3.141592f / 2.f ));
        segment = make_change(segment, make_translation( {0.f, -.1f-i*.1f, 0.f }));
        leg = concatenate( leg, segment );
		////printf("seg%ld\n", segment.positions.size());
    }
	/*
	//printf("leg%ld\n", leg.positions.size());
	for (int i = 0; i < leg.positions.size(); i++) {
		//printf("LEGGY=%f,%f,%f\n", leg.positions[i].x, leg.positions[i].y, leg.positions[i].z);
	}
	//printf("\n\n\n");
	*/
    return leg;

}

//Not a dome
SimpleMeshData make_dome(std::size_t aSubdivs, Mat44f aPreTransform ) {
	std::vector<Vec3f> pos;
	std::vector<Vec3f> norm;
	std::vector<Vec2f> tex;
	Mat33f const N = mat44_to_mat33( transpose(invert(aPreTransform)) );

	int originSize = sizeof(kCubePositions)/sizeof(kCubePositions[0]);
	//float rightKCube[sizeof(kCubePositions)/sizeof(kCubePositions[0]) * 2];// = new float[kCubePositions.length];
	for (int i = 0; i < originSize-12; i+=3) {
		pos.emplace_back( Vec3f{ kCubePositions[i], kCubePositions[i+1], kCubePositions[i+2] } );
		//norm.emplace_back( Vec3f{ kCubePositions[i], kCubePositions[i+1], kCubePositions[i+2] } );
		//norm.emplace_back( Vec3f{ 0.f, prevY, prevZ } );
	}
	norm = calcNorms(pos);

	for (int i = 0; i < 5; i++) {
		tex.emplace_back( Vec2f{ 0.f, 0.f} );
		tex.emplace_back( Vec2f{ 1.f, 0.f} );
		tex.emplace_back( Vec2f{ 1.f, 1.f} );

		tex.emplace_back( Vec2f{ 0.f, 0.f} );
		tex.emplace_back( Vec2f{ 1.f, 1.f} );
		tex.emplace_back( Vec2f{ 0.f, 1.f} );
	}

	
	for( auto& n : norm ) {
		Vec3f t = N * n;
		n = t;
	}
	
	for( auto& p : pos ) {
		Vec4f p4{ p.x, p.y, p.z, 1.f };
		Vec4f t = make_rotation_x(-3.14/2) * p4;
		t = make_scaling( 2.f, 2.f, 2.f ) * t;
		t = aPreTransform * t;
		t /= t.w;
		p = Vec3f{ t.x, t.y, t.z };
	}
	return SimpleMeshData{ std::move(pos), std::move(norm), std::move(tex) };
}
