/*
	see copyright notice in squirrel.h
*/
#include "sqpcheader.h"
#include "sqcompiler.h"
#include "sqfuncproto.h"
#include "sqstring.h"
#include "sqtable.h"
#include "sqopcodes.h"
#include "sqfuncstate.h"

#ifdef _DEBUG_DUMP
SQInstructionDesc g_InstrDesc[]={
	{_SC("_OP_LINE")},
	{_SC("_OP_LOAD")},
	{_SC("_OP_TAILCALL")},
	{_SC("_OP_CALL")},
	{_SC("_OP_PREPCALL")},
	{_SC("_OP_PREPCALLK")},
	{_SC("_OP_GETK")},
	{_SC("_OP_MOVE")},
	{_SC("_OP_NEWSLOT")},
	{_SC("_OP_DELETE")},
	{_SC("_OP_SET")},
	{_SC("_OP_GET")},
	{_SC("_OP_EQ")},
	{_SC("_OP_NE")},
	{_SC("_OP_ARITH")},
	{_SC("_OP_BITW")},
	{_SC("_OP_RETURN")},
	{_SC("_OP_LOADNULL")},
	{_SC("_OP_LOADNULLS")},
	{_SC("_OP_LOADROOTTABLE")},
	{_SC("_OP_DMOVE")},
	{_SC("_OP_JMP")},
	{_SC("_OP_JNZ")},
	{_SC("_OP_JZ")},
	{_SC("_OP_LOADFREEVAR")},
	{_SC("_OP_VARGC")},
	{_SC("_OP_GETVARGV")},
	{_SC("_OP_NEWTABLE")},
	{_SC("_OP_NEWARRAY")},
	{_SC("_OP_APPENDARRAY")},
	{_SC("_OP_GETPARENT")},
	{_SC("_OP_MINUSEQ")},
	{_SC("_OP_PLUSEQ")},
	{_SC("_OP_INC")},
	{_SC("_OP_INCL")},
	{_SC("_OP_PINC")},
	{_SC("_OP_PINCL")},
	{_SC("_OP_G")},
	{_SC("_OP_GE")},
	{_SC("_OP_L")},
	{_SC("_OP_LE")},
	{_SC("_OP_EXISTS")},
	{_SC("_OP_INSTANCEOF")},
	{_SC("_OP_AND")},
	{_SC("_OP_OR")},
	{_SC("_OP_NEG")},
	{_SC("_OP_NOT")},
	{_SC("_OP_BWNOT")},
	{_SC("_OP_CLOSURE")},
	{_SC("_OP_YIELD")},
	{_SC("_OP_RESUME")},
	{_SC("_OP_FOREACH")},
	{_SC("_OP_DELEGATE")},
	{_SC("_OP_CLONE")},
	{_SC("_OP_TYPEOF")},
	{_SC("_OP_PUSHTRAP")},
	{_SC("_OP_POPTRAP")},
	{_SC("_OP_THROW")},
	{_SC("_OP_CLASS")},
	{_SC("_OP_NEWSLOTA")}
	
};
#endif
void DumpLiteral(SQObjectPtr &o)
{
	switch(type(o)){
		case OT_STRING:	scprintf(_SC("\"%s\""),_stringval(o));break;
		case OT_FLOAT: scprintf(_SC("{%f}"),_float(o));break;
		case OT_INTEGER: scprintf(_SC("{%d}"),_integer(o));break;
	}
}

SQFuncState::SQFuncState(SQSharedState *ss,SQFunctionProto *func,SQFuncState *parent)
{
		_nliterals = 0;
		_literals = SQTable::Create(ss,0);
		_sharedstate = ss;
		_lastline = 0;
		_optimization = true;
		_func = func;
		_parent = parent;
		_stacksize = 0;
		_traps = 0;
		_returnexp = 0;
		_varparams = false;
}

