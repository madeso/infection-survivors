#include <list>
#include <vector>
#include <map>
#include "boost/smart_ptr.hpp"
#include "boost/function.hpp"
#include "sgl/sgl_OpenGl.hpp"
#include "sgl/sgl_Assert.hpp"

#include "lunarlady/World3.hpp"
#include "lunarlady/OpenGL_util.hpp"
#include "lunarlady/Tweak.hpp"
#include "lunarlady/math/vec3.hpp"
#include "lunarlady/math/vec2.hpp"
#include "lunarlady/Error.hpp"

#include "lunarlady/Xml.hpp"
#include "lunarlady/File.hpp"
#include "lunarlady/Script.hpp"
#include "lunarlady/Config.hpp"

#include "lunarlady/math/vec3.hpp"
#include "lunarlady/math/Quaternion.hpp"

#include "lunarlady/File.hpp"

#include "lunarlady/Display.hpp"
#include "lunarlady/files/WorldDefinition.hpp"

#include "lunarlady/TemplatedMedia.hpp"
#include "lunarlady/Loaded.hpp"

#include "lunarlady/Rgb.hpp"
#include "lunarlady/MaterialDefinition.hpp"
#include "lunarlady/Game.hpp"
#include "lunarlady/Image.hpp"

namespace lunarlady {
	const std::size_t RING_BUFFER_MOUSE_SIZE = 10;

	template<typename T>
	class RingBuffer {
	public:
		explicit RingBuffer(std::size_t iSize) : mBufferSize(0), mInsertionIndex(0) {
			setBufferSize(iSize);
		}
		void add(const T& iValue) {
			const std::size_t currentSize = getCurrentSize();
			if( currentSize < mBufferSize) {
				mBuffer.push_back(iValue);
			}
			else {
				mBuffer[mInsertionIndex] = iValue;
				++mInsertionIndex;
				if( mInsertionIndex == mBufferSize ) {
					mInsertionIndex = 0;
				}
			}
		}
		void setBufferSize(std::size_t iSize) {
			mBufferSize = iSize;
			mBuffer.reserve(iSize);
			if( mInsertionIndex >= mBufferSize ) {
				mInsertionIndex = mBufferSize -1;
			}
		}
		bool isEmpty() const {
			return mBuffer.empty();
		}
		std::size_t getCurrentSize() const {
			return mBuffer.size();
		}

		const T& operator[](int i) const {
			return mBuffer[(i + mInsertionIndex) % getCurrentSize()];
		}
	private:
		RingBuffer() {
		}
		std::vector<T> mBuffer;
		std::size_t mBufferSize;
		std::size_t mInsertionIndex;
	};
	template<typename T>
	T GetAverage(const RingBuffer<T>& iRingBuffer, const T& iZero, real iFactor) {
		if( iRingBuffer.isEmpty() ) {
			return iZero;
		}

		T sum = iZero;
		std::size_t count=0;
		const std::size_t length = iRingBuffer.getCurrentSize();
		real factor = 1;
		real sumFactor = 0;
		for(int index=length-1; index>= 0; --index) {
			sumFactor += factor;
			sum += iRingBuffer[index] * factor;
			factor *= iFactor;
			++count;
		}

		return sum / sumFactor;
	}

	struct Attenuation {
		Attenuation() : constant(1), linear(0), quadratic(0) {
		}
		real constant;
		real linear;
		real quadratic;
	};
	struct BaseLightDefinition {
		Attenuation attenuation;
		Rgb ambient;
		Rgb diffuse;
		Rgb specular;
	};
	struct PointLightDefinition : BaseLightDefinition {
	};
	struct SpotLightDefinition : BaseLightDefinition {
		SpotLightDefinition() : exponent(0), cutoff(10) {
		}

		real exponent;
		real cutoff;
	};

	struct LightSetup {
		static void Clear() {
			sLight = GL_LIGHT0;
			for(GLenum light=GL_LIGHT0; light<=GL_LIGHT7; ++light) {
				glDisable(light);
			}
		}
		static void AddLight(const PointLightDefinition& iPointLight) {
			if( CanShowLight() ) {
				SetupBaseLight(iPointLight);
				glLightf(sLight, GL_SPOT_CUTOFF, 180.0f);
				float zeroPosition[4];
				zeroPosition[0] = 0.0f;
				zeroPosition[1] = 0.0f;
				zeroPosition[2] = 0.0f;
				zeroPosition[3] = 1.0f;
				glLightfv(sLight, GL_POSITION, zeroPosition);
				NextLight();
			}
		}

