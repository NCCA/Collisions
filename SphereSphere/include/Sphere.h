#ifndef SPHERE_H_
#define SPHERE_H_


#include <ngl/ShaderLib.h>
#include <ngl/Transformation.h>
#include <ngl/Vec3.h>

/*! \brief a simple sphere class */
class Sphere
{
public :
	/// @brief ctor
	/// @param pos the position of the sphere
	/// @param rad the radius of the sphere */
	Sphere(const  ngl::Vec3  &_pos,  const ngl::Vec3 &_dir,		GLfloat _rad	);
	Sphere();
  /// draw method
  void draw(const std::string &_shaderName, const ngl::Mat4 &_globalTx, const ngl::Mat4 &_view, const ngl::Mat4 &_project  )const ;
  void loadMatricesToShader(ngl::Transformation &_transform, const ngl::Mat4 &_globalTx, const ngl::Mat4 &_view, const ngl::Mat4 &_project   )const;
	inline void reverse(){m_dir=m_dir*-1.0;}
	inline void setHit(){m_hit=true;}
	inline void setNotHit(){m_hit=false;}
	inline bool isHit()const {return m_hit;}
  inline ngl::Vec3 getPos() const {return m_pos;}
  inline ngl::Vec3 getNextPos() const {return m_nextPos;}
	inline GLfloat getRadius() const {return m_radius;}
  inline void setDirection(ngl::Vec3 _d){m_dir=_d;}
  inline ngl::Vec3 getDirection() const { return m_dir;}
  inline void setColour(const ngl::Vec4 &_c)
  {m_colour=_c;}

	void move();
	/// set the sphere values
	/// @param[in] _pos the position to set
	/// @param[in] _dir the direction of the sphere
	/// @param[in] _rad the radius of the sphere

		void set( const ngl::Vec3 &_pos, const  ngl::Vec3 &_dir,	GLfloat _rad );
private :
	/*! the position of the sphere */
  ngl::Vec3 m_pos;
	/*! the radius of the sphere */
	GLfloat m_radius;
	/*! flag to indicate if the sphere has been hit by ray */
	bool m_hit;
	// the direction of the sphere
  ngl::Vec3 m_dir;
	// the last position of the sphere
  ngl::Vec3 m_lastPos;
	// the next position of the sphere
  ngl::Vec3 m_nextPos;
  /// @brief the colour of the sphere
  ngl::Vec4 m_colour;



};





#endif
