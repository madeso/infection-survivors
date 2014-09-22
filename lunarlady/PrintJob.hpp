#ifndef LL_PRINT_JOB_HPP
#define LL_PRINT_JOB_HPP

#include "lunarlady/Font.hpp"
#include "lunarlady/math/vec2.hpp"
#include "boost/smart_ptr.hpp"
#include "boost/utility.hpp"
#include <map>
#include <list>

namespace lunarlady {
	typedef std::pair<std::string, std::wstring> Argument;
	typedef std::map<std::string, std::wstring> ArgumentMap;

	enum Justification {
		JUSTIFY_LEFT,
		JUSTIFY_CENTER,
		JUSTIFY_RIGHT
	};

	class PrintArguments : boost::noncopyable {
	public:
		PrintArguments(Font& iFont, const ArgumentMap& iArgumentMap);

		Font& font;
		const ArgumentMap& argumentMap;
	};

	class PrintComponent : boost::noncopyable {
	public:
		virtual ~PrintComponent() {}
		virtual void apply(PrintArguments& iArgument) = 0;
	};
	typedef boost::shared_ptr<PrintComponent> PrintComponentPtr;

	class PrintJob {
	public:
		explicit PrintJob(const std::string& iText);
		void print(Font& iFont, real iSize, const math::vec2 iLocation, Justification iJustification, real* oWidth, real* iCashedWidth, const ArgumentMap& iArguments) const;
		real getWidth(Font& iFont, real iSize, const ArgumentMap& iArguments) const;
	private:
		void insert(PrintComponent* iComponent);
		typedef std::list<PrintComponentPtr> PrintComponentList;
		PrintComponentList mPrintComponents;
	};
}

#endif