		static void AddLight(const SpotLightDefinition& iSpotLight) {
			if( CanShowLight() ) {
				SetupBaseLight(iSpotLight);
				glLightf(sLight, GL_SPOT_CUTOFF, iSpotLight.cutoff);
				glLightf(sLight, GL_SPOT_EXPONENT, iSpotLight.exponent);
				float zeroPosition[4];
				zeroPosition[0] = 0.0f;
				zeroPosition[1] = 0.0f;
				zeroPosition[2] = 0.0f;
				zeroPosition[3] = 1.0f;
				glLightfv(sLight, GL_POSITION, zeroPosition);

				float direction[4];
				direction[0] = 0.0f;
				direction[1] = 0.0f;
				direction[2] = -1.0f;
				glLightfv(sLight, GL_SPOT_DIRECTION, direction);

				NextLight();
			}
		}
	private:
		static void SetupBaseLight(const BaseLightDefinition& iBaseLight) {
			glEnable(sLight);
			glLightf(sLight, GL_CONSTANT_ATTENUATION, iBaseLight.attenuation.constant);
			glLightf(sLight, GL_LINEAR_ATTENUATION, iBaseLight.attenuation.linear);
			glLightf(sLight, GL_QUADRATIC_ATTENUATION, iBaseLight.attenuation.quadratic);

			float color[4];
			SetupRgbArray(color, iBaseLight.ambient);
			glLightfv(sLight, GL_AMBIENT, color);

			SetupRgbArray(color, iBaseLight.diffuse);
			glLightfv(sLight, GL_DIFFUSE, color);

			SetupRgbArray(color, iBaseLight.specular);
			glLightfv(sLight, GL_SPECULAR, color);
		}
		static void SetupRgbArray(float* iArray, const Rgb& iColor, real iAlpha = 1.0) {
			iArray[0] = iColor.getRed();
			iArray[1] = iColor.getGreen();
			iArray[2] = iColor.getBlue();
			iArray[3] = iAlpha;
		}
		static void NextLight() {
			sLight += 1;
		}
		static bool CanShowLight() {
			return sLight <= GL_LIGHT7;
		}

	private:
		static GLenum sLight;
	};
	GLenum LightSetup::sLight = GL_LIGHT0;

	class Data3 {
	public:
		virtual ~Data3() {
		}
	};
	typedef boost::shared_ptr<Data3> DataPtr3;
	typedef std::map<std::string, DataPtr3> DataMap3;

	enum ComponentMessageType {
		CMT_INIT,
		CMT_DEINIT,
		CMT_UPDATE,
		CMT_RENDER,
		CMT_KEY,
		CMT_MOUSEMOVE,
		CMT_SETUPVIEW,
		CMT_SETUPLIGHT
	};

	class Message3 {
	public:
		explicit Message3(ComponentMessageType iType) : mType(iType) {
		}

		ComponentMessageType getMessageType() const {
			return mType;
		}

	private:
		const ComponentMessageType mType;
	};

	// -------------------------------------------------------
	// Messages
	struct MessageInit3 : public Message3 {
		explicit MessageInit3() : Message3(CMT_INIT) {
		}
	};
	struct MessageSetupLight3 : public Message3 {
		explicit MessageSetupLight3() : Message3(CMT_SETUPLIGHT) {
		}
	};
	struct MessageDeinit3 : public Message3 {
		explicit MessageDeinit3() : Message3(CMT_DEINIT) {
		}
	};
	struct MessageUpdate3 : public Message3 {
		MessageUpdate3(real iTime) : Message3(CMT_UPDATE), time(iTime) {
		}
		const real time;
	};
	struct MessageRender3 : public Message3 {
		MessageRender3(real iTime) : Message3(CMT_RENDER), time(iTime) {
		}
		const real time;
	};
	struct MessageKey3 : public Message3 {
		MessageKey3(const sgl::Key& iKey, bool iDown) : Message3(CMT_KEY), key(iKey), down(iDown) {
		}
		const sgl::Key key;
		const bool down;
	};
	struct MessageMouseMove3 : public Message3 {
		MessageMouseMove3(const math::vec2& iMovement) : Message3(CMT_MOUSEMOVE), movement(iMovement) {
		}
		const math::vec2 movement;
	};
	struct MessageSetupView3 : public Message3 {
		explicit MessageSetupView3() : Message3(CMT_SETUPVIEW) {
		}
	};
	// -------------------------------------------------------

	class Object3;
	class Component3 {
	public:
		Component3(World3& iWorld, Object3& iObject);

		virtual ~Component3() {
		}

		virtual bool handleMessage(Message3& iMessage) = 0;
	protected:
		World3& mWorld;
		Object3& mObject;
		Variable& mVariable;
	};
	typedef boost::shared_ptr<Component3> ComponentPtr3;
	typedef std::list<ComponentPtr3> ComponentList3;

	template<class T>
	struct TData3 : Data3 {
		explicit TData3(const T& iData) : data(iData) {
		}
		T data;
	};

	class Object3 {
	public:
		Object3(World3& iWorld) : mWorld(iWorld) {
		}

		real& getMemberReal(const std::string& iName) {
			DataPtr3 value;
			typedef real Type;
			DataMap3::iterator result = mDataMap.find(iName);
			if( result == mDataMap.end() ) {
				value.reset( new TData3<Type>(0) );
				mDataMap.insert( DataMap3::value_type(iName, value) );
			}
			else {
				value = result->second;
			}
			TData3<Type>& data = dynamic_cast<TData3<Type>&>( *value.get() );
			return data.data;
		}
		math::vec3& getMemberVec3(const std::string& iName) {
			DataPtr3 value;
			typedef math::vec3 Type;
			DataMap3::iterator result = mDataMap.find(iName);
			if( result == mDataMap.end() ) {
				value.reset( new TData3<Type>( math::vec3(0,0,0) ) );
				mDataMap.insert( DataMap3::value_type(iName, value) );
			}
			else {
				value = result->second;
			}
			TData3<Type>& data = dynamic_cast<TData3<Type>&>( *value.get() );
			return data.data;
		}
		math::Quaternion& getMemberQuaternion(const std::string& iName) {
			DataPtr3 value;
			typedef math::Quaternion Type;
			DataMap3::iterator result = mDataMap.find(iName);
			if( result == mDataMap.end() ) {
				value.reset( new TData3<Type>( math::Quaternion() ) );
				mDataMap.insert( DataMap3::value_type(iName, value) );
			}
			else {
				value = result->second;
			}
			TData3<Type>& data = dynamic_cast<TData3<Type>&>( *value.get() );
			return data.data;
		}

