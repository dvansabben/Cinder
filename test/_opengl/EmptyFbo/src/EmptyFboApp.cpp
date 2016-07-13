#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class EmptyFboApp : public App {
  public:
	EmptyFboApp();
	void draw() override;
	void mouseDown( MouseEvent event ) override;
	void keyDown( KeyEvent event ) override;

	gl::Texture2dRef mTextureRed, mTextureGreen, mTextureBlue;
};

EmptyFboApp::EmptyFboApp()
{
	// create 3 test textures
	mTextureRed		= gl::Texture2d::create( getWindowWidth() / 3, getWindowHeight() );
	mTextureGreen	= gl::Texture2d::create( getWindowWidth() / 3, getWindowHeight() );
	mTextureBlue	= gl::Texture2d::create( getWindowWidth() / 3, getWindowHeight() );

	// create an empty fbo and bind it
	auto fbo = gl::Fbo::create();
	gl::ScopedFramebuffer scopedFbo( fbo );

	// attach the first texture and clear it with red
	gl::frameBufferTexture( mTextureRed, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0 );
	gl::clear( Color( 1.0f, 0.0f, 0.0f ) );
	
	// replace the first attachement by the second texture and clear it with green
	gl::frameBufferTexture( mTextureGreen, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0 );
	gl::clear( Color( 0.0f, 1.0f, 0.0f ) );
	
	// replace the first attachement by the third texture and clear it with blue
	gl::frameBufferTexture( mTextureBlue, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0 );
	gl::clear( Color( 0.0f, 0.0f, 1.0f ) );

	// just to make sure let's check if there's an issue with our fbo	
	gl::Fbo::checkStatus();

	// check for erros
	CI_CHECK_GL();

	// need more work here:	
	console() << "fbo->getAspectRatio()\t" << fbo->getAspectRatio() << endl;
	console() << "fbo->getBounds()\t" << fbo->getBounds() << endl;
	console() << "fbo->getColorTexture()\t" << fbo->getColorTexture() << endl;
	console() << "fbo->getDepthTexture()\t" << fbo->getDepthTexture() << endl;
	console() << "fbo->getFormat()\t" << fbo->getFormat().getLabel() << endl;
	console() << "fbo->getHeight()\t" << fbo->getHeight() << endl;
	console() << "fbo->getLabel()\t" << fbo->getLabel() << endl;
	console() << "fbo->getMultisampleId()\t" << fbo->getMultisampleId() << endl;
	console() << "fbo->getResolveId()\t" << fbo->getResolveId() << endl;
	console() << "fbo->getSize()\t" << fbo->getSize() << endl;
	console() << "fbo->getTexture2d()\t" << fbo->getTexture2d( GL_COLOR_ATTACHMENT0 ) << endl;
	console() << "fbo->getTextureBase()\t" << fbo->getTextureBase( GL_COLOR_ATTACHMENT0 ) << endl;
	console() << "fbo->getWidth()\t" << fbo->getWidth() << endl;
}

void EmptyFboApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
	
	gl::ScopedViewport viewport( getWindowSize() );
	gl::setMatricesWindow( getWindowSize() );
	gl::draw( mTextureRed );
	gl::draw( mTextureGreen, vec2( getWindowWidth() / 3, 0.0f ) );
	gl::draw( mTextureBlue, vec2( getWindowWidth() / 3 * 2, 0.0f ) );
}

void EmptyFboApp::mouseDown( MouseEvent event )
{
	// pick the texture under the mouse
	gl::Texture2dRef tex;
	if( event.getPos().x < getWindowWidth() / 3 ) tex = mTextureRed;
	else if( event.getPos().x > getWindowWidth() / 3 * 2 ) tex = mTextureBlue;
	else tex = mTextureGreen;
	
	if( event.isLeft() ) {
		// create an empty fbo, attach one of the textures and clear it with a random color
		auto fbo = gl::Fbo::create();
		gl::ScopedFramebuffer scopedFbo( fbo );
		gl::frameBufferTexture( tex, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0 );
		gl::Fbo::checkStatus();

		gl::clear( Color( randFloat(), randFloat(), randFloat() ) );
	}
	else if( event.isRight() && event.isShiftDown() ) {
		// create an empty fbo, attach one of the textures and draw a teapot without depthbuffer
		auto fbo = gl::Fbo::create();
		gl::ScopedFramebuffer scopedFbo( fbo );
		gl::frameBufferTexture( tex, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0 );
		gl::Fbo::checkStatus();
		
		gl::ScopedDepth scopedDepth( true ); // has no effect because we don't have a depth buffer
		gl::ScopedViewport viewport( tex->getSize() );
		gl::setMatrices( CameraPersp( tex->getWidth(), tex->getHeight(), 50.0f, 0.1f, 100.0f ).calcFraming( Sphere( vec3( 0.0f ), 2.0f ) ) );
		
		auto teapot = gl::Batch::create( geom::Teapot(), gl::getStockShader( gl::ShaderDef().lambert() ) );
		teapot->draw();
	}
	else if( event.isRight() ) {
		// create a depth texture
		auto depthTexture = gl::Texture2d::create( tex->getWidth(), tex->getHeight(), gl::Texture2d::Format().internalFormat( GL_DEPTH_COMPONENT24 ) );

		// create an empty fbo, attach one of the textures and draw a teapot with a depthbuffer
		auto fbo = gl::Fbo::create();
		gl::ScopedFramebuffer scopedFbo( fbo );
		gl::frameBufferTexture( tex, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0 );
		gl::frameBufferTexture( depthTexture, GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0 );
		gl::Fbo::checkStatus();
		
		gl::clear( GL_DEPTH_BUFFER_BIT );

		gl::ScopedDepth scopedDepth( true );
		gl::ScopedViewport viewport( tex->getSize() );
		gl::setMatrices( CameraPersp( tex->getWidth(), tex->getHeight(), 50.0f, 0.1f, 100.0f ).calcFraming( Sphere( vec3( 0.0f ), 2.0f ) ) );
		
		auto teapot = gl::Batch::create( geom::Teapot(), gl::getStockShader( gl::ShaderDef().lambert() ) );
		teapot->draw();
	}
}
void EmptyFboApp::keyDown( KeyEvent event ) 
{
	if( event.getCode() == KeyEvent::KEY_SPACE ) {
		// create an empty fbo, attach the 3 textures and clear them with black
		auto fbo = gl::Fbo::create();
		gl::ScopedFramebuffer scopedFbo( fbo );
		gl::frameBufferTexture( mTextureRed, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0 );
		gl::frameBufferTexture( mTextureGreen, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 0 );
		gl::frameBufferTexture( mTextureBlue, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, 0 );
		gl::Fbo::checkStatus();
	
		GLenum buffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		gl::drawBuffers( 3, &buffers[0] );
		gl::clear( Color::black() );
		gl::drawBuffer( GL_COLOR_ATTACHMENT0 );
	}
}

CINDER_APP( EmptyFboApp, RendererGl )
