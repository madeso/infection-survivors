#include <list>
#include <fstream>

#include "wx.hpp"
#include "wx_opengl.hpp"

#include "make_shape/World.hpp"


// http://www.flipcode.com/articles/article_tesselating.shtml


void CALLBACK  tcbCombine (GLdouble c[3], void *d[4], GLfloat w[4], void **out, void* data);

GLvoid CALLBACK  onError(GLenum err) {
	std::string str = (char *) gluErrorString(err);
}
struct VectorData {
	double x;
	double y;
	double z;
};

void CALLBACK  tcbBegin (GLenum prim, void *data);
void CALLBACK  tcbVertex (void *vertex, void *data);
void CALLBACK  tcbEnd (void *data);

class PolygonDrawer {
public:
	GLUtesselator* mTesselatorObject;
	std::list<VectorData> data;

	PolygonDrawer() {
		mTesselatorObject = gluNewTess();

		gluTessCallback (mTesselatorObject, GLU_TESS_BEGIN_DATA, reinterpret_cast<void (__stdcall *)(void)>(tcbBegin));
		gluTessCallback (mTesselatorObject, GLU_TESS_VERTEX_DATA, reinterpret_cast<void (__stdcall *)(void)>(tcbVertex));
		gluTessCallback (mTesselatorObject, GLU_TESS_END_DATA, reinterpret_cast<void (__stdcall *)(void)>(tcbEnd) );
		gluTessCallback (mTesselatorObject, GLU_TESS_COMBINE_DATA, reinterpret_cast<void (__stdcall *)(void)>(tcbCombine));
		
		//gluTessCallback(mTesselatorObject, GLU_ERROR,			reinterpret_cast<void (__stdcall *)(void)>(&onError) );
	}
	virtual ~PolygonDrawer() {
		gluDeleteTess(mTesselatorObject);
	}

	void setWindingRule(unsigned int pRule) {
		gluTessProperty(mTesselatorObject, GLU_TESS_WINDING_RULE, pRule);
	}
	void setBoundary(bool pBoundary) {
		gluTessProperty(mTesselatorObject, GLU_TESS_BOUNDARY_ONLY, pBoundary);
	}

	void contour_begin() {
		gluTessBeginContour(mTesselatorObject);
	}
	void contour_render(double x, double y, double z) {
		double* ptr = newVector(x, y, z);
		gluTessVertex (mTesselatorObject, ptr, ptr);
	}
	void contour_end() {
		gluTessEndContour(mTesselatorObject);
	}

	void polygon_begin() {
		gluTessBeginPolygon(mTesselatorObject, this);
	}
	void polygon_end() {
		gluTessEndPolygon(mTesselatorObject);
	}

	double* newVector(double x, double y, double z) {
		VectorData vec;
		vec.x = x;
		vec.y = y;
		vec.z = z;
		data.push_back( vec );
		return & (*(--data.end())).x;
	}

	virtual void begin(GLenum prim) {
		glBegin (prim);
	}
	virtual void vertex(double* vertex) {
		glVertex3dv (vertex);
	}
	virtual void end() {
		glEnd();
	}
};

void CALLBACK  tcbBegin (GLenum prim, void *data){
	PolygonDrawer* owner = (PolygonDrawer*) data;
	owner->begin(prim);
}

void CALLBACK  tcbVertex (void *vertex, void *data) {
	PolygonDrawer* owner = (PolygonDrawer*) data;
	owner->vertex((GLdouble *)vertex);
}
void CALLBACK  tcbEnd (void *data) {
	PolygonDrawer* owner = (PolygonDrawer*) data;
	owner->end();
}
void CALLBACK  tcbCombine (GLdouble c[3], void *d[4], GLfloat w[4], void **out, void* data){
	PolygonDrawer* owner = (PolygonDrawer*) data;
	*out = owner->newVector(c[0], c[1], c[2]);
}

Location::Location(const ::lunarlady::math::vec2& iLocation) : mLocation(iLocation) {
}

void Location::render() const {
	glVertex2d(mLocation.getX(), mLocation.getY());
}

void Location::render(PolygonDrawer& iDrawer) const {
	iDrawer.contour_render(mLocation.getX(), mLocation.getY(), 0);
}

void Location::moveTo(const ::lunarlady::math::vec2& iLocation) {
	mLocation = iLocation;
}

const ::lunarlady::math::vec2& Location::getLocation() const {
	return mLocation;
}

