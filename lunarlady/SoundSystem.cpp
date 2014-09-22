#include "fmod.hpp"
#include "fmod_errors.h"

#include "sgl/sgl_Assert.hpp"

#include "lunarlady/System.hpp"
#include "lunarlady/Game.hpp"
#include "lunarlady/Log.hpp"
#include "lunarlady/Script.hpp"
#include "lunarlady/File.hpp"
#include "lunarlady/Xml.hpp"
#include "lunarlady/StringUtils.hpp"
#include "lunarlady/Display.hpp"
#include "lunarlady/Config.hpp"
#include "lunarlady/Error.hpp"

#include "physfs.h"

namespace lunarlady {

	class SoundSystem;

	namespace {
		std::string GetNameAndVersion() {
			const unsigned int major = 0x0000FFFF & (FMOD_VERSION>>16);
			const unsigned int minor = 0x000000FF & (FMOD_VERSION>>8);
			const unsigned int dev = 0x000000FF & (FMOD_VERSION>>0);
			std::stringstream str;
			str << "FMOD Ex v"
				<< std::hex << major << "." << std::hex << minor << ":" << std::hex << dev;
			return str.str();
		}

		SoundSystem* gSoundSystem = 0;
	}

	FMOD_RESULT F_CALLBACK Open(const char *name, int unicode, unsigned int *filesize, void **handle, void **userdata) {
		if (name) {

			if ( !PHYSFS_exists(name) ) {
				return FMOD_ERR_FILE_NOTFOUND;
			}

			PHYSFS_file* fp = PHYSFS_openRead(name);
			if( !fp ) {
				return FMOD_ERR_FILE_BAD;
			}

			*filesize = PHYSFS_fileLength(fp);
			*userdata = (void *)0x12345678;
			*handle = fp;
		}

		return FMOD_OK;
	}

	FMOD_RESULT F_CALLBACK Close(void *handle, void *userdata) {
		if (!handle) {
			return FMOD_ERR_INVALID_PARAM;
		}
		PHYSFS_file* fp = (PHYSFS_file*) handle;
		PHYSFS_close(fp);

		return FMOD_OK;
	}

	FMOD_RESULT F_CALLBACK Read(void *handle, void *buffer, unsigned int sizebytes, unsigned int *bytesread, void *userdata) {
		if (!handle) {
			return FMOD_ERR_INVALID_PARAM;
		}

		if (bytesread) {
			PHYSFS_file* fp = (PHYSFS_file*) handle;
			*bytesread = (int)PHYSFS_read(fp, buffer, 1, sizebytes);
	    
			if (*bytesread < sizebytes) {
				return FMOD_ERR_FILE_EOF;
			}
		}

		return FMOD_OK;
	}

	FMOD_RESULT F_CALLBACK Seek(void *handle, unsigned int pos, void *userdata) {
		if (!handle) {
			return FMOD_ERR_INVALID_PARAM;
		}

		PHYSFS_file* fp = (PHYSFS_file*) handle;
		//fseek((FILE *)handle, pos, SEEK_SET);
		int result = PHYSFS_seek(fp, pos);
		if( result == 0 ) {
			return FMOD_ERR_FILE_COULDNOTSEEK;
		}
		return FMOD_OK;
	}

	typedef std::list<std::string> SoundSource;
	typedef std::map<std::string, std::string> SoundDefinitionMap2;
	typedef std::map<std::string, SoundSource> SoundDefinitionMap3;
	typedef std::map<std::string, FMOD::Sound*> StringSoundMap;
	typedef std::map<std::string, FMOD::Channel*> StringChannelMap;

	std::string AsValidPath(const std::string& iPath) {
		if( EndsWith(iPath, "/") ) {
			return iPath;
		}
		else {
			return iPath + "/";
		}
	}

