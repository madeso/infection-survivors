#include "lunarlady/ComponentObject2.hpp"
#include "sgl/sgl_Opengl.hpp"
#include "sgl/sgl_Assert.hpp"
#include "lunarlady/Game.hpp"
#include "lunarlady/File.hpp"
#include "lunarlady/Log.hpp"
#include "lunarlady/World2.hpp"
#include "lunarlady/StringUtils.hpp"
#include "lunarlady/Font.hpp"
#include "lunarlady/Printer.hpp"
#include "lunarlady/Language.hpp"
#include "lunarlady/Message.hpp"
#include "lunarlady/Script.hpp"
#include "lunarlady/math/Random.hpp"
#include "lunarlady/Error.hpp"

#include <fstream>

namespace lunarlady {

	namespace {
		typedef std::list<ComponentObject2*> ComponentObjectNakedList2;

		class State2 {
		public:
			State2(StatesObject2* iStateObject, ComponentObjectContainer2& iContainer, TiXmlElement* iElement) : mCanBeDisabled(true), mEnabled(true) {
				{
					const char* name = iElement->Attribute("name");
					if( name ) {
						mName = name;
					}
				}
				{
					const std::string* alwaysShow = iElement->Attribute(std::string("alwaysShow"));
					if( alwaysShow && ToLower(*alwaysShow) == "yes" ) {
						mCanBeDisabled = false;
					}
				}
				if( const char* leave = iElement->Attribute("leave")) {
					mOnLeave = leave;
				}
				if( const char* enter = iElement->Attribute("enter")) {
					mOnEnter = enter;
				}

				const std::string objectName = "object";
				for(TiXmlElement* child=iElement->FirstChildElement(objectName); child; child = child->NextSiblingElement(objectName) ) {
					ComponentObject2* object = iContainer.addObject(child, iStateObject);
					mObjects.push_back(object);
				}
			}
			const std::string& getName() const {
				return mName;
			}
			bool canBeDisabled() const {
				return mCanBeDisabled;
			}
			void enable() {
				if( !canBeDisabled() ) return;
				if( !mEnabled ) {
					mEnabled = true;
					enableObjects();
				}
			}
			void disable() {
				if( !canBeDisabled() ) return;
				if( mEnabled ) {
					mEnabled = false;
					disableObjects();
				}
			}

		protected:
			void disableObjects() {
				for(ComponentObjectNakedList2::iterator objectIterator=mObjects.begin(); objectIterator != mObjects.end(); ++objectIterator) {
					ComponentObject2* object = *objectIterator;
					object->disable();
					object->broadcast_object( MessageFocus(false) );
					object->broadcast_object( MessageEnable(false) );
				}
				if( !mOnLeave.empty() ) {
					Call(mOnLeave).getReturnedVoid();
				}
			}
			void enableObjects() {
				for(ComponentObjectNakedList2::iterator objectIterator=mObjects.begin(); objectIterator != mObjects.end(); ++objectIterator) {
					ComponentObject2* object = *objectIterator;
					object->broadcast_object( MessageFocus(true) );
					object->broadcast_object( MessageEnable(true) );
					object->enable();
				}
				if( !mOnEnter.empty() ) {
					Call(mOnEnter).getReturnedVoid();
				}
			}
		private:
			std::string mName;
			bool mCanBeDisabled;
			bool mEnabled;
			std::string mOnEnter;
			std::string mOnLeave;
			ComponentObjectNakedList2 mObjects;
		};
		typedef boost::shared_ptr<State2> StatePtr2;
		typedef std::list<StatePtr2> StateList2;

		struct QuadDefinition {
			QuadDefinition(real iLeft, real iRight, real iTop, real iBottom) : left(iLeft), right(iRight), top(iTop), bottom(iBottom) {
			}
			real left;
			real right;
			real top;
			real bottom;

			real getWidth() const {
				return right - left;
			}
			real getHeight() const {
				return top - bottom;
			}
		};

		bool Within(const math::vec2& iMouse, const math::vec2& iQuad, const QuadDefinition& iExtent, math::vec2* iLocation=0) {
			const math::vec2 location = iMouse - iQuad;
			if( iLocation ) {
				*iLocation = location;
			}
			return location.getX() > iExtent.left &&
				location.getX() < iExtent.right &&
				location.getY() < iExtent.top &&
				location.getY() > iExtent.bottom;
		}

		typedef std::map<std::string, std::wstring> StringWStringMap;
	}
	class StatesObject2 : boost::noncopyable {
	public:
		StatesObject2(ComponentObjectContainer2& iContainer, TiXmlElement* iElement) : mContainer(iContainer) {
			bool isFirst = true;
			traverse(isFirst, iContainer, iElement);
		}

		void traverse(bool& isFirst, ComponentObjectContainer2& iContainer, TiXmlElement* iElement) {
			for(TiXmlElement* child=iElement->FirstChildElement(); child; child = child->NextSiblingElement() ) {
				const std::string name = child->Value();
				if( name == "state" ) {
					StatePtr2 state;
					const char* src = child->Attribute("src");
					if( src ) {
						try {
							ReadFile file(src);
							TiXmlDocument docObj(src);
							docObj.Parse(file.getBuffer(), 0, TIXML_DEFAULT_ENCODING);
							state.reset( new State2(this, iContainer, docObj.FirstChildElement("state")) );
						}
						catch( const std::runtime_error& error){
							const std::string what = error.what();
							throw std::runtime_error(what + "\nin states file: " + src);
						}
					}
					else {
						state.reset( new State2(this, iContainer, child) );
					}
					mStates.push_back( state );
					if( isFirst ) {
						isFirst = false;
						addToStack(state->getName());
					}
					else {
						state->disable();
					}
				}
				else {
					throw FileDataError(name + "is unknown element, want state");
				}
			}
		}

		void goToPrevious() {
			const std::string previous = popStack();
			transitionToState(previous, false);
		}
		void transitionToState(const std::string& iName, bool iPushBack=true) {
			StatePtr2 theState;
			for( StateList2::iterator stateIterator=mStates.begin(); stateIterator != mStates.end(); ++stateIterator) {
				StatePtr2 state = *stateIterator;
				if( state->canBeDisabled() ) {
					if( state->getName() == iName ) {
						theState = state;
					}
					else {
						state->disable();
					}
				}
			}
			if( theState.get() ) {
				theState->enable();
				if( iPushBack ) {
					addToStack(theState->getName());
				}
			}
		}

		const std::wstring& getValue(const std::string& iName) {
			return mVariables[iName];
		}
		void setValue(const std::string& iName, const std::wstring& iValue) {
			mVariables[iName] = iValue;
		}

		void addToStack(const std::string& iName) {
			if(! mCurrentStateName.empty() ) {
				mStateStack.push_back(mCurrentStateName);
			}
			mCurrentStateName = iName;
		}
		std::string popStack() {
			if( mStateStack.empty() ) {
				return "";
			}
			else {
				mCurrentStateName = *mStateStack.rbegin();
				mStateStack.pop_back();
				return mCurrentStateName;
			}
		}
	private:
		ComponentObjectContainer2& mContainer;
		StateList2 mStates;
		StringWStringMap mVariables;
		std::string mCurrentStateName;
		std::list<std::string> mStateStack;
	};

	ComponentObject2::ComponentObject2(TiXmlElement* iElement, ComponentObjectContainer2& iContainer, StatesObject2* iStateObject) : mContainer(iContainer), mDisplay(0), mIsBroadcastEnabled(true) {
		for(TiXmlElement* child = iElement->FirstChildElement(); child; child = child->NextSiblingElement()) {
			ComponentPtr2 component(iContainer.buildComponent(*this, child->Value(), child, iStateObject));
			if( !mDisplay ) {
				// asDisplay returns 0 if it not a display, and itself as a display if it really is a display
				mDisplay = component->asDisplay();
			}
			mComponents.push_back( component );
		}
	}

	World2& ComponentObject2::getWorld() {
		return mContainer.getWorld();
	}
	const std::string ComponentObject2::getBaseFont() const {
		return mContainer.getBaseFont();
	}

