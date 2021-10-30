#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/NGLInit.h>
#include <ngl/ShaderLib.h>
#include <ngl/Random.h>
#include <ngl/VAOFactory.h>
#include <ngl/SimpleVAO.h>


NGLScene::NGLScene(int _numTriangles)
{
  m_numTriangles=_numTriangles;
  setTitle("Ray->Triangle Intersections");
  // create the points for our ray
  m_rayStart.set(0,0,0.2f);
  m_rayEnd.set(0,0,-20);

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
  ngl::Vec3 from(0.0f,1.0f,15.0f);
  ngl::Vec3 to(0.0f,0.0f,0.0f);
  ngl::Vec3 up(0.0f,1.0f,0.0f);
  m_view=ngl::lookAt(from,to,up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_project=ngl::perspective(45.0f,720.0f/576.0f,0.5f,150.0f);
  // now to load the shader and set the values
  
  ngl::ShaderLib::use("nglDiffuseShader");

  ngl::ShaderLib::setUniform("Colour",1.0f,1.0f,1.0f,1.0f);
  ngl::ShaderLib::setUniform("lightPos",1.0f,1.0f,1.0f);
  ngl::ShaderLib::setUniform("lightDiffuse",1.0f,1.0f,1.0f,1.0f);

  ngl::ShaderLib::use("nglColourShader");
  ngl::ShaderLib::setUniform("Colour",1.0f,1.0f,1.0f,1.0f);

  glEnable(GL_DEPTH_TEST); // for removal of hidden surfaces


  ngl::VAOPrimitives::createSphere("smallSphere",0.05f,10.0f);


  for (int i=0; i<m_numTriangles; ++i)
  {
   ngl::Vec3 c=ngl::Random::getRandomVec3()*10.0f; 
   ngl::Vec3 v0(ngl::Random::randomNumber(2)+0.1f,ngl::Random::randomNumber(2)+0.1f,-ngl::Random::randomPositiveNumber(2)+0.1f);
   ngl::Vec3 v1(ngl::Random::randomNumber(2)+0.1f,ngl::Random::randomNumber(2)+0.1f,-ngl::Random::randomPositiveNumber(2)+0.1f);
   ngl::Vec3 v2(ngl::Random::randomNumber(2)+0.1f,ngl::Random::randomNumber(2)+0.1f,-ngl::Random::randomPositiveNumber(2)+0.1f);
   m_triangleArray.emplace_back(new Triangle(c+v0,c+v1,c+v2));
  }
  // as re-size is not explicitly called we need to do this.
  glViewport(0,0,width(),height());
}


void NGLScene::loadMatricesToShader()
{
  ngl::ShaderLib::use("nglDiffuseShader");

  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;

  MVP=  m_project*m_view*m_mouseGlobalTX*m_transform.getMatrix();
  normalMatrix=m_view*m_mouseGlobalTX*m_transform.getMatrix();
  normalMatrix.inverse().transpose();
  ngl::ShaderLib::setUniform("MVP",MVP);
  ngl::ShaderLib::setUniform("normalMatrix",normalMatrix);
}

void NGLScene::loadMatricesToColourShader()
{
  ngl::ShaderLib::use("nglColourShader");
  ngl::Mat4 MVP;
  MVP=m_project*m_view *m_mouseGlobalTX*m_transform.getMatrix();
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

  // Rotation based on the mouse position for our global
  // transform
  // grab an instance of the shader manager
  ngl::ShaderLib::use("nglDiffuseShader");

	// draw a cube at the ray start points
	m_transform.reset();
	{
    ngl::ShaderLib::setUniform("Colour",1.0f,1.0f,1.0f,1.0f);
		m_transform.setPosition(m_rayStart);
		loadMatricesToShader();
		ngl::VAOPrimitives::draw("cube");
	}

	// we build up a vertex array for the lines of the start and end points and draw
	m_transform.reset();
	{
    std::unique_ptr< ngl::AbstractVAO> vao( ngl::VAOFactory::createVAO("simpleVAO",GL_LINES));
		vao->bind();
		ngl::Vec3 points[2];
		points[0]=m_rayStart;
		points[1]=m_rayEnd;
    vao->setData(ngl::SimpleVAO::VertexData(2*sizeof(ngl::Vec3),points[0].m_x));
		vao->setVertexAttributePointer(0,3,GL_FLOAT,sizeof(ngl::Vec3),0);
		vao->setNumIndices(2);
		loadMatricesToColourShader();
		vao->draw();
    vao->removeVAO();
	}
	// draw all the triangles
	ngl::ShaderLib::use("nglDiffuseShader");
  for(auto &t : m_triangleArray)
	{
    ngl::ShaderLib::setUniform("Colour",1.0f,1.0f,0.0f,0.0f);
		t->rayTriangleIntersect(m_rayStart,m_rayEnd);
    t->draw("nglDiffuseShader",m_mouseGlobalTX,m_view,m_project);
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
  if ( _event->angleDelta().x() > 0 )
  {
    m_modelPos.m_z += ZOOM;
  }
  else if ( _event->angleDelta().x() < 0 )
  {
    m_modelPos.m_z -= ZOOM;
  }
  update();
}
//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  static float s_increment=0.5;
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
    // escape key to quite
    case Qt::Key_Escape : QGuiApplication::exit(EXIT_SUCCESS); break;
    case Qt::Key_Up : m_rayEnd.m_y+=s_increment; break;
    case Qt::Key_Down : m_rayEnd.m_y-=s_increment; break;
    case Qt::Key_Left : m_rayEnd.m_x-=s_increment; break;
    case Qt::Key_Right : m_rayEnd.m_x+=s_increment; break;
    case Qt::Key_W : m_rayStart.m_y+=s_increment; break;
    case Qt::Key_Z : m_rayStart.m_y-=s_increment; break;
    case Qt::Key_A : m_rayStart.m_x-=s_increment; break;
    case Qt::Key_S : m_rayStart.m_x+=s_increment; break;


    default : break;
  }
  // finally update the GLWindow and re-draw
  //if (isExposed())
    update();
}
