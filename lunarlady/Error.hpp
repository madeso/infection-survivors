#ifndef LL_ERROR_HPP
#define LL_ERROR_HPP

namespace lunarlady {
#define Throw(str) do { std::ostringstream str; str << msg; throw std::runtime_error(str.str()); } while(false);

	class BasicError : public std::runtime_error {
	public:
		BasicError(const std::string& iType, const std::string& iWhat) : std::runtime_error(iType + std::string(": ") + iWhat) {
		}
	};

#define OpenGLError(msg) BasicError("OpenGL",  msg)
#define DevILError(msg) BasicError("DevIL",  msg)
#define PhysFSError(msg) BasicError("PhysFS", msg)
#define XmlError(msg) BasicError("TinyXML", msg)
#define MissingStringFormatError(msg) BasicError("Missing string format", msg)
#define MissingStringError(msg) BasicError("Missing string", msg)
#define FileDataError(msg) BasicError("File data error", msg)
#define SettingsError(msg) BasicError("Settings error", msg)
#define ScriptError(msg) BasicError("Script error", msg)
#define MissingComponent3Error(componentName) BasicError("Missing component ", componentName);
#define MissingObject3Error(objectName) BasicError("Missing object ", objectName);
#define Message3NotHandledError(message) BasicError("Didn't handle the message", message);

#define SoundError(msg) BasicError("Sound error", msg)
}

#endif