	void ComponentObject2::doUpdate(real iTime) {
		broadcast_object( MessageUpdate(iTime) );
	}
	void ComponentObject2::doRender(real iTime) {
		if( mDisplay ) {
#ifdef _DEBUG
			DataType2<QuadDefinition>* extentPtr( dynamic_cast<DataType2<QuadDefinition>*>(findData("Extent")) );
			if( extentPtr ) {
				math::vec2& location = dynamic_cast<DataType2<math::vec2>*>(findData("Location"))->getReference();
				QuadDefinition extent = extentPtr->getReference();
				const real x = location.getX();
				const real y = location.getY();
				quad(Rgba(1, 0, 0, 0.5), extent.left+x, extent.right+x, extent.top+y, extent.bottom+y);
			}
#endif
			mDisplay->display(iTime);
		}
	}

	Data2* ComponentObject2::findData(const std::string& iName) {
		DataMap2::iterator result = mDatas.find(iName);
		if( result != mDatas.end() ) {
			return &(*result->second);
		}

		return 0;
	}

	namespace {
		DataMap2 gCommonDatas;
	};

	Data2& ComponentObject2::getOrCreateData(const std::string& iType, const std::string& iName, TiXmlElement* iObjectElement, bool iAddName) {
		{
			Data2* data = findData(iName);
			if( data ) return *data;
		}

		const char* objectName = iObjectElement->Attribute("global");
		const bool global = objectName ? true : false;
		const std::string globalId = global ? std::string(objectName) + std::string(".") + iName : "";

		if( global ) {
			const std::size_t size = gCommonDatas.size();
			DataMap2::iterator result = gCommonDatas.find( globalId );
			if( result != gCommonDatas.end() ) {
				return *result->second;
			}
		}

		DataPtr2 result;
		/*  */ if( iType == "real" ) {
			real value = 0;
			iObjectElement->QueryDoubleAttribute(iName, &value);
			result.reset( new DataType2<real>(value) );
		} else if( iType == "pos" ) {
			real x = 0;
			real y = 0;
			if( iAddName ) {
				iObjectElement->QueryDoubleAttribute(iName + "_x", &x);
				iObjectElement->QueryDoubleAttribute(iName + "_y", &y);
			}
			else {
				iObjectElement->QueryDoubleAttribute("x", &x);
				iObjectElement->QueryDoubleAttribute("y", &y);
			}
			result.reset( new DataType2<math::vec2>( math::vec2(x,y) ) );
		} else if( iType == "size" ) {
			real x = Registrator().getAspect();
			real y = 1;
			if( iAddName ) {
				iObjectElement->QueryDoubleAttribute(iName + "_width", &x);
				iObjectElement->QueryDoubleAttribute(iName + "_height", &y);
			}
			else {
				iObjectElement->QueryDoubleAttribute("width", &x);
				iObjectElement->QueryDoubleAttribute("height", &y);
			}
			result.reset( new DataType2<math::vec2>( math::vec2(x,y) ) );
		} else if( iType == "quad" ) {
			real left = 0;
			real right = 0;
			real top = 0;
			real bottom = 0;
			iObjectElement->QueryDoubleAttribute(iName + "_left", &left);
			iObjectElement->QueryDoubleAttribute(iName + "_right", &right);
			iObjectElement->QueryDoubleAttribute(iName + "_top", &top);
			iObjectElement->QueryDoubleAttribute(iName + "_bottom", &bottom);
			result.reset( new DataType2<QuadDefinition>( QuadDefinition(left, right, top, bottom) ) );
		} else if( iType == "timer" ) {
			int value = 0;
			if( iAddName ) {
				iObjectElement->QueryIntAttribute(iName + "_timer", &value);
			}
			else {
				iObjectElement->QueryIntAttribute(iName + "timer", &value);
			}
			result.reset( new DataType2<int>( value ) );
		//} else if( iType == "" ) {
		}

		Assert(result.get(), "Failed to create a valid data b/c the type is non-existant");

		if( global ) {
			gCommonDatas.insert( DataMap2::value_type(globalId, result) );
		}
		mDatas.insert( DataMap2::value_type(iName, result) );
		return *result;
	}

	void ComponentObject2::broadcast_object(const Message& iMessage) {
		for( ComponentList2::iterator component = mComponents.begin(); component!= mComponents.end(); ++component) {
			ComponentPtr2 theComponent = *component;
			theComponent->onMessage(iMessage);
		}
	}
	void ComponentObject2::broadcast_system(const Message& iMessage) {
		mContainer.broadcast(iMessage, this);
	}


	void ComponentObject2::setBroadcastEnable(bool isEnabled) {
		mIsBroadcastEnabled = isEnabled;
	}
	bool ComponentObject2::isBroadcastEnabled() const {
		return mIsBroadcastEnabled;
	}

	////////////////////

	ComponentObjectContainer2::ComponentObjectContainer2( World2& iWorld) : mWorld(iWorld), mBaseFont("fonts/base.fnt"), mHasSentFocusTrue(false), mIsInitialized(false) {
	}

	Component2* ComponentObjectContainer2::buildComponent(ComponentObject2& iObject, const std::string& iComponentName, TiXmlElement* iChild, StatesObject2* iStateObject) {
		BuildComponentMap2::iterator result = mComponentsBuilder.find(iComponentName);
		if( result == mComponentsBuilder.end() ) {
			throw FileDataError(iComponentName + " is a unknown component");
		}
		Component2* component = result->second(iStateObject, iObject, iChild);
		if( component == 0 ) throw FileDataError(std::string("Failed to construct component") + iComponentName);
		return component;
	}

	void ComponentObjectContainer2::load(const std::string iFileName) {
		try {
			ReadFile file(iFileName);
			TiXmlDocument docObj(iFileName.c_str());
			docObj.Parse(file.getBuffer(), 0, TIXML_DEFAULT_ENCODING);
			TiXmlHandle doc(&docObj);
			TiXmlElement* root = doc.FirstChild("objects").Element();
			if( root ) {
				const char* fontFile = root->Attribute("font");
				if( fontFile ) {
					mBaseFont = fontFile;
				}
			}
			for( TiXmlElement* child = doc.FirstChild("objects").FirstChildElement().Element(); child; child = child->NextSiblingElement() ) {
				handleChild(child);
			}
		}
		catch( const std::runtime_error& error){
			const std::string what = error.what();
			throw std::runtime_error(what + "\nin file: " + iFileName);
		}
	}
	void ComponentObjectContainer2::includeFile(const std::string& iFileName) {
		try {
			ReadFile file(iFileName);
			TiXmlDocument docObj(iFileName.c_str());
			docObj.Parse(file.getBuffer(), 0, TIXML_DEFAULT_ENCODING);
			TiXmlHandle doc(&docObj);
			for( TiXmlElement* child = doc.FirstChild("include").FirstChildElement().Element(); child; child = child->NextSiblingElement() ) {
				handleChild(child);
			}
		}
		catch( const std::runtime_error& error){
			const std::string what = error.what();
			throw std::runtime_error(what + "\nincluded in file: " + iFileName);
		}
	}
	void ComponentObjectContainer2::handleChild(TiXmlElement* child) {
		const std::string name = child->Value();
		/*  */ if( name == "object") {
			addObject(child, 0);
		} else if( name == "states") {
			addStates(child);
		} else if( name == "include") {
			const char* src = child->Attribute("src");
			if( !src ) {
				throw FileDataError(StringAndLine("include is missing src attribute", child));
			}
			includeFile(src);
		}
		else {
			throw FileDataError(StringAndLine(name + " is an unknown element", child));
		}
	}
	ComponentObject2* ComponentObjectContainer2::addObject(TiXmlElement* iObjectElement, StatesObject2* iStateObject) {
		ComponentObject2* object = new ComponentObject2(iObjectElement, *this, iStateObject);
		mWorld.add( object );
		mObjects.push_back( object );
		return object;
	}
	void ComponentObjectContainer2::addStates(TiXmlElement* iStatesElement) {
		StatesObjectPtr2 states(new StatesObject2(*this, iStatesElement));
		mStates.push_back( states );
	}
	void ComponentObjectContainer2::broadcast(const Message& iMessage, ComponentObject2* iIgnoreThis) {
		for( ComponentObjectList2::iterator object = mObjects.begin(); object != mObjects.end(); ++object) {
			ComponentObject2* theObject = *object;
			theObject->setBroadcastEnable( theObject->isEnabled() );
		}
		for( ComponentObjectList2::iterator object = mObjects.begin(); object != mObjects.end(); ++object) {
			ComponentObject2* theObject = *object;
			if( theObject->isBroadcastEnabled() && theObject != iIgnoreThis ) {
				theObject->broadcast_object(iMessage);
			}
		}
	}
	void ComponentObjectContainer2::registerBuilder(const std::string& iName, BuildComponentFunction2 iFunction) {
		mComponentsBuilder.insert( BuildComponentMap2::value_type(iName, iFunction) );
	}
	