Triangle::Triangle() {
}
bool Triangle::collision(const ::lunarlady::math::vec2& iLocation) const {
	std::vector<::lunarlady::math::vec2> dir;
	for(unsigned int i=0; i< location.size(); ++i){
		dir.push_back((location[i] - iLocation).getNormalized());
	}
	::lunarlady::real sum = 0;
	for(unsigned int i=0; i< location.size()-1; ++i) {
		sum += ::lunarlady::math::acos(dir[i] dot dir[i+1]);
	}
	sum += ::lunarlady::math::acos(dir[location.size()-1] dot dir[0]);

	const bool result = ::lunarlady::math::equal3(sum, ::lunarlady::math::PI * 2);
	return result;
}
void Triangle::add(const ::lunarlady::math::vec2& iLocation) {
	location.push_back(iLocation);
}

Shape::Shape() {
}

void Shape::render(bool iOpen, const ::lunarlady::math::vec2& iLocation) const {
	std::size_t size = mLocations.size();
	if ( size == 0 ) return;

	if( iOpen ) {
		size++;
		if( size >= 1 ) {
			glBegin(GL_POINTS);
			renderLocations(iLocation);
			glEnd();
		}
		if( size >= 2 ) {
			glBegin(GL_LINE_STRIP);
			renderLocations(iLocation);
			glEnd();
		}
	}
	else {
		if( size >= 1 ) {
			glBegin(GL_POINTS);
			renderLocations();
			glEnd();
		}
		if( size >= 2 ) {
			glBegin(GL_LINE_LOOP);
			renderLocations();
			glEnd();
		}
	}
}

void Shape::fillShape(PolygonDrawer& iDrawer) const {
	if( mLocations.size() <3 ) return;
	iDrawer.contour_begin();
	struct RenderLocation {
		RenderLocation(PolygonDrawer& iDrawer) : mDrawer(iDrawer) {
		}
		void operator()(const LocationPtr& iLocation) {
			iLocation->render(mDrawer);
		}
		PolygonDrawer& mDrawer;
	};
	std::for_each(mLocations.begin(), mLocations.end(), RenderLocation(iDrawer) );
	iDrawer.contour_end();
}

void Shape::renderLocations(const ::lunarlady::math::vec2& iLocation) const {
	struct RenderLocation {
		RenderLocation(Location* iLocation) : mLocation(iLocation) {
		}
		void operator()(const LocationPtr& iLocation) {
			if( iLocation.get() == mLocation ) {
				glColor3d(0.5, 0.5, 1);
			}
			else {
				glColor3d(0, 0, 0);
			}
			iLocation->render();
		}
		Location* mLocation;
	};
	std::for_each(mLocations.begin(), mLocations.end(), RenderLocation( selectPoint(iLocation) ));
	glVertex2d(iLocation.getX(), iLocation.getY());
}

void Shape::renderLocations() const {
	struct RenderLocation {
		void operator()(const LocationPtr& iLocation) {
			iLocation->render();
		}
	};
	std::for_each(mLocations.begin(), mLocations.end(), RenderLocation());
}

void Shape::addPoint(const ::lunarlady::math::vec2& iLocation) {
	LocationPtr location( new Location(iLocation) );
	mLocations.push_back(location);
	buildCollisionMesh();
}

struct PolygonBuilder : PolygonDrawer {
	PolygonBuilder(::std::vector<Triangle>* oGeoms) : mGeoms(oGeoms), mPrim(GL_TRIANGLES) {
		mGeoms->clear();
	}