#ifdef _DEBUG_DUMP
void SQFuncState::Dump()
{
	unsigned int n=0,i;
	SQFunctionProto *func=_funcproto(_func);
	scprintf(_SC("SQInstruction sizeof %d\n"),sizeof(SQInstruction));
	scprintf(_SC("SQObject sizeof %d\n"),sizeof(SQObject));
	scprintf(_SC("--------------------------------------------------------------------\n"));
	scprintf(_SC("*****FUNCTION [%s]\n"),type(func->_name)==OT_STRING?_stringval(func->_name):_SC("unknown"));
	scprintf(_SC("-----LITERALS\n"));
	SQObjectPtr refidx,key,val;
	SQInteger idx;
	SQObjectPtrVec templiterals;
	templiterals.resize(_nliterals);
	while((idx=_table(_literals)->Next(refidx,key,val))!=-1) {
		refidx=idx;
		templiterals[_integer(val)]=key;
	}
	for(i=0;i<templiterals.size();i++){
		scprintf(_SC("[%d] "),n);
		DumpLiteral(templiterals[i]);
		scprintf(_SC("\n"));
		n++;
	}
	scprintf(_SC("-----PARAMS\n"));
	if(_varparams)
		scprintf(_SC("<<VARPARAMS>>\n"));
	n=0;
	for(i=0;i<_parameters.size();i++){
		scprintf(_SC("[%d] "),n);
		DumpLiteral(_parameters[i]);
		scprintf(_SC("\n"));
		n++;
	}
	scprintf(_SC("-----LOCALS\n"));
	for(i=0;i<func->_localvarinfos.size();i++){
		SQLocalVarInfo lvi=func->_localvarinfos[i];
		scprintf(_SC("[%d] %s \t%d %d\n"),lvi._pos,_stringval(lvi._name),lvi._start_op,lvi._end_op);
		n++;
	}
	scprintf(_SC("-----LINE INFO\n"));
	for(i=0;i<_lineinfos.size();i++){
		SQLineInfo li=_lineinfos[i];
		scprintf(_SC("op [%d] line [%d] \n"),li._op,li._line);
		n++;
	}
	scprintf(_SC("-----dump\n"));
	n=0;
	for(i=0;i<_instructions.size();i++){
		SQInstruction &inst=_instructions[i];
		if(inst.op==_OP_LOAD || inst.op==_OP_PREPCALLK || inst.op==_OP_GETK ){
			
			scprintf(_SC("[%03d] %15s %d "),n,g_InstrDesc[inst.op].name,inst._arg0);
			if(inst._arg1==0xFFFF)
				scprintf(_SC("null"));
			else {
				int refidx;
				SQObjectPtr val,key,refo;
				while(((refidx=_table(_literals)->Next(refo,key,val))!= -1) && (_integer(val) != inst._arg1)) {
					refo = refidx;	
				}
				DumpLiteral(key);
			}
			scprintf(_SC(" %d %d \n"),inst._arg2,inst._arg3);
		}
		else if(inst.op==_OP_ARITH){
			scprintf(_SC("[%03d] %15s %d %d %d %c\n"),n,g_InstrDesc[inst.op].name,inst._arg0,inst._arg1,inst._arg2,inst._arg3);
		}
		else 
			scprintf(_SC("[%03d] %15s %d %d %d %d\n"),n,g_InstrDesc[inst.op].name,inst._arg0,inst._arg1,inst._arg2,inst._arg3);
		n++;
	}
	scprintf(_SC("-----\n"));
	scprintf(_SC("stack size[%d]\n"),func->_stacksize);
	scprintf(_SC("--------------------------------------------------------------------\n\n"));
}
#endif
int SQFuncState::GetStringConstant(const SQChar *cons)
{
	return GetConstant(SQString::Create(_sharedstate,cons));
}

int SQFuncState::GetNumericConstant(const SQInteger cons)
{
	return GetConstant(cons);
}

int SQFuncState::GetNumericConstant(const SQFloat cons)
{
	return GetConstant(cons);
}

int SQFuncState::GetConstant(SQObjectPtr cons)
{
	int n=0;
	SQObjectPtr val;
	if(!_table(_literals)->Get(cons,val))
	{
		val=_nliterals;
		_table(_literals)->NewSlot(cons,val);
		_nliterals++;
		if(_nliterals>MAX_LITERALS) throw ParserException(_SC("internal compiler error: too many literals"));
	}
	return _integer(val);
}

void SQFuncState::SetIntructionParams(int pos,int arg0,int arg1,int arg2,int arg3)
{
	_instructions[pos]._arg0=*((unsigned int *)&arg0);
	_instructions[pos]._arg1=*((unsigned int *)&arg1);
	_instructions[pos]._arg2=*((unsigned int *)&arg2);
	_instructions[pos]._arg3=*((unsigned int *)&arg3);
}