	void ComponentObjectContainer2::sendKey( sgl::Key iKey, bool iDown) {
		broadcast(MessageKey(iKey, iDown));
	}
	void ComponentObjectContainer2::onFocus(bool iGained) {
		if( iGained ) {
			mHasSentFocusTrue = true;
		}
		else {
			broadcast(MessageOnTop(false));
		}
	}
	void ComponentObjectContainer2::sendMouseMovement(const math::vec2& iMovement) {
		broadcast(MessageMouseMove(iMovement));
	}
	void ComponentObjectContainer2::sendChar(wchar_t iChar) {
		broadcast(MessageChar(iChar));
	}

	World2& ComponentObjectContainer2::getWorld() {
		return mWorld;
	}

	const std::string ComponentObjectContainer2::getBaseFont() const {
		return mBaseFont;
	}

	void ComponentObjectContainer2::step() {
		if( mHasSentFocusTrue ) {
			broadcast(MessageOnTop(true));
			mHasSentFocusTrue = false;
		}
	}

	void ComponentObjectContainer2::init() {
		//if( !mIsInitialized ) {
			mIsInitialized = true;
			broadcast(MessageInit());
		//}
	}
	
	///////////////////

#define DATA_REF(owner, name, type, typeName, element) dynamic_cast<DataType2<type>&>(owner.getOrCreateData(typeName, name, element->Parent()->ToElement())).getReference()
#define DATA_REF_NAME(owner, name, type, typeName, element) dynamic_cast<DataType2<type>&>(owner.getOrCreateData(typeName, name, element->Parent()->ToElement(), true)).getReference()

	class QuadDisplay2 : public Display2 {
	public:
		QuadDisplay2(ComponentObject2& iOwner, TiXmlElement* iElement) : Display2(iOwner),
			mTextureCoords(0, 1, 1, 0),
			mCoords( DATA_REF(iOwner, "Extent", QuadDefinition, "quad", iElement) ),
			mLocation( DATA_REF(iOwner, "Location", math::vec2, "pos", iElement) ),
			mAnchor( DATA_REF_NAME(iOwner, "Anchor", math::vec2, "pos", iElement) ),
			mRotation( DATA_REF(iOwner, "Rotation", real, "real", iElement) ),
			mTextureDisplacement( DATA_REF_NAME(iOwner, "uv", math::vec2, "pos", iElement) ), mShouldHover(false), mHover(false) {

				const math::vec2 size( DATA_REF(iOwner, "Size", math::vec2, "size", iElement) );
				mCoords.right = size.getX();
				mCoords.top = size.getY();
				const char* texture = iElement->Attribute("texture");
				if( texture ) {
					mImage.reset( new Image(ImageDescriptor(texture) ) );
				}
				const char* alternate = iElement->Attribute("alternate");
				if( alternate ) {
					mImageAlternate.reset( new Image(ImageDescriptor(alternate) ) );
				}
#define ATTRIBUTE(name, value) do {if(iElement->QueryDoubleAttribute(name, &(value)) == TIXML_WRONG_TYPE) throw FileDataError(StringAndLine("Failed to read " name ", wrong format in quad display!", iElement)); } while(false)
				ATTRIBUTE("uvLeft", mTextureCoords.left);
				ATTRIBUTE("uvRight", mTextureCoords.right);
				ATTRIBUTE("uvTop", mTextureCoords.top);
				ATTRIBUTE("uvBottom", mTextureCoords.bottom);

				ATTRIBUTE("red", mColor.red);
				ATTRIBUTE("green", mColor.green);
				ATTRIBUTE("blue", mColor.blue);
				ATTRIBUTE("alpha", mColor.alpha);
#undef ATTRIBUTE
				const char* canHover = iElement->Attribute("hover");
				if( canHover && ToLower(canHover)=="yes") {
					mShouldHover = true;
				}

				const char* test = iElement->Attribute("test");
				if( test ) {
					mTest = test;
				}
		}

		void display(real iTime) {
			glPushMatrix();

			glTranslated(mLocation.getX(), mLocation.getY(), 0);

			glTranslated(mAnchor.getX(), mAnchor.getY(), 0);
			glRotated(mRotation, 0,0,1);
			glTranslated(-mAnchor.getX(), -mAnchor.getY(), 0);
			

			if( mShouldHover && !mHover ) {
				const real gray = 0.7;
				glColor4d(mColor.red*gray, mColor.green*gray, mColor.blue*gray, mColor.alpha);
			}
			else {
				glColor4d(mColor.red, mColor.green, mColor.blue, mColor.alpha);
			}

			bool mainImage = true;
			bool isTextureBound = false;

			if( !mTest.empty() ) {
				mainImage = Call(mTest).getReturnedBool();
			}

			if( mainImage ) {
				if( mImage.get() ) {
					glBindTexture(GL_TEXTURE_2D, (*mImage)->getId() );
					isTextureBound = true;
				}
			}
			else{
				if( mImageAlternate.get() ) {
					glBindTexture(GL_TEXTURE_2D, (*mImageAlternate)->getId() );
					isTextureBound = true;
				}
			}

			if( !isTextureBound ) {
				glDisable(GL_TEXTURE_2D);
			}

			const QuadDefinition
				textureCoords(	mTextureCoords.left + mTextureDisplacement.getX(),
								mTextureCoords.right + mTextureDisplacement.getX(),
								mTextureCoords.top + mTextureDisplacement.getY(),
								mTextureCoords.bottom + mTextureDisplacement.getY());

			glBegin(GL_QUADS);
#define COORD(y, x) do { glTexCoord2d(textureCoords.x, textureCoords.y); glVertex2d(mCoords.x, mCoords.y); } while(false)
				COORD(bottom, left);
				COORD(bottom, right);
				COORD(top, right);
				COORD(top, left);
#undef COORD
			glEnd();

			if( !isTextureBound ) {
				glEnable(GL_TEXTURE_2D);
			}

			glPopMatrix();
		}

		void setHover(bool iHover) {
			if( iHover != mHover ) {
				mHover = iHover;
				mOwner.broadcast_object( MessageHoverChanged(iHover) );
			}
		}

		void onMessage(const Message& iMessage) {
			if( iMessage.getType() == MT_INIT ) {
				if( mImage.get() ) {
					mImage->load();
				}
				if( mImageAlternate.get() )  {
					mImageAlternate->load();
				}
			}
			else if( iMessage.getType() == MT_HOVER ) {
				if( mShouldHover ) {
					const MessageHover& msg = static_cast<const MessageHover&>(iMessage);
					setHover(Within(msg.location, mLocation, mCoords) );
				}
			}
			else if( iMessage.getType() == MT_FOCUS ) {
				if( mShouldHover ) {
					const MessageFocus& msg = static_cast<const MessageFocus&>(iMessage);
					if( !msg.focus ) {
						setHover(false);
					}
				}
			}
		}

		

		struct Color {
			Color() : red(1), green(1), blue(1), alpha(1) {
			}
			real red;
			real green;
			real blue;
			real alpha;
		};

		QuadDefinition mTextureCoords;
		QuadDefinition& mCoords;
		Color mColor;
		std::auto_ptr<Image> mImage;
		std::auto_ptr<Image> mImageAlternate;
		math::vec2& mLocation;
		math::vec2& mAnchor;
		real& mRotation;
		math::vec2& mTextureDisplacement;
		bool mShouldHover;
		bool mHover;
		std::string mTest;
	};

