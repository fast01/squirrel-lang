/*	see copyright notice in squirrel.h */
#ifndef _SQOBJECT_H_
#define _SQOBJECT_H_

#include "squtils.h"

#define SQ_CLOSURESTREAM_HEAD (('S'<<24)|('Q'<<16)|('I'<<8)|('R'))
#define SQ_CLOSURESTREAM_PART (('P'<<24)|('A'<<16)|('R'<<8)|('T'))
#define SQ_CLOSURESTREAM_TAIL (('T'<<24)|('A'<<16)|('I'<<8)|('L'))

#define SQ_TRY try
#define SQ_CATCH(type, ex) catch(type &ex)

struct SQSharedState;

enum SQMetaMethod{
	MT_ADD=0,
	MT_SUB=1,
	MT_MUL=2,
	MT_DIV=3,
	MT_UNM=4,
	MT_MODULO=5,
	MT_SET=6,
	MT_GET=7,
	MT_TYPEOF=8,
	MT_NEXTI=9,
	MT_CMP=10,
	MT_CALL=11,
	MT_CLONED=12,
	MT_NEWSLOT=13,
	MT_DELSLOT=14,
};

#define MM_ADD		_SC("_add")
#define MM_SUB		_SC("_sub")
#define MM_MUL		_SC("_mul")
#define MM_DIV		_SC("_div")
#define MM_UNM		_SC("_unm")
#define MM_MODULO	_SC("_modulo")
#define MM_SET		_SC("_set")
#define MM_GET		_SC("_get")
#define MM_TYPEOF	_SC("_typeof")
#define MM_NEXTI	_SC("_nexti")
#define MM_CMP		_SC("_cmp")
#define MM_CALL		_SC("_call")
#define MM_CLONED	_SC("_cloned")
#define MM_NEWSLOT	_SC("_newslot")
#define MM_DELSLOT	_SC("_delslot")

#define MINPOWER2 4

struct SQRefCounted
{
	unsigned int _uiRef;
	virtual void Release()=0;
};

struct SQObjectPtr;

#define __AddRef(type,unval) if(ISREFCOUNTED(type))	\
		{ \
			unval.pRefCounted->_uiRef++; \
		}  

#define __Release(type,unval) if(ISREFCOUNTED(type) && ((--unval.pRefCounted->_uiRef)<=0))	\
		{	\
			unval.pRefCounted->Release();	\
		}	

#define type(obj) ((obj)._type)
#define raw_type(obj) _RAW_TYPE((obj)._type)

#define _integer(obj) (obj)._unVal.nInteger
#define _float(obj) (obj)._unVal.fFloat
#define _string(obj) (obj)._unVal.pString
#define _table(obj) (obj)._unVal.pTable
#define _array(obj) (obj)._unVal.pArray
#define _closure(obj) (obj)._unVal.pClosure
#define _generator(obj) (obj)._unVal.pGenerator
#define _nativeclosure(obj) (obj)._unVal.pNativeClosure
#define _userdata(obj) (obj)._unVal.pUserData
#define _userpointer(obj) (obj)._unVal.pUserPointer
#define _thread(obj) (obj)._unVal.pThread
#define _funcproto(obj) (obj)._unVal.pFunctionProto
#define _rawval(obj) (obj)._unVal.pRefCounted

#define _stringval(obj) (obj)._unVal.pString->_val
#define _userdataval(obj) (obj)._unVal.pUserData->_val