	virtual void begin(GLenum prim) {
		const bool tri = prim == GL_TRIANGLES;
		const bool trif = prim == GL_TRIANGLE_FAN;
		const bool tris = prim == GL_TRIANGLE_STRIP;
		const bool quad = prim == GL_QUADS;
		const bool quads = prim == GL_QUAD_STRIP;
		const bool poly = prim == GL_POLYGON;
		
		mPrim = prim;
	}
	virtual void vertex(double* vertex) {
		mMemory.push_back( ::lunarlady::math::vec2(vertex[0], vertex[1]) );
	}
	virtual void end() {
		switch(mPrim) {
			case GL_TRIANGLES:
				// Treats each triplet of vertices as an independent triangle
				{
					for(unsigned int i=0; i<mMemory.size()-2; i+=3) {
						Triangle tri;
						tri.add(mMemory[i]);
						tri.add(mMemory[i+1]);
						tri.add(mMemory[i+2]);
						mGeoms->push_back( tri );
					}
				}
				break;
			case GL_TRIANGLE_FAN:
				{
					//  Draws a connected group of triangles. One triangle is defined for each vertex presented after the first two vertices. 
					for(unsigned int i=1; i<mMemory.size()-1; ++i) {
						Triangle tri;
						tri.add(mMemory[0]);
						tri.add(mMemory[i]);
						tri.add(mMemory[i+1]);
						mGeoms->push_back( tri );
					}
				}
				break;
			case GL_TRIANGLE_STRIP:
				{
					// Draws a connected group of triangles. One triangle is defined for each vertex presented after the first two vertices.
					for(unsigned int i=0; i<mMemory.size()-2; ++i) {
						Triangle tri;
						if( i % 2 == 0 ) {
							tri.add(mMemory[i]);
							tri.add(mMemory[i+1]);
							tri.add(mMemory[i+2]);
						}
						else {
							tri.add(mMemory[i+1]);
							tri.add(mMemory[i]);
							tri.add(mMemory[i+2]);
						}
						mGeoms->push_back( tri );
					}
				}
				break;
			default:
				{
					int i=0;
					i++;
				}
				break;
		}
		mMemory.clear();
	}

	::std::vector<Triangle>* mGeoms;
	::std::vector<::lunarlady::math::vec2> mMemory;
	GLenum mPrim;
};

void Shape::buildCollisionMesh() {
	if( mLocations.size() <3 ) return;
	PolygonBuilder builder(&mCollisionGeometry);

	builder.polygon_begin();
	fillShape(builder);
	builder.polygon_end();
}

bool Shape::isValid() const {
	return mLocations.size() >=3;
}

bool Shape::over(const ::lunarlady::math::vec2& iLocation) const {
	struct Collision {
		Collision(bool& iResult, const ::lunarlady::math::vec2& iLocation) : mLocation(iLocation), result(iResult) {
			result = false;
		}

		void operator()(const Triangle& iTriangle) {
			result = result || iTriangle.collision(mLocation);
		}

		bool& result;
		const ::lunarlady::math::vec2 mLocation;
	};

	bool result = false;
	std::for_each(mCollisionGeometry.begin(), mCollisionGeometry.end(), Collision(result, iLocation) );
	return result;
}

Location* Shape::selectPoint(const ::lunarlady::math::vec2& iLocation) const {
	Location* result = 0;
	struct FindPoint {
		FindPoint(Location** iResult, const ::lunarlady::math::vec2& iLocation) : mResult(iResult), mLocation(iLocation) {
		}
		void operator()(const LocationPtr& iLocation) {
			::lunarlady::real diff = (iLocation->getLocation() - mLocation).getLength();
			if( diff < 0.009 ) {
				*mResult = iLocation.get();
			}
		}

		Location** mResult;
		const ::lunarlady::math::vec2 mLocation;
	};
	std::for_each(mLocations.begin(), mLocations.end(), FindPoint(&result, iLocation) );
	return result;
}

World::World() : mAspect(14.0 / 9.0), mCursorLocation(0.5, 0.5), mValidCursorLocation(false), mCurrentShape(0), mCurrentLocation(0), mRed(0.5), mGreen(0.5), mBlue(1.0){
}

void World::setAspect(::lunarlady::real iAspect) {
	mAspect = iAspect;
}

bool World::selectPoint() {
	if( mCurrentShape ) {
		mCurrentLocation = mCurrentShape->selectPoint(mCursorLocation);
		return mCurrentLocation != 0;
	}
	return false;
}

void World::fillShapes(PolygonDrawer& iDrawer, Shape* iOver) const {
	struct FillShape {
		FillShape(Shape* iOver, PolygonDrawer& iDrawer) : mOver(iOver), mDrawer(iDrawer){
		}
		void operator()(const ShapePtr& iShape) {
			if( iShape.get() != mOver ) {
				iShape->fillShape(mDrawer);
			}
		}

		Shape* mOver;
		PolygonDrawer& mDrawer;
	};

	iDrawer.polygon_begin();
	std::for_each(mShapes.begin(), mShapes.end(), FillShape(iOver, iDrawer) );
	iDrawer.polygon_end();
}