	class SliderDisplay2 : public Display2 {
	public:
		SliderDisplay2(ComponentObject2& iOwner, TiXmlElement* iElement) : Display2(iOwner),
			mCoords( DATA_REF(iOwner, "Extent", QuadDefinition, "quad", iElement) ),
			mLocation( DATA_REF(iOwner, "Location", math::vec2, "pos", iElement) ), mShouldHover(false), mHover(false), mKnobSize(0.1), mOffsetX(0) {
				const math::vec2 size( DATA_REF(iOwner, "Size", math::vec2, "size", iElement) );
				mCoords.right = size.getX();
				mCoords.top = size.getY();
				const char* slider = iElement->Attribute("slider");
				if( slider ) {
					mImageBkg.reset( new Image(ImageDescriptor(slider) ) );
				}
				const char* knob = iElement->Attribute("knob");
				if( knob ) {
					mImageKnob.reset( new Image(ImageDescriptor(knob) ) );
				}
				else {
					mImageKnob.reset();
				}

				const char* canHover = iElement->Attribute("hover");
				if( canHover && ToLower(canHover)=="yes") {
					mShouldHover = true;
				}

				const char* test = iElement->Attribute("test");
				if( test ) {
					mGetValue = test;
				}

				if( iElement->QueryDoubleAttribute("knobSize", &mKnobSize)==TIXML_WRONG_TYPE) {
					throw FileDataError( StringAndLine("knobSize is of wrong type", iElement));
				}

				if( iElement->QueryDoubleAttribute("offsetX", &mOffsetX) == TIXML_WRONG_TYPE ) {
					throw FileDataError( StringAndLine("Wrong type for offsetX", iElement) );
				}
		}

		void display(real iTime) {
			glPushMatrix();

			glTranslated(mLocation.getX(), mLocation.getY(), 0);

			if( mShouldHover && !mHover ) {
				const real gray = 0.7;
				glColor4d(gray, gray, gray, 1);
			}
			else {
				glColor4d(1, 1, 1, 1);
			}

			const real offset = mKnobSize;
			
			const real value = (mGetValue.empty()) ? 1.0 : Call(mGetValue).getReturnedReal();
			const real sliderScale = mImageKnob.get() ? 1.0 : value;

			glBindTexture(GL_TEXTURE_2D, (*mImageBkg)->getId() );
			
			const real sliderWidth = mCoords.right-mCoords.left;
			const real smallWidth = mOffsetX/2;
			const real scaledWidth = (sliderWidth-mOffsetX) * sliderScale;
			QuadDefinition
				coords(0, smallWidth, mCoords.top, mCoords.bottom);

			glPushMatrix();
			glTranslated(-mCoords.left, 0, 0);
			glBegin(GL_QUADS);
				coords.left = 0; coords.right = smallWidth;
				quad(QuadDefinition(0, 0.3, 1, 0), coords);
				coords.left = smallWidth; coords.right = smallWidth + scaledWidth;
				quad(QuadDefinition(0.3, 1-0.3, 1, 0), coords);
				coords.left = smallWidth + scaledWidth; coords.right = smallWidth*2+scaledWidth;
				quad(QuadDefinition(1-0.3, 1, 1, 0), coords);
			glEnd();
			glPopMatrix();

			if( mImageKnob.get() ) {
				const real slider = (1-offset/sliderWidth)*value;
				glTranslated(sliderWidth*slider,0,0);
				glBindTexture(GL_TEXTURE_2D, (*mImageKnob)->getId() );
				const QuadDefinition coords(0, offset, offset, 0);
				const QuadDefinition textureCoords(0, 1, 1, 0);
				glBegin(GL_QUADS);
#define COORD(y, x) do { glTexCoord2d(textureCoords.x, textureCoords.y); glVertex2d(coords.x, coords.y); } while(false)
					COORD(bottom, left);
					COORD(bottom, right);
					COORD(top, right);
					COORD(top, left);
#undef COORD
				glEnd();
			}

			glPopMatrix();
		}

		void quad(const QuadDefinition& iTextureCoords, const QuadDefinition& iCoords) {
#define COORD(y, x) do { glTexCoord2d(iTextureCoords.x, iTextureCoords.y); glVertex2d(iCoords.x, iCoords.y); } while(false)
				COORD(bottom, left);
				COORD(bottom, right);
				COORD(top, right);
				COORD(top, left);
#undef COORD
		}

		void setHover(bool iHover) {
			if( iHover != mHover ) {
				mHover = iHover;
				mOwner.broadcast_object( MessageHoverChanged(iHover) );
			}
		}

		void onMessage(const Message& iMessage) {
			if( iMessage.getType() == MT_INIT ) {
				if( mImageBkg.get() ) {
					mImageBkg->load();
				}
				if( mImageKnob.get() ) {
					mImageKnob->load();
				}
			}
			else if( iMessage.getType() == MT_HOVER ) {
				if( mShouldHover ) {
					const MessageHover& msg = static_cast<const MessageHover&>(iMessage);
					setHover(Within(msg.location, mLocation, mCoords) );
				}
			}
			else if( iMessage.getType() == MT_FOCUS ) {
				if( mShouldHover ) {
					const MessageFocus& msg = static_cast<const MessageFocus&>(iMessage);
					if( !msg.focus ) {
						setHover(false);
					}
				}
			}
		}

		QuadDefinition& mCoords;
		std::auto_ptr<Image> mImageBkg;
		std::auto_ptr<Image> mImageKnob;
		math::vec2& mLocation;
		bool mShouldHover;
		bool mHover;
		std::string mGetValue;
		real mKnobSize;
		real mOffsetX;
	};

	enum TextPrintTextType {
		TPTT_GLOBAL,
		TPTT_CALLBACK,
		TPTT_STRING,
	};

	class TextPrintArgument2 {
	public:
		TextPrintArgument2(const std::string& iName, TiXmlElement* iElement) :  mName(iName), mType(TPTT_GLOBAL) {
			if( const char* value = iElement->Attribute("value") ) {
				mValue = value;
			}

			if( const char* type = iElement->Attribute("type") ) {
				const std::string str = ToLower(type);
				/****/ if( str == "global" ) {
					mType = TPTT_GLOBAL;
				} else if( str == "callback" ) {
					mType = TPTT_CALLBACK;
				} else if( str == "string" ) {
					mType = TPTT_STRING;
				}
				else {
					throw FileDataError(StringAndLine("bad type value in text argument", iElement));
				}
			}
			else {
				throw FileDataError(StringAndLine("missing type in text argument", iElement));
			}
		}

		const std::string& getName() const {
			return mName;
		}
		const std::wstring getValue(StatesObject2* iStates) const {
			switch( mType ) {
				case TPTT_GLOBAL:
					if(! iStates ) throw FileDataError("Missing state, so I can't get global");
					return iStates->getValue(mValue);
					break;
				case TPTT_CALLBACK:
					{
						const std::string str = Call(mValue).getReturnedString();
						return std::wstring(str.begin(), str.end());
					}
					break;
				case TPTT_STRING:
					return String(mValue);
					break;
			}
			throw FileDataError("Failed to do something with type, not a global, callback or a string");
			return L"";
		}
	private:
		std::string mName;
		TextPrintTextType mType;
		std::string mValue;
	};
	typedef std::list<TextPrintArgument2> TextPrintArgumentContainer2;

	class TextDisplay2 : public Display2 {
	public:
		TextDisplay2(StatesObject2* iStates, ComponentObject2& iOwner, TiXmlElement* iElement, const std::string iFileName) : Display2(iOwner), mLocation( DATA_REF(iOwner, "Location", math::vec2, "pos", iElement) ), mExtent( DATA_REF(iOwner, "Extent", QuadDefinition, "quad", iElement) ), mFont( FontDescriptor(iOwner.getWorld(), iFileName) ), mText("text.null"), mJustification(JUSTIFY_LEFT), mShouldHover(false), mHover(false), mHasText(false), mPrintJobName("menu.label"), mStates(iStates) {
			{
				const char* text = iElement->Attribute("text");
				if( text ) {
					mText = text;
					mHasText = true;
				}
			}
			{
				const char* job = iElement->Attribute("job");
				if( job ) {
					mPrintJobName = job;
				}
			}

			{
				const char* hover = iElement->Attribute("hover");
				if( hover && ToLower(hover)=="yes") {
					mShouldHover = true;
				}
			}

			{
				const char* value = iElement->Attribute("justify");
				if( value ) {
					const std::string justify = ToLower(value);
					/*  */ if( justify == "left" ) {
						mJustification = JUSTIFY_LEFT;
					} else if( justify == "center" ) {
						mJustification = JUSTIFY_CENTER;
					} else if( justify == "right" ) {
						mJustification = JUSTIFY_RIGHT;
					}
				}
			}

			{
				const std::string elementName="text";
				for(TiXmlElement* child=iElement->FirstChildElement(elementName); child; child = child->NextSiblingElement(elementName) ) {
					const char* name=child->Attribute("name");
					mArguments.push_back( TextPrintArgument2(name, child) );
				}
			}
		}

