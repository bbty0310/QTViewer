#include "DrawComponent.h"
#include "BVH.h"
#include "pmp/io/io.h"
#include "pmp/bounding_box.h"
#include "pmp/algorithms/shapes.h"
#include "pmp/algorithms/normals.h"
#include "pmp/algorithms/differential_geometry.h"
#include "GL/glut.h"
#include "GL/freeglut.h"
#include <cmath>
#include <queue>
#include "boundingsphere.h"

using namespace std;
using namespace pmp;

vector<Face> faces;

void DrawComponent::Init()
{
	
	// load the model
	pmp::read(mesh, "models\\bunny_stanford.obj");
	pmp::vertex_normals(mesh);
	
	std::cout << "#f " << mesh.n_faces() << " #v " << mesh.n_vertices() << std::endl;

	radius = find_radius(mesh);
	center = find_center(mesh);
	printf("%f %f %f\n", center[0], center[1], center[2]);
	printf("%f\n", radius);
	

}

void DrawComponent::Draw()
{
	
	auto normals = mesh.vertex_property<pmp::Normal>("v:normal");
	loadSphereMesh(mesh, normals, center, radius, 1.5);
	
}

void DrawComponent::DrawSphere(float level)
{
	auto normals = mesh.vertex_property<pmp::Normal>("v:normal");

	sphere = uv_sphere(center, radius * level, 50, 50);
	
	for (auto f : sphere.faces())
	{
		faces.push_back(f);
		glBegin(GL_LINE_LOOP);
		for (auto v : sphere.vertices(f))
		{
			auto p = sphere.position(v);
			auto n = normals[v];
			glNormal3d(n[0], n[1], n[2]);
			glVertex3d(p[0], p[1], p[2]);

		}
		glEnd();
	}

}


