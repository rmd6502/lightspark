/**************************************************************************
    Lightspark, a free flash player implementation

    Copyright (C) 2009-2011  Alessandro Pignotti (a.pignotti@sssup.it)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include <pcre.h>

#include "ASString.h"
#include "compat.h"
#include "argconv.h"
#include "parsing/amf3_generator.h"

using namespace std;
using namespace lightspark;

REGISTER_CLASS_NAME2(ASString, "String", "");

ASString::ASString()
{
	type=T_STRING;
}

ASString::ASString(const string& s): data(s)
{
	type=T_STRING;
}

ASString::ASString(const tiny_string& s) : data(s)
{
	type=T_STRING;
}

ASString::ASString(const Glib::ustring& s) : data(s)
{
	type=T_STRING;
}

ASString::ASString(const char* s) : data(s, /*copy:*/true)
{
	type=T_STRING;
}

ASString::ASString(const char* s, uint32_t len)
{
	data = std::string(s,len);
	type=T_STRING;
}

ASFUNCTIONBODY(ASString,_constructor)
{
	ASString* th=static_cast<ASString*>(obj);
	if(args && argslen==1)
		th->data=args[0]->toString().raw_buf();
	return NULL;
}

ASFUNCTIONBODY(ASString,_getLength)
{
	return abstract_i(obj->toString().numChars());
}