		void display(real iTime) {
			const std::string jobName = (mShouldHover && mHover) ? mPrintJobName+".hover" : mPrintJobName;
			real width = 0;
			{
				Printer printer(mFont, jobName, mLocation, mJustification, &width);
				if( mHasText ) {
					printer.arg("text", String(mText));
				}
				for(TextPrintArgumentContainer2::iterator arg=mArguments.begin(); arg!=mArguments.end(); ++arg) {
					printer.arg( arg->getName(), arg->getValue(mStates) );
				}
			}
			mExtent.left = 0;
			mExtent.right = 0;
			mExtent.top = Printer::GetHeight(mFont) - Printer::GetBottom(mFont);
			mExtent.bottom = -Printer::GetBottom(mFont);
			switch( mJustification ) {
				case JUSTIFY_LEFT:
					mExtent.right = width;
					break;
				case JUSTIFY_CENTER:
					mExtent.right = width / 2;
					mExtent.left = -width / 2;
					break;
				case JUSTIFY_RIGHT:
					mExtent.right = -width;
					break;
			}
		}

		void setHover(bool iHover) {
			if( iHover != mHover ) {
				mHover = iHover;
				mOwner.broadcast_object( MessageHoverChanged(iHover) );
			}
		}

		void onMessage(const Message& iMessage) {
			if( iMessage.getType() == MT_INIT ) {
				mFont.load();
			}
			else if( iMessage.getType() == MT_HOVER ) {
				if( mShouldHover ) {
					const MessageHover& msg = static_cast<const MessageHover&>(iMessage);
					setHover(Within(msg.location, mLocation, mExtent) );
				}
			}
			else if( iMessage.getType() == MT_FOCUS ) {
				if( mShouldHover ) {
					const MessageFocus& msg = static_cast<const MessageFocus&>(iMessage);
					if( !msg.focus ) {
						setHover(false);
					}
				}
			}
		}
	private:
		math::vec2& mLocation;
		QuadDefinition& mExtent;
		Justification mJustification;
		std::string mText;
		std::string mPrintJobName;
		Font mFont;
		bool mShouldHover;
		bool mHover;
		bool mHasText;
		TextPrintArgumentContainer2 mArguments;
		StatesObject2* mStates;
	};

	Component2* BuildDisplayComponent(StatesObject2* iStateObject, ComponentObject2& iOwner, TiXmlElement* iElement) {
		const char* theType = iElement->Attribute("type");
		if( theType ) {
			const std::string type = ToLower(theType);
			/****/ if( type == "text" ) {
				std::string fileName = iOwner.getBaseFont();
				{
					const char* file = iElement->Attribute("font");
					if( file ) {
						fileName = file;
					}
				}
				return new TextDisplay2(iStateObject, iOwner, iElement, fileName);
			} else if( type == "quad") {
				return new QuadDisplay2(iOwner, iElement);
			} else if( type == "slider") {
				return new SliderDisplay2(iOwner, iElement);
			}
			//} else if( type == "") {
		}

		throw FileDataError(StringAndLine("Missing type for display", iElement));
		return 0;
	}

	///////////////////////////

//	namespace {
		class Action2 {
		public:
			virtual ~Action2() {}
			virtual void execute() = 0;
		};
		typedef boost::shared_ptr<Action2> ActionPtr2;
		typedef std::list<ActionPtr2> ActionList2;

		class ScriptAction2 : public Action2 {
		public:
			explicit ScriptAction2(TiXmlElement* iDataElement) : mFileName(iDataElement->GetDocument()->Value()) {
				mScript = iDataElement->GetText();
			}

			void execute() {
				Execute(mScript, mFileName, 0);
			}
		private:
			std::string mScript;
			const std::string mFileName;
		};

		class TransitionAction2 : public Action2 {
		public:
			explicit TransitionAction2(StatesObject2* iStates, TiXmlElement* iDataElement) : mStates(iStates) {
				const char* to = iDataElement->Attribute("to");
				if( to ) {
					mStateName = to;
				}
				else {
					throw FileDataError(StringAndLine("Transition missing to element", iDataElement));
				}
			}

			void execute() {
				mStates->transitionToState(mStateName);
			}
		private:
			StatesObject2* mStates;
			std::string mStateName;
		};

		class GoBackAction2 : public Action2 {
		public:
			explicit GoBackAction2(StatesObject2* iStates, TiXmlElement* iDataElement) : mStates(iStates) {
			}

			void execute() {
				mStates->goToPrevious();
			}
		private:
			StatesObject2* mStates;
		};

		class SendAction2 : public Action2 {
		public:
			SendAction2(ComponentObject2& iOwner, TiXmlElement* iElement, TiXmlElement* iDataElement, bool iSendSystem) :
				mLocation( DATA_REF(iOwner, "Location", math::vec2, "pos", iElement) ), mAnchor( DATA_REF_NAME(iOwner, "Anchor", math::vec2, "pos", iElement) ),
				mOwner(iOwner), mSendSystem(iSendSystem) {
				const char* messageName = iDataElement->Attribute("name");
				if( messageName ) {
					mMessageName = messageName;
				}

				const char* callback = iDataElement->Attribute("callback");
				if( callback ) {
					mCallback = callback;
				}
			}

			void execute() {
				const math::vec2 location = mLocation + mAnchor;
				if( !mCallback.empty() ) {
					bool execute = Call(mCallback).getReturnedBool();
					if( !execute ) return;
				}

				if( mSendSystem ) {
					mOwner.broadcast_system(MessageFile(mMessageName, location) );
				}
				else {
					mOwner.broadcast_object(MessageFile(mMessageName, location) );
				}
			}
		private:
			math::vec2& mLocation;
			math::vec2& mAnchor;
			ComponentObject2& mOwner;
			const bool mSendSystem;
			std::string mMessageName;
			std::string mCallback;
		};

		class SetTimerAction2 : public Action2 {
		public:
			SetTimerAction2(ComponentObject2& iOwner, TiXmlElement* iElement, TiXmlElement* iDataElement) :
			  mTimer( DATA_REF(iOwner, "Timer", int, "timer", iElement) ), mValue(0), mOwner(iOwner) {
				  if( iDataElement->QueryIntAttribute("value", &mValue) == TIXML_WRONG_TYPE ) {
					  throw FileDataError(StringAndLine("Bad value in set_timer action", iDataElement));
				  }
			}

			void execute() {
				mTimer = mValue;
				mOwner.broadcast_object(MessageTimer() );
			}
		private:
			int& mTimer;
			int mValue;
			ComponentObject2& mOwner;
		};

		class SetAction2 : public Action2 {
		public:
			SetAction2(StatesObject2* iStates, ComponentObject2& iOwner, TiXmlElement* iElement, TiXmlElement* iDataElement) :
			  mStates(iStates), mVarName(""), mStringValue(""), mEmptyValue(false) {
				  if( !mStates ) throw FileDataError("Missing states container");
				  const char* name = iDataElement->Attribute("name");
				  if( name ) mVarName = name;
				  const char* value = iDataElement->Attribute("string");
				  if( value ) mStringValue = value;

				  mEmptyValue = mStringValue.empty();
			}

			void execute() {
				std::wstring value;
				if( mEmptyValue ) {
					value = L"";
				}
				else {
					value = String(mStringValue);
				}
				mStates->setValue( mVarName, value );
			}
		private:
			StatesObject2* mStates;
			std::string mVarName;
			std::string mStringValue;
			bool mEmptyValue;
		};

