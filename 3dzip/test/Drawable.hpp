#ifndef DRAWABLE_HPP
#define DRAWABLE_HPP

class Drawable
{
public:
    virtual ~Drawable() {}
    virtual void draw() const = 0;
};

#endif
