#ifndef _GEF_DEBUG_DRAWER_H
#define _GEF_DEBUG_DRAWER_H

#include "LinearMath/btIDebugDraw.h"
#include "primitive_renderer.h"

namespace gef
{
	class Renderer3D;
}

class GEFDebugDrawer : public btIDebugDraw
{
public:
	GEFDebugDrawer(gef::Renderer3D* renderer_3d);
	~GEFDebugDrawer();

	virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;
	virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override;
	virtual void reportErrorWarning(const char* warningString) override;

	virtual void draw3dText(const btVector3& location, const char* textString) override;

	virtual void setDebugMode(int debugMode) override;

	virtual int getDebugMode() const override;

	virtual void clearLines() override;
	virtual void flushLines() override;

private:
	PrimitiveRenderer * primitive_renderer_;
	gef::Renderer3D* renderer_3d_;
	int mode_;
};

#endif // !_GEF_DEBUG_DRAWER_H