		class ActionsContainer2 : boost::noncopyable {
		public:
			ActionsContainer2(StatesObject2* iStateObject, ComponentObject2& iOwner, TiXmlElement* iContainer) {
				if( iContainer ) {
					const std::string actionName = "action";
					for( TiXmlElement* child = iContainer->FirstChildElement(actionName); child; child = child->NextSiblingElement(actionName) ) {
						const char* attribute = child->Attribute("type");
						if(! attribute ) {
							throw FileDataError(StringAndLine("Missing attribute type for action container", child));
						}
						std::string type = attribute;
						type = ToLower(type);
						/*  */ if( type == "send_system" ) {
							add( new SendAction2(iOwner, iContainer, child, true) );
						} else if( type == "send_local" ) {
							add( new SendAction2(iOwner, iContainer, child, false) );
						} else if( type == "transition" ) {
							add( new TransitionAction2(iStateObject, child) );
						} else if( type == "script" ) {
							add( new ScriptAction2(child) );
						} else if( type == "set_timer" ) {
							add( new SetTimerAction2(iOwner, iContainer, child) );
						} else if( type == "set" ) {
							add( new SetAction2(iStateObject, iOwner, iContainer, child) );
						} else if( type == "goback" ) {
							add( new GoBackAction2(iStateObject, child) );
						}
						else {
							throw FileDataError(StringAndLine("Action-Type is not a valid one", child));
						}
					}
				}
			}
			void add(Action2* iAction) {
				ActionPtr2 action(iAction);
				mActions.push_back(action);
			}
			void execute() {
				for(ActionList2::iterator actionIterator=mActions.begin(); actionIterator != mActions.end(); ++actionIterator) {
					ActionPtr2 action = *actionIterator;
					action->execute();
				}
			}
		private:
			ActionList2 mActions;
		};
//	}

	///////////////////////////

	class RotationComponent2 : public Component2 {
	public:
		RotationComponent2(ComponentObject2& iOwner, TiXmlElement* iElement) : Component2(iOwner),
			mRotation( DATA_REF(iOwner, "Rotation", real, "real", iElement) ), mSpeed(1), mMin(0), mMax(360), mWrap(true) {
#define ATTRIBUTE(name, value) do {if(iElement->QueryDoubleAttribute(name, &(value)) == TIXML_WRONG_TYPE) throw FileDataError(StringAndLine("Failed to read " name ", wrong format!", iElement)); } while(false)
				ATTRIBUTE("speed", mSpeed);
				ATTRIBUTE("min", mMin);
				ATTRIBUTE("max", mMax);
				
				const char* wrap = iElement->Attribute("wrap");
				if( wrap && ToLower(wrap)=="no" ) {
					mWrap = false;
				}
#undef ATTRIBUTE
		}

		void onMessage(const Message& iMessage) {
			if( iMessage.getType() == MT_UPDATE ) {
				const MessageUpdate& msg = static_cast<const MessageUpdate&>(iMessage);
				mRotation += msg.time * mSpeed;
				bool changeDirection = false;
				if( mRotation > mMax ) {
					if( !mWrap ) {
						changeDirection = true;
						mRotation = mMax;
					}
					else {
						mRotation = mMin;
					}
				}
				if( mRotation < mMin ) {
					if( !mWrap ) {
						changeDirection = true;
						mRotation = mMin;
					}
					else {
						mRotation = mMax;
					}
				}
				if( changeDirection ) {
					mSpeed *= -1;
				}
			}
		}
	private:
		real& mRotation;
		real mSpeed;
		real mMin;
		real mMax;
		bool mWrap;
	};
	Component2* BuildComponent_Rotation(StatesObject2* iStateObject, ComponentObject2& iOwner, TiXmlElement* iElement) {
		return new RotationComponent2(iOwner, iElement);
	}

	///////////////////////////

	class OnKeyComponent2 : public Component2 {
	public:
		OnKeyComponent2(StatesObject2* iStateObject, ComponentObject2& iOwner, TiXmlElement* iElement) : Component2(iOwner), mKey(sgl::Key::Escape), mActions(iStateObject, iOwner, iElement), mState(true) {
				const char* keyName = iElement->Attribute("key");
				if( keyName ) {
					mKey = sgl::Key::asData( ToLower(keyName) );
				}
				const char* down = iElement->Attribute("down");
				if( down && ToLower(down)=="no") {
					mState = false;
				}
		}
		void onMessage(const Message& iMessage) {
			if( iMessage.getType() == MT_KEY ) {
				const MessageKey& msg = static_cast<const MessageKey&>(iMessage);
				if( msg.key == mKey && msg.down==mState ) {
					mActions.execute();
				}
			}
		}
	private:
		sgl::Key mKey;
		ActionsContainer2 mActions;
		bool mState;
	};
	Component2* BuildComponent_OnKey(StatesObject2* iStateObject, ComponentObject2& iOwner, TiXmlElement* iElement) {
		return new OnKeyComponent2(iStateObject, iOwner, iElement);
	}

	/////////////////////////

	class MovementComponent2 : public Component2 {
	public:
		MovementComponent2(ComponentObject2& iOwner, TiXmlElement* iElement) : Component2(iOwner), mLocation(DATA_REF(iOwner, "Location", math::vec2, "pos", iElement)) {
		}

		void onMessage(const Message& iMessage) {
			if( iMessage.getType() == MT_MOUSE_MOVE ) {
				const MessageMouseMove& msg = static_cast<const MessageMouseMove&>(iMessage);
				mLocation += msg.movement;
			}
		}
	private:
		math::vec2& mLocation;
	};
	Component2* BuildComponent_Movement(StatesObject2* iStateObject, ComponentObject2& iOwner, TiXmlElement* iElement) {
		return new MovementComponent2(iOwner, iElement);
	}

	/////////////////////////

	class ConstraintComponent2 : public Component2 {
	public:
		ConstraintComponent2(ComponentObject2& iOwner, TiXmlElement* iElement, const std::string iName) : Component2(iOwner), mLocation(DATA_REF(iOwner, iName, math::vec2, "pos", iElement)), mSize(DATA_REF(iOwner, "Size", math::vec2, "size", iElement)), mLeft(0), mRight(Registrator().getAspect()), mTop(1), mBottom(0), mIncludeSize(true) {
			const char* include = iElement->Attribute("includeSize");
			if( include && ToLower(include)=="no") {
				mIncludeSize = false;
			}
			if( iElement->QueryDoubleAttribute("left", &mLeft) == TIXML_WRONG_TYPE ) throw FileDataError(StringAndLine("Wrong type in left contraint component", iElement));
			if( iElement->QueryDoubleAttribute("right", &mRight) == TIXML_WRONG_TYPE ) throw FileDataError(StringAndLine("Wrong type in right contraint component", iElement));
			if( iElement->QueryDoubleAttribute("top", &mTop) == TIXML_WRONG_TYPE ) throw FileDataError(StringAndLine("Wrong type in top contraint component", iElement));
			if( iElement->QueryDoubleAttribute("bottom", &mBottom) == TIXML_WRONG_TYPE ) throw FileDataError(StringAndLine("Wrong type in bottom contraint component", iElement));
		}

		void onMessage(const Message& iMessage) {
			if( iMessage.getType() == MT_UPDATE ) {
				real x = mLocation.getX();
				real y = mLocation.getY();

				real right = mRight;
				real top = mTop;
				if( mIncludeSize ) {
					right -= mSize.getX();
					top -= mSize.getY();
				}
				if( x < mLeft ) x = mLeft;
				if( x > right ) x = right;
				if( y < mBottom ) y = mBottom;
				if( y > top ) y = top;

				mLocation = math::vec2(x,y);
			}
		}
	private:
		math::vec2& mLocation;
		math::vec2& mSize;
		real mLeft;
		real mRight;
		real mTop;
		real mBottom;
		bool mIncludeSize;
	};
	Component2* BuildComponent_Constraint(StatesObject2* iStateObject, ComponentObject2& iOwner, TiXmlElement* iElement) {
		std::string name = "Location";
		const char* value = iElement->Attribute("pos");
		if( value ) {
			name = value;
		}
		return new ConstraintComponent2(iOwner, iElement, name);
	}

	/////////////////////////

	class UVMoveComponent2 : public Component2 {
	public:
		UVMoveComponent2(ComponentObject2& iOwner, TiXmlElement* iElement) : Component2(iOwner), mTextureDisplacement( DATA_REF_NAME(iOwner, "uv", math::vec2, "pos", iElement) ), mUSpeed(0), mVSpeed(0) {
#define ATTRIBUTE(name, value) do {if(iElement->QueryDoubleAttribute(name, &(value)) == TIXML_WRONG_TYPE) throw FileDataError(StringAndLine("Failed to read " name ", wrong format!", iElement)); } while(false)
			ATTRIBUTE("xSpeed", mUSpeed);
			ATTRIBUTE("ySpeed", mVSpeed);
#undef ATTRIBUTE
		}

