#include "WorldEditor/World.hpp"

#include "wx.hpp"
#include "wx_opengl.hpp"

#include "lunarlady/math/CollisionResult.hpp"
#include "boost/bind.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>

Lock::Lock(AxisLock iLock, Point* iPoint,const ::lunarlady::math::Ray& iAxisRay) : axis(iLock), lockedOn(iPoint), axisRay(iAxisRay) {
}

TemporaryPoint::TemporaryPoint() : point(0), line(0), ratio(0), location(0,0,0), valid(false), midpoint(false) {
}

void TemporaryPoint::setPoint(Point* iPoint) {
	setNull();
	point = iPoint;
	location = iPoint->location;
	valid = true;
}

bool TemporaryPoint::hasLock() {
	return !locks.empty();
}

void TemporaryPoint::setLine(Line* iLine, ::lunarlady::real iRatio) {
	setNull();
	line = iLine;
	ratio = iRatio;

	const ::lunarlady::real range = 0.02;
	const ::lunarlady::real middlePoint = 0.5;
	if( ::lunarlady::math::withinRange(ratio, middlePoint, range ) ) {
		ratio = middlePoint;
		midpoint = true;
	}
	else {
		midpoint = false;
	}

	::lunarlady::math::Ray ray = ::lunarlady::math::Ray::FromTo(iLine->from->location, iLine->to->location);
	location = ray.getPoint(ratio);

	valid = true;
}

void TemporaryPoint::setLocation(const ::lunarlady::math::vec3& iLocation) {
	setNull();
	location = iLocation;
	valid = true;
}
void TemporaryPoint::lockOn(const ::lunarlady::math::vec3& iLocation, Point* iPoint, AxisLock iAxis, const ::lunarlady::math::Ray& iAxisRay) {
	if( !valid ) {
		int i=0;
		++i;
	}
	location = iLocation;
	locks.push_back( Lock(iAxis, iPoint, iAxisRay) );
}

void TemporaryPoint::setNull() {
	line = 0;
	point = 0;
	valid = false;
	midpoint = false;
	origin = false;
	locks.clear();
}

bool TemporaryPoint::isPoint() {
	return valid && ( !line && point );
}
bool TemporaryPoint::isLine() {
	return valid && ( line && !point );
}
bool TemporaryPoint::isLocation() {
	return valid && ( !line && !point && !origin);
}
bool TemporaryPoint::isMidpoint() {
	assert(valid);
	assert(isLine());
	return midpoint;
}
void TemporaryPoint::setOrigin() {
	setNull();
	location = ::lunarlady::math::op::vec3::origo;
	origin = true;
	valid = true;
}
bool TemporaryPoint::isOrigin() {
	return valid && origin;
}

::lunarlady::math::vec3 TemporaryPoint::getLocation() {
	if( !valid ) {
		int i=0;
		++i;
	}
	if( !locks.empty() ) {
		return location;
	}
	else
	if( isPoint() ) {
		return point->location;
	}
	else 
	if( isLine() ) {
		::lunarlady::math::Ray ray = ::lunarlady::math::Ray::FromTo(line->from->location, line->to->location);
		return ray.getPoint( ratio );
	}
	else {
		return location;
	}
}

World::World() : mCameraPosition(0,0,0), mCursorRay(::lunarlady::math::vec3(0,0,0), ::lunarlady::math::vec3(1,1,1)), mFrom(0), mAxisLock(AL_NONE), mLockedRay(::lunarlady::math::vec3(0,0,0), ::lunarlady::math::vec3(1,1,1)), mIsLocked(false), mToolAction(TA_ALL), mRotating(false) {
}

World::~World() {
}

void World::selectToolAction(ToolAction iToolAction) {
	mToolAction = iToolAction;
}

bool World::defaultRefreshValue() {
	return mFrom!=0;
}

Line* World::gatherLine() {
	if( !mCurrentPoint.isLine() ) return 0;
	return mCurrentPoint.line;
}
void World::markForDeletion(Line* iLine) {
	assert(iLine);
	// if we are not currently deleting that line, lets delete it
	if( !isDeletingLine(iLine ) ) {
		mLinesToDelete.push_back(iLine);
	}
}