	class SoundSystem : public System {
	public:
		SoundSystem() : System(GetNameAndVersion()), mSystem(0) {
			Assert(!gSoundSystem, "Already has a sound system");
			test( FMOD::System_Create(&mSystem), "creating system" );
			Assert(mSystem, "internal error, bug?");
			// A buffer of 0 means all reads go directly to the pointer specified. 2048 bytes is the size of a CD sector on most CD ISO formats so it is chosen as the default, for optimal reading speed from CD media.
			test(mSystem->setFileSystem(Open, Close, Read, Seek, 2048), "setting file system");
			test( mSystem->init(100, FMOD_INIT_NORMAL, 0), "initializing system" );
			gSoundSystem = this;

			test(mSystem->getMasterChannelGroup(&mMasterChannel), "getting master channel group");
			test(mSystem->createChannelGroup("interface", &mInterfaceChannel), "creating interface channel group");
			test(mSystem->createChannelGroup("music", &mMusicChannel), "creating music channel group");
			test( mMasterChannel->addGroup(mInterfaceChannel), "adding interface group");
			test( mMasterChannel->addGroup(mMusicChannel), "adding music group");

			loadConfig();

			loadDefinitionsDirectory("sfx/");
		}
		~SoundSystem() {
			Assert(gSoundSystem, "Need a sound system");

			try {
				Assert(mSystem, "internal error, bug?");
				saveConfig();
				test(mInterfaceChannel->release(), "releasing interface channel");
				test(mMusicChannel->release(), "releasing music channel");
				test(mSystem->release(), "releasing system");
			}
			catch(...) {
			}

			gSoundSystem = 0;
		}

		FMOD::Channel* getPlayingStreamChannel2(const std::string& iStreamId) {
			StringChannelMap::iterator result = mPlayingStreams.find(iStreamId);
			if( result == mPlayingStreams.end() ) return 0;
			else return result->second;
		}

		void playStream2(const std::string& iStreamId, const std::string& iFileName, bool iLoop) {
			Assert(mSystem, "internal error, bug?");
			FMOD::Sound* sound = 0;
			FMOD::Channel* channel = getPlayingStreamChannel2(iStreamId);
			if( channel ) {
				bool playing = false;
				test(channel->isPlaying(&playing), "getting isplaying from stream channel");
				if( playing ) {
					bool paused = false;
					test( channel->getPaused(&paused), "getting paused state" );
					if( !paused ) {
						stopStream2(iStreamId);
						channel = 0;
					}
				}
			}
			if( channel == 0 ) {
				test(mSystem->createStream(iFileName.c_str(), FMOD_DEFAULT | FMOD_2D, 0, &sound), "loading sound");
				if( iLoop ) {
					test(sound->setMode(FMOD_LOOP_NORMAL), "setting loop normal");
				}
				test(mSystem->playSound(FMOD_CHANNEL_FREE, sound, true, &channel), "playing sound");
				test( channel->setChannelGroup(mMusicChannel), "setting stream to music channel");
			}
			test(channel->setPaused(false), "setting paused on channel");
			mPlayingStreams[iStreamId] = channel;
		}
		void pauseStream2(const std::string& iStreamId) {
			FMOD::Channel* channel = getPlayingStreamChannel2(iStreamId);
			if( channel ) {
				test(channel->setPaused(true), "setting paused on channel");
			}
		}
		void stopStream2(const std::string& iStreamId) {
			FMOD::Channel* channel = getPlayingStreamChannel2(iStreamId);
			if( channel ) {
				FMOD::Sound* sound = 0;
				test(channel->getCurrentSound(&sound), "destroying stream, getting sound");
				test(channel->stop(), "stopping channel");
				mPlayingStreams[iStreamId] = 0;
				sound->release();
			}
		}

		void step(real iTime) {
			Assert(mSystem, "internal error, bug?");
			test(mSystem->update(), "updating");
			float cpu = 0;
			test(mSystem->getCPUUsage(NULL, NULL, NULL, &cpu), "grabbing cpu usage");
			int memory = 0;
			test(FMOD::Memory_GetStats(&memory, NULL), "getting memroy stats");
			DisplayPrecision("fmod cpu", cpu, 1, true);
			DisplayPrecision("fmod mem", memory/1024.0, 1, true);
		}