		void onMessage(const Message& iMessage) {
			if( iMessage.getType() == MT_UPDATE ) {
				const MessageUpdate& msg = static_cast<const MessageUpdate&>(iMessage);
				mTextureDisplacement = math::vec2(	math::wrapRange(-1, mTextureDisplacement.getX() + mUSpeed*msg.time, 1),
													math::wrapRange(-1, mTextureDisplacement.getY() + mVSpeed*msg.time, 1) );
			}
		}
	private:
		math::vec2& mTextureDisplacement;
		real mUSpeed;
		real mVSpeed;
	};
	Component2* BuildComponent_UVMoveComponent2(StatesObject2* iStateObject, ComponentObject2& iOwner, TiXmlElement* iElement) {
		return new UVMoveComponent2(iOwner, iElement);
	}

	/////////////////////////

	class OnComponent2 : public Component2 {
	public:
		OnComponent2(StatesObject2* iStateObject, ComponentObject2& iOwner, TiXmlElement* iElement) : Component2(iOwner), mLocation( DATA_REF(iOwner, "Location", math::vec2, "pos", iElement)), mExtent( DATA_REF(iOwner, "Extent", QuadDefinition, "quad", iElement) ), mActions(iStateObject, iOwner, iElement), mOffsetX(0) {
			const char* name = iElement->Attribute("value");
			if( name ) {
				mName = name;
			}

			const char* call = iElement->Attribute("call");
			if( call ) {
				mCall = call;
			}

			if( iElement->QueryDoubleAttribute("offsetX", &mOffsetX) == TIXML_WRONG_TYPE ) throw FileDataError(StringAndLine("Wrong type for offsetX", iElement));
		}

		void onMessage(const Message& iMessage) {
			if( iMessage.getType() == MT_FILE_DEFINED ) {
				const MessageFile& msg = static_cast<const MessageFile&>(iMessage);
				math::vec2 clicked(0,0);
				if( Within(msg.location, mLocation, mExtent, &clicked) && msg.name == mName ) {
					if( !mCall.empty() ) {
						const real w = mExtent.getWidth();
						const real x = clicked.getX() - mOffsetX*0.5;
						const real V = (x/w)/(1-mOffsetX/w);
						Call(mCall).arg(math::limitRange(0, V, 1) ).arg(clicked.getY()/mExtent.getHeight()).getReturnedVoid();
					}
					mActions.execute();
				}
			}
		}
	private:
		math::vec2& mLocation;
		QuadDefinition& mExtent;
		ActionsContainer2 mActions;
		std::string mName;
		std::string mCall;
		real mOffsetX;
	};
	Component2* BuildComponent_OnComponent2(StatesObject2* iStateObject, ComponentObject2& iOwner, TiXmlElement* iElement) {
		return new OnComponent2(iStateObject, iOwner, iElement);
	}

	/////////////////////////

	class RandomMovementComponent2 : public Component2 {
	public:
		RandomMovementComponent2(ComponentObject2& iOwner, TiXmlElement* iElement, const std::string iName) : Component2(iOwner), mLocation( DATA_REF(iOwner, iName, math::vec2, "pos", iElement)), mWidth(1), mHeight(0), mMaxSpeed(0.1), mMaxForce(0.1), mMass(1), mSlowingDistance(0.1), mNewTargetDistance(0.1) {
			if( iElement->QueryDoubleAttribute("speed", &mMaxSpeed) == TIXML_WRONG_TYPE ) throw FileDataError(StringAndLine("Speed has the wrong type", iElement));
			if( iElement->QueryDoubleAttribute("force", &mMaxForce) == TIXML_WRONG_TYPE ) throw FileDataError(StringAndLine("Force has the wrong type", iElement));
			if( iElement->QueryDoubleAttribute("slowDistance", &mSlowingDistance) == TIXML_WRONG_TYPE ) throw FileDataError(StringAndLine("slowDistance has the wrong type", iElement));
			if( iElement->QueryDoubleAttribute("newTargetDistance", &mNewTargetDistance) == TIXML_WRONG_TYPE ) throw FileDataError(StringAndLine("newTargetDistance has the wrong type", iElement));
			if( iElement->QueryDoubleAttribute("mass", &mMass) == TIXML_WRONG_TYPE ) throw FileDataError(StringAndLine("mass has the wrong type", iElement));
			if( iElement->QueryDoubleAttribute("width", &mWidth) == TIXML_WRONG_TYPE ) throw FileDataError(StringAndLine("width has the wrong type", iElement));
			if( iElement->QueryDoubleAttribute("height", &mHeight) == TIXML_WRONG_TYPE ) throw FileDataError(StringAndLine("height has the wrong type", iElement));
			selectNewTarget();
		}

		void onMessage(const Message& iMessage) {
			if( iMessage.getType() == MT_UPDATE ) {
				const MessageUpdate& msg = static_cast<const MessageUpdate&>(iMessage);
				const real speedTime = mVelocity.getLength() * msg.time;
				const real length = math::op::vec2::lengthBetween(mLocation, mTarget);

				if( length < mNewTargetDistance ) {
					//mLocation = mTarget;
					selectNewTarget();
				}
				else {
					const math::vec2 targetOffset = mTarget - mLocation;
					const real distance = targetOffset.getLength();
					const real rampedSpeed = mMaxSpeed * (distance / mSlowingDistance);
					const real clippedSpeed = math::Min(rampedSpeed, mMaxSpeed);
					const math::vec2 desiredVelocity = targetOffset * (clippedSpeed / distance);
					const math::vec2 steering = desiredVelocity - mVelocity;
					
					/*const math::vec2 desiredVelocity = (mTarget - mLocation).getNormalized() * mMaxSpeed;
				    const math::vec2 steering = desiredVelocity - mVelocity;*/

					const math::vec2 steeringForce = math::op::vec2::Truncate (steering, mMaxForce);
					const math::vec2 acceleration = steeringForce / mMass;
					mVelocity = math::op::vec2::Truncate(mVelocity + acceleration*msg.time, mMaxSpeed);
					mLocation += mVelocity * msg.time;
				}
			}
		}

		void selectNewTarget() {
			mTarget = math::vec2(mRandom.randomReal()*mWidth, mRandom.randomReal()*mHeight);
			
#ifdef _DEBUG
			std::wstringstream str;
			str << L"New Target: " << mTarget.getX() << ", " << mTarget.getY();
			ConsoleMessage(CMT_ECHO, str.str());
#endif
		}
	private:
		math::vec2& mLocation;
		math::Random mRandom;
		math::vec2 mTarget;
		real mWidth;
		real mHeight;

		math::vec2 mVelocity;
		real mMaxSpeed;
		real mMaxForce;
		real mMass;
		real mSlowingDistance;
		real mNewTargetDistance;
	};
	Component2* BuildComponent_RandomMovementComponent2(StatesObject2* iStateObject, ComponentObject2& iOwner, TiXmlElement* iElement) {
		std::string name = "Location";
		const char* value = iElement->Attribute("move");
		if( value ) {
			name = value;
		}
		return new RandomMovementComponent2(iOwner, iElement, name);
	}

	/////////////////////////

	class SendHoverComponent2 : public Component2 {
	public:
		SendHoverComponent2(ComponentObject2& iOwner, TiXmlElement* iElement) : Component2(iOwner),
			mLocation( DATA_REF(iOwner, "Location", math::vec2, "pos", iElement) ), mAnchor( DATA_REF_NAME(iOwner, "Anchor", math::vec2, "pos", iElement) ) {
		}

		void onMessage(const Message& iMessage) {
			if( iMessage.getType() == MT_UPDATE ) {
				const math::vec2 location = mLocation + mAnchor;
				mOwner.broadcast_system(MessageHover(location) );
			}
		}
	private:
		math::vec2& mLocation;
		math::vec2& mAnchor;
	};
	Component2* BuildComponent_SendHoverComponent2(StatesObject2* iStateObject, ComponentObject2& iOwner, TiXmlElement* iElement) {
		return new SendHoverComponent2(iOwner, iElement);
	}

	/////////////////////////