		void addComponent(Component3* iComponent) {
			ComponentPtr3 component(iComponent);
			mComponents.push_back(component);
		}

		bool handleMessage(Message3& iMessage) {
			bool final = false;
			for(ComponentList3::iterator component=mComponents.begin(); component != mComponents.end(); ++component) {
				const bool result = (*component)->handleMessage(iMessage);
				final = final || result;
			}
			return final;
		}
		Variable& getVariable() {
			return mVariable;
		}
	private:
		ComponentList3 mComponents;
		World3& mWorld;
		Variable mVariable;
		DataMap3 mDataMap;
	};
	typedef boost::shared_ptr<Object3> ObjectPtr3;
	typedef std::list<ObjectPtr3> ObjectList3;

	Component3::Component3(World3& iWorld, Object3& iObject) : mWorld(iWorld), mObject(iObject), mVariable(iObject.getVariable()) {
	}

	// -------------------------------------------------------------

	struct ComponentValue3 {
		std::string value;
	};
	typedef std::map<std::string, ComponentValue3> ComponentValueMap3;

	struct ComponentDefinition3 {
		std::string componentName;
		ComponentValueMap3 arguments;
	};
	typedef boost::shared_ptr<ComponentDefinition3> ComponentDefinitionPtr3;
	typedef std::list<ComponentDefinitionPtr3> ComponentDefinitionList3;

	struct ObjectDefinition3 {
		ComponentDefinitionList3 components;
	};
	typedef boost::shared_ptr<ObjectDefinition3> ObjectDefinitionPtr3;
	typedef std::map<std::string, ObjectDefinitionPtr3> ObjectDefinitionMap3;

	typedef boost::function<Component3* (World3&, Object3&, const ComponentValueMap3&)> ComponentBuilder3;
	typedef std::map<std::string, ComponentBuilder3> ComponentBuilderMap3;

	// ----------------------------------------------------------------------------
	class ViewComponent3 : public Component3 {
	public:
		ViewComponent3(World3& iWorld, Object3& iObject, const ComponentValueMap3& iValues) : Component3(iWorld, iObject), mLocation(iObject.getMemberVec3("loc")), mRotation(iObject.getMemberQuaternion("rot")) {
		}
		static Component3* Build(World3& iWorld, Object3& iObject, const ComponentValueMap3& iValues) {
			return new ViewComponent3(iWorld, iObject, iValues);
		}

		bool handleMessage(Message3& iMessage) {
			if( iMessage.getMessageType() == CMT_SETUPVIEW ) {
				SetDisplay3d();
				math::vec3 axis(0,0,0);
				math::Angle angle;
				mRotation.toAxisAngle(&axis, &angle);
				glRotated(-angle.inDegrees(), axis.getX(), axis.getY(), axis.getZ());
				glTranslated(-mLocation.getX(), -mLocation.getY(), -mLocation.getZ());
				return true;
			}
			return false;
		}

		math::vec3& mLocation;
		math::Quaternion& mRotation;
	};

	class PointLightComponent3 : public Component3 {
	public:
		PointLightComponent3(World3& iWorld, Object3& iObject, const ComponentValueMap3& iValues) : Component3(iWorld, iObject), mLocation(iObject.getMemberVec3("loc")), mRotation(iObject.getMemberQuaternion("rot")) {
		}
		static Component3* Build(World3& iWorld, Object3& iObject, const ComponentValueMap3& iValues) {
			return new PointLightComponent3(iWorld, iObject, iValues);
		}

		bool handleMessage(Message3& iMessage) {
			if( iMessage.getMessageType() == CMT_SETUPLIGHT ) {
				math::vec3 axis(0,0,0);
				math::Angle angle;
				mRotation.toAxisAngle(&axis, &angle);

				TweakSlider("att.quad", &mDefinition.attenuation.quadratic, 0, 1);
				TweakSlider("att.lin", &mDefinition.attenuation.linear, 0, 1);
				TweakSlider("att.const", &mDefinition.attenuation.constant, 0, 1);

				glPushMatrix();
				glTranslated(mLocation.getX(), mLocation.getY(), mLocation.getZ());
				glRotated(angle.inDegrees(), axis.getX(), axis.getY(), axis.getZ());
				LightSetup::AddLight(mDefinition);
				glPopMatrix();
				return true;
			}
			return false;
		}

		math::vec3& mLocation;
		math::Quaternion& mRotation;
		PointLightDefinition mDefinition;
	};
	class SpotLightComponent3 : public Component3 {
	public:
		SpotLightComponent3(World3& iWorld, Object3& iObject, const ComponentValueMap3& iValues) : Component3(iWorld, iObject), mLocation(iObject.getMemberVec3("loc")), mRotation(iObject.getMemberQuaternion("rot")) {
		}
		static Component3* Build(World3& iWorld, Object3& iObject, const ComponentValueMap3& iValues) {
			return new SpotLightComponent3(iWorld, iObject, iValues);
		}

		bool handleMessage(Message3& iMessage) {
			if( iMessage.getMessageType() == CMT_SETUPLIGHT ) {
				math::vec3 axis(0,0,0);
				math::Angle angle;
				mRotation.toAxisAngle(&axis, &angle);

				TweakSlider("att.quad", &mDefinition.attenuation.quadratic, 0, 1);
				TweakSlider("att.lin", &mDefinition.attenuation.linear, 0, 1);
				TweakSlider("att.const", &mDefinition.attenuation.constant, 0, 1);
				TweakSlider("cutoff", &mDefinition.cutoff, 0, 90);
				TweakSlider("expo", &mDefinition.exponent, 0, 128);

				glPushMatrix();
				glTranslated(mLocation.getX(), mLocation.getY(), mLocation.getZ());
				glRotated(angle.inDegrees(), axis.getX(), axis.getY(), axis.getZ());
				LightSetup::AddLight(mDefinition);
				glPopMatrix();
				return true;
			}
			return false;
		}