void World::removeAlonePoints() {
	int i = 0;
	i++;
	struct Delete {
		static bool point(PointPtr& iPoint, World* iWorld) {
			bool remove = iPoint->lines.empty();
			if( remove ) {
				iWorld->onRemovePoint( iPoint.get() );
			}
			return remove;
		}
	};
	mPoints.erase(std::remove_if(mPoints.begin(), mPoints.end(), boost::bind(&Delete::point, _1, this) ),
		mPoints.end() );
}

bool World::isDeletingLine(Line* iLine) {
	bool deleteMe = std::search_n(mLinesToDelete.begin(), mLinesToDelete.end(), 1, iLine) != mLinesToDelete.end();
	return deleteMe;
}

void World::deleteLines() {
	if( mLinesToDelete.empty() ) return;
	struct Delete {
		static bool line(LinePtr& iLine, World* iWorld) {
			bool shouldDelete = iWorld->isDeletingLine( iLine.get() );
			if( shouldDelete ) {
				iLine->from->lines.remove( iLine.get() );
				iLine->to->lines.remove( iLine.get() );
			}
			return shouldDelete;
		}
	};
	mLines.erase(std::remove_if(mLines.begin(), mLines.end(), boost::bind(&(Delete::line), _1, this) ),
	mLines.end());
	mLinesToDelete.clear();

	removeAlonePoints();
}

void World::changeRotation(const ::lunarlady::math::vec2& rot) {
	mCameraRotation =	::lunarlady::math::Quaternion(::lunarlady::math::op::vec3::yAxisPositive, rot.getX()) * 
						::lunarlady::math::Quaternion(mCameraRotation.getRight(), rot.getY()) *
						mCameraRotation;
}
void World::changePosition(const ::lunarlady::math::vec2& move) {
	mCameraPosition +=	  mCameraRotation.getRight() * - move.getX()
						+ ::lunarlady::math::op::vec3::yAxisPositive * move.getY();
}

void World::increaseZoom( ::lunarlady::real iPower ) {
	mCameraPosition +=	  mCameraRotation.getIn() * iPower;
}

void World::moveOpengl() {
	::lunarlady::math::vec3 move = -mCameraPosition;
	glTranslated(move.getX(), move.getY(), move.getZ());
}
void World::rotateOpengl() {
	::lunarlady::math::vec3 axis(0,0,0);
	::lunarlady::math::Angle angle;
	mCameraRotation.getConjugate().toAxisAngle(&axis, &angle);
	glTranslated(0, 0, -20);

	glRotated(angle.inDegrees(),
		axis.getX(), axis.getY(), axis.getZ());

	
}

void World::getTransformationArrays() {
	glGetDoublev(GL_MODELVIEW_MATRIX, mModelView);
	glGetDoublev(GL_PROJECTION_MATRIX, mProjection);
	glGetIntegerv(GL_VIEWPORT, mViewPort);
}

void World::render(RenderMode iRenderMode) {
	//glTranslatef(0.0f,0.0f,- mZoom);
	rotateOpengl();

	if( mRotating ) {
		drawAxis(iRenderMode, 10, 0.0);
	}

	getTransformationArrays();
	moveOpengl();

	
	drawWorld(iRenderMode);
	drawAxis(iRenderMode, 1000, 1);

	if( mFrom && mFrom->valid ) {
		::lunarlady::math::vec3 from = mFrom->getLocation();
		glTranslated(from.getX(), from.getY(), from.getZ());
		drawAxis(iRenderMode, 5, 0);
	}
	/*
	else {
		if( mCurrentPoint.valid ) {
			::lunarlady::math::vec3 from = mCurrentPoint.getLocation();
			glTranslated(from.getX(), from.getY(), from.getZ());
			drawAxis(iRenderMode, 5, 0);
		}
	}*/
}

Point::Point(const ::lunarlady::math::vec3& iLocation) : location(iLocation) {
}

Line::Line(Point* iFrom, Point* iTo) : from(iFrom), to(iTo) {
}

