#include "Color.hpp"

#include <cassert>

namespace lunarlady {

Color::Color(Data pd) :d(pd) {
	assert(Color::isValid(pd));
}

Color::Color(const std::string& pString) :d( asData(pString) ) {
}

// assignment
void Color::operator=(const Color& pOther) {
	assert(Color::isValid(pOther.d));
	d = pOther.d;
}

void Color::operator=(const Data& pOther) {
	assert(Color::isValid(pOther));
	d = pOther;
}

// equality
bool Color::operator==(const Color& pOther) const {
	return Color::isEqual(d, pOther.d);
}

bool Color::operator==(const Data& pOther) const {
	return Color::isEqual(d, pOther);
}

// inequality
bool Color::operator!=(const Color& pOther) const {
	return !Color::isEqual(d, pOther.d);
}

bool Color::operator!=(const Data& pOther) const {
	return !Color::isEqual(d, pOther);
}

const char* Color::asString(Data d) {
	switch(d) {
		case Black:
			return "Black";
		case Silver:
			return "Silver";
		case Maroon:
			return "Maroon";
		case Red:
			return "Red";
		case Navy:
			return "Navy";
		case Blue:
			return "Blue";
		case Purple:
			return "Purple";
		case Fuchia:
			return "Fuchia";
		case Green:
			return "Green";
		case Lime:
			return "Lime";
		case Olive:
			return "Olive";
		case Yellow:
			return "Yellow";
		case Teal:
			return "Teal";
		case Aqua:
			return "Aqua";
		case Gray:
			return "Gray";
		case White:
			return "White";
		case Orange:
			return "Orange";
		default:
			return "Unknown";
	}
}

const Color::Data Color::asData(const std::string& pString) {
	LookupMap::const_iterator result = sStringToDataMap.find(pString);
		if( result == sStringToDataMap.end() ) {
		std::ostringstream message;
		message << pString << " is not a valid Color";
		throw std::runtime_error(message.str());
	}

	// this should not assert
	assert( isValid(result->second) );
	return result->second;
}

bool Color::isValid(Data d) {
	switch(d) {
		case Black:
		case Silver:
		case Maroon:
		case Red:
		case Navy:
		case Blue:
		case Purple:
		case Fuchia:
		case Green:
		case Lime:
		case Olive:
		case Yellow:
		case Teal:
		case Aqua:
		case Gray:
		case White:
		case Orange:
			return true;
		default:
			return false;;
	}
}

const char* Color::toString() const {
	return asString(d);
}

bool Color::isEqual(const Data& a, const Data& b) {
	assert(Color::isValid(a));
	assert(Color::isValid(b));
	return a == b;
}

Color::operator Data() const {
	return d;
}

Color::Color() {
}
const Color::LookupMap Color::buildMap() {
	LookupMap map;
	map.insert(LookupPair("black", Black) );
	map.insert(LookupPair("silver", Silver) );
	map.insert(LookupPair("maroon", Maroon) );
	map.insert(LookupPair("red", Red) );
	map.insert(LookupPair("navy", Navy) );
	map.insert(LookupPair("blue", Blue) );
	map.insert(LookupPair("purple", Purple) );
	map.insert(LookupPair("fuchia", Fuchia) );
	map.insert(LookupPair("green", Green) );
	map.insert(LookupPair("lime", Lime) );
	map.insert(LookupPair("olive", Olive) );
	map.insert(LookupPair("yellow", Yellow) );
	map.insert(LookupPair("teal", Teal) );
	map.insert(LookupPair("aqua", Aqua) );
	map.insert(LookupPair("gray", Gray) );
	map.insert(LookupPair("white", White) );
	map.insert(LookupPair("orange", Orange) );
	return map;
}

Color::LookupMap Color::sStringToDataMap = Color::buildMap();

} // lunarlady

std::ostream& operator<<( std::ostream& os, const lunarlady::Color& e ) {
	os << e.toString();
	return os;
}
bool operator!=(const lunarlady::Color::Data& pOther, const lunarlady::Color& e ) {
	return e != pOther;
}

bool operator==(const lunarlady::Color::Data& pOther, const lunarlady::Color& e) {
	return e == pOther;
}