		math::vec3& mLocation;
		math::Quaternion& mRotation;
		SpotLightDefinition mDefinition;
	};

	class KeyScriptLink3 {
	public:
		KeyScriptLink3(const std::string& iKeyName, const std::string& iVarName, Variable& iVariable) : mKey(iKeyName), mVarName(iVarName), mState(false), mVariable(iVariable) {
		}
		void testKey(const sgl::Key& iKey, bool iDown) {
			if( mKey == iKey && mState != iDown ) {
				mState = iDown;
				mVariable.setBool(mVarName, mState);
			}
		}
	private:
		sgl::Key mKey;
		const std::string mVarName;
		bool mState;
		Variable& mVariable;
	};
	typedef boost::shared_ptr<KeyScriptLink3> KeyScriptLinkPtr3;
	typedef std::list<KeyScriptLinkPtr3> KeyScriptLinkList3;

	void testRingBuffer() {
		RingBuffer<real> buff(3);
		buff.add(1);
		buff.add(2);
		buff.add(3);
		buff.add(4);
		buff.add(5);

		real a = buff[0];
		real b = buff[1];
		real c = buff[2];
		real d = buff[3];
		real e = buff[4];
		real f = buff[5];
	}

	class ScriptFunctionComponent3 : public Component3 {
	public:
		ScriptFunctionComponent3(World3& iWorld, Object3& iObject, const ComponentValueMap3& iValues) : Component3(iWorld, iObject), mMouseMovementBuffer(RING_BUFFER_MOUSE_SIZE), mFactor(0.25) {
			ComponentValueMap3::const_iterator result = iValues.find("keys");
			if( result == iValues.end() ) {
				throw MissingStringError("string arg 'keys' for movescript component");
			}
			const std::string fileName = result->second.value;

			result = iValues.find("function");
			if( result == iValues.end() ) {
				throw MissingStringError("string arg 'function' for movescript component");
			}
			mFunctionName = result->second.value;

			ReadFile file(fileName);
			TiXmlDocument doc(fileName);
			doc.Parse(file.getBuffer(), 0, TIXML_ENCODING_LEGACY);
			for(TiXmlElement* key = TiXmlHandle(&doc).FirstChildElement("keys").FirstChildElement("key").ToElement(); key; key = key->NextSiblingElement("key")) {
				const std::string varName = GetStringAttribute(key, "key", "var", fileName);
				const std::string configName = GetStringAttribute(key, "key", "key", fileName);
				const std::string keyName = GetString(configName);
				KeyScriptLinkPtr3 keyScript( new KeyScriptLink3(keyName, varName, mVariable) );
				mKeyScripts.push_back( keyScript );
			}
		}
		static Component3* Build(World3& iWorld, Object3& iObject, const ComponentValueMap3& iValues) {
			return new ScriptFunctionComponent3(iWorld, iObject, iValues);
		}

		bool handleMessage(Message3& iMessage) {
			if( iMessage.getMessageType() == CMT_KEY ) {
				MessageKey3& msg = static_cast<MessageKey3&>(iMessage);
				
				for(KeyScriptLinkList3::iterator key=mKeyScripts.begin(); key != mKeyScripts.end(); ++key) {
					(*key)->testKey( msg.key, msg.down );
				}
				return true;
			}
			if( iMessage.getMessageType() == CMT_UPDATE ) {
				if( mHandleMouse ) {
					testRingBuffer();
					mMouseMovementBuffer.add(mMouseMovement);
					const math::vec2 mouse = GetAverage(mMouseMovementBuffer, math::vec2(0,0), mFactor);
					mVariable.setReal("mouseMovementRight", mouse.getX());
					mVariable.setReal("mouseMovementUp", mouse.getY());

					mMouseMovement = math::vec2(0,0);
				}
				MessageUpdate3& msg = static_cast<MessageUpdate3&>(iMessage);
				Call(mFunctionName, &mVariable).arg(msg.time).getReturnedVoid();
				return true;
			}
			if( iMessage.getMessageType() == CMT_RENDER ) {
				TweakSlider("sfactor", &mFactor, 0, 1);
			}
			if( mHandleMouse && iMessage.getMessageType() == CMT_MOUSEMOVE) {
				MessageMouseMove3& msg = static_cast<MessageMouseMove3&>(iMessage);
				mMouseMovement += msg.movement;
				return true;
			}
			return false;
		}
	private:
		KeyScriptLinkList3 mKeyScripts;
		std::string mFunctionName;
		bool mHandleMouse;
		math::vec2 mMouseMovement;
		real mFactor;
		RingBuffer<math::vec2> mMouseMovementBuffer;
	};

	class SimpleMovementComponent3;

	struct MoveForwardScriptFunction {
		MoveForwardScriptFunction(SimpleMovementComponent3& iComponent, real iDirection) : mComponent(iComponent), mDirection(iDirection){
		}
		void operator()(FunctionArgs& iArgs);

		SimpleMovementComponent3& mComponent;
		real mDirection;
	};
	struct MoveRightScriptFunction {
		MoveRightScriptFunction(SimpleMovementComponent3& iComponent, real iDirection) : mComponent(iComponent), mDirection(iDirection){
		}
		void operator()(FunctionArgs& iArgs);

