#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/Random.h>
#include <ngl/ShaderLib.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>



NGLScene::NGLScene(int _numSpheres)
{
  m_numSpheres=_numSpheres;
  m_rayUpdateTimer=startTimer(50);
  m_animate=true;
  // now create the actual spheres for our program
  float x;
  float y;

	for (int i=0; i<m_numSpheres; ++i)
	{
		x=ngl::Random::randomNumber(10);
		y=ngl::Random::randomNumber(8);

		m_sphereArray.push_back(Sphere(ngl::Vec3(x,y,0),ngl::Random::randomPositiveNumber(1)+0.2));
	}

	// create the points for our ray
	m_rayStart.set(0,10,0);
	m_rayEnd.set(0,-5,0);
	m_rayStart2.set(0,0,20);
	m_rayEnd2.set(0,0,-5);
	setTitle("Ray->Sphere intersetions");

}


NGLScene::~NGLScene()
{
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
}

void NGLScene::resizeGL( int _w, int _h )
{
  m_project=ngl::perspective( 45.0f, static_cast<float>( _w ) / _h, 0.05f, 350.0f );
  m_win.width  = static_cast<int>( _w * devicePixelRatio() );
  m_win.height = static_cast<int>( _h * devicePixelRatio() );
}

void NGLScene::initializeGL()
{
  // we must call this first before any other GL commands to load and link the
  // gl commands from the lib, if this is not done program will crash
  ngl::NGLInit::initialize();

  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);
  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from(0.0f,0.0f,-25.0f);
  ngl::Vec3 to(0.0f,0.0f,0.0f);
  ngl::Vec3 up(0.0f,1.0f,0.0f);
  m_view=ngl::lookAt(from,to,up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_project=ngl::perspective(45.0f,(float)720.0f/576.0f,0.5f,150.0f);
  ngl::ShaderLib::use("nglDiffuseShader");

  ngl::ShaderLib::setUniform("Colour",1.0f,1.0f,1.0f,1.0f);
  ngl::ShaderLib::setUniform("lightPos",1.0f,1.0f,1.0f);
  ngl::ShaderLib::setUniform("lightDiffuse",1.0f,1.0f,1.0f,1.0f);

  ngl::ShaderLib::use("nglColourShader");
  ngl::ShaderLib::setUniform("Colour",1.0f,1.0f,1.0f,1.0f);

  glEnable(GL_DEPTH_TEST); // for removal of hidden surfaces

  ngl::VAOPrimitives::createSphere("sphere",1.0,40);
  ngl::VAOPrimitives::createSphere("smallSphere",0.2,10);


  // as re-size is not explicitly called we need to do this.
  glViewport(0,0,width(),height());
}


void NGLScene::loadMatricesToShader()
{
  ngl::ShaderLib::use("nglDiffuseShader");

  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  M=m_transform.getMatrix();
  MV=  m_view*m_mouseGlobalTX*M;
  MVP=m_project*MV;
  normalMatrix=MV;
  normalMatrix.inverse().transpose();
  ngl::ShaderLib::setUniform("MVP",MVP);
  ngl::ShaderLib::setUniform("normalMatrix",normalMatrix);
}

void NGLScene::loadMatricesToColourShader()
{
   ngl::ShaderLib::use("nglColourShader");
   ngl::Mat4 MVP;
   MVP=m_project*m_view*m_mouseGlobalTX*m_transform.getMatrix();
   ngl::ShaderLib::setUniform("MVP",MVP);
}


