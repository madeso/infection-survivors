#ifndef LL_SYSTEM_HPP
#define LL_SYSTEM_HPP

#include <string>

#include "boost/smart_ptr.hpp"
#include "boost/utility.hpp"

#include "lunarlady/Types.hpp"

namespace lunarlady {
	class System : boost::noncopyable {
	public:
		System(const std::string& iName);
		virtual ~System();
		const std::string& getName() const;

		virtual void step(real iTime) = 0;
	private:
		const std::string mName;
	};
	typedef boost::shared_ptr<System> SystemPtr;
}

#endif