void World::render() const {
	glPointSize(3.0f);
	if( mValidCursorLocation ) {
		glColor3d(0, 0, 1);
		glBegin(GL_POINTS);
			glVertex2d(mCursorLocation.getX(), mCursorLocation.getY());
		glEnd();
	}

	struct RenderShape {
		RenderShape(const ::lunarlady::math::vec2& iLocation, Shape* iShape) : mLocation(iLocation), mShape(iShape) {
		}
		void operator()(const ShapePtr& iShape) {
			bool open = iShape.get() == mShape;
			iShape->render(open, mLocation);
		}
		const ::lunarlady::math::vec2 mLocation;
		Shape* mShape;
	};

	{
		sendColorToOpenGL();
		PolygonDrawer drawer;
		fillShapes(drawer/*, getOverShape()*/ );

		/*std::vector<Triangle> triangles;
		PolygonBuilder builder(&triangles);
		fillShapes(builder);

		glBegin(GL_TRIANGLES);
		const unsigned int triSize = triangles.size();
		for(unsigned int i=0; i<triSize; ++i) {
			assert(triangles[i].location.size() == 3 );
			for(int j=0; j<3; ++j) {
				::lunarlady::real x = triangles[i].location[j].getX();
				::lunarlady::real y = triangles[i].location[j].getY();
				glVertex2d(x, y);
			}
		}
		glEnd();*/
	}

	glColor3d(0,0,0);
	std::for_each(mShapes.begin(), mShapes.end(), RenderShape(mCursorLocation, mCurrentShape) );

	glPointSize(1.0f);
}

void World::placePoint() {
	if( !mCurrentShape ) {
		newShape();
	}
	mCurrentShape->addPoint( mCursorLocation );
}

void World::newShape() {
	removeInvalidShapes();
	ShapePtr shape ( new Shape() );
	mShapes.push_back( shape );
	mCurrentShape = shape.get();
}

void World::removeInvalidShapes() {
	struct InvalidShape {
		static bool Test(ShapePtr& iShape) {
			return !iShape->isValid();
		}
	};
	mShapes.erase( std::remove_if(mShapes.begin(), mShapes.end(), &InvalidShape::Test), mShapes.end() );
	mCurrentShape = 0;
}

Shape* World::getOverShape() const {
	Shape* result = 0;
	struct FindShape {
		FindShape(Shape** result, const ::lunarlady::math::vec2& location) : mShape(result), mLocation(location) {
		}
		void operator()(const ShapePtr& iShape) {
			//if( *mShape ) return;
			if( iShape->over(mLocation) ) {
				*mShape = iShape.get();
			}
		}

		Shape** mShape;
		::lunarlady::math::vec2 mLocation;
	};

	std::for_each(mShapes.begin(), mShapes.end(), FindShape(&result, mCursorLocation) );

	return result;
}

void World::exportToFile(const std::string& iFileName, const std::string& iTextureName) {
	std::ofstream file(iFileName.c_str(), std::ios::out | std::ios::binary);
	if( !file ) return;
	file.write((char*) &mRed, sizeof(::lunarlady::real));
	file.write((char*) &mGreen, sizeof(::lunarlady::real));
	file.write((char*) &mBlue, sizeof(::lunarlady::real));

 	const unsigned int textureSize = iTextureName.length();
	file.write((char*) &textureSize, sizeof(unsigned int));
	file.write( iTextureName.c_str(), sizeof(char) * iTextureName.length() );

	std::vector<Triangle> triangles;
	PolygonBuilder builder(&triangles);
	fillShapes(builder);

	const unsigned int triSize = triangles.size();
	file.write( (char*) &triSize, sizeof(unsigned int) );


	::lunarlady::real xmax=triangles[0].location[0].getX(), ymax=triangles[0].location[0].getY();
	::lunarlady::real xmin=triangles[0].location[0].getX(), ymin=triangles[0].location[0].getY();

	for(unsigned int i=0; i<triSize; ++i) {
		assert(triangles[i].location.size() == 3 );
		for(int j=0; j<3; ++j) {
			::lunarlady::real x = triangles[i].location[j].getX();
			::lunarlady::real y = triangles[i].location[j].getY();
			xmax = ::lunarlady::math::Max(xmax, x);
			ymax = ::lunarlady::math::Max(ymax, y);
			xmin = ::lunarlady::math::Min(xmin, x);
			ymin = ::lunarlady::math::Min(ymin, y);
		}
	}


	const ::lunarlady::real width = xmax-xmin;
	const ::lunarlady::real height = ymax-ymin;
	const ::lunarlady::real size = ::lunarlady::math::Max(width, height);

	for(unsigned int i=0; i<triSize; ++i) {
		assert(triangles[i].location.size() == 3 );
		for(int j=0; j<3; ++j) {
			::lunarlady::real x = triangles[i].location[j].getX();
			::lunarlady::real y = triangles[i].location[j].getY();
			file.write((char*) &x, sizeof(::lunarlady::real));
			file.write((char*) &y, sizeof(::lunarlady::real));

			// write uv coordinates
			::lunarlady::real u = (x-xmin)/size;
			::lunarlady::real v = (y-ymin)/size;
			file.write((char*) &u, sizeof(::lunarlady::real));
			file.write((char*) &v, sizeof(::lunarlady::real));
		}
	}
	file.close();
}

