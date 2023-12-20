#include "gef_debug_drawer.h"
#include "maths/vector4.h"
#include "graphics/colour.h"
#include "graphics/renderer_3d.h"

GEFDebugDrawer::GEFDebugDrawer(gef::Renderer3D * renderer_3d) :
	renderer_3d_(renderer_3d)
{
	primitive_renderer_ = new PrimitiveRenderer(const_cast<gef::Platform&>(renderer_3d_->platform()), 8192);
	mode_ = 0;
}

GEFDebugDrawer::~GEFDebugDrawer()
{
	delete primitive_renderer_;
}

void GEFDebugDrawer::drawLine(const btVector3 & from, const btVector3 & to, const btVector3 & color)
{
	primitive_renderer_->AddLine(gef::Vector4(from.x(), from.y(), from.z()), gef::Vector4(to.x(), to.y(), to.z()), gef::Colour(color.x(), color.y(), color.z(), 1.0f));
}

void GEFDebugDrawer::drawContactPoint(const btVector3 & PointOnB, const btVector3 & normalOnB, btScalar distance, int lifeTime, const btVector3 & color)
{
}

void GEFDebugDrawer::reportErrorWarning(const char * warningString)
{
}

void GEFDebugDrawer::draw3dText(const btVector3 & location, const char * textString)
{
}

void GEFDebugDrawer::setDebugMode(int debugMode)
{
	mode_ = debugMode;
}

int GEFDebugDrawer::getDebugMode() const
{
	return mode_;
}

void GEFDebugDrawer::clearLines()
{
	primitive_renderer_->Reset();
}

void GEFDebugDrawer::flushLines()
{
	primitive_renderer_->Render(*renderer_3d_);
}