Point* World::newPoint(TemporaryPoint* iPoint) {
	if( iPoint->isPoint() ) {
		return iPoint->point;
	}

	if( iPoint->isLine() ) {
		Line* line = iPoint->line;
		Point* from = line->from;
		Point* to = line->to;

		from->lines.remove(line);
		to->lines.remove(line);

		PointPtr point( new Point(iPoint->getLocation()) );

		if( mFrom && mFrom->isLine() && line == mFrom->line ) {
			mFrom->setNull();
		}

		markForDeletion(line);
		line = 0;
		//mLines.remove( LinePtr(line) );

		mPoints.push_back( point );
		Point* middle = point.get();

		LinePtr fromToMiddle( new Line(from, middle) );
		LinePtr middleToEnd( new Line(middle, to) );

		mLines.push_back( fromToMiddle );
		mLines.push_back( middleToEnd );

		from->lines.push_back( fromToMiddle.get() );
		middle->lines.push_back( fromToMiddle.get() );

		middle->lines.push_back( middleToEnd.get() );
		to->lines.push_back( middleToEnd.get() );
		deleteLines();

		addLock(middle);

		return middle;
	}

	PointPtr point( new Point(iPoint->getLocation()) );
	mPoints.push_back( point );
	addLock(point.get());
	return point.get();
}

::lunarlady::real World::getSuggestedRadius() {
	return 0.15; // for zoom of 20, add zoom dependency on radius
}

Line* World::gatherLineOverCursor(::lunarlady::real* oIntersection) {
	::lunarlady::math::CollisionResult collision;
	::lunarlady::real ratio = 0;
	Line* resultLine = 0;
	::lunarlady::real intersection = 0;

	const ::lunarlady::real radius = getSuggestedRadius();

	for(std::list<LinePtr>::iterator line=mLines.begin(); line != mLines.end(); ++line) {
		::lunarlady::math::Ray lineRay = ::lunarlady::math::Ray::FromTo((*line)->from->location, (*line)->to->location);

		if( collision.collision(mCursorRay, lineRay, false, radius, &intersection)) {
			if( collision.getCollisionRatio() > 0 ) {
				if( resultLine==0 || ratio > collision.getCollisionRatio() ) {
					ratio = collision.getCollisionRatio();
					resultLine = line->get();
					*oIntersection = intersection;
				}
			}
		}
	}

	return resultLine;
}

Point* World::gatherPointOverCursor() {
	::lunarlady::math::CollisionResult collision;
	::lunarlady::real ratio = 0;
	Point* resultPoint = 0;
	::lunarlady::math::Sphere pointSphere( getSuggestedRadius() );
	for(std::list<PointPtr>::iterator point=mPoints.begin(); point != mPoints.end(); ++point) {
		if( collision.collision(mCursorRay, pointSphere, (*point)->location) ) {
			if( collision.getCollisionRatio() > 0 ) {
				if( resultPoint==0 || ratio > collision.getCollisionRatio() ) {
					ratio = collision.getCollisionRatio();
					resultPoint = point->get();
				}
			}
		}
	}
	return resultPoint;
}

bool World::doesCursorRayCollidesWithAxis(const ::lunarlady::math::vec3& iAxis, ::lunarlady::math::vec3* oResult, AxisLock iLock) {
	assert( mFrom );
	assert( oResult );

	::lunarlady::math::CollisionResult collision;
	::lunarlady::math::Ray axis = ::lunarlady::math::Ray::FromTo(mFrom->location, mFrom->location + iAxis);

	::lunarlady::real intersection = 0;
	if( collision.collision(mCursorRay, axis, true, getSuggestedRadius(), &intersection) ){
		*oResult = axis.getPoint(intersection);
		mAxisLock = iLock;
		return true;
	}

	return false;
};

bool World::getAxisCollisionPoint(::lunarlady::math::vec3* oPoint) {
	if( mFrom ) {
		if( doesCursorRayCollidesWithAxis(::lunarlady::math::op::vec3::xAxisPositive, oPoint, AL_X) ) return true;
		if( doesCursorRayCollidesWithAxis(::lunarlady::math::op::vec3::yAxisPositive, oPoint, AL_Y) ) return true;
		if( doesCursorRayCollidesWithAxis(::lunarlady::math::op::vec3::zAxisPositive, oPoint, AL_Z) ) return true;
	}
	return false;
}

void World::updateCurrentPoint() {
	mAxisLock = AL_NONE;
	mCurrentPoint.setNull();

	if( mToolAction == TA_ALL ) {
		if( Point* resultPoint = gatherPointOverCursor() ) {
			mCurrentPoint.setPoint( resultPoint );
			addLock( resultPoint );
			return;
		}
	}

	::lunarlady::real intersection = 0;
	if( Line* resultLine = gatherLineOverCursor(&intersection) ) {
		mCurrentPoint.setLine(resultLine, intersection);
		return;
	}

	if( mToolAction == TA_ALL ) {
		::lunarlady::math::CollisionResult collision;
		::lunarlady::math::Sphere sphere(getSuggestedRadius());
		if( collision.collision(mCursorRay, sphere, ::lunarlady::math::op::vec3::origo) ) {
			mCurrentPoint.setOrigin();
			return;
		}

		
		::lunarlady::math::vec3 point(0,0,0);
		if( getAxisCollisionPoint(&point) ) {
			mCurrentPoint.setLocation( point );
		}
	}
}

