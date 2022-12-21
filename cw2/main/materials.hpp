#pragma once

struct Material {
	Vec3f ambient;
	Vec3f diffuse;
	Vec3f specular;
	Vec3f emissive;
	float shininess;
};

//Random guess
Material stone = {
	Vec3f{.01f, .01f, .01f},
	Vec3f{.4f, .4f, .4f},
	Vec3f{0.f, 0.f, 0.f},
	Vec3f{0.01f, 0.01f, 0.01f},
	.4f
};
Material vantaBlack = {
	Vec3f{0.f, 0.f, 0.f},
	Vec3f{0.f, 0.f, 0.f},
	Vec3f{0.f, 0.f, 0.f},
	Vec3f{0.0f, 0.0f, 0.0f},
	.4f
};
Material lampGlass = {
	Vec3f{0.4f, 0.4f, 0.4f},
	Vec3f{0.780392f, 0.568627f, 0.113725f},
	Vec3f{0.6f, 0.6f, 0.6f},
	Vec3f{0.05f, 0.05f, 0.05f}, //Most objects don't emit light
	0.8f
};

//For coursework objective
Material uranium = {
	Vec3f{.01f, .01f, .01f},
	Vec3f{0.f, 0.f, 0.f},
	Vec3f{0.f, 0.f, 0.f},
	Vec3f{1.f, .5f, 0.3f},
	.4f
};
Material shinyShiny = {
	Vec3f{.01f, .01f, .01f},
	Vec3f{0.5f, 0.5f, 0.f},
	Vec3f{1.f, 1.f, 0.f},
	Vec3f{0.f, 0.f, 0.f},
	.4f
};
Material highDiffuse = {
	Vec3f{.01f, .01f, .01f},
	Vec3f{1.f, 1.f, 0.f},
	Vec3f{0.f, 0.f, 0.f},
	Vec3f{0.f, 0.f, 0.f},
	.4f
};
//From Here http://devernay.free.fr/cours/opengl/materials.html
Material brass = {
	Vec3f{0.329412f, 0.223529f, 0.027451f},
	Vec3f{0.780392f, 0.568627f, 0.113725f},
	Vec3f{0.992157f, 0.941176f, 0.807843f},
	Vec3f{0.05f, 0.05f, 0.05f}, //Most objects don't emit light
	0.21794872f
};
//From MTL files:
Material cushion = {
	Vec3f{0.f, 0.f, 0.f},
	Vec3f{0.203922f, 0.305882f, 0.556863f},
	Vec3f{0.009961f, 0.009961f, 0.009961f},
	Vec3f{0.f, 0.f, 0.f},
	.4f
};
Material wood = {
	Vec3f{0.f, 0.f, 0.f},
	Vec3f{0.356863f, 0.223529f, 0.019608f},
	Vec3f{0.009961f, 0.009961, 0.009961f},
	Vec3f{0.f, 0.f, 0.f},
	.4f
};

inline
void setMaterial(Material material, ShaderProgram* prog)
{
	glUniform3fv(glGetUniformLocation(prog->programId(), "material.ambient"), 1, &material.ambient.x);
	glUniform3fv(glGetUniformLocation(prog->programId(), "material.diffuse"), 1, &material.diffuse.x);
	glUniform3fv(glGetUniformLocation(prog->programId(), "material.specular"), 1, &material.specular.x);
	glUniform3fv(glGetUniformLocation(prog->programId(), "material.emissive"), 1, &material.emissive.x);
	glUniform1f(glGetUniformLocation(prog->programId(), "material.shininess"), material.shininess);
}