void ASString::sinit(Class_base* c)
{
	c->setSuper(Class<ASObject>::getRef());
	c->setConstructor(Class<IFunction>::getFunction(_constructor));
	c->setDeclaredMethodByQName("split",AS3,Class<IFunction>::getFunction(split),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("substr",AS3,Class<IFunction>::getFunction(substr),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("substring",AS3,Class<IFunction>::getFunction(substring),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("replace",AS3,Class<IFunction>::getFunction(replace),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("concat",AS3,Class<IFunction>::getFunction(concat),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("match",AS3,Class<IFunction>::getFunction(match),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("search",AS3,Class<IFunction>::getFunction(search),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("indexOf",AS3,Class<IFunction>::getFunction(indexOf),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("lastIndexOf",AS3,Class<IFunction>::getFunction(lastIndexOf),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("charCodeAt",AS3,Class<IFunction>::getFunction(charCodeAt),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("charAt",AS3,Class<IFunction>::getFunction(charAt),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("slice",AS3,Class<IFunction>::getFunction(slice),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("toLocaleLowerCase",AS3,Class<IFunction>::getFunction(toLowerCase),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("toLocaleUpperCase",AS3,Class<IFunction>::getFunction(toUpperCase),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("toLowerCase",AS3,Class<IFunction>::getFunction(toLowerCase),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("toUpperCase",AS3,Class<IFunction>::getFunction(toUpperCase),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("fromCharCode",AS3,Class<IFunction>::getFunction(fromCharCode),NORMAL_METHOD,false);
	c->setDeclaredMethodByQName("length","",Class<IFunction>::getFunction(_getLength),GETTER_METHOD,true);
	c->setDeclaredMethodByQName("toString",AS3,Class<IFunction>::getFunction(_toString),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("valueOf",AS3,Class<IFunction>::getFunction(_toString),NORMAL_METHOD,true);

	c->prototype->setVariableByQName("split","",Class<IFunction>::getFunction(split),DYNAMIC_TRAIT);
	c->prototype->setVariableByQName("substring","",Class<IFunction>::getFunction(substring),DYNAMIC_TRAIT);
	c->prototype->setVariableByQName("replace","",Class<IFunction>::getFunction(replace),DYNAMIC_TRAIT);
	c->prototype->setVariableByQName("concat","",Class<IFunction>::getFunction(concat),DYNAMIC_TRAIT);
	c->prototype->setVariableByQName("match","",Class<IFunction>::getFunction(match),DYNAMIC_TRAIT);
	c->prototype->setVariableByQName("search","",Class<IFunction>::getFunction(search),DYNAMIC_TRAIT);
	c->prototype->setVariableByQName("indexOf","",Class<IFunction>::getFunction(indexOf),DYNAMIC_TRAIT);
	c->prototype->setVariableByQName("lastIndexOf","",Class<IFunction>::getFunction(lastIndexOf),DYNAMIC_TRAIT);
	c->prototype->setVariableByQName("charCodeAt","",Class<IFunction>::getFunction(charCodeAt),DYNAMIC_TRAIT);
	c->prototype->setVariableByQName("charAt","",Class<IFunction>::getFunction(charAt),DYNAMIC_TRAIT);
	c->prototype->setVariableByQName("slice","",Class<IFunction>::getFunction(slice),DYNAMIC_TRAIT);
	c->prototype->setVariableByQName("toLocaleLowerCase","",Class<IFunction>::getFunction(toLowerCase),DYNAMIC_TRAIT);
	c->prototype->setVariableByQName("toLocaleUpperCase","",Class<IFunction>::getFunction(toUpperCase),DYNAMIC_TRAIT);
	c->prototype->setVariableByQName("toLowerCase","",Class<IFunction>::getFunction(toLowerCase),DYNAMIC_TRAIT);
	c->prototype->setVariableByQName("toUpperCase","",Class<IFunction>::getFunction(toUpperCase),DYNAMIC_TRAIT);
	c->prototype->setVariableByQName("toString","",Class<IFunction>::getFunction(_toString),DYNAMIC_TRAIT);
	c->prototype->setVariableByQName("valueOf","",Class<IFunction>::getFunction(_toString),DYNAMIC_TRAIT);
}

void ASString::buildTraits(ASObject* o)
{
}

ASFUNCTIONBODY(ASString,search)
{
	tiny_string data = obj->toString();
	int ret = -1;
	if(argslen == 0 || args[0]->getObjectType() == T_UNDEFINED)
		return abstract_i(-1);

	int options=PCRE_UTF8;
	tiny_string restr;
	if(args[0]->getClass() && args[0]->getClass()==Class<RegExp>::getClass())
	{
		RegExp* re=static_cast<RegExp*>(args[0]);
		restr = re->source;
		if(re->ignoreCase)
			options|=PCRE_CASELESS;
		if(re->extended)
			options|=PCRE_EXTENDED;
		if(re->multiline)
			options|=PCRE_MULTILINE;
		if(re->dotall)
			options|=PCRE_DOTALL;
	}
	else
	{
		restr = args[0]->toString();
	}

	const char* error;
	int errorOffset;
	pcre* pcreRE=pcre_compile(restr.raw_buf(), options, &error, &errorOffset,NULL);
	if(error)
		return abstract_i(ret);
	//Verify that 30 for ovector is ok, it must be at least (captGroups+1)*3
	int capturingGroups;
	int infoOk=pcre_fullinfo(pcreRE, NULL, PCRE_INFO_CAPTURECOUNT, &capturingGroups);
	if(infoOk!=0)
	{
		pcre_free(pcreRE);
		return abstract_i(ret);
	}
	assert_and_throw(capturingGroups<10);
	int ovector[30];
	int offset=0;
	//Global is not used in search
	int rc=pcre_exec(pcreRE, NULL, data.raw_buf(), data.numBytes(), offset, 0, ovector, 30);
	if(rc<0)
	{
		//No matches or error
		pcre_free(pcreRE);
		return abstract_i(ret);
	}
	ret=ovector[0];
	// pcre_exec returns byte position, so we have to convert it to character position 
	tiny_string tmp = data.substr_bytes(0, ret);
	ret = tmp.numChars();
	return abstract_i(ret);
}

ASFUNCTIONBODY(ASString,match)
{
	tiny_string data = obj->toString();
	if(argslen == 0 || args[0]->getObjectType()==T_NULL || args[0]->getObjectType()==T_UNDEFINED)
		return new Null;
	ASObject* ret=NULL;
	RegExp* re;

	if(args[0]->getClass() && args[0]->getClass()==Class<RegExp>::getClass())
	{
		re = args[0]->as<RegExp>();
		re->incRef();
	}
	else
	{
		re = Class<RegExp>::getInstanceS(args[0]->toString());
	}

	if (re->global)
	{
		Array *resarr = Class<Array>::getInstanceS();
		int prevLastIndex = 0;
		re->lastIndex = 0;

		while (true)
		{
			ASObject *match = re->match(data);

			if (match->is<Null>())
				break;

			if (re->lastIndex == prevLastIndex)
				// ECMA-262 Section 15.5.4.10 says
				// that we should increase
				// re->lastIndex by one and repeat,
				// but this is closer to the observed
				// behaviour.
				break;

			prevLastIndex = re->lastIndex;

			assert(match->is<Array>());
			resarr->push(match->as<Array>()->at(0));
		}

		if (resarr->size() == 0)
		{
			resarr->decRef();
			ret = new Null;
		}
		else
			ret = resarr;
	}
	else
	{
		ret = re->match(data);
	}

	re->decRef();

	return ret;
}

ASFUNCTIONBODY(ASString,_toString)
{
	if(Class<ASString>::getClass()->prototype == obj)
		return Class<ASString>::getInstanceS("");
	if(!obj->is<ASString>())
		throw Class<TypeError>::getInstanceS("String.toString is not generic");
	assert_and_throw(argslen==0);

	//As ASStrings are immutable, we can just return ourself
	obj->incRef();
	return obj;
}

ASFUNCTIONBODY(ASString,split)
{
	tiny_string data = obj->toString();
	Array* ret=Class<Array>::getInstanceS();
	ASObject* delimiter=args[0];
	if(argslen == 0 || delimiter->getObjectType()==T_UNDEFINED)
	{
		ret->push(Class<ASString>::getInstanceS(data));
		return ret;
	}

	if(args[0]->getClass() && args[0]->getClass()==Class<RegExp>::getClass())
	{
		RegExp* re=static_cast<RegExp*>(args[0]);

		if(re->source.empty())
		{
			//the RegExp is empty, so split every character
			for(auto i=data.begin();i!=data.end();++i)
				ret->push( Class<ASString>::getInstanceS( tiny_string::fromChar(*i) ) );
			return ret;
		}

		const char* error;
		int offset;
		int options=PCRE_UTF8;
		if(re->ignoreCase)
			options|=PCRE_CASELESS;
		if(re->extended)
			options|=PCRE_EXTENDED;
		if(re->multiline)
			options|=PCRE_MULTILINE;
		if(re->dotall)
			options|=PCRE_DOTALL;
		pcre* pcreRE=pcre_compile(re->source.raw_buf(), options, &error, &offset,NULL);
		if(error)
			return ret;
		//Verify that 30 for ovector is ok, it must be at least (captGroups+1)*3
		int capturingGroups;
		int infoOk=pcre_fullinfo(pcreRE, NULL, PCRE_INFO_CAPTURECOUNT, &capturingGroups);
		if(infoOk!=0)
		{
			pcre_free(pcreRE);
			return ret;
		}
		assert_and_throw(capturingGroups<10);
		int ovector[30];
		offset=0;
		unsigned int end;
		uint32_t lastMatch = 0;
		do
		{
			//offset is a byte offset that must point to the beginning of an utf8 character
			int rc=pcre_exec(pcreRE, NULL, data.raw_buf(), data.numBytes(), offset, 0, ovector, 30);
			end=ovector[0];
			if(rc<0)
				break;
			if(ovector[0] == ovector[1])
			{ //matched the empty string
				offset++;
				continue;
			}
			//Extract string from last match until the beginning of the current match
			ASString* s=Class<ASString>::getInstanceS(data.substr_bytes(lastMatch,end-lastMatch));
			ret->push(s);
			lastMatch=offset=ovector[1];

			//Insert capturing groups
			for(int i=1;i<rc;i++)
			{
				//use string interface through raw(), because we index on bytes, not on UTF-8 characters
				ASString* s=Class<ASString>::getInstanceS(data.substr_bytes(ovector[i*2],ovector[i*2+1]-ovector[i*2]));
				ret->push(s);
			}
		}
		while(end<data.numBytes());
		if(lastMatch != data.numBytes()+1)
		{
			ASString* s=Class<ASString>::getInstanceS(data.substr_bytes(lastMatch,data.numBytes()-lastMatch));
			ret->push(s);
		}
		pcre_free(pcreRE);
	}
	else
	{
		const tiny_string& del=args[0]->toString();
		if(del.empty())
		{
			//the string is empty, so split every character
			for(auto i=data.begin();i!=data.end();++i)
				ret->push( Class<ASString>::getInstanceS( tiny_string::fromChar(*i) ) );
			return ret;
		}
		unsigned int start=0;
		do
		{
			int match=data.find(del,start);
			if(del.empty())
				match++;
			if(match==-1)
				match=data.numChars();
			ASString* s=Class<ASString>::getInstanceS(data.substr(start,(match-start)));
			ret->push(s);
			start=match+del.numChars();
		}
		while(start<data.numChars());
	}

	return ret;
}

ASFUNCTIONBODY(ASString,substr)
{
	tiny_string data = obj->toString();
	int start=0;
	if(argslen>=1)
		start=args[0]->toInt();
	if(start<0) {
		start=data.numChars()+start;
		if(start<0)
			start=0;
	}
	if(start>(int)data.numChars())
		start=data.numChars();

	int len=0x7fffffff;
	if(argslen==2)
		len=args[1]->toInt();

	return Class<ASString>::getInstanceS(data.substr(start,len));
}

ASFUNCTIONBODY(ASString,substring)
{
	tiny_string data = obj->toString();
	int start=0;
	if (argslen>=1)
		start=args[0]->toInt();
	if(start<0)
		start=0;
	if(start>(int)data.numChars())
		start=data.numChars();

	int end=0x7fffffff;
	if(argslen>=2)
		end=args[1]->toInt();
	if(end<0)
		end=0;
	if(end>(int)data.numChars())
		end=data.numChars();

	if(start>end) {
		int tmp=start;
		start=end;
		end=tmp;
	}

	return Class<ASString>::getInstanceS(data.substr(start,end-start));
}

tiny_string ASString::toString_priv() const
{
	return data;
}

/* Note that toNumber() is not virtual.
 * ASString::toNumber is directly called by ASObject::toNumber
 */
double ASString::toNumber() const
{
	assert_and_throw(implEnable);

	/* TODO: data holds a utf8-character sequence, not ascii! */
	const char *s=data.raw_buf();
	char *end=NULL;
	while(g_ascii_isspace(*s))
		s++;
	double val=g_ascii_strtod(s, &end);

	// strtod converts case-insensitive "inf" and "infinity" to
	// inf, flash only accepts case-sensitive "Infinity".
	if(std::isinf(val)) {
		const char *tmp=s;
		while(g_ascii_isspace(*tmp))
			tmp++;
		if(*tmp=='+' || *tmp=='-')
			tmp++;
		if(strncasecmp(tmp, "inf", 3)==0 && strcmp(tmp, "Infinity")!=0)
			return numeric_limits<double>::quiet_NaN();
	}

	// Fail if there is any rubbish after the converted number
	while(*end) {
		if(!g_ascii_isspace(*end))
			return numeric_limits<double>::quiet_NaN();
		end++;
	}

	return val;
}

int32_t ASString::toInt()
{
	assert_and_throw(implEnable);
	const char* cur=data.raw_buf();

	errno=0;
	char *end;
	int64_t val=g_ascii_strtoll(cur, &end, 0);

	if(errno==ERANGE || end > cur)
		return static_cast<int32_t>(val);
	else
		return 0;
}

uint32_t ASString::toUInt()
{
	assert_and_throw(implEnable);
	return static_cast<uint32_t>(toInt());
}

bool ASString::isEqual(ASObject* r)
{
	assert_and_throw(implEnable);
	switch(r->getObjectType())
	{
		case T_STRING:
		{
			const ASString* s=static_cast<const ASString*>(r);
			return s->data==data;
		}
		case T_INTEGER:
		case T_UINTEGER:
		case T_NUMBER:
		case T_BOOLEAN:
			return toNumber()==r->toNumber();
		case T_NULL:
		case T_UNDEFINED:
			return false;
		default:
			return r->isEqual(this);
	}
}

TRISTATE ASString::isLess(ASObject* r)
{
	//ECMA-262 11.8.5 algorithm
	assert_and_throw(implEnable);
	_R<ASObject> rprim=r->toPrimitive();
	if(getObjectType()==T_STRING && rprim->getObjectType()==T_STRING)
	{
		ASString* rstr=static_cast<ASString*>(rprim.getPtr());
		return (data<rstr->data)?TTRUE:TFALSE;
	}
	number_t a=toNumber();
	number_t b=rprim->toNumber();
	if(std::isnan(a) || std::isnan(b))
		return TUNDEFINED;
	return (a<b)?TTRUE:TFALSE;
}

void ASString::serialize(ByteArray* out, std::map<tiny_string, uint32_t>& stringMap,
				std::map<const ASObject*, uint32_t>& objMap,
				std::map<const Class_base*, uint32_t>& traitsMap) const
{
	out->writeByte(string_marker);
	out->writeStringVR(stringMap, data);
}

ASFUNCTIONBODY(ASString,slice)
{
	tiny_string data = obj->toString();
	int startIndex=0;
	if(argslen>=1)
		startIndex=args[0]->toInt();
	if(startIndex<0) {
		startIndex=data.numChars()+startIndex;
		if(startIndex<0)
			startIndex=0;
	}
	if(startIndex>(int)data.numChars())
		startIndex=data.numChars();

	int endIndex=0x7fffffff;
	if(argslen>=2)
		endIndex=args[1]->toInt();
	if(endIndex<0) {
		endIndex=data.numChars()+endIndex;
		if(endIndex<0)
			endIndex=0;
	}
	if(endIndex>(int)data.numChars())
		endIndex=data.numChars();

	if(endIndex<=startIndex)
		return Class<ASString>::getInstanceS("");
	else
		return Class<ASString>::getInstanceS(data.substr(startIndex,endIndex-startIndex));
}

ASFUNCTIONBODY(ASString,charAt)
{
	tiny_string data = obj->toString();
	int index;
	ARG_UNPACK (index, 0);

	int maxIndex=data.numChars();
	if(index<0 || index>=maxIndex)
		return Class<ASString>::getInstanceS();
	return Class<ASString>::getInstanceS( tiny_string::fromChar(data.charAt(index)) );
}

ASFUNCTIONBODY(ASString,charCodeAt)
{
	tiny_string data = obj->toString();
	unsigned int index;
	ARG_UNPACK (index, 0);
	if(index<data.numChars())
	{
		//Character codes are expected to be positive
		return abstract_i(data.charAt(index));
	}
	else
		return abstract_d(Number::NaN);
}

ASFUNCTIONBODY(ASString,indexOf)
{
	if (argslen == 0)
		return abstract_i(-1);
	tiny_string data = obj->toString();
	tiny_string arg0=args[0]->toString();
	int startIndex=0;
	if(argslen>1)
		startIndex=args[1]->toInt();

	size_t pos = data.find(arg0.raw_buf(), startIndex);
	if(pos == data.npos)
		return abstract_i(-1);
	else
		return abstract_i(pos);
}

ASFUNCTIONBODY(ASString,lastIndexOf)
{
	assert_and_throw(argslen==1 || argslen==2);
	tiny_string data = obj->toString();
	tiny_string val=args[0]->toString();
	size_t startIndex=data.npos;
	if(argslen > 1 && args[1]->getObjectType() != T_UNDEFINED && !std::isnan(args[1]->toNumber()))
	{
		int32_t i = args[1]->toInt();
		if(i<0)
			return abstract_i(-1);
		startIndex = i;
	}

	size_t pos=data.rfind(val.raw_buf(), startIndex);
	if(pos==data.npos)
		return abstract_i(-1);
	else
		return abstract_i(pos);
}

ASFUNCTIONBODY(ASString,toLowerCase)
{
	tiny_string data = obj->toString();
	return Class<ASString>::getInstanceS(data.lowercase());
}

ASFUNCTIONBODY(ASString,toUpperCase)
{
	tiny_string data = obj->toString();
	return Class<ASString>::getInstanceS(data.uppercase());
}

ASFUNCTIONBODY(ASString,fromCharCode)
{
	ASString* ret=Class<ASString>::getInstanceS();
	for(uint32_t i=0;i<argslen;i++)
	{
		ret->data += tiny_string::fromChar(args[i]->toUInt());
	}
	return ret;
}

ASFUNCTIONBODY(ASString,replace)
{
	tiny_string data = obj->toString();
	enum REPLACE_TYPE { STRING=0, FUNC };
	REPLACE_TYPE type;
	ASString* ret=Class<ASString>::getInstanceS(data);

	tiny_string replaceWith;
	if(argslen < 2)
	{
		type = STRING;
		replaceWith="";
	}
	else if(args[1]->getObjectType()!=T_FUNCTION)
	{
		type = STRING;
		replaceWith=args[1]->toString();
	}
	else
		type = FUNC;

	if(args[0]->getClass()==Class<RegExp>::getClass())
	{
		RegExp* re=static_cast<RegExp*>(args[0]);

		const char* error;
		int errorOffset;
		int options=PCRE_UTF8;
		if(re->ignoreCase)
			options|=PCRE_CASELESS;
		if(re->extended)
			options|=PCRE_EXTENDED;
		if(re->multiline)
			options|=PCRE_MULTILINE;
		if(re->dotall)
			options|=PCRE_DOTALL;
		pcre* pcreRE=pcre_compile(re->source.raw_buf(), options, &error, &errorOffset,NULL);
		if(error)
			return ret;
		//Verify that 30 for ovector is ok, it must be at least (captGroups+1)*3
		int capturingGroups;
		int infoOk=pcre_fullinfo(pcreRE, NULL, PCRE_INFO_CAPTURECOUNT, &capturingGroups);
		if(infoOk!=0)
		{
			pcre_free(pcreRE);
			return ret;
		}
		assert_and_throw(capturingGroups<20);
		int ovector[60];
		int offset=0;
		int retDiff=0;
		do
		{
			int rc=pcre_exec(pcreRE, NULL, ret->data.raw_buf(), ret->data.numBytes(), offset, 0, ovector, 60);
			if(rc<0)
			{
				//No matches or error
				pcre_free(pcreRE);
				return ret;
			}
			if(type==FUNC)
			{
				//Get the replace for this match
				IFunction* f=static_cast<IFunction*>(args[1]);
				ASObject** subargs = g_newa(ASObject*, 3+capturingGroups);
				//we index on bytes, not on UTF-8 characters
				subargs[0]=Class<ASString>::getInstanceS(ret->data.substr_bytes(ovector[0],ovector[1]-ovector[0]));
				for(int i=0;i<capturingGroups;i++)
					subargs[i+1]=Class<ASString>::getInstanceS(ret->data.substr_bytes(ovector[i*2+2],ovector[i*2+3]-ovector[i*2+2]));
				subargs[capturingGroups+1]=abstract_i(ovector[0]-retDiff);
				
				subargs[capturingGroups+2]=Class<ASString>::getInstanceS(data);
				ASObject* ret=f->call(new Null, subargs, 3+capturingGroups);
				replaceWith=ret->toString().raw_buf();
				ret->decRef();
			} else {
					size_t pos, ipos = 0;
					tiny_string group;
					int i, j;
					while((pos = replaceWith.find("$", ipos)) != tiny_string::npos) {
						i = 0;
						ipos = pos;
						/* docu is not clear what to do if the $nn value is higher 
						 * than the number of matching groups, 
						 * but the ecma3/String/eregress_104375 test indicates
						 * that we should take it as an $n value
						 */
						   
						while (++ipos < replaceWith.numChars() && i<10) {
						j = replaceWith.charAt(ipos)-'0';
							if (j <0 || j> 9 || rc < 10*i + j)
								break;
							i = 10*i + j;
						}
						if (i == 0)
							continue;
						group = (i >= rc) ? "" : ret->data.substr_bytes(ovector[i*2], ovector[i*2+1]-ovector[i*2]);
						replaceWith.replace(pos, ipos-pos, group);
					}
			}
			ret->data.replace_bytes(ovector[0],ovector[1]-ovector[0],replaceWith);
			offset=ovector[0]+replaceWith.numBytes();
			retDiff+=replaceWith.numBytes()-(ovector[1]-ovector[0]);
		}
		while(re->global);

		pcre_free(pcreRE);
	}
	else
	{
		const tiny_string& s=args[0]->toString();
		int index=ret->data.find(s,0);
		if(index==-1) //No result
			return ret;
		assert_and_throw(type==STRING);
		ret->data.replace(index,s.numChars(),replaceWith);
	}

	return ret;
}

ASFUNCTIONBODY(ASString,concat)
{
	tiny_string data = obj->toString();
	ASString* ret=Class<ASString>::getInstanceS(data);
	for(unsigned int i=0;i<argslen;i++)
		ret->data+=args[i]->toString().raw_buf();

	return ret;
}

ASFUNCTIONBODY(ASString,generator)
{
	assert(argslen<=1);
	if (argslen == 0)
		return Class<ASString>::getInstanceS("");
	else
		return Class<ASString>::getInstanceS(args[0]->toString());
}