TemporaryPoint* World::placePoint() {
	if( !mCurrentPoint.valid ) return 0;
	for( int i=0; i<MAX_TEMPORARY_POINTS; ++i) {
		if( ! mTemporaryPoints[i].valid ) {
			mTemporaryPoints[i] = mCurrentPoint;
			mCurrentPoint.setNull();
			return &mTemporaryPoints[i];
		}
	}
	assert( 0 && "Failed to find a free point, this shouldn't happen!!!");
	return 0;
}

::lunarlady::math::vec3 World::getSuggestedPosition() {
	return mCurrentPoint.getLocation();
}

void World::addLine(TemporaryPoint* iFrom, TemporaryPoint* iTo) {
	assert(iFrom);
	assert(iTo);
	if( iFrom->getLocation() == iTo->getLocation() ) {
		iFrom->setNull();
		return;
	}

	if( iFrom->isLine() && iTo->isLine() && iFrom->line == iTo->line) {
		iFrom->setNull();
		return;
	}

	Point* from = newPoint(iFrom);
	Point* to = newPoint(iTo);
	iFrom->setNull();

	// if one of the points contain lines to the other we shouldn't add this line since it already exist
	if( !from->lines.empty() && !to->lines.empty() ) {
		// they shouldn't be empty here, if they are we have some error
		assert( ! from->lines.empty() );
		assert( ! to->lines.empty() );

		struct FindPoint {
			static void onLine(Line* iLine, Point* iPoint, bool* iExist) {
				if( iLine->from == iPoint ) *iExist = true;
				if( iLine->to == iPoint ) *iExist = true;
			}
		};

		bool exist = false;
		for_each( from->lines.begin(), from->lines.end(),
			boost::bind(&FindPoint::onLine, _1, to, &exist));

		// if there exist a line between from and to, so abort, else continue adding the line
		if( exist ) return;
	}

	LinePtr line( new Line(from, to) );
	mLines.push_back( line );
	from->lines.push_back( line.get() );
	to->lines.push_back( line.get() );
}

void World::setFrom(TemporaryPoint* iFrom) {
	mFrom = iFrom;

	if( !iFrom ) {
		for( int i=0; i< MAX_TEMPORARY_POINTS; ++i) {
			mTemporaryPoints[i].setNull();
		}
	}
}

bool World::isOver(Point* iPoint) {
	if( !mCurrentPoint.isPoint() ) return false;
	return mCurrentPoint.point == iPoint;
}

bool World::isOver(Line* iLine) {
	if( !mCurrentPoint.isLine() ) return false;
	return mCurrentPoint.line == iLine;
}

void drawPoint(TemporaryPoint& iPoint) {
	if( iPoint.valid ) {
		::lunarlady::math::vec3 point = iPoint.getLocation();
		if( iPoint.isPoint() || iPoint.isOrigin() ) {
			glColor3d(0, 1, 0);
			glVertex3d(point.getX(), point.getY(), point.getZ());
		}
		else if( iPoint.isLine() ) {
			if( iPoint.isMidpoint() ) {
				glColor3d(0, 0, 1);
				glVertex3d(point.getX(), point.getY(), point.getZ());
			}
			else {
				glColor3d(1, 0, 0);
				glVertex3d(point.getX(), point.getY(), point.getZ());
			}
		}
	}
}

/*
EXT_swap_control
WGL_EXT_swap_control
GLX_MESA_swap_control
GL_EXT_SWAP_CONTROL 
*/

const ::lunarlady::real HALO_WIDTH = 8.0;
const ::lunarlady::real HALO_DIFF = 0.0003;

void selectBackgroundColor() {
	glColor3f(1, 1, 1);
}

