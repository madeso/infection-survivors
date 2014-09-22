#include "lunarlady/Script.hpp"

#pragma warning(push)
#pragma warning(disable:4311)
#pragma warning(disable:4312)
#pragma warning(disable:4244)
#include "gmThread.h"
#include "gmMathLib.h"
#include "gmStringLib.h"
#pragma warning(pop)

#include "sgl/sgl_Assert.hpp"

#include "lunarlady/Message.hpp"

#include "lunarlady/System.hpp"
#include "lunarlady/Game.hpp"
#include "lunarlady/Log.hpp"
#include "lunarlady/Error.hpp"
#include "lunarlady/StringMap.hpp"
#include "lunarlady/Xml.hpp"
#include "lunarlady/File.hpp"
#include "lunarlady/StringUtils.hpp"
#include "lunarlady/Error.hpp"

#pragma warning(push)
#pragma warning(disable:4312)
#pragma warning(disable:4311)

namespace lunarlady {	
	struct FunctionDefinition {
		FunctionDefinition(const std::string& iFunctionName, ScriptFunction iFunction, const std::string& iDocumentation) : name(iFunctionName), function(iFunction), documentation(iDocumentation) {
		}
		const std::string name;
		ScriptFunction function;
		const std::string documentation;
	};

	namespace {
		typedef std::map<std::string, std::string> StringStringMap;
		typedef std::pair<std::string, std::string> StringStringPair;
		typedef util::StringMap<std::string, char> StringLookup;

		typedef std::list<FunctionDefinition> FunctionDefinitionList;

		FunctionDefinitionList& GetFunctionList() {
			static FunctionDefinitionList gGlobalScriptFunctions;
			return gGlobalScriptFunctions;
		}
	}

	class FunctionArgs : boost::noncopyable {
	public:
		FunctionArgs(gmThread* iThread) : thread(iThread), error(false){
		}
		bool error;
		gmThread* thread;
	};
	int ArgCount(FunctionArgs& iArguments) {
		gmThread* thread = iArguments.thread;
		return thread->GetNumParams();;
	}

	void ImplReturn(FunctionArgs& iArguments, real iValue) {
		gmThread* thread = iArguments.thread;
		thread->PushFloat( static_cast<gmfloat>(iValue) );
	}
	void ImplReturn(FunctionArgs& iArguments, const std::string& iValue) {
		gmThread* thread = iArguments.thread;
		thread->PushNewString(iValue.c_str());
	}
	void ImplReturn(FunctionArgs& iArguments, int iValue) {
		gmThread* thread = iArguments.thread;
		thread->PushInt(iValue);
	}
	void ImplReturn(FunctionArgs& iArguments, bool iValue) {
		gmThread* thread = iArguments.thread;
		const int value = iValue ? 1 : 0;
		thread->PushInt(value);
	}

	void ArgError(FunctionArgs& iArguments, int iIndex, const std::string& iTypeName) {
		gmThread* thread = iArguments.thread;
		::std::stringstream str;
		const int BUFFER_LENGTH = 200;
		char buffer[BUFFER_LENGTH];
		str << "Argument #" << iIndex+1 << "=" << thread->Param(0).AsStringWithType(thread->GetMachine(), buffer, BUFFER_LENGTH) << " to " << thread->GetFunctionObject()->GetDebugName() << " should be " << iTypeName;
		ArgError(iArguments, str.str() );
	}
	void ArgError(FunctionArgs& iArguments, const std::string& iError) {
		ArgWarning(iArguments, iError);
		iArguments.error = true;
	}
	void ArgWarning(FunctionArgs& iArguments, const std::string& iWarning) {
		gmThread* thread = iArguments.thread;
		gmLog &log = thread->GetMachine()->GetLog();
		const std::string error = iWarning.substr(0, GMLOG_CHAINSIZE);
		log.LogEntry("%s", error.c_str());
	}

