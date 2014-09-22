#include <list>

#include "lunarlady/World2.hpp"
#include "lunarlady/Object2.hpp"

#include "lunarlady/OpenGL_util.hpp"

namespace lunarlady {
	struct World2::World2Pimpl {
		World2Pimpl() {
		}

		~World2Pimpl() {
		}

		void add(Object2* iObject) {
			Object2Ptr object(iObject);
			mObjects.push_back(object);
		}
		
		void render(real iTime) {
			SetDisplay2d( );
			struct RenderObjects {
				RenderObjects(real iTime) : mTime(iTime) {
				}
				void operator()(Object2Ptr& iObject) {
					iObject->render(mTime);
				}
				real mTime;
			};
			std::for_each(mObjects.begin(), mObjects.end(), RenderObjects(iTime) );
		}

		void update(real iTime) {
			struct UpdateObjects {
				UpdateObjects(real iTime) : mTime(iTime) {
				}
				void operator()(Object2Ptr& iObject) {
					iObject->update(mTime);
				}
				static bool RemoveDeadObjects(Object2Ptr& iObject) {
					return ! iObject->isAlive();
				}
				real mTime;
			};
			std::for_each(mObjects.begin(), mObjects.end(), UpdateObjects(iTime) );
			mObjects.erase(std::remove_if(mObjects.begin(), mObjects.end(), &UpdateObjects::RemoveDeadObjects)
				, mObjects.end());
		}
		std::list<Object2Ptr> mObjects;
	};

	World2::World2() : mPimpl( new World2Pimpl() ) {
	}

	World2::~World2() {
	}

	void World2::add(Object2* iObject) {
		mPimpl->add(iObject);
	}
		
	void World2::render(real iTime) {
		mPimpl->render(iTime);
	}
	void World2::update(real iTime) {
		mPimpl->update(iTime);
	}
}