void drawLine(RenderMode iRenderMode, const ::lunarlady::math::vec3& from, const ::lunarlady::math::vec3& to) {
	glBegin(GL_LINES);
		glVertex3d(from.getX(), from.getY(), from.getZ());
		glVertex3d(to.getX(), to.getY(), to.getZ());
	glEnd();

	if( iRenderMode == RM_HALOED_LINE ) {
		glLineWidth(HALO_WIDTH);
		selectBackgroundColor();
		glDepthRange (HALO_DIFF, 1.0);
			
		glBegin(GL_LINES);
			glVertex3d(from.getX(), from.getY(), from.getZ());
			glVertex3d(to.getX(), to.getY(), to.getZ());
		glEnd();

		glDepthRange (0.0, 1 - HALO_DIFF); 
		glLineWidth(1);
	}
}

void World::addLock(Point* iPoint) {
	if( std::search_n(mLockedPoints.begin(), mLockedPoints.end(), 1, iPoint) == mLockedPoints.end() )  {
		mLockedPoints.push_back(iPoint);
		while( mLockedPoints.size() > MAX_MEMORY_SIZE ) {
			mLockedPoints.pop_front();
		}
	}
}

void SetOpenGlColor(AxisLock axis, ::lunarlady::real gray) {
	switch( axis ) {
		case AL_NONE:
			glColor3d(gray,gray,gray);
			break;
		case AL_X:
			glColor3d(1,0,0);
			break;
		case AL_Y:
			glColor3d(0,1,0);
			break;
		case AL_Z:
			glColor3d(0,0,1);
			break;
		default:
			assert( 0 && "Bad axis lock value" );
			break;
	}
}

void testGl() {
	GLenum error = glGetError();
	if( error != GL_NO_ERROR ) {
		std::fstream file("glerr.txt", std::ios::app);
		std::stringstream message;

		switch(error) {
#define CASE(value, desc) case value: message << #value << ": " << desc; break;
			CASE(GL_INVALID_ENUM, "An unacceptable value is specified for an enumerated argument. The offending command	is ignored, and	has no other side effect than to set the error flag.")
			CASE(GL_INVALID_VALUE, "A numeric argument is out of range.	The offending command is ignored, and has no other side effect than to set the error flag.")
			CASE(GL_INVALID_OPERATION, "The specified operation	is not allowed in the current state. The offending command is ignored, and has no other side effect than to set the error flag.")
			CASE(GL_STACK_OVERFLOW, "This command would cause a stack overflow. The offending command	is ignored, and	has no other side effect than to set the error flag.")
			CASE(GL_STACK_UNDERFLOW, "This command would cause a stack underflow. The offending command is ignored, and has	no other side effect than to set the error flag.")
			CASE(GL_OUT_OF_MEMORY, "There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded.")
#undef CASE
			default: message << "Undefined error code: #" << error;
		}
		file << message.str() << std::endl;
	}
}