		SimpleMovementComponent3& mComponent;
		real mDirection;
	};
	struct MoveUpScriptFunction {
		MoveUpScriptFunction(SimpleMovementComponent3& iComponent, real iDirection) : mComponent(iComponent), mDirection(iDirection){
		}
		void operator()(FunctionArgs& iArgs);

		SimpleMovementComponent3& mComponent;
		real mDirection;
	};
	struct LookScriptFunction {
		explicit LookScriptFunction(SimpleMovementComponent3& iComponent) : mComponent(iComponent) {
		}
		void operator()(FunctionArgs& iArgs);

		SimpleMovementComponent3& mComponent;
	};
	
	class SimpleMovementComponent3 : public Component3 {
	public:
		SimpleMovementComponent3(World3& iWorld, Object3& iObject, const ComponentValueMap3& iValues) : Component3(iWorld, iObject), mLocation( iObject.getMemberVec3("loc")), mRotation(iObject.getMemberQuaternion("rot")), mSpeed(5) {
			mVariable.setFunction("moveForward", MoveForwardScriptFunction(*this, 1), "moves the object forward");
			mVariable.setFunction("moveBackward", MoveForwardScriptFunction(*this, -1), "moves the object backward");
			mVariable.setFunction("moveRight", MoveRightScriptFunction(*this, 1), "moves the object right");
			mVariable.setFunction("moveLeft", MoveRightScriptFunction(*this, -1), "moves the object left");
			mVariable.setFunction("moveUp", MoveUpScriptFunction(*this, 1), "moves the object up");
			mVariable.setFunction("moveDown", MoveUpScriptFunction(*this, -1), "moves the object down");
			mVariable.setFunction("look", LookScriptFunction(*this), "make the object look");
		}
		static Component3* Build(World3& iWorld, Object3& iObject, const ComponentValueMap3& iValues) {
			return new SimpleMovementComponent3(iWorld, iObject, iValues);
		}

		bool handleMessage(Message3& iMessage) {
			return false;
		}

		void moveForward(real iMovement) {
			mLocation += mRotation.getIn() * iMovement * mSpeed;
		}
		void moveRight(real iMovement) {
			mLocation += mRotation.getRight() * iMovement * mSpeed;
		}
		void moveUp(real iMovement) {
			mLocation += math::op::vec3::yAxisPositive * iMovement * mSpeed;
		}
		void look(real iTime, real x, real y) {
			mRotation = math::Quaternion(math::op::vec3::yAxisPositive, -x)
				* math::Quaternion(mRotation.getRight(), y) * mRotation;
		}
	private:
		math::vec3& mLocation;
		math::Quaternion& mRotation;
		real mSpeed;
	};

	void MoveForwardScriptFunction::operator()(FunctionArgs& iArgs) {
		if( ArgCount(iArgs) != 1 ) {
			ArgReportError(iArgs, "move forward/backward need 1 time arg");
		}
		ArgVarReal(time, iArgs, 0);
		mComponent.moveForward(time*mDirection);
	}
	void MoveRightScriptFunction::operator()(FunctionArgs& iArgs) {
		if( ArgCount(iArgs) != 1 ) {
			ArgReportError(iArgs, "move right/left need 1 time arg");
		}
		ArgVarReal(time, iArgs, 0);
		mComponent.moveRight(time*mDirection);
	}
	void MoveUpScriptFunction::operator()(FunctionArgs& iArgs) {
		if( ArgCount(iArgs) != 1 ) {
			ArgReportError(iArgs, "move up/down need 1 time arg");
		}
		ArgVarReal(time, iArgs, 0);
		mComponent.moveUp(time*mDirection);
	}
	void LookScriptFunction::operator()(FunctionArgs& iArgs) {
		if( ArgCount(iArgs) != 3) {
			ArgReportError(iArgs, "lookRight need time x and y arg");
		}
		ArgVarReal(time, iArgs, 0);
		ArgVarReal(lookx, iArgs, 1);
		ArgVarReal(looky, iArgs, 2);
		mComponent.look(time, lookx, looky);
	}

	class WorldDisplayComponent3 : public Component3 {
	public:
		WorldDisplayComponent3(World3& iWorld, Object3& iObject, const ComponentValueMap3& iValues) : Component3(iWorld, iObject), mLocation(iObject.getMemberVec3("loc")), mRotation(iObject.getMemberQuaternion("rot")) {
		}
		static Component3* Build(World3& iWorld, Object3& iObject, const ComponentValueMap3& iValues) {
			return new WorldDisplayComponent3(iWorld, iObject, iValues);
		}

		void renderVector(const math::vec3& iVec) {
			math::vec3 vec = iVec * 2;
			glBegin(GL_LINES);
				glVertex3d(0,0,0);
				glVertex3d(vec.getX(), vec.getY(), vec.getZ());
			glEnd();
		}

