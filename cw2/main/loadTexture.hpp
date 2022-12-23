#include "simple_mesh.hpp"
#include <string>


unsigned int loadTexture(char const * path);
unsigned int loadCubeMap(std::vector<std::string> faces);
unsigned int createTexture(int width, int height, uint8_t * pixels);