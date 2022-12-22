#include <vector>

#include <cstdlib>

#include "simple_mesh.hpp"
#include "cylinder.hpp"

#include "../vmlib/vec3.hpp"
#include "../vmlib/mat44.hpp"


struct objectCentres {
    Vec3f head;
    int headCount;
    Vec3f body;
    int bodyCount;
    //Vec3f centre;
    std::vector<Vec3f> legs;
    std::vector<Vec3f> legTop;
    int legCount;
    int segment;
    int segHeight;
    int legNo;
};

struct JellyModel {
    SimpleMeshData data;
    objectCentres centreData;
};

//SimpleMeshData make_partial_cylinder( bool aCapped, std::size_t aSubdivs, Vec3f aColor, Mat44f aPreTransform );
JellyModel make_jellyfish(std::size_t legs, Mat44f aPreTransform );
SimpleMeshData make_dome(std::size_t aSubdivs, Mat44f aPreTransform );

SimpleMeshData createLeg(Mat44f aPreTransform);
