#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/Camera.h>
#include <ngl/Light.h>
#include <ngl/Material.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/Random.h>
#include <ngl/ShaderLib.h>



NGLScene::NGLScene()
{

  setTitle("Sphere -> Sphere Collision");
  m_animate=true;
}


NGLScene::~NGLScene()
{
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
}

void NGLScene::resizeGL( int _w, int _h )
{
  m_cam.setShape( 45.0f, static_cast<float>( _w ) / _h, 0.05f, 350.0f );
  m_win.width  = static_cast<int>( _w * devicePixelRatio() );
  m_win.height = static_cast<int>( _h * devicePixelRatio() );
}


void NGLScene::initializeGL()
{
  // we must call this first before any other GL commands to load and link the
  // gl commands from the lib, if this is not done program will crash
  ngl::NGLInit::instance();

  glClearColor(1.4f, 1.4f, 1.4f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);
  ngl::Vec3 from(0.0f,0.0f,-20.0f);
  ngl::Vec3 to(0.0f,0.0f,0.0f);
  ngl::Vec3 up(0.0f,1.0f,0.0f);
  m_cam.set(from,to,up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_cam.setShape(45,(float)720.0f/576.0f,0.5f,150.0f);
  // now to load the shader and set the values
  // grab an instance of shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)["nglDiffuseShader"]->use();

  shader->setShaderParam4f("Colour",1.0f,1.0f,1.0f,1.0f);
  shader->setShaderParam3f("lightPos",1.0f,1.0f,1.0f);
  shader->setShaderParam4f("lightDiffuse",1.0f,1.0f,1.0f,1.0f);

  glEnable(GL_DEPTH_TEST); // for removal of hidden surfaces

  ngl::VAOPrimitives *prim =  ngl::VAOPrimitives::instance();
  prim->createSphere("sphere",1.0f,40.0f);
  // create vectors for the position and direction
  ngl::Vec3 pos;
  ngl::Vec3 dir;
  // now set two bigger spheres with no movement
  pos.set(-10.0f,0.0f,0.0f);
  dir.set(0.0f,0.0f,0.0f);
  m_sphereArray[0].set(pos,dir,2.0f);
  m_sphereArray[0].setColour(ngl::Colour(1.0f,1.0f,0.0f));
  pos.set(10.0f,0.0f,0.0f);
  dir.set(0.0f,0.0f,0.0f);
  m_sphereArray[1].set(pos,dir,2.0f);
  m_sphereArray[1].setColour(ngl::Colour(1.0f,1.0f,0.0f));
  // and two smaller ones to move and bounce.
  pos.set(-7.0f,0.0f,0.0f);
  dir.set(0.5f,0.0f,0.0f);
  m_sphereArray[2].set(pos,dir,1.0f);
  m_sphereArray[2].setColour(ngl::Colour(1.0f,0.0f,0.0f));

  pos.set(7.0f,0.0f,0.0f);
  dir.set(-0.5f,0.0f,0.0f);
  m_sphereArray[3].set(pos,dir,1.0f);
  m_sphereArray[3].setColour(ngl::Colour(0.0f,0.0f,1.0f));
  m_sphereUpdateTimer=startTimer(20);

}




void NGLScene::paintGL()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0,0,m_win.width,m_win.height);
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


	for(Sphere s : m_sphereArray)
	{
		s.draw("nglDiffuseShader", m_mouseGlobalTX, &m_cam);
	}
}

void NGLScene::updateScene()
{
	m_sphereArray[2].move();
	m_sphereArray[3].move();
	checkCollisions();

}
void NGLScene::timerEvent( QTimerEvent *_event )
{
	if(_event->timerId() == m_sphereUpdateTimer)
	{
		if (m_animate !=true)
		{
			return;
		}
		updateScene();
		update();
	}
}
bool NGLScene::sphereSphereCollision(ngl::Vec3 _pos1, GLfloat _radius1, ngl::Vec3 _pos2, GLfloat _radius2 )
{
  // the relative position of the spheres
  ngl::Vec3 relPos;
  //min an max distances of the spheres
  GLfloat dist;
  GLfloat minDist;
  GLfloat len;
  relPos =_pos1-_pos2;
  // and the distance
  len=relPos.length();
  dist=len*len;
  minDist =_radius1+_radius2;
  // if it is a hit
  if(dist <=(minDist * minDist))
  {
    return true;
  }
  else
  {
    return false;
  }
}

void  NGLScene::checkCollisions()
{
  //first check the small spheres against each other
  bool collide =sphereSphereCollision(m_sphereArray[2].getPos(),m_sphereArray[2].getRadius(),
                    m_sphereArray[3].getPos(),m_sphereArray[3].getRadius());
  if(collide== true)
  {
    m_sphereArray[2].reverse();
    m_sphereArray[3].reverse();
  }
  // now for the little spheres against the big
  collide =sphereSphereCollision(
                                  m_sphereArray[0].getPos(),
                                  m_sphereArray[0].getRadius(),
                                  m_sphereArray[2].getPos(),
                                  m_sphereArray[2].getRadius()
                                );
  // if we have a collision reverse the direction
  if(collide== true)
  {
    m_sphereArray[2].reverse();
  }
  collide =sphereSphereCollision(
                                  m_sphereArray[1].getPos(),
                                  m_sphereArray[1].getRadius(),
                                  m_sphereArray[3].getPos(),
                                  m_sphereArray[3].getRadius()
                                 );
  if(collide== true)
  {
    m_sphereArray[3].reverse();
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
  default : break;
  }
  // finally update the GLWindow and re-draw
  //if (isExposed())
    update();
}