void SQFuncState::SetIntructionParam(int pos,int arg,int val)
{
	switch(arg){
		case 0:_instructions[pos]._arg0=*((unsigned int *)&val);break;
		case 1:_instructions[pos]._arg1=*((unsigned int *)&val);break;
		case 2:_instructions[pos]._arg2=*((unsigned int *)&val);break;
		case 3:_instructions[pos]._arg3=*((unsigned int *)&val);break;
	};
}

int SQFuncState::AllocStackPos()
{
	int npos=_vlocals.size();
	_vlocals.push_back(SQLocalVarInfo());
	if(_vlocals.size()>((unsigned int)_stacksize)) {
		if(_stacksize>MAX_FUNC_STACKSIZE) throw ParserException(_SC("internal compiler error: too many locals"));
		_stacksize=_vlocals.size();
	}
	return npos;
}

int SQFuncState::PushTarget(int n)
{
	if(n!=-1){
		_targetstack.push_back(n);
		return n;
	}
	n=AllocStackPos();
	_targetstack.push_back(n);
	return n;
}

int SQFuncState::GetUpTarget(int n){
	return _targetstack[((_targetstack.size()-1)-n)];
}

int SQFuncState::TopTarget(){
	return _targetstack.back();
}
int SQFuncState::PopTarget()
{
	int npos=_targetstack.back();
	SQLocalVarInfo t=_vlocals[_targetstack.back()];
	if(type(t._name)==OT_NULL){
		_vlocals.pop_back();
	}
	_targetstack.pop_back();
	return npos;
}

int SQFuncState::GetStackSize()
{
	return _vlocals.size();
}

void SQFuncState::SetStackSize(int n)
{
	int size=_vlocals.size();
	while(size>n){
		size--;
		SQLocalVarInfo lvi=_vlocals.back();
		if(type(lvi._name)!=OT_NULL){
			lvi._end_op=GetCurrentPos();
			_localvarinfos.push_back(lvi);
		}
		_vlocals.pop_back();
	}
}

bool SQFuncState::IsLocal(unsigned int stkpos)
{
	if(stkpos>=_vlocals.size())return false;
	else if(type(_vlocals[stkpos]._name)!=OT_NULL)return true;
	return false;
}

int SQFuncState::PushLocalVariable(const SQObjectPtr &name)
{
	int pos=_vlocals.size();
	SQLocalVarInfo lvi;
	lvi._name=name;
	lvi._start_op=GetCurrentPos()+1;
	lvi._pos=_vlocals.size();
	_vlocals.push_back(lvi);
	if(_vlocals.size()>((unsigned int)_stacksize))_stacksize=_vlocals.size();
	
	return pos;
}

int SQFuncState::GetLocalVariable(const SQObjectPtr &name)
{
	int locals=_vlocals.size();
	while(locals>=1){
		if(type(_vlocals[locals-1]._name)==OT_STRING && _string(_vlocals[locals-1]._name)==_string(name)){
			return locals-1;
		}
		locals--;
	}
	return -1;
}

int SQFuncState::GetOuterVariable(const SQObjectPtr &name)
{
	int outers = _outervalues.size();
	for(int i = 0; i<outers; i++) {
		if(_string(_outervalues[i]._name) == _string(name))
			return i;
	}
	return -1;
}

void SQFuncState::AddOuterValue(const SQObjectPtr &name)
{
	//AddParameter(name);
	int pos=-1;
	if(_parent) pos = _parent->GetLocalVariable(name);
	if(pos!=-1)
		_outervalues.push_back(SQOuterVar(name,SQObjectPtr(SQInteger(pos)),true)); //local
	else
		_outervalues.push_back(SQOuterVar(name,name,false)); //global
}

void SQFuncState::AddParameter(const SQObjectPtr &name)
{
	PushLocalVariable(name);
	_parameters.push_back(name);
}

void SQFuncState::AddLineInfos(int line,bool lineop,bool force)
{
	if(_lastline!=line || force){
		SQLineInfo li;
		li._line=line;li._op=(GetCurrentPos()+1);
		if(lineop)AddInstruction(_OP_LINE,0,line);
		_lineinfos.push_back(li);
		_lastline=line;
	}
}

