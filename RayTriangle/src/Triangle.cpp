#include "Triangle.h"
#include <ngl/Util.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/VAOFactory.h>
#include <ngl/MultiBufferVAO.h>

Triangle::Triangle(ngl::Vec3 _p0, ngl::Vec3 _p1,  ngl::Vec3 _p2)
{
  // set the default values
  m_u=0.0;
  m_v=0.0;
  m_w=0.0;
  m_v0=_p0;
  m_v1=_p1;
  m_v2=_p2;
  // calculate the edge
  m_edge1 = m_v1 - m_v0;
  m_edge2 = m_v2 - m_v0;
  // the center of the tri is the 3 verts average
  m_center=(m_v0+m_v1+m_v2)/3.0;
  m_hit=false;
  // create the m_points array for drawing the triangel
  m_points.push_back(m_v0);
  m_points.push_back(m_v1);
  m_points.push_back(m_v2);
  // now calculate the normal and create a normal array
  ngl::Vec3 normal=ngl::calcNormal(m_v0,m_v1,m_v2);

  m_normals.push_back(normal);
  m_normals.push_back(normal);
  m_normals.push_back(normal);
  // first we draw the triangle
  // we build up a vertex array for the lines of the start and end points and draw
  m_vao= ngl::VAOFactory::createVAO("multiBufferVAO",GL_TRIANGLES);
  m_vao->bind();
  m_vao->setData(ngl::MultiBufferVAO::VertexData(3*sizeof(ngl::Vec3),m_points[0].m_x));
  m_vao->setVertexAttributePointer(0,3,GL_FLOAT,sizeof(ngl::Vec3),0);
  m_vao->setData(ngl::MultiBufferVAO::VertexData(3*sizeof(ngl::Vec3),m_normals[0].m_x));
  m_vao->setVertexAttributePointer(1,3,GL_FLOAT,sizeof(ngl::Vec3),0);
  m_vao->setNumIndices(3);
  m_vao->unbind();
}

Triangle::~Triangle()
{
}

void Triangle::loadMatricesToShader( ngl::Transformation &_tx,const ngl::Mat4 &_globalMat, const ngl::Mat4 &_view , const ngl::Mat4 &_project) const
{

  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  M=_globalMat*_tx.getMatrix();
  MV=  _view*M;
  MVP= _project*MV;

  normalMatrix=MV;
  normalMatrix.inverse().transpose();
  ngl::ShaderLib::setUniform("MVP",MVP);
  ngl::ShaderLib::setUniform("normalMatrix",normalMatrix);
}



void Triangle::draw(const std::string &_shaderName, const ngl::Mat4 &_globalMat, const ngl::Mat4 &_view , const ngl::Mat4 &_project)
{

	// draw wireframe if hit
	if(m_hit)
	{
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	}


  ngl::ShaderLib::use(_shaderName);
  // grab an instance of the primitives for drawing
  ngl::Transformation t;

  loadMatricesToShader(t,_globalMat,_view,_project);
	m_vao->bind();
	m_vao->draw();
	m_vao->unbind();


	// draw the cube to indicate vertex 0
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	t.setPosition(m_v0);
  t.setScale(0.06f,0.06f,0.06f);
  loadMatricesToShader(t,_globalMat,_view,_project);
  ngl::VAOPrimitives::draw("cube");

   // draw the hit point
   if(m_hit)
   {
      t.setPosition(m_hitPoint);
      t.setScale(2.0,2.0,2.0);
      loadMatricesToShader(t,_globalMat,_view,_project);
      ngl::VAOPrimitives::draw("smallSphere");
   }

}


void Triangle::rayTriangleIntersect(ngl::Vec3 _rayStart,ngl::Vec3 _rayEnd )
{
  m_hit=false;
  // Calculate the ray direction
  ngl::Vec3 dir=_rayEnd-_rayStart;

  ngl::Vec3 tvec, pvec, qvec;
  float det, inv_det;
  // get the vector of the first edge
  pvec = dir.cross(m_edge2);
  // calculate the determinant
  det = m_edge1.dot(pvec);
  // if this is 0 no hit
  if (det > -0.00001f && det < 0.00001f)
  {
    return ;
  }
  // get the inverse det
  inv_det = 1.0f / det;
  // calculate the 2nd vector
  tvec = _rayStart - m_v0;
  // get the dot product of this and inv det
  m_u = tvec.dot(pvec) * inv_det;
  // if out of range no hit
  if (m_u < -0.001f || m_u > 1.001f)
  {
    return;
  }
  // check the 2nd vector edge
  qvec = tvec.cross(m_edge1);
  // get the dot product
  m_v = dir.dot(qvec) * inv_det;
  // if out of range no hit
  if (m_v < -0.001f || m_u + m_v > 1.001f)
  {
    return;
  }
  // check the final value
  m_w = m_edge2.dot(qvec) * inv_det;
  // if less than 0 no hit
  if (m_w <= 0)
  {
    return;
  }
  // otherwise we are inside the triangle
  // so get the hit point
  // see http://softsurfer.com/Archive/algorithm_0105/algorithm_0105.htm#intersect_RayTriangle()
  // get intersect point of ray with triangle plane
  // calculate the normal
  ngl::Vec3 n=ngl::calcNormal(m_v0,m_v1,m_v2);
  float a = -n.dot(tvec);
  float b = n.dot(dir);
  float r=a/b;
  // intersect point of ray and plane
  m_hitPoint=_rayStart + r * dir;
  m_hit=true;
}