void World::drawWorld(RenderMode iRenderMode) {
	testGl();


	/* // test triangle shape
	glBegin(GL_TRIANGLES);					// Start Drawing The Pyramid
		glColor3f(1.0f,0.0f,0.0f);			// Red
		glVertex3f( 0.0f, 1.0f, 0.0f);			// Top Of Triangle (Front)
		glColor3f(0.0f,1.0f,0.0f);			// Green
		glVertex3f(-1.0f,-1.0f, 1.0f);			// Left Of Triangle (Front)
		glColor3f(0.0f,0.0f,1.0f);			// Blue
		glVertex3f( 1.0f,-1.0f, 1.0f);			// Right Of Triangle (Front)

		glColor3f(1.0f,0.0f,0.0f);			// Red
		glVertex3f( 0.0f, 1.0f, 0.0f);			// Top Of Triangle (Right)
		glColor3f(0.0f,0.0f,1.0f);			// Blue
		glVertex3f( 1.0f,-1.0f, 1.0f);			// Left Of Triangle (Right)
		glColor3f(0.0f,1.0f,0.0f);			// Green
		glVertex3f( 1.0f,-1.0f, -1.0f);			// Right Of Triangle (Right)

		glColor3f(1.0f,0.0f,0.0f);			// Red
		glVertex3f( 0.0f, 1.0f, 0.0f);			// Top Of Triangle (Back)
		glColor3f(0.0f,1.0f,0.0f);			// Green
		glVertex3f( 1.0f,-1.0f, -1.0f);			// Left Of Triangle (Back)
		glColor3f(0.0f,0.0f,1.0f);			// Blue
		glVertex3f(-1.0f,-1.0f, -1.0f);			// Right Of Triangle (Back)

		glColor3f(1.0f,0.0f,0.0f);			// Red
		glVertex3f( 0.0f, 1.0f, 0.0f);			// Top Of Triangle (Left)
		glColor3f(0.0f,0.0f,1.0f);			// Blue
		glVertex3f(-1.0f,-1.0f,-1.0f);			// Left Of Triangle (Left)
		glColor3f(0.0f,1.0f,0.0f);			// Green
		glVertex3f(-1.0f,-1.0f, 1.0f);			// Right Of Triangle (Left)
	glEnd();*/

	if( mToolAction == TA_ALL ) {
		glPointSize(6.0f);
		glBegin(GL_POINTS);
			drawPoint( mCurrentPoint );
			if( mIsLocked ) {
				drawPoint( mLockedCurrentPoint  );
			}
		glEnd();
	}

	testGl();

	if( mToolAction == TA_ALL ) {
		glPointSize(3.0f);
		glColor3d(0, 0, 0);
		glBegin(GL_POINTS);
			struct Draw {
				static void OnLockedPoint(Point* iPoint) {
					glVertex3d(iPoint->location.getX(), iPoint->location.getY(), iPoint->location.getZ());
				}
			};
			std::for_each(mLockedPoints.begin(), mLockedPoints.end(),
				&Draw::OnLockedPoint);
		glEnd();
	}


	glPointSize(1.0f);

	if( iRenderMode == RM_HALOED_LINE ) {
		glLineWidth(HALO_WIDTH);
		selectBackgroundColor();
		glDepthRange (HALO_DIFF, 1.0);
		glBegin(GL_LINES);
			struct DrawLines {
				static void onLine(LinePtr& iLine, World* iWorld) {
					const ::lunarlady::math::vec3& from = iLine->from->location;
					const ::lunarlady::math::vec3& to = iLine->to->location;

					glVertex3d(from.getX(), from.getY(), from.getZ());
					glVertex3d(to.getX(), to.getY(), to.getZ());
				}
			};
			std::for_each( mLines.begin(), mLines.end(), boost::bind(&DrawLines::onLine, _1, this) );
		glEnd();
		glDepthRange (0.0, 1 - HALO_DIFF); 
		glLineWidth(1);
	}

	testGl();

	glColor3d(0,0,0);
	glBegin(GL_LINES);
		struct DrawLines {
			static void onLine(LinePtr& iLine, World* iWorld) {
				const ::lunarlady::math::vec3& from = iLine->from->location;
				const ::lunarlady::math::vec3& to = iLine->to->location;

				const bool deleteLine = iWorld->isDeletingLine(iLine.get());

				if( deleteLine ) {
					glColor3d(0,0,0.7);
				}

				glVertex3d(from.getX(), from.getY(), from.getZ());
				glVertex3d(to.getX(), to.getY(), to.getZ());

				if( deleteLine ) {
					glColor3d(0,0,0);
				}
			}
		};
		std::for_each( mLines.begin(), mLines.end(), boost::bind(&DrawLines::onLine, _1, this) );
	glEnd();

	testGl();

	if( mFrom && mFrom->valid && mCurrentPoint.valid ) {
		SetOpenGlColor(mAxisLock, 0.5);

		if( mAxisLock != AL_NONE ) {
			glLineWidth( 3.0f );
		}

		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x0F0F);
		drawLine( iRenderMode, mFrom->getLocation(), getSuggestedPosition() );
		glLineWidth(1);
		glDisable(GL_LINE_STIPPLE);
	}

	testGl();


	if( mCurrentPoint.valid ) {
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x0F0F);
		glColor3d(0.7, 0.7, 0.7);
		
		if( mIsLocked ) {
			drawLine( iRenderMode, mCurrentPoint.getLocation(), mLockedCurrentPoint.getLocation());
		}
		else {
			struct DrawLock {
				static void OnLock(Lock& iLock, const ::lunarlady::math::vec3& iStart, RenderMode iRenderMode) {
					// color according to axis lock
					SetOpenGlColor(iLock.axis, 0.7);
					glLineWidth( 2.0f );
					drawLine( iRenderMode, iStart, iLock.lockedOn->location );
				}
			};
			std::for_each(mCurrentPoint.locks.begin(), mCurrentPoint.locks.end(),
				boost::bind(&DrawLock::OnLock, _1, getSuggestedPosition(), iRenderMode)
				);
		}
		
		glDisable(GL_LINE_STIPPLE);

		testGl();
	}
}