		bool handleMessage(Message3& iMessage) {
			if( iMessage.getMessageType() == CMT_RENDER ) {
				static real redr = 1;
				static real locr = 1;
				glDisable(GL_TEXTURE_2D);

				math::vec3 axis(0,0,0);
				math::Angle angle;
				mRotation.toAxisAngle(&axis, &angle);
				glTranslated(mLocation.getX(), mLocation.getY(), mLocation.getZ());

				glLineWidth(3);
				glColor3f(1, 0 , 0);
				renderVector(mRotation.getRight());

				glColor3f(0, 1 , 0);
				renderVector(mRotation.getUp());

				glColor3f(0, 0 , 1);
				renderVector(mRotation.getIn());
				glLineWidth(1);

				glRotated(angle.inDegrees(), axis.getX(), axis.getY(), axis.getZ());

				TweakSlider("red", &redr, 0.1, 1);
				TweakSlider("loc", &locr, 0.2, 2);

				const float red = redr;
				const float loc = locr;

				glBegin(GL_TRIANGLES);
					glColor3f(red,0.0f,0.0f);
					glVertex3f( 0.0f, loc, 0.0f);
					glColor3f(0.0f,1.0f,0.0f);
					glVertex3f(-1.0f,-1.0f, 1.0f);
					glColor3f(0.0f,0.0f,1.0f);
					glVertex3f( 1.0f,-1.0f, 1.0f);

					glColor3f(red,0.0f,0.0f);
					glVertex3f( 0.0f, loc, 0.0f);
					glColor3f(0.0f,0.0f,1.0f);
					glVertex3f( 1.0f,-1.0f, 1.0f);
					glColor3f(0.0f,1.0f,0.0f);
					glVertex3f( 1.0f,-1.0f, -1.0f);

					glColor3f(red,0.0f,0.0f);
					glVertex3f( 0.0f, loc, 0.0f);
					glColor3f(0.0f,1.0f,0.0f);
					glVertex3f( 1.0f,-1.0f, -1.0f);
					glColor3f(0.0f,0.0f,1.0f);
					glVertex3f(-1.0f,-1.0f, -1.0f);

					glColor3f(red,0.0f,0.0f);
					glVertex3f( 0.0f, loc, 0.0f);
					glColor3f(0.0f,0.0f,1.0f);
					glVertex3f(-1.0f,-1.0f,-1.0f);
					glColor3f(0.0f,1.0f,0.0f);
					glVertex3f(-1.0f,-1.0f, 1.0f);
				glEnd();

				glEnable(GL_TEXTURE_2D);
			}
			return false;
		}

		math::vec3& mLocation;
		math::Quaternion& mRotation;
	};

	/*
	class WorldDisplayComponent3 : public Component3 {
		WorldDisplayComponent3(World3& iWorld, Object3& iObject, const ComponentValueMap3& iValues) : Component3(iWorld, iObject) {
		}
		static Component3* Build(World3& iWorld, Object3& iObject, const ComponentValueMap3& iValues) {
			return new WorldDisplayComponent3(iWorld, iObject, iValues);
		}

		bool handleMessage(Message3& iMessage) {
			return false;
		}
	};
	*/

	// ----------------------------------------------------------------------------

	class ObjectBuilder3 {
	public:
		explicit ObjectBuilder3(World3* iWorld, const std::string& iFileName) : mWorld(*iWorld) {
			ReadFile file(iFileName);
			TiXmlDocument doc(iFileName);
			doc.Parse(file.getBuffer(), 0, TIXML_ENCODING_LEGACY);
			const std::string objectName = "object";
			for( TiXmlElement* object = TiXmlHandle(&doc).FirstChildElement("objects").FirstChildElement(objectName).ToElement(); object; object = object->NextSiblingElement(objectName) ) {
				const std::string type = GetStringAttribute(object, objectName, "type", iFileName);
				const std::string componentName = "component";
				ObjectDefinitionPtr3 obj( new ObjectDefinition3() );
				mObjectDefinitions[type] = obj;
				for(TiXmlElement* component=object->FirstChildElement(componentName); component; component = component->NextSiblingElement(componentName) ) {
					ComponentDefinitionPtr3 cmp( new ComponentDefinition3() );
					cmp->componentName = GetStringAttribute(component, componentName, "type", iFileName);

					for( TiXmlAttribute* attribute = component->FirstAttribute(); attribute; attribute = attribute->Next() ) {
						const std::string name = attribute->Name();
						const std::string value = attribute->Value();
						ComponentValue3 val;
						val.value = value;
						cmp->arguments[name] = val;
					}
					obj->components.push_back(cmp);
				}
			}
		}

		void registerDefaultComponents() {
#define RegisterComponent( name ) addComponentBuilder( #name, name ## Component3::Build )
			RegisterComponent(View);
			RegisterComponent(ScriptFunction);
			RegisterComponent(SimpleMovement);
			RegisterComponent(WorldDisplay);
			RegisterComponent(PointLight);
			RegisterComponent(SpotLight);
#undef RegisterComponent
		}

		Object3* buildObject(const std::string& iType) {
			ObjectDefinitionMap3::iterator result = mObjectDefinitions.find(iType);
			if( result == mObjectDefinitions.end() ) {
				throw MissingObject3Error(iType);
			}
			std::auto_ptr<Object3> object(new Object3(mWorld) );
			Object3& ref = *object.get();
			ObjectDefinitionPtr3 def = result->second;
			for(ComponentDefinitionList3::iterator cdef = def->components.begin(); cdef != def->components.end(); ++cdef) {
				const std::string componentName = (*cdef)->componentName;
				const ComponentValueMap3& componentValues = (*cdef)->arguments;
				ref.addComponent( buildComponent(ref, componentName, componentValues ) );
			}
			return object.release();
		}

		void addComponentBuilder(const std::string& iName, ComponentBuilder3 iBuilder) {
			mComponentBuilders.insert( ComponentBuilderMap3::value_type(iName, iBuilder) );
		}

		Component3* buildComponent(Object3& iObject, const std::string& iName, const ComponentValueMap3& iComponentValues) {
			ComponentBuilderMap3::iterator result = mComponentBuilders.find(iName);
			if( result == mComponentBuilders.end() ) {
				throw MissingComponent3Error(iName);
			}

			return result->second(mWorld, iObject, iComponentValues);
		}

