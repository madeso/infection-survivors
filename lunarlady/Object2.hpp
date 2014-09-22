#ifndef LL_OBJECT2_HPP
#define LL_OBJECT2_HPP

#include "boost/smart_ptr.hpp"
#include "boost/utility.hpp"

#include "lunarlady/Types.hpp"
#include "lunarlady/Rgba.hpp"

namespace lunarlady {
	class Object2 : boost::noncopyable {
	public:
		Object2();
		virtual ~Object2();

		bool isAlive() const;

		bool isEnabled() const;
		void enable();
		void disable();

		void update(real iTime);
		void render(real iTime);
	protected:
		bool mIsAlive;

		void quad(Rgba iColor, real iLeft, real iRight, real iTop, real iBottom);

		virtual void doUpdate(real iTime) = 0;
		virtual void doRender(real iTime) = 0;

	private:
		bool mIsEnabled;
	};
	typedef boost::shared_ptr<Object2> Object2Ptr;
}

#endif