#pragma once
#include "pmp/surface_mesh.h"

class DrawComponent
{
public:
    void Init();
    void Draw();
    void DrawSphere(float level);
    void reduceRadius();

    pmp::SurfaceMesh mesh;
    
    pmp::SurfaceMesh sphere;
    pmp::Point vmax, vmin, center;
    float x, y, z; //sphere center
    pmp::Scalar radius; //sphere radius
};