void World::setCursorLocation( const int iWidth, const int iHeight, const ::lunarlady::math::vec2& iMovement ) {
	mValidCursorLocation = false;
	::lunarlady::real x = iMovement.getX();
	::lunarlady::real y = iMovement.getY();

	const ::lunarlady::real aspect = (::lunarlady::real) iWidth / (::lunarlady::real) iHeight;
	const ::lunarlady::real raspect = mAspect;
	if( ::lunarlady::math::equal(aspect, mAspect) ) {
		y /= iHeight;
		x /= iWidth;
	}
	else {
		if( aspect > mAspect ) {
			// current width is greater than supported
			// base new on current height
			const ::lunarlady::real width = iHeight * mAspect;
			const ::lunarlady::real theRest = iWidth - width;
			const ::lunarlady::real borderSize = theRest / 2.0;

			x = (x - borderSize)/ width;
			y /= iHeight;
		}
		else {
			// current height is greater than supported
			// base new on current width
			const ::lunarlady::real height = iWidth / mAspect;
			const ::lunarlady::real theRest = iHeight - height;
			const ::lunarlady::real borderSize = theRest / 2.0;

			x /= iWidth;
			y = (y - borderSize)/ height;
		}
	}

	mValidCursorLocation = true;

	if( y > 1 ) y = 1;
	if( y < 0 ) y = 0;

	if( x > 1 ) x = 1;
	if( x < 0 ) x = 0;

	mCursorLocation = ::lunarlady::math::vec2(x*mAspect, y);

	if( mCurrentLocation ) {
		mCurrentLocation->moveTo( mCursorLocation );
	}
}

void World::selectShape() {
	if( mCurrentLocation != 0 ) {
		mCurrentLocation = 0;
		mCurrentShape->buildCollisionMesh();
		return;
	}
	if( mCurrentShape != 0 ) {
		mCurrentShape = 0;
		removeInvalidShapes();
		return;
	}
	mCurrentShape = getOverShape();
}

void World::setColor( ::lunarlady::real iRed, ::lunarlady::real iGreen, ::lunarlady::real iBlue) {
	mRed = iRed;
	mGreen = iGreen;
	mBlue = iBlue;
}

void World::renderBorder(int x, int y, int iWidth, int iHeight) const {
	const ::lunarlady::real aspect = (::lunarlady::real) iWidth / (::lunarlady::real) iHeight;
	glViewport(x, y, iWidth, iHeight);
	glClear(GL_COLOR_BUFFER_BIT);
	const ::lunarlady::real raspect = mAspect;
	if( ::lunarlady::math::equal(aspect, mAspect) ) {
		// equal, dont draw anything, draw in fullscreen
	}
	else {
		if( aspect > mAspect ) {
			// current width is greater than supported
			// base new on current height
			const ::lunarlady::real width = iHeight * mAspect;
			const ::lunarlady::real theRest = iWidth - width;
			const ::lunarlady::real borderSize = theRest / 2.0;

			glViewport(x+borderSize, y, width, iHeight);
		}
		else {
			// current height is greater than supported
			// base new on current width
			const ::lunarlady::real height = iWidth / mAspect;
			const ::lunarlady::real theRest = iHeight - height;
			const ::lunarlady::real borderSize = theRest / 2.0;
			glViewport(x, y+borderSize, iWidth, height);
		}
	}

	// background
	const ::lunarlady::real width = mAspect;
	const ::lunarlady::real height = 1;

	// setup 2d
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, 0, height, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);

	glColor3d(1, 1, 1);
	glBegin(GL_QUADS);
		glVertex2d(0, 0);
		glVertex2d(width, 0);
		glVertex2d(width, height);
		glVertex2d(0, height);
	glEnd();
}

void World::sendColorToOpenGL() const {
	glColor3d(mRed, mGreen, mBlue);
}