	std::string ArgString(FunctionArgs& iArguments, int iIndex) {
		gmThread* thread = iArguments.thread;
		Assert( thread->ParamType(iIndex) == GM_STRING, "Need a string" );
		const std::string value = ((gmStringObject*) (thread->Param(iIndex).m_value.m_ref))->GetString();
		return value;
	}
	int ArgInt(FunctionArgs& iArguments, int iIndex) {
		gmThread* thread = iArguments.thread;
		Assert( thread->ParamType(iIndex) == GM_INT, "Need a int" );
		return thread->Param(iIndex).m_value.m_int;
	}
	real ArgReal(FunctionArgs& iArguments, int iIndex) {
		gmThread* thread = iArguments.thread;
		Assert( thread->ParamType(iIndex) == GM_FLOAT, "Need a float" );
		return thread->Param(iIndex).m_value.m_float;
	}
	bool ArgIsString(FunctionArgs& iArguments, int iIndex) {
		gmThread* thread = iArguments.thread;
		return thread->ParamType(iIndex) == GM_STRING;
	}
	bool ArgIsInt(FunctionArgs& iArguments, int iIndex) {
		gmThread* thread = iArguments.thread;
		return thread->ParamType(iIndex) == GM_INT;
	}
	bool ArgIsReal(FunctionArgs& iArguments, int iIndex) {
		gmThread* thread = iArguments.thread;
		return thread->ParamType(iIndex) == GM_FLOAT;
	}
	void ArgErrorString(FunctionArgs& iArguments, int iIndex) {
		gmThread* thread = iArguments.thread;
		ArgError(iArguments, iIndex, "string");
	}
	void ArgErrorInt(FunctionArgs& iArguments, int iIndex) {
		gmThread* thread = iArguments.thread;
		ArgError(iArguments, iIndex, "int");
	}
	void ArgErrorBool(FunctionArgs& iArguments, int iIndex) {
		gmThread* thread = iArguments.thread;
		ArgError(iArguments, iIndex, "bool");
	}
	void ArgErrorReal(FunctionArgs& iArguments, int iIndex) {
		gmThread* thread = iArguments.thread;
		ArgError(iArguments, iIndex, "real");
	}

	void RegisterFunction(const std::string& iFunctionName, ScriptFunction iFunction, const std::string& iDocumentation) {
		GetFunctionList().push_back( FunctionDefinition(iFunctionName, iFunction, iDocumentation) );
	}

	class FunctionWrapper {
	public:
		FunctionWrapper(ScriptFunction iFunction) : mFunction(iFunction) {
		}
		int operator()(gmThread *a_thread ) {
			FunctionArgs args(a_thread);
			mFunction(args);
			if( args.error ) {
				return GM_EXCEPTION;
			}
			return GM_OK;
		}

		ScriptFunction mFunction;
	};

	void PrintCallback(gmMachine* a_machine, const char* a_string) {
		const std::string msg = a_string;
		const std::wstring wmsg(msg.begin(), msg.end());
		ConsoleMessage(CMT_ECHO, wmsg);
	}

	class ScriptSystem : public System {
	public:
		ScriptSystem() : System("Game Monkey v" GM_VERSION), mNameLookup(""), mHasRegistered(false) {
			Assert(!sInstance, "Already has a script instance");
			sInstance = this;

			gmBindMathLib(&mMachine);
			gmBindStringLib(&mMachine);

			gmMachine::s_printCallback = PrintCallback;
		}
		~ScriptSystem() {
			Assert(sInstance, "No script instance exist, bug?");
			sInstance = 0;
		}

