#ifndef SCENE_HPP
#define SCENE_HPP

#include <string>
#include "Drawable.hpp"

class Scene : public Drawable
{
    unsigned onum_ver, onum_tri;

    static const int N_BO = 2;
    unsigned bo[N_BO];

public:
    Scene(const std::string &filename);
    ~Scene();
    virtual void draw() const;
};

#endif