void SQFuncState::AddInstruction(SQInstruction &i)
{
	int size = _instructions.size();
	if(size > 0 && _optimization){ //simple optimizer
		SQInstruction &pi = _instructions[size-1];//previous instruction
		switch(i.op) {
		case _OP_RETURN:
			if( _parent && i._arg0 != 0xFF && pi.op == _OP_CALL && _returnexp < size-1) {
				pi.op = _OP_TAILCALL;
			}
		break;
		case _OP_GET:
			if( pi.op == _OP_LOAD	&& pi._arg0 == i._arg2 && (!IsLocal(pi._arg0))){
				pi._arg2 = (unsigned char)i._arg1;
				pi.op = _OP_GETK;
				pi._arg0 = i._arg0;
				return;
			}
		break;
		case _OP_PREPCALL:
			if( pi.op == _OP_LOAD && pi._arg0 == i._arg1 && (!IsLocal(pi._arg0))){
				pi.op = _OP_PREPCALLK;
				pi._arg0 = i._arg0;
				pi._arg2 = i._arg2;
				pi._arg3 = i._arg3;
				return;
			}
			break;
		case _OP_APPENDARRAY:
			if(pi.op == _OP_LOAD && pi._arg0 == i._arg1 && (!IsLocal(pi._arg0))){
				pi.op = _OP_APPENDARRAY;
				pi._arg0 = i._arg0;
				pi._arg1 = pi._arg1;
				pi._arg2 = 0xFF;
				pi._arg3 = 0xFF;
				return;
			}
			break;
		case _OP_MOVE: 
			if((pi.op == _OP_GET || pi.op == _OP_ARITH || pi.op == _OP_BITW/*
				|| pi.op == _OP_MUL || pi.op == _OP_DIV || pi.op == _OP_SHIFTL
				|| pi.op == _OP_SHIFTR || pi.op == _OP_BWOR	|| pi.op == _OP_BWXOR
				|| pi.op == _OP_BWAND*/) && (pi._arg0 == i._arg1))
			{
				pi._arg0 = i._arg0;
				_optimization = false;
				return;
			}

			if(pi.op == _OP_MOVE)
			{
				pi.op = _OP_DMOVE;
				pi._arg2 = i._arg0;
				pi._arg3 = (unsigned char)i._arg1;
				return;
			}
			break;

		//case _OP_ADD:case _OP_SUB:case _OP_MUL:case _OP_DIV:
		case _OP_EQ:case _OP_NE:case _OP_G:case _OP_GE:case _OP_L:case _OP_LE:
			if(pi.op == _OP_LOAD && pi._arg0 == i._arg1 && (!IsLocal(pi._arg0) ))
			{
				pi.op = i.op;
				pi._arg0 = i._arg0;
				pi._arg2 = i._arg2;
				pi._arg3 = 0xFF;
				return;
			}
			break;
		case _OP_LOADNULLS:
		case _OP_LOADNULL:
			if((pi.op == _OP_LOADNULL && pi._arg0 == i._arg0-1) ||
				(pi.op == _OP_LOADNULLS && pi._arg0+pi._arg1 == i._arg0)
				) {
				
				pi._arg1 = pi.op == _OP_LOADNULL?2:pi._arg1+1;
				pi.op = _OP_LOADNULLS;
				return;
			}
            break;
		case _OP_LINE:
			if(pi.op == _OP_LINE) {
				_instructions.pop_back();
				_lineinfos.pop_back();
			}
			break;
		}
	}
	_optimization = true;
	_instructions.push_back(i);
}

void SQFuncState::Finalize()
{
	SQFunctionProto *f=_funcproto(_func);
	f->_literals.resize(_nliterals);
	SQObjectPtr refidx,key,val;
	SQInteger idx;
	while((idx=_table(_literals)->Next(refidx,key,val))!=-1) {
		f->_literals[_integer(val)]=key;
		refidx=idx;
	}
	f->_functions.resize(_functions.size());
	f->_functions.copy(_functions);
	f->_parameters.resize(_parameters.size());
	f->_parameters.copy(_parameters);
	f->_outervalues.resize(_outervalues.size());
	f->_outervalues.copy(_outervalues);
	f->_instructions.resize(_instructions.size());
	f->_instructions.copy(_instructions);
	f->_localvarinfos.resize(_localvarinfos.size());
	f->_localvarinfos.copy(_localvarinfos);
	f->_lineinfos.resize(_lineinfos.size());
	f->_lineinfos.copy(_lineinfos);
	f->_varparams = _varparams;
}