	private:
		World3& mWorld;
		ComponentBuilderMap3 mComponentBuilders;
		ObjectDefinitionMap3 mObjectDefinitions;
	};

	// -----------------------------------------------------------

	typedef boost::shared_array<::lunarlady::files::optimized::Face> FaceArray;
	typedef boost::shared_array<real> RealArray;

	struct StaticMeshDescriptor {
		RealArray vertices;
		RealArray normals;
		RealArray texCoords;
		FaceArray faces;
		std::size_t faceCount;
	};

	class StaticMeshLoaded : public Loaded {
	public:
		virtual void render() = 0;

		static StaticMeshLoaded* Load(const StaticMeshDescriptor& iDescription);
		static void Unload(StaticMeshLoaded* iLoadedStaticMesh) {
			delete iLoadedStaticMesh;
		}
	private:
	};

	typedef TemplatedMedia<StaticMeshLoaded, StaticMeshDescriptor> StaticMesh;

	class StaticMeshLoaded_ImmediateMode : public StaticMeshLoaded {
	public:
		StaticMeshLoaded_ImmediateMode(const StaticMeshDescriptor& iDescription) : mDescription(iDescription) {
		}
		void render() {
			glBegin(GL_TRIANGLES);
			for(std::size_t index=0; index<mDescription.faceCount; ++index) {
				for(std::size_t i=0; i<3; ++i) {
					const std::size_t base = mDescription.faces[index].vertex[i];
					const std::size_t vertexIndex = base*3;
					const std::size_t normalIndex = base*3;
					const std::size_t texcoordIndex = base*2;
					glNormal3d(mDescription.normals[normalIndex+0], mDescription.normals[normalIndex+1], mDescription.normals[normalIndex+2]);
					glTexCoord2d(mDescription.texCoords[texcoordIndex+0], mDescription.texCoords[texcoordIndex+1]);
					glVertex3d(mDescription.vertices[vertexIndex+0], mDescription.vertices[vertexIndex+1], mDescription.vertices[vertexIndex+2]);
				}
			}
			glEnd();
		}
	private:
		const StaticMeshDescriptor& mDescription;
	};