void World::setMousePosition(const ::lunarlady::math::vec2& iMousePos) {
	double x, y, z;
	gluUnProject( iMousePos.getX(),
			      iMousePos.getY(),
			      1.0f,
			      mModelView,
			      mProjection,
			      mViewPort,
			      &x,
			      &y,
			      &z );
	::lunarlady::math::vec3 end = ::lunarlady::math::vec3(x, y, z) + mCameraPosition;

	gluUnProject( iMousePos.getX(),
			      iMousePos.getY(),
			      0.0f,
			      mModelView,
			      mProjection,
			      mViewPort,
			      &x,
			      &y,
			      &z );
	::lunarlady::math::vec3 start = ::lunarlady::math::vec3(x, y, z) + mCameraPosition;

	bool wasValid = false;
	mCursorRay.set(start, end);
	updateCurrentPoint();
	updateLocking();
}

void World::onRemovePoint(Point* iPoint) {
	mLockedPoints.remove( iPoint );
}

void World::setLocking(bool iLocked) {
	if( mIsLocked ) {
		if( !iLocked ) {
			// disabling lock
			mIsLocked = false;
		}
	}
	else {
		if( iLocked && mFrom) {
			// enabling lock
			mIsLocked = true;
			assert(mFrom);
			mLockedRay.set( mFrom->getLocation(), mCurrentPoint.getLocation() );
			//mLockedCurrentPoint = mCurrentPoint;
		}
	}
}
void World::updateLocking() {
	if( mIsLocked ) {
		mLockedCurrentPoint = mCurrentPoint;
		mCurrentPoint.setLocation( mLockedRay.getPoint(
				mLockedRay.getClosestPointOnLine(mCurrentPoint.getLocation())
			) );
	}
	else {
		struct On {
			static void Point(Point* iPoint, ::lunarlady::math::Ray& iCursorRay, TemporaryPoint* iFrom, TemporaryPoint* iCurrent, World* iWorld) {
				if( iFrom && iFrom->isPoint() && iFrom->point == iPoint ) return;
				// end point
				if( iCurrent->isPoint() ) return;

				// on line or midpoint
				if( iCurrent->isLine() ) return;
				

				bool locked = false;
				::lunarlady::math::vec3 lockedLocation(0,0,0);

				::lunarlady::math::CollisionResult collision;
				::lunarlady::math::Ray axisRay = iCursorRay; // temporary value
				::lunarlady::real intersection = 0;
				AxisLock axis = AL_NONE;

				if( !locked ) {
					axisRay.set(iPoint->location, iPoint->location + ::lunarlady::math::op::vec3::xAxisPositive);
					if( collision.collision(iCursorRay, axisRay, true, iWorld->getSuggestedRadius(), &intersection ) ) {
						locked = true;
						lockedLocation = axisRay.getPoint(intersection);
						axis = AL_X;
					}
				}

				if( !locked ) {
					axisRay.set(iPoint->location, iPoint->location + ::lunarlady::math::op::vec3::yAxisPositive);
					if( collision.collision(iCursorRay, axisRay, true, iWorld->getSuggestedRadius(), &intersection ) ) {
						locked = true;
						lockedLocation = axisRay.getPoint(intersection);
						axis = AL_Y;
					}
				}

				if( !locked ) {
					axisRay.set(iPoint->location, iPoint->location + ::lunarlady::math::op::vec3::zAxisPositive);
					if( collision.collision(iCursorRay, axisRay, true, iWorld->getSuggestedRadius(), &intersection ) ) {
						locked = true;
						lockedLocation = axisRay.getPoint(intersection);
						axis = AL_Z;
					}
				}

				if( locked ) {
					struct CheckAxis {
						static void OnLock(Lock& iPoint, ::lunarlady::math::Ray& iCursorRay, ::lunarlady::math::Ray& iCurrentLock, World* iWorld, ::lunarlady::math::vec3* iCollisionPoint) {
							::lunarlady::math::CollisionResult collision;
							::lunarlady::real intersection = 0;
							::lunarlady::real radius = iWorld->getSuggestedRadius();

							if( collision.collision(iPoint.axisRay, iCurrentLock, true, radius, &intersection) ) {
								::lunarlady::math::vec3 cp = iCurrentLock.getPoint(intersection);
								*iCollisionPoint = cp;
							}
						}
					};
					std::for_each(iCurrent->locks.begin(), iCurrent->locks.end(), boost::bind( &CheckAxis::OnLock, _1, iCursorRay, axisRay, iWorld, &lockedLocation) );

					if( iWorld->hasAxisLock() ) {
						assert( iFrom );
						::lunarlady::math::Ray axis = ::lunarlady::math::Ray::FromTo(iFrom->getLocation(), iCurrent->getLocation());
						lockedLocation = axis.getPoint( axis.getClosestPointOnLine(lockedLocation) );
					}

					if( !iCurrent->valid ) {
						iCurrent->setLocation(lockedLocation);
					}
					iCurrent->lockOn( lockedLocation, iPoint, axis, axisRay);
					
				}
			}
		};
		if( mToolAction == TA_ALL ) {
			std::for_each(mLockedPoints.begin(), mLockedPoints.end(), boost::bind( &On::Point, _1, mCursorRay, mFrom, &mCurrentPoint, this) );
		}
	}
}

