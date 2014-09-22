#ifndef LL_STATIC_OBJECT2_HPP
#define LL_STATIC_OBJECT2_HPP

#include <string>
#include <list>
#include <map>
#include <memory>
#include "boost/function.hpp"

#include "lunarlady/Object2.hpp"
#include "lunarlady/math/vec2.hpp"
#include "lunarlady/Image.hpp"
#include "lunarlady/Xml.hpp"
#include "boost/utility.hpp"
#include "sgl/Key.hpp"

namespace lunarlady {
	class World2;

	class Component2;
	class Data2;
	class ComponentObject2;

	class Data2 : boost::noncopyable {
	public:
		Data2() {}
		virtual ~Data2() {}
	private:
	};
	typedef boost::shared_ptr<Data2> DataPtr2;
	typedef std::map<std::string, DataPtr2> DataMap2;

	enum MessageType {
		MT_UPDATE,
		MT_LOAD,
		MT_INIT,
		MT_KEY,
		MT_MOUSE_MOVE,
		MT_FILE_DEFINED,
		MT_HOVER,
		MT_HOVER_CHANGED,
		MT_FOCUS,
		MT_CHAR,
		MT_ENABLE,
		MT_TIMER,
		MT_ONTOP,
		MT_USER
	};

	class Message : boost::noncopyable {
	public:
		Message(const MessageType& iType) : mMessageType(iType) {}
		const MessageType& getType() const { return mMessageType; }; 
	private:
		MessageType mMessageType;
	};

	class Display2;

	class Component2 : boost::noncopyable {
	public:
		Component2(ComponentObject2& iOwner) : mOwner(iOwner) { }
		virtual ~Component2() { }
		virtual void onMessage(const Message& iMessage) {};
		virtual Display2* asDisplay() {return 0;}
	protected:
		ComponentObject2& mOwner;
	};
	typedef boost::shared_ptr<Component2> ComponentPtr2;
	typedef std::list<ComponentPtr2> ComponentList2;

	struct MessageUpdate : public Message{
		explicit MessageUpdate(real iTime) : Message(MT_UPDATE), time(iTime) {}
		real time;
	};
	struct MessageChar : public Message{
		explicit MessageChar(wchar_t iChar) : Message(MT_CHAR), c(iChar) {}
		wchar_t c;
	};
	struct MessageInit : public Message{
		explicit MessageInit() : Message(MT_INIT) {}
	};
	struct MessageLoad : public Message{
		explicit MessageLoad() : Message(MT_LOAD) {}
	};
	struct MessageEnable : public Message{
		explicit MessageEnable(bool iEnabled) : Message(MT_ENABLE), enabled(iEnabled) {}
		bool enabled;
	};
	struct MessageFocus : public Message{
		explicit MessageFocus(bool iGotFocus) : Message(MT_FOCUS), focus(iGotFocus) {}
		bool focus;
	};
	struct MessageTimer : public Message{
		explicit MessageTimer() : Message(MT_TIMER) {}
		real time;
	};
	struct MessageFile : public Message{
		explicit MessageFile(const std::string& iName, const math::vec2& iLocation) : Message(MT_FILE_DEFINED), name(iName), location(iLocation) {}
		std::string name;
		math::vec2 location;
	};
	struct MessageHover : public Message {
		explicit MessageHover(const math::vec2& iLocation) : Message(MT_HOVER), location(iLocation) {}
		math::vec2 location;
	};
	struct MessageHoverChanged : public Message {
		explicit MessageHoverChanged(bool iEnter) : Message(MT_HOVER_CHANGED), enter(iEnter) {}
		bool enter;
	};
	struct MessageKey : public Message {
		MessageKey(const sgl::Key& iKey, bool iDown) : Message(MT_KEY), key(iKey), down(iDown) {}
		sgl::Key key;
		bool down;
	};
	struct MessageMouseMove : public Message {
		explicit MessageMouseMove(const math::vec2& iMovement) : Message(MT_MOUSE_MOVE), movement(iMovement) {}
		math::vec2 movement;
	};
	struct MessageOnTop : public Message {
		explicit MessageOnTop(bool iTop) : Message(MT_ONTOP), top(iTop) {}
		bool top;
	};

	template<class Type>
	class DataType2 : public Data2 {
	public:
		DataType2() {}
		explicit DataType2(const Type& iType) : t(iType) {}
		Type& getReference() {
			return t;
		}
		Type* getPointer() {
			return &t;
		}
	private:
		Type t;
	};

	class Display2 : public Component2 {
	public:
		Display2(ComponentObject2& iOwner) : Component2(iOwner) { }
		virtual ~Display2() {}
		virtual void display(real iTime) = 0;
		Display2* asDisplay() { return this; }
	};

	class StatesObject2;
	typedef boost::shared_ptr<StatesObject2> StatesObjectPtr2;
	typedef std::list<StatesObjectPtr2> StatesObjectList2;

	typedef boost::function<Component2* (StatesObject2*, ComponentObject2&, TiXmlElement*)> BuildComponentFunction2;
	typedef std::map<std::string, BuildComponentFunction2> BuildComponentMap2;

	class ComponentObjectContainer2;

	class ComponentObject2 : public Object2 {
	public:
		ComponentObject2(TiXmlElement* iElement, ComponentObjectContainer2& iContainer, StatesObject2* iStateObject);

		void doUpdate(real iTime);
		void doRender(real iTime);

		Data2& getOrCreateData(const std::string& iType, const std::string& iName, TiXmlElement* iObjectElement, bool iAddName=false);
		Data2* findData(const std::string& iName);

		void broadcast_object(const Message& iMessage);
		void broadcast_system(const Message& iMessage);

		World2& getWorld();
		const std::string getBaseFont() const;

		void setBroadcastEnable(bool isEnable);
		bool isBroadcastEnabled() const;
	private:
		ComponentObjectContainer2& mContainer;
		Display2* mDisplay;
		ComponentList2 mComponents;
		DataMap2 mDatas;
		bool mIsBroadcastEnabled;
	};
	typedef std::list<ComponentObject2*> ComponentObjectList2;

	class ComponentObjectContainer2 : boost::noncopyable {
	public:
		ComponentObjectContainer2(World2& iWorld);

		void load(const std::string iFileName);
		void broadcast(const Message& iMessage, ComponentObject2* iIgnoreThis=0);
		void registerBuilder(const std::string& iName, BuildComponentFunction2 iFunction);
		void registerDefaultBuilders();

		void sendKey( sgl::Key iKey, bool iDown);
		void sendMouseMovement(const math::vec2& iMovement);
		void sendChar(wchar_t iChar);

		// this must be called for enter-messages to be processed
		void step();

		Component2* buildComponent(ComponentObject2& iObject, const std::string& iComponentName, TiXmlElement* iChild, StatesObject2* iStateObject);
		
		ComponentObject2* addObject(TiXmlElement* iObjectElement, StatesObject2* iStateObject);
		World2& getWorld();
		const std::string getBaseFont() const;

		void onFocus(bool iGained);

		void init();
	protected:
		void addStates(TiXmlElement* iObjectElement);
		void includeFile(const std::string& iFileName);
		void handleChild(TiXmlElement* child);
	private:
		World2& mWorld;
		StatesObjectList2 mStates;
		ComponentObjectList2 mObjects;
		BuildComponentMap2 mComponentsBuilder;
		std::string mBaseFont;
		bool mHasSentFocusTrue;
		bool mIsInitialized;
	};
}

#endif