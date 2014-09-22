#ifndef COLOR_HPP
#define COLOR_HPP

#include <map>
#include <string>
#include <sstream>

namespace lunarlady {

class Color {
public:
	enum Data {
		Black,
		Silver,
		Maroon,
		Red,
		Navy,
		Blue,
		Purple,
		Fuchia,
		Green,
		Lime,
		Olive,
		Yellow,
		Teal,
		Aqua,
		Gray,
		White,
		Orange,
		COUNT
	};

	Color(Data pd);
	explicit Color(const std::string& pString);

	// assignment
	void operator=(const Color& pOther);
	void operator=(const Data& pOther);

	// equality
	bool operator==(const Color& pOther) const;
	bool operator==(const Data& pOther) const;

	// inequality
	bool operator!=(const Color& pOther) const;
	bool operator!=(const Data& pOther) const;

	static const char* asString(Data d);
	static const Data asData(const std::string& pString);
	static bool isValid(Data d);
	const char* toString() const;
	static bool isEqual(const Data& a, const Data& b);

	operator Data() const;

private:
	Color();
	Data d;

	typedef std::map<std::string, Data> LookupMap;
	typedef std::pair<std::string, Data> LookupPair;

	static LookupMap sStringToDataMap;
	static const LookupMap buildMap();
};

} // lunarlady

std::ostream& operator<<( std::ostream& os, const lunarlady::Color& e );
bool operator!=(const lunarlady::Color::Data& pOther, const lunarlady::Color& e );
bool operator==(const lunarlady::Color::Data& pOther, const lunarlady::Color& e);

#endif //COLOR_HPP