	class TimerStepComponent2 : public Component2 {
	public:
		TimerStepComponent2(ComponentObject2& iOwner, TiXmlElement* iElement) : Component2(iOwner), mTimer(DATA_REF(iOwner, "Timer", int, "timer", iElement)),
			mStep(1), mTime(0), mTimeStep(0.1), mIgnoreTimer(false) {
				if( iElement->QueryIntAttribute("step", &mStep) == TIXML_WRONG_TYPE ) throw FileDataError(StringAndLine("Wrong type in timer step", iElement));
				if( iElement->QueryDoubleAttribute("tickTime", &mTimeStep) == TIXML_WRONG_TYPE ) throw FileDataError(StringAndLine("Wrong type in timer tickTime", iElement));
		}
		void onMessage(const Message& iMessage) {
			if( iMessage.getType() == MT_TIMER ) {
				if( !mIgnoreTimer ) {
					mTime = 0;
				}
			}
			else
			if( iMessage.getType() == MT_UPDATE ) {
				const MessageUpdate& msg = static_cast<const MessageUpdate&>(iMessage);
				mTime += msg.time;
				
				if( mTime > mTimeStep ) {
					mTime -= mTimeStep;
					mTimer += mStep;
					mIgnoreTimer = true;
					mOwner.broadcast_object( MessageTimer() );
					mIgnoreTimer = false;
				}
			}
		}
	private:
		int& mTimer;
		int mIgnoreTimer;
		int mStep;
		real mTime;
		real mTimeStep;
	};

	class TimerActionComponent2 : public Component2 {
	public:
		TimerActionComponent2(StatesObject2* iStateObject, ComponentObject2& iOwner, TiXmlElement* iElement) : Component2(iOwner), mTimer(DATA_REF(iOwner, "Timer", int, "timer", iElement)), mActions(iStateObject, iOwner, iElement), mValue(0) {
			iElement->QueryIntAttribute("onValue", &mValue);
		}
		void onMessage(const Message& iMessage) {
			if( iMessage.getType() == MT_TIMER ) {
				if( mTimer == mValue ) {
					mActions.execute();
				}
			}
		}
	private:
		int& mTimer;
		ActionsContainer2 mActions;
		int mValue;
	};
	Component2* BuildComponent_Timer(StatesObject2* iStateObject, ComponentObject2& iOwner, TiXmlElement* iElement) {
		if( iElement->Attribute("onValue")==0 ) return new TimerStepComponent2(iOwner, iElement);
		else return new TimerActionComponent2(iStateObject, iOwner, iElement);
	}
	
	/////////////////////////

	class OnHoverComponent2 : public Component2 {
	public:
		OnHoverComponent2(StatesObject2* iStateObject, ComponentObject2& iOwner, TiXmlElement* iElement) : Component2(iOwner),
		 mEnterActions(iStateObject, iOwner, iElement->FirstChildElement("enter")),
		 mLeaveActions(iStateObject, iOwner, iElement->FirstChildElement("leave")){
		}
		void onMessage(const Message& iMessage) {
			if( iMessage.getType() == MT_HOVER_CHANGED ) {
				const MessageHoverChanged& msg = static_cast<const MessageHoverChanged&>(iMessage);
				if( msg.enter ) {
					mEnterActions.execute();
				}
				else {
					mLeaveActions.execute();
				}
			}
		}
	private:
		ActionsContainer2 mEnterActions;
		ActionsContainer2 mLeaveActions;
	};
	Component2* BuildComponent_OnHoverComponent2(StatesObject2* iStateObject, ComponentObject2& iOwner, TiXmlElement* iElement) {
		return new OnHoverComponent2(iStateObject, iOwner, iElement);
	}

	/////////////////////////

	class OnInitComponent2 : public Component2 {
	public:
		OnInitComponent2(StatesObject2* iStateObject, ComponentObject2& iOwner, TiXmlElement* iElement) : Component2(iOwner),
		 mInitActions(iStateObject, iOwner, iElement){
		}
		void onMessage(const Message& iMessage) {
			if( iMessage.getType() == MT_INIT ) {
				mInitActions.execute();
			}
		}
	private:
		ActionsContainer2 mInitActions;
	};
	Component2* BuildComponent_OnInitComponent2(StatesObject2* iStateObject, ComponentObject2& iOwner, TiXmlElement* iElement) {
		return new OnInitComponent2(iStateObject, iOwner, iElement);
	}

	/////////////////////////

	class OnCharComponent2 : public Component2 {
	public:
		OnCharComponent2(StatesObject2* iStateObject, ComponentObject2& iOwner, TiXmlElement* iElement) : Component2(iOwner), mStateObject(iStateObject), mActions(iStateObject, iOwner, iElement) {
#define STRING(name) if( const char* name = iElement->Attribute(#name) )
			STRING(set) {
				mSet = set;
			}
			STRING(get) {
				mGet = get;
			}
#undef STRING
		}
		void onMessage(const Message& iMessage) {
			if( iMessage.getType() == MT_CHAR ) {
				const MessageChar& msg = static_cast<const MessageChar&>(iMessage);
				std::string val = Call(mGet).getReturnedString();
				std::wstring value(val.begin(), val.end());
				bool update = true;

				if( msg.c == 8 ) {// backspace
					if( !value.empty() ) {
						value = value.substr(0, value.length()-1);
					}
					else {
						update = false;
					}
				}
				else if( msg.c == 13 ) {
					// enter
					update = false;
				}
				else {
					value += msg.c;
				}

				if( update ) {
					const std::string str(value.begin(), value.end());
					Call(mSet).arg(str).getReturnedVoid();
					mActions.execute();
				}
			}
		}
	private:
		ActionsContainer2 mActions;
		StatesObject2* mStateObject;
		std::string mGet;
		std::string mSet;
	};
	Component2* BuildComponent_OnCharComponent2(StatesObject2* iStateObject, ComponentObject2& iOwner, TiXmlElement* iElement) {
		return new OnCharComponent2(iStateObject, iOwner, iElement);
	}

	
	/////////////////////////

	class OnFocusComponent2 : public Component2 {
	public:
		OnFocusComponent2(StatesObject2* iStateObject, ComponentObject2& iOwner, TiXmlElement* iElement) : Component2(iOwner),
		 mGainedActions(iStateObject, iOwner, iElement->FirstChildElement("gained")),
		 mLostActions(iStateObject, iOwner, iElement->FirstChildElement("lost")){
		}
		void onMessage(const Message& iMessage) {
			if( iMessage.getType() == MT_ONTOP ) {
				const MessageOnTop& msg = static_cast<const MessageOnTop&>(iMessage);
				if( msg.top ) {
					mGainedActions.execute();
				}
				else {
					mLostActions.execute();
				}
			}
		}
	private:
		ActionsContainer2 mGainedActions;
		ActionsContainer2 mLostActions;
	};
	Component2* BuildComponent_OnFocusComponent2(StatesObject2* iStateObject, ComponentObject2& iOwner, TiXmlElement* iElement) {
		return new OnFocusComponent2(iStateObject, iOwner, iElement);
	}

	/////////////////////////

	/*class RotationComponent2 : public Component2 {
	public:
		RotationComponent2(ComponentObject2& iOwner, TiXmlElement* iElement) : Component2(iOwner) {
		}
	private:
	};
	Component2* BuildComponent_(StatesObject2* iStateObject, ComponentObject2& iOwner, TiXmlElement* iElement) {
		return new Component2(iOwner, iElement);
	}*/

	///////////////////////////

	void ComponentObjectContainer2::registerDefaultBuilders() {
		registerBuilder("display", BuildDisplayComponent);
		registerBuilder("rotation", BuildComponent_Rotation);
		registerBuilder("onKey", BuildComponent_OnKey);
		registerBuilder("movement", BuildComponent_Movement);
		registerBuilder("constrainWithin", BuildComponent_Constraint);
		registerBuilder("textureMove", BuildComponent_UVMoveComponent2);
		registerBuilder("on", BuildComponent_OnComponent2);
		registerBuilder("randomMovement", BuildComponent_RandomMovementComponent2);
		registerBuilder("sendHover", BuildComponent_SendHoverComponent2);
		registerBuilder("timer", BuildComponent_Timer);
		registerBuilder("onHover", BuildComponent_OnHoverComponent2);
		registerBuilder("init", BuildComponent_OnInitComponent2);
		registerBuilder("onChar", BuildComponent_OnCharComponent2);
		registerBuilder("focus", BuildComponent_OnFocusComponent2);
	}
}