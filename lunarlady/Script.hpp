#ifndef LL_SCRIPT_HPP
#define LL_SCRIPT_HPP

#include <vector>
#include <string>
#include <sstream>
#include <memory>


#pragma warning(push)
#pragma warning(disable:4311)
#pragma warning(disable:4312)
#pragma warning(disable:4244)
#include "gmCall.h"
#pragma warning(pop)

#include "boost/function.hpp"
#include "boost/utility.hpp"
#include "boost/smart_ptr.hpp"
#include "lunarlady/Types.hpp"

namespace lunarlady {
	class FunctionArgs;
	typedef boost::function<void (FunctionArgs& iArgs)> ScriptFunction;

	class Variable {
	public:
		struct VariablePimpl;
		Variable();
		Variable(VariablePimpl* iImplementation);
		~Variable();
		std::string getString(const std::string& iName);
		real getReal(const std::string& iName);
		int getInt(const std::string& iName);
		bool getBool(const std::string& iName);
		
		void setString(const std::string& iName, const std::string& iValue);
		void setReal(const std::string& iName, real iValue);
		void setInt(const std::string& iName, int iValue);
		void setBool(const std::string& iName, bool iValue);
		void setFunction(const std::string& iName, ScriptFunction iFunction, const std::string& iDocumentation);
	public:
		boost::shared_ptr<VariablePimpl> m;
	};

	class Call : boost::noncopyable {
	public:
		Call(const std::string& iFunctionName, Variable* iVariable=0);
		Call& arg(const std::string& iValue);
		Call& arg(int iValue);
		Call& arg(real iValue);
		Call& arg(bool iValue);

		std::string getReturnedString();
		int getReturnedInt();
		real getReturnedReal();
		bool getReturnedBool();
		void getReturnedVoid();
	private:
		gmCall mCall;
		gmVariable This;
	};

	int ArgCount(FunctionArgs& iArguments);
	
	// consider using macros below instead
	std::string ArgString(FunctionArgs& iArguments, int iIndex);
	real ArgReal(FunctionArgs& iArguments, int iIndex);
	int ArgInt(FunctionArgs& iArguments, int iIndex);
	bool ArgIsString(FunctionArgs& iArguments, int iIndex);
	bool ArgIsReal(FunctionArgs& iArguments, int iIndex);
	bool ArgIsInt(FunctionArgs& iArguments, int iIndex);
	void ArgErrorString(FunctionArgs& iArguments, int iIndex);
	void ArgErrorReal(FunctionArgs& iArguments, int iIndex);
	void ArgErrorInt(FunctionArgs& iArguments, int iIndex);
	void ArgErrorBool(FunctionArgs& iArguments, int iIndex);
	
	void ArgError(FunctionArgs& iArguments, const std::string& iError);
	void ArgWarning(FunctionArgs& iArguments, const std::string& iWarning);

	void ImplReturn(FunctionArgs& iArguments, real iValue);
	void ImplReturn(FunctionArgs& iArguments, const std::string& iValue);
	void ImplReturn(FunctionArgs& iArguments, int iValue);
	void ImplReturn(FunctionArgs& iArguments, bool iValue);
	
	#define Return(iArguments, value)	do { ImplReturn(iArguments, value); return; } while(false)

	#define ArgReportError(arg, message) do { ::std::stringstream local_str; local_str << message; ArgError(arg, local_str.str()); return; }while(false)
	#define ArgReportWarning(arg, message) do { ::std::stringstream local_str; local_str << message; ArgWarning(arg, local_str.str()); }while(false)
	
	#define ArgVarString(var, args, index) if( !ArgIsString(args, index) ) { ArgErrorString(args, index); return; } const ::std::string var = ArgString(args, index)
	#define ArgVarInt(var, args, index) if( !ArgIsInt(args, index) ) { ArgErrorInt(args, index); return; } const int var = ArgInt(args, index)
	#define ArgVarReal(var, args, index) if( !ArgIsReal(args, index) ) { ArgErrorReal(args, index); return; } const real var = ArgReal(args, index)
	#define ArgVarBool(var, args, index) if( !ArgIsInt(args, index) ) { ArgErrorBool(args, index); return; } const bool var = ArgInt(args, index)==1

	
	
	void RegisterFunction(const std::string& iFunctionName, ScriptFunction iFunction, const std::string& iDocumentation);

	void Execute(const std::string& iStringToExecute, const std::string& iFileName, Variable* iThis=0);

	const std::wstring GetDocumentation(const std::string& iCommand);
	void FindCommands(const std::string& iCommand, std::vector<std::string>* oCommands);
}

#define SCRIPT_FUNCTION(name, function, doc) struct name##_registrator_class { \
		name##_registrator_class() { \
		::lunarlady::RegisterFunction(#name, function, doc); \
		} \
	} g_##name##_variable

#endif