void NGLScene::paintGL()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   // Rotation based on the mouse position for our global
   // transform
   ngl::Mat4 rotX;
   ngl::Mat4 rotY;
   // create the rotation matrices
   rotX.rotateX(m_win.spinXFace);
   rotY.rotateY(m_win.spinYFace);
   // multiply the rotations
   m_mouseGlobalTX=rotY*rotX;
   // add the translations
   m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
   m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
   m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;

  ngl::ShaderLib::use("nglDiffuseShader");
  // draw a cube at the ray start points
  m_transform.reset();
  {
    ngl::ShaderLib::setUniform("Colour",1.0f,1.0f,1.0f,1.0f);
    m_transform.setPosition(m_rayStart);
    loadMatricesToShader();
    ngl::VAOPrimitives::draw("cube");
  }

	m_transform.reset();
	{
		m_transform.setPosition(m_rayStart2);
		loadMatricesToShader();
		ngl::VAOPrimitives::draw("cube");
	}



	for(Sphere s : m_sphereArray)
	{
    ngl::ShaderLib::setUniform("Colour",1.0f,1.0f,0.0f,1.0f);

    s.draw("nglDiffuseShader",m_mouseGlobalTX,m_view,m_project);
		if(s.isHit())
		{
			ngl::Vec3 dir=m_rayEnd-m_rayStart;
			ngl::Vec3 dir2=m_rayEnd2-m_rayStart2;
			drawHitPoints(m_rayStart,dir,s.getPos(),s.getRadius());
			drawHitPoints(m_rayStart2,dir2,s.getPos(),s.getRadius());
		}
	}
	// we build up a VAO for the lines of the start and end points and draw
	m_transform.reset();
	{
		ngl::ShaderLib::use("nglColourShader");
    ngl::ShaderLib::setUniform("Colour",1.0f,1.0f,1.0f,1.0f);
    std::unique_ptr<ngl::AbstractVAO> vao( ngl::VAOFactory::createVAO("simpleVAO",GL_LINES));

    vao->bind();
		ngl::Vec3 points[4];
		points[0]=m_rayStart;
		points[1]=m_rayEnd;
		points[2]=m_rayStart2;
		points[3]=m_rayEnd2;
    vao->setData(ngl::SimpleVAO::VertexData(4*sizeof(ngl::Vec3),points[0].m_x));
		vao->setVertexAttributePointer(0,3,GL_FLOAT,sizeof(ngl::Vec3),0);
		vao->setNumIndices(4);
		loadMatricesToColourShader();
		vao->draw();
    vao->removeVAO();
	}
}


//----------------------------------------------------------------------------------------------------------------------
void NGLScene::drawHitPoints( ngl::Vec3 _raystart,  ngl::Vec3 _raydir,   ngl::Vec3 _pos,  GLfloat _radius	)
{
GLfloat A,B,C,discrim;
ngl::Vec3 p;
GLfloat discRoot;
GLfloat t1 ;        // earlier hit
GLfloat t2 ; 		// second hit
// points for the hit points
ngl::Vec3 h1 ;
ngl::Vec3 h2 ;

// normalize the ray
_raydir.normalize();
// cal the A value as the dotproduct a.a (see lecture notes)
A = _raydir.dot(_raydir);
//b= 2*d.(Po-Pc)
p=_raystart-_pos;
B= _raydir.dot(p)*2.0;
// C = (Po-Pc).(Po-Pc)-r^2
C=p.dot(p)-_radius*_radius;
// finally get the descrim
// b^2-4(ac)
discrim = B * B - 4.0*(A * C);
// if the discrim <= 0.0 it's not a hit
if(discrim >= 0.0)
{
	discRoot = sqrt(discrim);
	t1 = (-B-(discRoot))/2*A;        // earlier hit
	t2 = (-B+(discRoot))/2*A; 		// second hit
	// move along the ray from origin in ray direction with an offset of t1 / t2
	// to get the hit points
	h1=_raystart + (_raydir*t1);
	h2=_raystart + (_raydir*t2);
	// draw the hit points
	ngl::ShaderLib::setUniform("Colour",1.0f,0.0f,0.0f,0.0f);
	m_transform.reset();
	{
		m_transform.setPosition(h1);
		loadMatricesToShader();
		ngl::VAOPrimitives::draw("smallSphere");
	}

	m_transform.reset();
	{
    ngl::ShaderLib::setUniform("Colour",0.0f,1.0f,0.0f,0.0f);
		m_transform.setPosition(h2);

    loadMatricesToShader();
    ngl::VAOPrimitives::draw("smallSphere");
  }
}
}
//----------------------------------------------------------------------------------------------------------------------
bool NGLScene::raySphere(ngl::Vec3 _rayStart, ngl::Vec3 _rayDir,  ngl::Vec3 _pos,  GLfloat _radius  )
{
  // variables for the Quadratic roots and discriminator
  GLfloat A,B,C,discrim;
  ngl::Vec3 p;
  // normalize the ray
  _rayDir.normalize();
  // cal the A value as the dotproduct a.a (see lecture notes)
  A = _rayDir.dot(_rayDir);
  //b= 2*d.(Po-Pc)
  p=_rayStart-_pos;
  B= _rayDir.dot(p)*2;
  // C = (Po-Pc).(Po-Pc)-r^2
  C=p.dot(p)-_radius*_radius;
  // finally get the descrim
  // b^2-4(ac)
  discrim = B * B - 4*(A * C);
  // if the discrim <= 0.0 it's not a hit
  if(discrim <= 0.0)
  {
    return false;
  }
  else
  {
    return true;
  }
}