		static gmTableObject* GetTable(gmMachine* iMachine, gmTableObject* iBase, const std::string& iName, std::string& oName) {
			gmTableObject* table = iBase;
			gmTableObject* previous = iBase; 

			std::vector<std::string> idents;
			SplitString(".", iName, &idents);
			const std::size_t size = idents.size();
			oName = idents[size-1];

			for(std::size_t index=0; index<size-1; ++index) {
				const std::string tableName = idents[index];

				gmVariable tabVar = previous->Get(iMachine, tableName.c_str()); 
				if( tabVar.m_type == GM_TABLE ) { 
					// Existing table found, use this 
					table = (gmTableObject *)tabVar.m_value.m_ref; 
				}
				else { 
					// Otherwise create a new table afterall 
					table = iMachine->AllocTableObject(); 
				}
				previous->Set(iMachine, tableName.c_str(), gmVariable(GM_TABLE, (gmptr) table)); 
				previous = table;
			}

			return table;
		}

		void registerFunction(FunctionDefinition& iDefinition) {
			//	void gmMachine::RegisterLibrary(gmFunctionEntry * a_functions, int a_numFunctions, const char * a_asTable, bool a_newTable)
			std::string functionName = "";
			gmTableObject* table = GetTable(&mMachine, mMachine.GetGlobals(), iDefinition.name, functionName);
			
			gmFunctionObject* funcObj = mMachine.AllocFunctionObject(FunctionWrapper(iDefinition.function));
			setDocumentationFor(funcObj, iDefinition.documentation);
			table->Set(&mMachine, functionName.c_str(), gmVariable(GM_FUNCTION, (gmptr)funcObj));
		}

		void setDocumentationFor(gmFunctionObject* iFunction, const std::string& iDoc) {
			mDocMap[iFunction] = iDoc;
		}
		std::string getDocumentationFor(gmFunctionObject* iFunction) {
			DocMap::iterator result = mDocMap.find(iFunction);
			if( result == mDocMap.end() ) return "";
			else return result->second;
		}
		typedef std::map<gmFunctionObject*, std::string> DocMap;
		typedef std::pair<gmFunctionObject*, std::string> Docpair;
		DocMap mDocMap;

		void registerEverything() {
			// Register functions
			for(FunctionDefinitionList::iterator definition=GetFunctionList().begin(); definition!=GetFunctionList().end(); ++definition) {
				//gmFunctionEntry entry = { definition->name.c_str(), FunctionWrapper(definition->function) };
				//mMachine.RegisterLibrary( &entry, 1, 0 );
				//addCommand(definition->name, definition->documentation);
				registerFunction(*definition);
			}
			GetFunctionList().clear();

			executeScriptFiles();

			load();
		}

		void execute(const std::string& iStringToExecute, const char* iFileName, gmVariable* iThis) {
			std::string code = iStringToExecute;
		
			{
				std::vector<std::string> commands;
				SplitString(" ", Trim(iStringToExecute), &commands);

				if( !commands.empty() ) {
					StringStringMap::iterator quickResult = mQuickCommands.find(Trim(commands[0]));
					if( quickResult != mQuickCommands.end() ) {
						code = quickResult->second;

						std::size_t length = commands.size();
						for(std::size_t index=1; index<length; ++index) {
							std::ostringstream str;
							str << "%" << index;
							StringReplace(&code, str.str(), commands[index]);
						}
					}
				}
			}
			runCode(code, iFileName, iThis, 0);
		}

		// true == error, false no error
		bool runCode(const std::string& iCode, const char* iFileName, gmVariable* iThis, std::string* oMessage) {
			const int compilationErrors = mMachine.ExecuteString(iCode.c_str(), 0, true, iFileName, iThis);
			if( compilationErrors != 0 ) {
				std::wstringstream str;
				str << compilationErrors << L" compilation error(s)";
				ConsoleMessage(CMT_ERROR, str.str());
				LOG1( compilationErrors << " compilation error(s)" );
			}

			gmLog &log = mMachine.GetLog();
			bool firstError = true;
			bool error = false;
			std::stringstream message;
			for(const char *err = log.GetEntry( firstError ); err; err = log.GetEntry( firstError ) ) {
				std::wstringstream str;
				str << err;
				message << err << "\n";
				LOG1( err );
				error = true;
				ConsoleMessage(CMT_ERROR, str.str());
			}
			log.Reset();
				
			if( error ) {
				std::wstring code(iCode.begin(), iCode.end());
				ConsoleMessage(CMT_ERROR, code);
				ConsoleMessage(CMT_ERROR, L"Error while executing: ");
				LOG1( "code: " << iCode );

				if( oMessage ) {
					*oMessage = message.str();
				}
			}

			return error;
		}