bool World::hasAxisLock() {
	return mAxisLock != AL_NONE;
}

void DrawAxis(bool iApplyColor, ::lunarlady::real AXIS_LENGTH, ::lunarlady::real iEndAlpha) {
	glBegin(GL_LINES);
		if( iApplyColor ) glColor4d(1.0f, 0.0f, 0.0f, 1);
		glVertex3d(0.0f, 0.0f, 0.0f);
		
		if( iApplyColor ) glColor4d(1.0f, 0.0f, 0.0f, iEndAlpha);
		glVertex3d(AXIS_LENGTH, 0.0f, 0.0f);
		
		if( iApplyColor ) glColor4d(0.0f, 1.0f, 0.0f, 1);
		glVertex3d(0.0f, 0.0f, 0.0f);

		if( iApplyColor ) glColor4d(0.0f, 1.0f, 0.0f, iEndAlpha);
		glVertex3d(0.0f, AXIS_LENGTH, 0.0f);
		
		if( iApplyColor ) glColor4d(0.0f, 0.0f, 1.0f, 1);
		glVertex3d(0.0f, 0.0f, 0.0f);

		if( iApplyColor ) glColor4d(0.0f, 0.0f, 1.0f, iEndAlpha);
		glVertex3d(0.0f, 0.0f, AXIS_LENGTH);
	glEnd();
	
	glEnable(GL_LINE_STIPPLE);
	glLineStipple(1, 0x0F0F);
	glBegin(GL_LINES);
		if( iApplyColor ) glColor4d(1.0f, 0.0f, 0.0f, 1);
		glVertex3d(0.0f, 0.0f, 0.0f);

		if( iApplyColor ) glColor4d(1.0f, 0.0f, 0.0f, iEndAlpha);
		glVertex3d(-AXIS_LENGTH, 0.0f, 0.0f);
		
		if( iApplyColor ) glColor4d(0.0f, 1.0f, 0.0f, 1);
		glVertex3d(0.0f, 0.0f, 0.0f);

		if( iApplyColor ) glColor4d(0.0f, 1.0f, 0.0f, iEndAlpha);
		glVertex3d(0.0f, -AXIS_LENGTH, 0.0f);
		
		if( iApplyColor ) glColor4d(0.0f, 0.0f, 1.0f, 1);
		glVertex3d(0.0f, 0.0f, 0.0f);

		if( iApplyColor ) glColor4d(0.0f, 0.0f, 1.0f, iEndAlpha);
		glVertex3d(0.0f, 0.0f, -AXIS_LENGTH);
	glEnd();
	glDisable(GL_LINE_STIPPLE);

	if( iApplyColor ) glColor4d(0, 0, 0, 1);
}

void World::drawAxis(RenderMode iRenderMode, ::lunarlady::real AXIS_LENGTH, ::lunarlady::real iEndAlpha) {
	if( iRenderMode == RM_HALOED_LINE ) {
		glLineWidth(HALO_WIDTH);
		selectBackgroundColor();
		glDepthRange (HALO_DIFF, 1.0);
		DrawAxis(false, AXIS_LENGTH, iEndAlpha);
		glDepthRange (0.0, 1 - HALO_DIFF); 
		glLineWidth(1);
	}

	DrawAxis(true, AXIS_LENGTH, iEndAlpha);
}