//----------------------------------------------------------------------------------------------------------------------
void NGLScene::updateScene()
{
  enum {FWD,BWD};

  static int s_direction=0;
  bool collide;
  ngl::Vec3 dir;
  ngl::Vec3 dir2;
  dir=m_rayEnd-m_rayStart;
  dir2=m_rayEnd2-m_rayStart2;

  // note here we need to iterate by reference as we want to modify the Sphere Objects
  // so we need to explicitly create our object as a reference object
  for(Sphere &s : m_sphereArray)
  {
    s.setNotHit();
    collide =raySphere(m_rayStart,dir,s.getPos(),s.getRadius());
    if(collide)
    {
      s.setHit();
    }
    collide =raySphere(m_rayStart2,dir2,s.getPos(),s.getRadius());
    if(collide)
    {
      s.setHit();
    }
  }

  // now update the rays
  if(s_direction==FWD)
  {
    m_rayEnd.m_x+=0.5;
    m_rayEnd2.m_x-=0.5;
    if(m_rayEnd.m_x>22.0)
    {
      s_direction=BWD;
    }
  }
  else
  {
    m_rayEnd.m_x-=0.5;
    m_rayEnd2.m_x+=0.5;
    if(m_rayEnd.m_x<=-22.0)
    {
      s_direction=FWD;
    }
  }
}


//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseMoveEvent( QMouseEvent* _event )
{
  // note the method buttons() is the button state when event was called
  // that is different from button() which is used to check which button was
  // pressed when the mousePress/Release event is generated
  if ( m_win.rotate && _event->buttons() == Qt::LeftButton )
  {
    int diffx = _event->x() - m_win.origX;
    int diffy = _event->y() - m_win.origY;
    m_win.spinXFace += static_cast<int>( 0.5f * diffy );
    m_win.spinYFace += static_cast<int>( 0.5f * diffx );
    m_win.origX = _event->x();
    m_win.origY = _event->y();
    update();
  }
  // right mouse translate code
  else if ( m_win.translate && _event->buttons() == Qt::RightButton )
  {
    int diffX      = static_cast<int>( _event->x() - m_win.origXPos );
    int diffY      = static_cast<int>( _event->y() - m_win.origYPos );
    m_win.origXPos = _event->x();
    m_win.origYPos = _event->y();
    m_modelPos.m_x += INCREMENT * diffX;
    m_modelPos.m_y -= INCREMENT * diffY;
    update();
  }
}


//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mousePressEvent( QMouseEvent* _event )
{
  // that method is called when the mouse button is pressed in this case we
  // store the value where the maouse was clicked (x,y) and set the Rotate flag to true
  if ( _event->button() == Qt::LeftButton )
  {
    m_win.origX  = _event->x();
    m_win.origY  = _event->y();
    m_win.rotate = true;
  }
  // right mouse translate mode
  else if ( _event->button() == Qt::RightButton )
  {
    m_win.origXPos  = _event->x();
    m_win.origYPos  = _event->y();
    m_win.translate = true;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseReleaseEvent( QMouseEvent* _event )
{
  // that event is called when the mouse button is released
  // we then set Rotate to false
  if ( _event->button() == Qt::LeftButton )
  {
    m_win.rotate = false;
  }
  // right mouse translate mode
  if ( _event->button() == Qt::RightButton )
  {
    m_win.translate = false;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::wheelEvent( QWheelEvent* _event )
{

  // check the diff of the wheel position (0 means no change)
  if ( _event->delta() > 0 )
  {
    m_modelPos.m_z += ZOOM;
  }
  else if ( _event->delta() < 0 )
  {
    m_modelPos.m_z -= ZOOM;
  }
  update();
}
//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quite
  case Qt::Key_Escape : QGuiApplication::exit(EXIT_SUCCESS); break;
  // show full screen
  case Qt::Key_F : showFullScreen(); break;
  // show windowed
  case Qt::Key_N : showNormal(); break;
  case  Qt::Key_Space : m_animate^=true; break;

  default : break;
  }
  // finally update the GLWindow and re-draw
  //if (isExposed())
    update();
}

void NGLScene::timerEvent(QTimerEvent *_event )
{
	if(_event->timerId() == m_rayUpdateTimer)
	{
		if (m_animate !=true)
		{
			return;
		}
		updateScene();
		update();
	}
}