		void step(real iTime) {
			if( !mHasRegistered) {
				registerEverything();
				mHasRegistered = true;
			}
		}

		static ScriptSystem& GetInstance() {
			return *GetInstancePtr();
		}
		static ScriptSystem* GetInstancePtr() {
			Assert(sInstance, "Need an script instance");
			return sInstance;
		}

		const std::wstring getDocumentation(const std::string& iCommand) {
			const std::string cmd = GetCommandName(iCommand);
			const StringStringMap::iterator result = mDocumentation.find(cmd);

			if( result == mDocumentation.end() ) {
				gmTableObject* globals = mMachine.GetGlobals();

				std::vector<std::string> idents;
				SplitString(".", cmd, &idents);
				const std::size_t size = idents.size();
				if( size == 0 ) return L"";
				for(std::size_t index=0; index<size-1; ++index) {
					gmVariable var = globals->Get(&mMachine, idents[index].c_str());
					if( var.m_type == GM_TABLE ) {
						globals = (gmTableObject *)var.m_value.m_ref;
					}
					else {
						return L"";
					}
				}

				const std::string cmd = idents[size-1];

				gmVariable var = globals->Get(&mMachine, cmd.c_str());

				if( var.IsNull() ) return L"";
				if( var.m_type == GM_TABLE ) return L"";

				if( var.m_type == GM_FUNCTION ) {
					gmFunctionObject* obj = (gmFunctionObject*) var.m_value.m_ref;
					const std::string value = getDocumentationFor(obj);
					if( !value.empty() ) {
						const std::wstring wvalue(value.begin(), value.end());
						return wvalue;
					}
				}

				const int BUFFER_LENGTH = 100;
				char buffer[BUFFER_LENGTH];
				const std::string value = var.AsStringWithType(&mMachine, buffer, BUFFER_LENGTH);
				const std::wstring wvalue(value.begin(), value.end());
				return wvalue;
			}
			const std::wstring documentation(result->second.begin(), result->second.end());
			return documentation;
		}
		void findCommands(const std::string& iCommand, std::vector<std::string>* oCommands) {
			const std::string cmd = GetCommandName(iCommand);

			//if( cmd.empty() ) return;

			if( StartsWith(cmd, ":" ) ) {
				mNameLookup.find(cmd, oCommands);
				return;
			}

			const int BUFFER_LENGTH = 100;
			char buffer[BUFFER_LENGTH];

			gmTableObject* globals = mMachine.GetGlobals();

			std::vector<std::string> idents;
			SplitString(".", cmd, &idents);
			if( cmd.empty() || ( cmd.size()!=1 && EndsWith(cmd, ".")) ) {
				idents.push_back("");
			}
			const std::size_t size = idents.size();
			if( size == 0 ) return;
			for(std::size_t index=0; index<size-1; ++index) {
				gmVariable var = globals->Get(&mMachine, idents[index].c_str());
				if( var.m_type == GM_TABLE ) {
					globals = (gmTableObject *)var.m_value.m_ref;
				}
				else {
					return;
				}
			}

			const std::string cmdName = idents[size-1];

			gmTableIterator iter;
			for(gmTableNode* node = globals->GetFirst(iter); !globals->IsNull(iter);  node = globals->GetNext(iter)) {
				const std::string cmdName = idents[size-1];
				const std::string name = node->m_key.AsString(&mMachine, buffer, BUFFER_LENGTH);

				// if this is is a good/valid function name and it isn't stored in the lookup
				if( StartsWith(name, cmdName) ) {
					oCommands->push_back(name);
				}
			}
		}

