#ifndef SCENE_HPP
#define SCENE_HPP

#include <stdexcept>

class Scene
{
    unsigned num_meshes;
    float** ver;
    unsigned **tri;
    unsigned *num_ver, *num_tri;

    float** over;
    unsigned **otri;
    unsigned *onum_ver, *onum_tri;

    unsigned* vao;
    unsigned* vbo;

    void build_3d(const std::string& filename);
    void build_3d(std::istream &is);
    void compress(unsigned i, std::stringstream& ss);
    void uncompress(unsigned i, std::stringstream& ss);
    void exception_handle(unsigned i, const std::exception& e);
    void normalize(const int idx);

public:
    Scene(const std::string& filename);
    ~Scene();
    unsigned getNumMeshes() const {
	return num_meshes;
    }
    unsigned getNumTri(const unsigned i) {
	if (i >= num_meshes)
	    throw std::out_of_range("Invalid part index");
	return onum_tri[i];
    }
    virtual void draw(unsigned i = 13, const bool show_orig = false) const;
};

#endif