	class StaticMeshLoaded_VertexArray : public StaticMeshLoaded {
	public:
		StaticMeshLoaded_VertexArray(const StaticMeshDescriptor& iDescription) : mDescription(iDescription), mFaceIndices( new GLuint[mDescription.faceCount*3] ), mFaceCount(mDescription.faceCount) {
			for(std::size_t index=0; index<mDescription.faceCount; ++index) {
				for(std::size_t i=0; i<3; ++i) {
					const std::size_t base = mDescription.faces[index].vertex[i];
					mFaceIndices[index*3+i] = base;
				}
			}
		}
		void render() {
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glEnableClientState(GL_NORMAL_ARRAY);
			glVertexPointer(3, GL_DOUBLE, 0, mDescription.vertices.get());
			glTexCoordPointer(2, GL_DOUBLE, 0, mDescription.texCoords.get());
			glNormalPointer(GL_DOUBLE, 0, mDescription.normals.get());
			glDrawElements(	GL_TRIANGLES,
						mFaceCount*3,
						GL_UNSIGNED_INT,
						mFaceIndices.get());
			glDisableClientState(GL_NORMAL_ARRAY);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY);
		}
	private:
		const StaticMeshDescriptor& mDescription;
		boost::scoped_array<GLuint> mFaceIndices;
		GLsizei mFaceCount;
	};

	StaticMeshLoaded* StaticMeshLoaded::Load(const StaticMeshDescriptor& iDescription) {
		return new StaticMeshLoaded_VertexArray(iDescription);
	}

	typedef boost::shared_ptr<Image> ImagePtr;
	typedef std::map<std::string, ImagePtr> ImageMap;

	class Material {
	public:
		Material(const MaterialDefinition& iMaterialDefinition) {
			mAmbient = iMaterialDefinition.ambient;
			mDiffuse = iMaterialDefinition.diffuse;
			mEmissive = iMaterialDefinition.emissive;
			mSpecular = iMaterialDefinition.specular;
			mExponent = iMaterialDefinition.exponent;
			mAlpha = iMaterialDefinition.alpha;

			const std::size_t length = iMaterialDefinition.textureMedia.size();
			for(std::size_t index=0; index < length; ++index) {
				const std::string name = iMaterialDefinition.textureMedia[index];
				ImagePtr ptr(new Image(ImageDescriptor(name)));
				mImageMedia.insert( ImageMap::value_type(name, ptr));
			}

			selectColorTexture( iMaterialDefinition.colorTextureName );
		}
		void begin() {
			if( mColorTexture==0 ) {
				glDisable(GL_TEXTURE_2D);
			}
			else {
				glBindTexture(GL_TEXTURE_2D, (*mColorTexture)->getId());
			}
			glColor3d(mDiffuse.getRed(), mDiffuse.getGreen(), mDiffuse.getBlue());
		}
		void end() {
			if( mColorTexture == 0 ) {
				glEnable(GL_TEXTURE_2D);
			}
		}

		void selectColorTexture(const std::string& iTextureName) {
			mColorTexture = selectTexture(iTextureName);
		}

	private:
		Image* selectTexture(const std::string& iTextureName) {
			if( iTextureName.empty() ) {
				return 0;
			}
			ImageMap::iterator result = mImageMedia.find(iTextureName);
			if( result == mImageMedia.end() ) {
				throw "";
			}

			return result->second.get();
		}
		Rgb mAmbient;
		Rgb mDiffuse;
		Rgb mEmissive;
		Rgb mSpecular;
		real mExponent;
		real mAlpha;

		// images
		Image* mColorTexture;
		/*
		std::auto_ptr<Image> mNormal;
		std::auto_ptr<Image> mSpecular;
		std::auto_ptr<Image> mEmissive;
		std::auto_ptr<Image> mGlow;
		*/

		ImageMap mImageMedia;
	};

	class Part {
	public:
		Part(const ::lunarlady::files::optimized::FaceList& iFace, const std::string& iMaterialName, RealArray iVertices, RealArray iNormals, RealArray iTexCoords) : mMaterial( GetMaterialDefinition(iMaterialName) ) {
			StaticMeshDescriptor descriptor;
			descriptor.vertices = iVertices;
			descriptor.normals = iNormals;
			descriptor.texCoords = iTexCoords;
			descriptor.faceCount = iFace.size();
			descriptor.faces.reset( new ::lunarlady::files::optimized::Face[descriptor.faceCount] );
			for(std::size_t i=0; i<descriptor.faceCount; ++i) {
				descriptor.faces[i] = iFace[i];
			}
			mMesh.reset( new StaticMesh(descriptor) );
		}
		void render() {
			StaticMesh& mesh = *mMesh;
			mMaterial.begin();
			mesh->render();
			mMaterial.end();
		}
	private:
		Material mMaterial;
		std::auto_ptr<StaticMesh> mMesh;
	};
	typedef boost::shared_ptr<Part> PartPtr;
	typedef std::vector<PartPtr> PartList;

	class StaticDisplayBase {
	public:
		StaticDisplayBase(const std::vector<::lunarlady::files::optimized::Vertex>& iVertices, const ::lunarlady::files::optimized::FaceMap& iFaces) {
			const std::size_t vertexCount = iVertices.size();
			mVertices.reset( new real[ vertexCount * 3 ] );
			mNormals.reset( new real[ vertexCount * 3 ] );
			mTexCoords.reset( new real[ vertexCount * 2 ] );
			for(std::size_t i=0; i<vertexCount; ++i) {
				mVertices[i*3+0] = iVertices[i].vertex.getX();
				mVertices[i*3+1] = iVertices[i].vertex.getY();
				mVertices[i*3+2] = iVertices[i].vertex.getZ();

				mNormals[i*3+0] = iVertices[i].normal.getX();
				mNormals[i*3+1] = iVertices[i].normal.getY();
				mNormals[i*3+2] = iVertices[i].normal.getZ();

				mTexCoords[i*2+0] = iVertices[i].texcoord.getX();
				mTexCoords[i*2+1] = iVertices[i].texcoord.getY();
			}

			mParts.reserve(iFaces.size());
			
			for(::lunarlady::files::optimized::FaceMap::const_iterator faceMaterial=iFaces.begin(); faceMaterial != iFaces.end(); ++faceMaterial) {
				PartPtr part( new Part(faceMaterial->second, faceMaterial->first, mVertices, mNormals, mTexCoords) );
				mParts.push_back(part);
			}
		}

		void render() {
			for(PartList::iterator partIterator=mParts.begin(); partIterator != mParts.end(); ++partIterator) {
				PartPtr& part = *partIterator;
				part->render();
			}
		}
	private:
		RealArray mVertices;
		RealArray mNormals;
		RealArray mTexCoords;

		PartList mParts;
	};

	// -----------------------------------------------------------

	class SimpleWorld3 : public World3 {
	public:
		explicit SimpleWorld3 (const ReadFile& iFile) : mCamera(0) {
			mBuilder.reset(new ObjectBuilder3(this, "game/objects.xml"));
			mBuilder->registerDefaultComponents();

			::lunarlady::files::optimized::Model model;
			model.load(iFile.getBuffer(), iFile.getSize());
			mDisplay.reset( new StaticDisplayBase(model.vertices, model.faces));

			addObject("Camera");
		}

		void addObject(const std::string& iName) {
			addObject( mBuilder->buildObject(iName) );
		}
		void addObject(Object3* iObject) {
			ObjectPtr3 object(iObject);
			mObjects.push_back(object);
			mCamera = iObject;
		}

		void sendMessage(Message3& iMessage) {
			for(ObjectList3::iterator object = mObjects.begin(); object != mObjects.end(); ++object) {
				(*object)->handleMessage( iMessage );
			}
		}

		void update(real iTime) {
			sendMessage( MessageUpdate3(iTime) );
		}

		void render(real iTime) {
			if( mCamera ) {
				if( !mCamera->handleMessage( MessageSetupView3() ) ) {
					throw Message3NotHandledError("setup view");
				}
				glEnable(GL_LIGHTING);
				LightSetup::Clear();
				sendMessage( MessageSetupLight3() );
				mDisplay->render();
				sendMessage( MessageRender3(iTime) );
				glDisable(GL_LIGHTING);
			}
		}

		void onMouseMovement(const math::vec2& iMovement){
			sendMessage( MessageMouseMove3(iMovement) );
		}

		void onKey(const sgl::Key& iKey, bool iDown){
			sendMessage( MessageKey3(iKey, iDown) );
		}
	private:
		std::auto_ptr<ObjectBuilder3> mBuilder;
		ObjectList3 mObjects;
		Object3* mCamera;
		std::auto_ptr<StaticDisplayBase> mDisplay;
	};

	// -------------------------------------------------------------

	World3* World3::Load(const std::string &iName) {
		ReadFile file(iName);
		return new SimpleWorld3(file);
	}
}