		void load() {
			loadExtraDocumentation();
			loadQuickCommands();
		}

		void loadExtraDocumentation() {
			const std::string fileName = "scripts/help.xml";
			ReadFile file(fileName);
			TiXmlDocument doc(fileName);
			doc.Parse(file.getBuffer() );
			TiXmlHandle docHandle(&doc);

			const std::string name = "help";
			for(TiXmlElement* cmd=docHandle.FirstChildElement("helps").FirstChildElement(name).ToElement(); cmd; cmd=cmd->NextSiblingElement(name)) {
				const std::string cmdName = GetStringAttribute(cmd, name, "name", fileName);
				const std::string help = GetStringAttribute(cmd, name, "help", fileName);
				addCommand(cmdName, help);
			}
		}
		void loadQuickCommands() {
			const std::string fileName = "scripts/quick.xml";
			ReadFile file(fileName);
			TiXmlDocument doc(fileName);
			doc.Parse(file.getBuffer());
			TiXmlHandle docHandle(&doc);

			const std::string name = "command";
			for(TiXmlElement* cmd=docHandle.FirstChildElement("commands").FirstChildElement(name).ToElement(); cmd; cmd=cmd->NextSiblingElement(name)) {
				const std::string cmdName = GetStringAttribute(cmd, name, "name", fileName);
				const std::string help = GetStringAttribute(cmd, name, "help", fileName);
				std::stringstream stream;
				for(TiXmlNode* node=cmd->FirstChild(); node; node=node->NextSibling()) {
					stream << *node;
				}
				addCommand(cmdName, stream.str(), help);
			}
		}

		gmMachine* getMachine() {
			return &mMachine;
		}

		void addCommand(const std::string& iCommandName, const std::string& iDocumentation) {
			gmTableObject* globals = mMachine.GetGlobals();

			std::vector<std::string> idents;
			SplitString(".", iCommandName, &idents);
			const std::size_t size = idents.size();
			for(std::size_t index=0; index<size-1; ++index) {
				gmVariable var = globals->Get(&mMachine, idents[index].c_str());
				if( var.m_type == GM_TABLE ) {
					globals = (gmTableObject *)var.m_value.m_ref;
				}
				else {
					return; // error
				}
			}

			const std::string cmd = idents[size-1];

			gmVariable var = globals->Get(&mMachine, cmd.c_str());

			if( var.m_type == GM_FUNCTION ) {
				gmFunctionObject* obj = (gmFunctionObject*) var.m_value.m_ref;
				setDocumentationFor(obj, iDocumentation);
			}
		}
		void addCommand(const std::string& iCommandName, const std::string& iSource, const std::string& iDocumentation) {
			const std::string name = std::string(":") + iCommandName;
			mNameLookup.insert(name);
			mDocumentation.insert( StringStringPair(name, iDocumentation) );
			mQuickCommands.insert( StringStringPair(name, iSource) );
		}

		static std::string GetCommandName(const std::string& iCommandLine) {
			std::vector<std::string> commands;
			SplitString(" (),", Trim(iCommandLine), &commands);
			const std::size_t size = commands.size();
			if( size > 0 ) {
				return commands[ size-1 ];
			}
			else {
				return "";
			}
		}

		void executeScriptFiles() {
			std::vector<std::string> files;
			GetFileListing("scripts", &files);
			const std::size_t size = files.size();
			for(std::size_t index=0; index<size; ++index) {
				const std::string& fileName = files[index];
				if( EndsWith(fileName, ".gm") ) {
					ReadFile file( fileName );
					std::size_t size = file.getSize();
					const std::string code(file.getBuffer(), size);
					std::string message;
					if( runCode(code, fileName.c_str(), 0, &message) ) {
						throw ScriptError(message + "\nIn script file " + fileName);
					}
				}
			}
		}

