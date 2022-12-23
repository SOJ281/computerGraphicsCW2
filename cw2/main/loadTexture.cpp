#include "loadTexture.hpp"

#include <stb_image.h>
#include <stb_image_write.h>

//#include <unistd.h>
#include <io.h>

#include "../support/error.hpp"


unsigned int loadTexture(char const * path) {
	
    assert( path );
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    //if( !data )
    //    throw Error( "Unable to load image ’%s’\n", path );
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

unsigned int loadCubeMap(std::vector<std::string> faces) {

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
        GLenum format = 0;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            printf("Cubemap texture failed to load at: %s\n", faces[i].c_str());
        }
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


    return textureID;
}


unsigned int createTexture(int width, int height, uint8_t * pixels) {
	int CHANNEL_NUM = 3;


	//std::string number = std::to_string(i);
    int fullSize = width * height * CHANNEL_NUM;
    uint8_t* reversedPixels = new uint8_t[fullSize];
    //for (int i = 0; i < windowWidth * windowHeight * 3; i++)
    for (int i = 0; i < fullSize; i += 3) {
        reversedPixels[fullSize - i - 3] = pixels[i];
        reversedPixels[fullSize - i - 2] = pixels[i+1];
        reversedPixels[fullSize - i -1] = pixels[i+2];
    }

    int c = 1;
    //char* fileName = "screenshot"+ (char*)c+".png";
    //for (;access(("pointLights["+std::to_string(c)+"].position").c_str(), F_OK ); c++);
    int wid, heig, nrComponents;
    //for (;stbi_load(("Screenshot"+std::to_string(c)+".png").c_str(), &wid, &heig, &nrComponents, 0); c++);
    //for (; access( ("Screenshot"+std::to_string(c)+".png").c_str(), F_OK ) != -1 ; c++);
    for (; access(("Screenshot" + std::to_string(c) + ".png").c_str(), 00) != -1; c++);
    stbi_write_png(("Screenshot"+std::to_string(c)+".png").c_str(), width, height, CHANNEL_NUM, reversedPixels, width * CHANNEL_NUM);
    return 1;
}