		void test(FMOD_RESULT iResult, const std::string& iDescription) {
			if( iResult != FMOD_OK ) {
				std::stringstream str;
				str << "FMOD error! (" << iResult << ") " << FMOD_ErrorString(iResult) << " while " << iDescription;
				LOG1( str.str() );
				throw str.str();
			}
		}

		void loadDefinitionsDirectory(const std::string& iDirectory) {
			std::vector<std::string> files;
			GetFileListing(iDirectory, &files);
			const std::size_t count = files.size();
			for(std::size_t fileIndex=0; fileIndex<count; ++fileIndex) {
				const std::string file = files[fileIndex];
				if( EndsWith(file, ".sfx") ) {
					loadDefinitionsFile(file);
				}
			}
		}
		void loadDefinitionsFile(const std::string& iFile) {
			ReadFile file(iFile);
			TiXmlDocument document(iFile.c_str());
			document.Parse(file.getBuffer(), 0, TIXML_ENCODING_LEGACY);
			loadDefinitions(document.FirstChildElement("sounds"), "");
		}
		void loadDefinitions(TiXmlElement* iContainer, const std::string& iBase) {
			if( !iContainer ) return;
			std::string name="";
			// handle base
			for(TiXmlElement* child=iContainer->FirstChildElement("base"); child; child=child->NextSiblingElement("base") ) {
				const char* src = child->Attribute("src");
				std::string base = iBase;
				if( src ) {
					base = AsValidPath(base+src);
				}
				loadDefinitions(child, base);
			}

			// directory
			for(TiXmlElement* child=iContainer->FirstChildElement("directory"); child; child=child->NextSiblingElement("directory") ) {
				const char* src = child->Attribute("src");
				if( !src ) {
					throw SoundError("Directory element missing source attribute");
				}
				loadDefinitionsDirectory( AsValidPath(iBase + src) );
			}

			// sound
			for(TiXmlElement* child=iContainer->FirstChildElement("sound"); child; child=child->NextSiblingElement("sound") ) {
				const char* name = child->Attribute("name");
				if( !name ) {
					throw SoundError("Missing name-attribute for sound element");
				}
				const char* src = child->Attribute("src");
				if( src ) {
					mDefinedSounds2d[name] = iBase + src;
				}
				else {
					SoundSource& soundSource = mDefinedSounds3d[name];
					for(TiXmlElement* file=child->FirstChildElement("file"); file; file=file->NextSiblingElement("file") ) {
						const char* src = file->Attribute("src");
						if( !src ) {
							throw SoundError("File element missing source attribute");
						}
						soundSource.push_back(iBase + src);
					}
				}
			}
		}

		void playSound2(const std::string& iSoundId) {
			StringSoundMap::iterator res = mLoadedSounds2d.find(iSoundId);
			if( res == mLoadedSounds2d.end() ) {
				throw SoundError("sound id isn't preloaded");
			}
			FMOD::Sound* sound = res->second;
			FMOD::Channel* channel = 0;
			test( mSystem->playSound(FMOD_CHANNEL_FREE, sound, false, &channel), "playing sound-id" );
			test(channel->setChannelGroup(mInterfaceChannel), "setting sound to interface channel");
		}
		void preloadSound2(const std::string& iSoundId) {
			if( mLoadedSounds2d.find(iSoundId) != mLoadedSounds2d.end() ) return;
			SoundDefinitionMap2::iterator res = mDefinedSounds2d.find(iSoundId);
			if( res == mDefinedSounds2d.end() ) {
				throw SoundError("Failed to preload sound, since id is missing");
			}
			const std::string fileName = res->second;
			FMOD::Sound* sound = 0;
			test(mSystem->createSound(fileName.c_str(), FMOD_DEFAULT | FMOD_2D, 0, &sound), "preloading sound");
			mLoadedSounds2d.insert( StringSoundMap::value_type(iSoundId, sound) );
		}

