#ifndef LLW_WORLD_HPP
#define LLW_WORLD_HPP

#include "lunarlady/math/vec2.hpp"
#include "lunarlady/math/vec3.hpp"
#include "lunarlady/math/Quaternion.hpp"
#include "lunarlady/math/Ray.hpp"
#include "lunarlady/math/Plane.hpp"
#include "boost/smart_ptr.hpp"
#include "WorldEditor/RenderMode.hpp"
#include "WorldEditor/ToolAction.hpp"
#include <list>

struct Point;
struct Line;

struct Point {
	Point(const ::lunarlady::math::vec3& iLocation);
	::lunarlady::math::vec3 location;
	std::list<Line*> lines;
};
typedef boost::shared_ptr<Point> PointPtr;

struct Line {
	Line(Point* iFrom, Point* iTo);
	Point* from;
	Point* to;
};
typedef boost::shared_ptr<Line> LinePtr;

enum AxisLock {
	AL_NONE,
	AL_X,
	AL_Y,
	AL_Z
};

struct Lock {
	Lock(AxisLock iLock, Point* iPoint, const ::lunarlady::math::Ray& iAxisRay);
	AxisLock axis;
	Point* lockedOn;
	::lunarlady::math::Ray axisRay;
};

struct TemporaryPoint {
	TemporaryPoint();

	void setPoint(Point* iPoint);
	void setLine(Line* iLine, ::lunarlady::real iRatio);
	void setLocation(const ::lunarlady::math::vec3& iLocation);
	void setOrigin();

	bool isOrigin();
	bool isPoint();
	bool isLine();
	bool isLocation();
	bool isMidpoint(); // assert when isLine is false or point is not valid(hasent been set)

	void lockOn(const ::lunarlady::math::vec3& iLocation, Point* iPoint, AxisLock iAxis, const ::lunarlady::math::Ray& iAxisRay);

	bool hasLock();

	::lunarlady::math::vec3 getLocation();

	void setNull();

	Point* point;
	Line* line; ::lunarlady::real ratio;
	::lunarlady::math::vec3 location;
	bool midpoint;
	bool valid;
	bool origin;

	std::list<Lock> locks;
};

class World {
public:
	World();
	~World();
	void render( RenderMode iRenderMode );

	void setMousePosition(const ::lunarlady::math::vec2& iMousePos);

	void changeRotation(const ::lunarlady::math::vec2& rot);
	void changePosition(const ::lunarlady::math::vec2& rot);

	void increaseZoom( ::lunarlady::real iPower );

	Line* gatherLine();
	void markForDeletion(Line* iLine);
	void deleteLines();

	TemporaryPoint* placePoint();
	void addLine(TemporaryPoint* from, TemporaryPoint* to);
	void setFrom(TemporaryPoint* iFrom);

	bool defaultRefreshValue();

	bool isOver(Point* iPoint);
	bool isOver(Line* iLine);
	bool isDeletingLine(Line* iLine);

	void setLocking(bool iLocked);

	void onRemovePoint(Point* iPoint);

	bool hasAxisLock();

	void selectToolAction(ToolAction iToolAction);
protected:
	Point* newPoint(TemporaryPoint* iPoint);
	Point* gatherPointOverCursor();
	Line* gatherLineOverCursor(::lunarlady::real* oIntersection);

	void drawAxis(RenderMode iRenderMode, ::lunarlady::real AXIS_LENGTH, ::lunarlady::real iEndAlpha);
	void drawWorld(RenderMode iRenderMode);

	void moveOpengl();
	void rotateOpengl();

	void getTransformationArrays();

	void updateCurrentPoint();

	::lunarlady::math::vec3 getSuggestedPosition();
	::lunarlady::real getSuggestedRadius();

	void removeAlonePoints();

	bool doesCursorRayCollidesWithAxis(const ::lunarlady::math::vec3& iAxis, ::lunarlady::math::vec3* oResult, AxisLock iLock);
	bool World::getAxisCollisionPoint(::lunarlady::math::vec3* oPoint);

	void updateLocking();

	void addLock(Point* iPoint);

	enum {
		MAX_MEMORY_SIZE = 3
	};

public:
	bool mRotating;
private:
	::lunarlady::math::Quaternion mCameraRotation;
	::lunarlady::math::vec3 mCameraPosition;

	double mModelView[16];
	double mProjection[16];
	int mViewPort[4];

	std::list<PointPtr> mPoints;
	std::list<LinePtr> mLines;

	//::lunarlady::math::vec3 mMousePosition;
	::lunarlady::math::Ray mCursorRay;

	enum { MAX_TEMPORARY_POINTS = 2 };

	TemporaryPoint mTemporaryPoints[MAX_TEMPORARY_POINTS];
	TemporaryPoint mCurrentPoint;

	TemporaryPoint* mFrom;

	std::list<Line*> mLinesToDelete;
	AxisLock mAxisLock;

	bool mIsLocked;
	::lunarlady::math::Ray mLockedRay;
	TemporaryPoint mLockedCurrentPoint;

	std::list<Point*> mLockedPoints;

	ToolAction mToolAction;
};

#endif