		StringStringMap mDocumentation;
		StringStringMap mQuickCommands;
		StringLookup mNameLookup;

		gmMachine mMachine;
		static ScriptSystem* sInstance;
		bool mHasRegistered;
	};
	ScriptSystem* ScriptSystem::sInstance = 0;

	struct Variable::VariablePimpl {
		Variable::VariablePimpl(gmMachine* iMachine) : mMachine(iMachine), var(iMachine->AllocTableObject()) {
			table = var.GetTableObjectSafe();
		}
		Variable::VariablePimpl(gmMachine* iMachine, gmTableObject* iTable) : mMachine(iMachine), var(iTable) {
			table = iTable;
		}

		~VariablePimpl() {
		}
		gmVariable getVariable(const std::string& iName) {
			return table->Get(mMachine, iName.c_str());
		}
		void setVariable(const std::string& iName, const gmVariable& iVar) {
			table->Set(mMachine, iName.c_str(), iVar);
		}
		gmVariable getThis() {
			return var;
		}
		gmMachine* mMachine;
		gmVariable var;
		gmTableObject* table;
	};

	void Execute(const std::string& iStringToExecute, const std::string& iFileName, Variable* iThis) {
		const char* file = (iFileName.empty()) ? 0 : iFileName.c_str();
		gmVariable* thisVar = 0;
		if( iThis ) {
			thisVar = &(iThis->m->getThis());
		}
		ScriptSystem::GetInstance().execute(iStringToExecute, file, thisVar);
	}
	const std::wstring GetDocumentation(const std::string& iCommand) {
		return ScriptSystem::GetInstance().getDocumentation(iCommand);
	}
	void FindCommands(const std::string& iCommand, std::vector<std::string>* oCommands) {
		ScriptSystem::GetInstance().findCommands(iCommand, oCommands);
	}

	void LoopTable(gmMachine* iMachine, gmTableObject* iTable, TiXmlElement* iElement) {
		const int BUFFER_LENGTH = 200;
		char buffer[BUFFER_LENGTH];
		gmTableIterator iter;
		for(gmTableNode* node = iTable->GetFirst(iter); !iTable->IsNull(iter);  node = iTable->GetNext(iter)) {
			const std::string name = node->m_key.AsString(iMachine, buffer, BUFFER_LENGTH);
			const std::string value = node->m_value.AsStringWithType(iMachine, buffer, BUFFER_LENGTH);
			TiXmlElement* element = (TiXmlElement*) iElement->InsertEndChild( TiXmlElement(name) );
			element->SetAttribute("value", value);
			if( node->m_value.m_type == GM_TABLE ) {
				LoopTable(iMachine, (gmTableObject*)node->m_value.m_value.m_ref, element);
			}
		}
	}
	void DumpMachineScriptFunction(FunctionArgs& iArgs) {
		if( ArgCount(iArgs) != 1 ) {
			ArgReportError(iArgs, "Function needs 1 string file to dump the gamemonkey to");
		}
		ArgVarString(fileName, iArgs, 0);
		gmMachine* machine = ScriptSystem::GetInstance().getMachine();
		gmTableObject* globals = machine->GetGlobals();
		TiXmlDocument doc(fileName);
		TiXmlElement* root = (TiXmlElement*) doc.InsertEndChild( TiXmlElement("machine") );
		LoopTable(machine, globals, root);
		TiXmlPrinter printer;
		doc.Accept(&printer);
		const std::string& file = printer.Str();
		WriteFile(fileName, file.c_str(), file.length());
	}
#pragma warning(pop)

	SCRIPT_FUNCTION(dumpMachine, DumpMachineScriptFunction, "Dumps all the global variables that exist in the script machine");
	LL_SYSTEM(ScriptSystem, 900);

	