		real getMasterVolume() {
			return getVolume(mMasterChannel);
		}
		real getInterfaceVolume() {
			return getVolume(mInterfaceChannel);
		}
		real getMusicVolume() {
			return getVolume(mMusicChannel);
		}

		void setMasterVolume(real iVolume) {
			setVolume(mMasterChannel, iVolume);
		}
		void setInterfaceVolume(real iVolume) {
			setVolume(mInterfaceChannel, iVolume);
		}
		void setMusicVolume(real iVolume) {
			setVolume(mMusicChannel, iVolume);
		}

		void saveConfig() {
			SetReal("volume.master", getMasterVolume());
			SetReal("volume.interface", getInterfaceVolume());
			SetReal("volume.music", getMusicVolume());
		}
		void loadConfig() {
			setMasterVolume( GetReal("volume.master") );
			setInterfaceVolume( GetReal("volume.interface") );
			setMusicVolume( GetReal("volume.music") );
		}

	private:
		real getVolume(FMOD::ChannelGroup* iGroup) {
			float volume = 0;
			test(iGroup->getVolume(&volume), "getting volume from group");
			return volume;
		}
		void setVolume(FMOD::ChannelGroup* iGroup, real iVolume) {
			const float volume = iVolume;
			test(iGroup->setVolume(volume), "setting volume for group");
		}

		FMOD::System* mSystem;

		FMOD::ChannelGroup* mMasterChannel;
		FMOD::ChannelGroup* mInterfaceChannel;
		FMOD::ChannelGroup* mMusicChannel;

		//FMOD::ChannelGroup* mEffectChannel;
		//FMOD::ChannelGroup* mVoiceChannel;
		//FMOD::ChannelGroup* mConversationChannel;

		SoundDefinitionMap2 mDefinedSounds2d;
		SoundDefinitionMap3 mDefinedSounds3d;
		StringSoundMap mLoadedSounds2d;
		StringChannelMap mPlayingStreams;
	};

	void PlayStream2(const std::string iStreamId, const std::string iFileName, bool iLoop) {
		Assert(gSoundSystem, "Need a sound system");
		gSoundSystem->playStream2(iStreamId, iFileName, iLoop);
	}
	void PauseStream2(const std::string iStreamId) {
		Assert(gSoundSystem, "Need a sound system");
		gSoundSystem->pauseStream2(iStreamId);
	}
	void StopStream2(const std::string iStreamId) {
		Assert(gSoundSystem, "Need a sound system");
		gSoundSystem->stopStream2(iStreamId);
	}

	void PlaySound2(const std::string iSoundId) {
		Assert(gSoundSystem, "Need a sound system");
		gSoundSystem->playSound2(iSoundId);
	}
	void PreloadSound2(const std::string iSoundId) {
		Assert(gSoundSystem, "Need a sound system");
		gSoundSystem->preloadSound2(iSoundId);
	}