#define tofloat(num) ((type(num)==OT_INTEGER)?(SQFloat)_integer(num):_float(num))
#define tointeger(num) ((type(num)==OT_FLOAT)?(SQInteger)_float(num):_integer(num))
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
struct SQObjectPtr : public SQObject
{
	SQObjectPtr()
	{
		_type=OT_NULL;
		_unVal.pUserPointer=NULL;
	}
	SQObjectPtr(const SQObjectPtr &o)
	{
		_type=o._type;
		_unVal=o._unVal;
		__AddRef(_type,_unVal);
	}
	SQObjectPtr(const SQObject &o)
	{
		_type=o._type;
		_unVal=o._unVal;
		__AddRef(_type,_unVal);
	}
	SQObjectPtr(SQTable *pTable)
	{
		_type=OT_TABLE;
		_unVal.pTable=pTable;
		assert(_unVal.pTable);
		__AddRef(_type,_unVal);
	}
	SQObjectPtr(SQArray *pArray)
	{
		_type=OT_ARRAY;
		_unVal.pArray=pArray;
		assert(_unVal.pArray);
		__AddRef(_type,_unVal);
	}
	SQObjectPtr(SQClosure *pClosure)
	{
		_type=OT_CLOSURE;
		_unVal.pClosure=pClosure;
		assert(_unVal.pClosure);
		__AddRef(_type,_unVal);
	}
	SQObjectPtr(SQGenerator *pGenerator)
	{
		_type=OT_GENERATOR;
		_unVal.pGenerator=pGenerator;
		assert(_unVal.pGenerator);
		__AddRef(_type,_unVal);
	}
	SQObjectPtr(SQNativeClosure *pNativeClosure)
	{
		_type=OT_NATIVECLOSURE;
		_unVal.pNativeClosure=pNativeClosure;
		assert(_unVal.pNativeClosure);
		__AddRef(_type,_unVal);
	}
	SQObjectPtr(SQString *pString)
	{
		_type=OT_STRING;
		_unVal.pString=pString;
		assert(_unVal.pString);
		__AddRef(_type,_unVal);
	}
	SQObjectPtr(SQUserData *pUserData)
	{
		_type=OT_USERDATA;
		_unVal.pUserData=pUserData;
		assert(_unVal.pUserData);
		__AddRef(_type,_unVal);
	}
	SQObjectPtr(SQVM *pThread)
	{
		_type=OT_THREAD;
		_unVal.pThread=pThread;
		assert(_unVal.pThread);
		__AddRef(_type,_unVal);
	}
	SQObjectPtr(SQFunctionProto *pFunctionProto)
	{
		_type=OT_FUNCPROTO;
		_unVal.pFunctionProto=pFunctionProto;
		assert(_unVal.pFunctionProto);
		__AddRef(_type,_unVal);
	}
	SQObjectPtr(SQInteger nInteger)
	{
		_type=OT_INTEGER;
		_unVal.nInteger=nInteger;
	}
	SQObjectPtr(SQFloat fFloat)
	{
		_type=OT_FLOAT;
		_unVal.fFloat=fFloat;
	}
	SQObjectPtr(SQUserPointer pUserPointer)
	{
		_type=OT_USERPOINTER;
		_unVal.pUserPointer=pUserPointer;
	}
	~SQObjectPtr()
	{
		__Release(_type,_unVal);
	}
	inline SQObjectPtr& operator=(const SQObjectPtr& obj)
	{ 
		SQObjectType tOldType;
		SQObjectValue unOldVal;
		tOldType=_type;
		unOldVal=_unVal;
		_unVal = obj._unVal;
		_type = obj._type;
		__AddRef(_type,_unVal);
		__Release(tOldType,unOldVal);
		return *this;
	}
	inline SQObjectPtr& operator=(const SQObject& obj)
	{ 
		SQObjectType tOldType;
		SQObjectValue unOldVal;
		tOldType=_type;
		unOldVal=_unVal;
		_unVal = obj._unVal;
		_type = obj._type;
		__AddRef(_type,_unVal);
		__Release(tOldType,unOldVal);
		return *this;
	}
	private:
		SQObjectPtr(const SQChar *){} //safety
};
/////////////////////////////////////////////////////////////////////////////////////
struct SQException
{
	SQException(SQSharedState *ss,const SQChar *str);
	SQException(const SQObjectPtr &desc);
	SQException(const SQException &b);
	SQObjectPtr _description;
};

#ifndef NO_GARBAGE_COLLECTOR
#define MARK_FLAG 0x80000000
struct SQCollectable : public SQRefCounted {
	SQCollectable *_next;
	SQCollectable *_prev;
	SQSharedState *_sharedstate;
	virtual void Release()=0;
	virtual void Mark(SQCollectable **chain)=0;
	void UnMark();
	virtual void Finalize()=0;
	static void AddToChain(SQCollectable **chain,SQCollectable *c);
	static void RemoveFromChain(SQCollectable **chain,SQCollectable *c);
};


#define ADD_TO_CHAIN(chain,obj) AddToChain(chain,obj)
#define REMOVE_FROM_CHAIN(chain,obj) {if(!(_uiRef&MARK_FLAG))RemoveFromChain(chain,obj);}
#define CHAINABLE_OBJ SQCollectable
#define INIT_CHAIN() {_next=NULL;_prev=NULL;_sharedstate=ss;}
#else

#define ADD_TO_CHAIN(chain,obj) ((void)0)
#define REMOVE_FROM_CHAIN(chain,obj) ((void)0)
#define CHAINABLE_OBJ SQRefCounted
#define INIT_CHAIN() ((void)0)
#endif

unsigned int TranslateIndex(const SQObjectPtr &idx);
typedef sqvector<SQObjectPtr> SQObjectPtrVec;
typedef sqvector<int> SQIntVec;

#endif //_SQOBJECT_H_