	Variable::Variable()  : m(new VariablePimpl(ScriptSystem::GetInstance().getMachine()) ) {
	}
	Variable::Variable(VariablePimpl* pimpl)  : m( pimpl ) {
	}
	Variable::~Variable() {
	}
	std::string Variable::getString(const std::string& iName) {
		return m->getVariable(iName).GetCStringSafe();
	}
	real Variable::getReal(const std::string& iName) {
		return m->getVariable(iName).GetFloatSafe();
	}
	int Variable::getInt(const std::string& iName) {
		return m->getVariable(iName).GetIntSafe();
	}
	bool Variable::getBool(const std::string& iName) {
		return getInt(iName) == 1;
	}
	
	void Variable::setFunction(const std::string& iName, ScriptFunction iFunction, const std::string& iDocumentation) {
		gmFunctionObject* funcObj = ScriptSystem::GetInstance().getMachine()->AllocFunctionObject(FunctionWrapper(iFunction));
		ScriptSystem::GetInstance().setDocumentationFor(funcObj, iDocumentation);
		gmVariable var(funcObj);
		m->setVariable(iName, var);
	}
	void Variable::setString(const std::string& iName, const std::string& iValue) {
		gmVariable var( ScriptSystem::GetInstance().getMachine()->AllocStringObject(iValue.c_str()) );
		m->setVariable(iName, var);
	}
	void Variable::setReal(const std::string& iName, real iValue) {
		const float flt = iValue;
		gmVariable var(flt);
		m->setVariable(iName, var);
	}
	void Variable::setInt(const std::string& iName, int iValue) {
		gmVariable var(iValue);
		m->setVariable(iName, var);
	}
	void Variable::setBool(const std::string& iName, bool iValue) {
		const int value = iValue? 1 : 0;
		setInt(iName, value);
	}

	Call::Call(const std::string& iFunctionName, Variable* iVariable) : This(ScriptSystem::GetInstance().getMachine()->AllocTableObject()) {
		bool result = false;
		if( iVariable ) {
			result = mCall.BeginGlobalFunction(ScriptSystem::GetInstance().getMachine(), iFunctionName.c_str(), iVariable->m->getThis() ); // This
		}
		else {
			result = mCall.BeginGlobalFunction(ScriptSystem::GetInstance().getMachine(), iFunctionName.c_str());
		}
		if(! result ) {
			throw ScriptError("Error calling function");
		}
	}
	Call& Call::arg(const std::string& iValue) {
		mCall.AddParamString(iValue.c_str());
		return *this;
	}
	Call& Call::arg(int iValue) {
		mCall.AddParamInt(iValue);
		return *this;
	}
	Call& Call::arg(real iValue) {
		mCall.AddParamFloat( static_cast<gmfloat>(iValue) );
		return *this;
	}
	Call& Call::arg(bool iValue) {
		int value = 0;
		if( iValue ) {
			value = 1;
		}
		mCall.AddParamInt(value);
		return *this;
	}

	std::string Call::getReturnedString() {
		mCall.End();
		const char* result;
		if(! mCall.GetReturnedString(result) ) {
			throw ScriptError("Function didn't return a string");
		}
		return result;
	}
	int Call::getReturnedInt() {
		mCall.End();
		int result;
		if(! mCall.GetReturnedInt(result) ) {
			throw ScriptError("Function didn't return a int");
		}
		return result;
	}
	real Call::getReturnedReal() {
		mCall.End();
		float result;
		if(! mCall.GetReturnedFloat(result) ) {
			throw ScriptError("Function didn't return a real");
		}
		return result;
	}
	bool Call::getReturnedBool() {
		mCall.End();
		int result;
		if(! mCall.GetReturnedInt(result) ) {
			throw ScriptError("Function didn't return a bool");
		}
		return result==1;
	}
	void Call::getReturnedVoid() {
		mCall.End();
	}
}