	void PlayStream2ScriptFunction(FunctionArgs& iArgs) {
		const int args = ArgCount(iArgs);
		if( args != 3) ArgReportError(iArgs, "syntax: (string id, string path, bool loop)");
		ArgVarString(id, iArgs, 0);
		ArgVarString(file, iArgs, 1);
		ArgVarInt(loop, iArgs, 2);
		try {
			PlayStream2(id, file, loop==1);
		}
		catch(const std::runtime_error& error) {
			ArgError(iArgs, error.what());
			ArgReportError(iArgs, "Failed to stream2 " << file );
		}
	}
	void PauseStream2ScriptFunction(FunctionArgs& iArgs) {
		const int args = ArgCount(iArgs);
		if( args != 1) ArgReportError(iArgs, "syntax: (string id)");
		ArgVarString(id, iArgs, 0);
		PauseStream2(id);
	}
	void StopStream2ScriptFunction(FunctionArgs& iArgs) {
		const int args = ArgCount(iArgs);
		if( args != 1) ArgReportError(iArgs, "syntax: (string id)");
		ArgVarString(id, iArgs, 0);
		StopStream2(id);
	}
	void PlaySound2ScriptFunction(FunctionArgs& iArgs) {
		const int args = ArgCount(iArgs);
		if( args != 1) ArgReportError(iArgs, "Need 1 string sound-id argument");
		ArgVarString(file, iArgs, 0);
		try {
			PlaySound2(file);
		}
		catch(const std::runtime_error& error) {
			ArgError(iArgs, error.what());
			ArgReportError(iArgs, "Failed to play sound2 " << file );
		}
	}
	void PreloadSound2ScriptFunction(FunctionArgs& iArgs) {
		const int args = ArgCount(iArgs);
		if( args != 1) ArgReportError(iArgs, "Need 1 string sound-id argument");
		ArgVarString(file, iArgs, 0);
		try {
			PreloadSound2(file);
		}
		catch(const std::runtime_error& error) {
			ArgError(iArgs, error.what());
			ArgReportError(iArgs, "Failed to preload sound2 " << file );
		}
	}

	SCRIPT_FUNCTION(playStream2, PlayStream2ScriptFunction, "Starts to play a stream, if playing it is reset, if paused it continues");
	SCRIPT_FUNCTION(pauseStream2, PauseStream2ScriptFunction, "Pauses a stream, to unpause use playStream2");
	SCRIPT_FUNCTION(stopStream2, StopStream2ScriptFunction, "Stops a stream");
	SCRIPT_FUNCTION(playSound2, PlaySound2ScriptFunction, "Plays a sound id, must be pre-loaded");
	SCRIPT_FUNCTION(preloadSound2, PreloadSound2ScriptFunction, "Preloads a sound");

	void MasterVolumeScriptFunction(FunctionArgs& iArgs) {
		Assert(gSoundSystem, "Need a sound system");
		if( ArgCount(iArgs) == 0 ) {
			const real value = gSoundSystem->getMasterVolume();
			Return(iArgs, value);
		}
		else {
			if( ArgCount(iArgs) != 1 ) {
				ArgReportError(iArgs, "Need 1 or 0 floating arguments range, 0-1");
			}
			else {
				ArgVarReal(volume, iArgs, 0);
				gSoundSystem->setMasterVolume(volume);
			}
		}
	}
	SCRIPT_FUNCTION(masterVolume, MasterVolumeScriptFunction, "sets or gets the master volume, range 0-1");

	void InterfaceVolumeScriptFunction(FunctionArgs& iArgs) {
		Assert(gSoundSystem, "Need a sound system");
		if( ArgCount(iArgs) == 0 ) {
			const real value = gSoundSystem->getInterfaceVolume();
			Return(iArgs, value);
		}
		else {
			if( ArgCount(iArgs) != 1 ) {
				ArgReportError(iArgs, "Need 1 or 0 floating arguments range, 0-1");
			}
			else {
				ArgVarReal(volume, iArgs, 0);
				gSoundSystem->setInterfaceVolume(volume);
			}
		}
	}
	SCRIPT_FUNCTION(interfaceVolume, InterfaceVolumeScriptFunction, "sets or gets the interface volume, range 0-1");

	void MusicVolumeScriptFunction(FunctionArgs& iArgs) {
		Assert(gSoundSystem, "Need a sound system");
		if( ArgCount(iArgs) == 0 ) {
			const real value = gSoundSystem->getMusicVolume();
			Return(iArgs, value);
		}
		else {
			if( ArgCount(iArgs) != 1 ) {
				ArgReportError(iArgs, "Need 1 or 0 floating arguments range, 0-1");
			}
			else {
				ArgVarReal(volume, iArgs, 0);
				gSoundSystem->setMusicVolume(volume);
			}
		}
	}
	SCRIPT_FUNCTION(musicVolume, MusicVolumeScriptFunction, "sets or gets the music volume, range 0-1");

	LL_SYSTEM(SoundSystem, 1000);
}