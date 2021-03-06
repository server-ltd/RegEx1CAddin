﻿
#include "stdafx.h"

#if defined( __linux__ ) || defined(__APPLE__)
#include <iconv.h>
#endif

#include <stdio.h>
#include <wchar.h>
#include "AddInNative.h"
#include <string>

static std::map<std::u16string, long> mMethods;
static std::vector<std::u16string> vMethods;
static std::map<std::u16string, long> mMethods_ru;
static std::vector<std::u16string> vMethods_ru;

static std::map<std::u16string, long> mProps;
static std::vector<std::u16string> vProps;
static std::map<std::u16string, long> mProps_ru;
static std::vector<std::u16string> vProps_ru;

static const wchar_t g_kClassNames[] = L"CAddInNative";
static IAddInDefBase *pAsyncEvent = NULL;

uint32_t convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, uint32_t len = 0);
uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len = 0);
uint32_t getLenShortWcharStr(const WCHAR_T* Source);
static AppCapabilities g_capabilities = eAppCapabilitiesInvalid;
static WcharWrapper s_names(g_kClassNames);

//---------------------------------------------------------------------------//
long GetClassObject(const WCHAR_T* wsName, IComponentBase** pInterface)
{
    if(!*pInterface)
    {
        *pInterface= new CAddInNative;
        return (long)*pInterface;
    }
    return 0;
}
//---------------------------------------------------------------------------//
AppCapabilities SetPlatformCapabilities(const AppCapabilities capabilities)
{
    g_capabilities = capabilities;
    return eAppCapabilitiesLast;
}
//---------------------------------------------------------------------------//
long DestroyObject(IComponentBase** pIntf)
{
    if(!*pIntf)
        return -1;

    delete *pIntf;
    *pIntf = 0;
    return 0;
}
//---------------------------------------------------------------------------//
const WCHAR_T* GetClassNames()
{
    return s_names;
}
//---------------------------------------------------------------------------//

// CAddInNative
//---------------------------------------------------------------------------//
CAddInNative::CAddInNative()
{
    m_iMemory = 0;
    m_iConnect = 0;

	iCurrentPosition = 0;
	m_PropCountOfItemsInSearchResult = 0;
	sErrorDescription = "";
	bThrowExceptions = false;
	bIgnoreCase = false;
	wcsPattern.clear();
	wcsPattern.assign(L"");
	bGlobal = false;

	if (mMethods.size() == 0) {
		mMethods = { 
		{u"matches", eMethMatches},
		{u"ismatch", eMethIsMatch},
		{u"next", eMethNext},
		{u"replace", eMethReplace},
		{u"count", eMethCount},
		{u"version", eMethVersion},
		};
		vMethods = { u"Matches", u"IsMatch", u"Next", u"Replace", u"Count", u"Version"};
	}

	if (mMethods_ru.size() == 0) {
		mMethods_ru = { 
		{u"найтисовпадения", eMethMatches},
		{u"совпадает", eMethIsMatch},
		{u"следующий", eMethNext},
		{u"заменить", eMethReplace},
		{u"количество", eMethCount},
		{u"версия", eMethVersion},
		};
		vMethods_ru = { u"НайтиСовпадения", u"Совпадает", u"Следующий", u"Заменить", u"Количество", u"Версия" };
	}

	if (mProps.size() == 0) {
		mProps = { 
		{u"currentvalue", ePropCurrentValue},
		{u"ignorecase", ePropIgnoreCase},
		{u"errordescription", ePropErrorDescription},
		{u"throwexceptions", ePropThrowExceptions},
		{u"pattern", ePropPattern},
		{u"global", ePropGlobal},
		};
		vProps = { u"CurrentValue", u"IgnoreCase", u"ErrorDescription", u"ThrowExceptions", u"Pattern", u"Global" };
	}

	if (mProps_ru.size() == 0) {
		mProps_ru = { 
		{u"текущеезначение", ePropCurrentValue},
		{u"игнорироватьрегистр", ePropIgnoreCase},
		{u"описаниеошибки", ePropErrorDescription},
		{u"вызыватьисключения", ePropThrowExceptions},
		{u"шаблон", ePropPattern},
		{u"всесовпадения", ePropGlobal},
		};
		vProps_ru = { u"ТекущееЗначение", u"ИгнорироватьРегистр", u"ОписаниеОшибки", u"ВызыватьИсключения", u"Шаблон", u"ВсеСовпадения" };
	}
}
//---------------------------------------------------------------------------//
CAddInNative::~CAddInNative()
{
}
//---------------------------------------------------------------------------//
bool CAddInNative::Init(void* pConnection)
{ 
    m_iConnect = (IAddInDefBase*)pConnection;
    return m_iConnect != NULL;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetInfo()
{ 
    // Component should put supported component technology version 
    // This component supports 2.0 version
    return 2000; 
}
//---------------------------------------------------------------------------//
void CAddInNative::Done()
{

}
/////////////////////////////////////////////////////////////////////////////
// ILanguageExtenderBase
//---------------------------------------------------------------------------//
bool CAddInNative::RegisterExtensionAs(WCHAR_T** wsExtensionName)
{ 
	char16_t name[] = u"RegEx";

	if (!m_iMemory || !m_iMemory->AllocMemory(reinterpret_cast<void **>(wsExtensionName), sizeof(name))) {
		return false;
	};

	memcpy(*wsExtensionName, name, sizeof(name));

	return true;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNProps()
{ 
    return ePropLast;
}
//---------------------------------------------------------------------------//
long CAddInNative::FindProp(const WCHAR_T* wsPropName)
{ 
	std::basic_string<char16_t> usPropName = (char16_t*)(wsPropName);
	std::transform(usPropName.begin(), usPropName.end(), usPropName.begin(), ::towlower);

	auto it = mProps.find(usPropName);
	if (it != mProps.end())
		return it->second;

	it = mProps_ru.find(usPropName);
	if (it != mProps_ru.end())
		return it->second;

	return -1;
}

//---------------------------------------------------------------------------//
const WCHAR_T* CAddInNative::GetPropName(long lPropNum, long lPropAlias)
{ 
    if (lPropNum >= ePropLast)
        return NULL;

	std::basic_string<char16_t> *usCurrentName;

    switch(lPropAlias)
    {
    case 0: // First language
		usCurrentName = &vProps[lPropNum];
        break;
    case 1: // Second language
		usCurrentName = &vProps_ru[lPropNum];
		break;
	default:
        return 0;
    }

	if (usCurrentName->length() == 0) {
		return nullptr;
	}

	WCHAR_T *result = nullptr;

	size_t bytes = (usCurrentName->length() + 1) * sizeof(char16_t);

	if (!m_iMemory || !m_iMemory->AllocMemory(reinterpret_cast<void **>(&result), bytes)) {
		return nullptr;
	};

	memcpy(result, usCurrentName->c_str(), bytes);

	return result;
}
//---------------------------------------------------------------------------//
bool CAddInNative::GetPropVal(const long lPropNum, tVariant* pvarPropVal)
{
	switch (lPropNum)
	{
	case ePropCurrentValue:
	{
		TV_VT(pvarPropVal) = VTYPE_PWSTR;
		std::wstring* wsCurrentValue = &vResults[iCurrentPosition];

#if defined( __linux__ ) || defined(__APPLE__)
		if (m_iMemory->AllocMemory((void**)&pvarPropVal->pwstrVal, (wsCurrentValue->length() + 1) * sizeof(WCHAR_T)))
		{
			WCHAR_T* str_WCHAR_T = 0;
			convToShortWchar(&str_WCHAR_T, wsCurrentValue->c_str(), wsCurrentValue->length() + 1);
			memcpy(pvarPropVal->pwstrVal, str_WCHAR_T, (wsCurrentValue->length() + 1) * sizeof(WCHAR_T));
			pvarPropVal->wstrLen = wsCurrentValue->length();
			return true;
		}
#else

		if (m_iMemory->AllocMemory((void**)&pvarPropVal->pwstrVal, (wsCurrentValue->length() + 1) * sizeof(wchar_t)))
		{
			memcpy(pvarPropVal->pwstrVal, wsCurrentValue->c_str(), (wsCurrentValue->length() + 1) * sizeof(wchar_t));
			pvarPropVal->wstrLen = wsCurrentValue->length();
			return true;
		}
#endif
		return false;
	};
	case ePropErrorDescription:
	{
		if (m_iMemory->AllocMemory((void**)&pvarPropVal->pstrVal, (sErrorDescription.length() + 1) * sizeof(char)))
		{
			strcpy(pvarPropVal->pstrVal, sErrorDescription.c_str());
			TV_VT(pvarPropVal) = VTYPE_PSTR;
			pvarPropVal->strLen = sErrorDescription.length();
			return true;
		}
		else
		{
			TV_VT(pvarPropVal) = VTYPE_PWSTR;
			pvarPropVal->pwstrVal = NULL;
			pvarPropVal->wstrLen = 0;
			return true;
		}
	}
	case ePropThrowExceptions:
	{
		TV_VT(pvarPropVal) = VTYPE_BOOL;
		pvarPropVal->bVal = bThrowExceptions;
		return true;
	}
	case ePropPattern:
	{
		if (m_iMemory->AllocMemory((void**)&pvarPropVal->pstrVal, (wcsPattern.length() + 1) * sizeof(WCHAR)))
		{
			memcpy(pvarPropVal->pwstrVal, wcsPattern.c_str(), wcsPattern.length() * sizeof(WCHAR));
			TV_VT(pvarPropVal) = VTYPE_PWSTR;
			pvarPropVal->wstrLen = wcsPattern.length();
			return true;
		}
		else
		{
			TV_VT(pvarPropVal) = VTYPE_PWSTR;
			pvarPropVal->pwstrVal = NULL;
			pvarPropVal->wstrLen = 0;
			return true;
		}
	}
	case ePropGlobal:
	{
		TV_VT(pvarPropVal) = VTYPE_BOOL;
		pvarPropVal->bVal = bGlobal;
		return true;
	}
	default:
		return false;
	}

	return true;
}
//---------------------------------------------------------------------------//
bool CAddInNative::SetPropVal(const long lPropNum, tVariant *varPropVal)
{ 
    switch(lPropNum)
    { 
    case ePropThrowExceptions:
	{
		if (TV_VT(varPropVal) != VTYPE_BOOL)
			return false;
		bThrowExceptions = TV_BOOL(varPropVal);
		return true;
	}
	case ePropIgnoreCase: {
		bIgnoreCase = TV_BOOL(varPropVal);
		return true;
	}
	case ePropPattern: {
#if defined( __linux__ ) || defined(__APPLE__)
		// Сконвертируем в строку с wchar_t символами
		wchar_t* str_wchar_t1 = 0;
		convFromShortWchar(&str_wchar_t1, varPropVal->pwstrVal, varPropVal->wstrLen + 1);
		wcsPattern.assign(str_wchar_t1);
		rePattern.assign(str_wchar_t1, (bIgnoreCase) ? boost::regex::icase : boost::regex_constants::normal);
		delete[] str_wchar_t1;
#else
		wcsPattern.assign(varPropVal->pwstrVal);
		rePattern.assign(varPropVal->pwstrVal, (bIgnoreCase) ? boost::regex::icase : boost::regex_constants::normal);
#endif
		return true;
	}
	case ePropGlobal:
	{
		bGlobal = TV_BOOL(varPropVal);
		return true;
	}
    default:
        return false;
    }

    return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::IsPropReadable(const long lPropNum)
{ 
    switch(lPropNum)
    { 
    case ePropCurrentValue:
        return true;
	case ePropErrorDescription:
		return true;
	case ePropIgnoreCase:
		return true;
	case ePropThrowExceptions:
		return true;
	case ePropPattern:
		return true;
	case ePropGlobal:
		return true;
    default:
        return false;
    }

    return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::IsPropWritable(const long lPropNum)
{
    switch(lPropNum)
    { 
    case ePropIgnoreCase:
        return true;
    case ePropThrowExceptions:
        return true;
	case ePropPattern:
		return true;
	case ePropGlobal:
		return true;
    default:
        return false;
    }

    return false;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNMethods()
{ 
    return eMethLast;
}
//---------------------------------------------------------------------------//
long CAddInNative::FindMethod(const WCHAR_T* wsMethodName)
{

	std::basic_string<char16_t> usMethodName = (char16_t*)(wsMethodName);
	std::transform(usMethodName.begin(), usMethodName.end(), usMethodName.begin(), ::towlower);

	auto it = mMethods.find(usMethodName);
	if (it != mMethods.end())
		return it->second;
	
	it = mMethods_ru.find(usMethodName);
	if (it != mMethods_ru.end())
		return it->second;

	return -1;
}
//---------------------------------------------------------------------------//
const WCHAR_T* CAddInNative::GetMethodName(const long lMethodNum, const long lMethodAlias)
{ 

	if (lMethodNum >= eMethLast)
		return NULL;

	std::basic_string<char16_t> *usCurrentName;

	switch (lMethodAlias)
	{
	case 0: // First language
		usCurrentName = &vMethods[lMethodNum];
		break;
	case 1: // Second language
		usCurrentName = &vMethods_ru[lMethodNum];
		break;
	default:
		return 0;
	}

	WCHAR_T *result = nullptr;

	size_t bytes = (usCurrentName->length() + 1) * sizeof(char16_t);

	if (!m_iMemory || !m_iMemory->AllocMemory(reinterpret_cast<void **>(&result), bytes)) {
		return nullptr;
	};

	memcpy(result, usCurrentName->c_str(), bytes);

	return result;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNParams(const long lMethodNum)
{ 
    switch(lMethodNum)
    { 
    case eMethMatches:
        return 2;
    case eMethIsMatch:
        return 2;
	case eMethReplace:
		return 3;
    default:
        return 0;
    }
    
    return 0;
}
//---------------------------------------------------------------------------//
bool CAddInNative::GetParamDefValue(const long lMethodNum, const long lParamNum,
                        tVariant *pvarParamDefValue)
{ 
    switch(lMethodNum)
    { 
	case eMethMatches:
	{
		if (lParamNum == 1)
		{
			TV_VT(pvarParamDefValue) = VTYPE_PWSTR;
			pvarParamDefValue->wstrLen = 0;
			return true;
		}
	}
    case eMethIsMatch:
		if (lParamNum == 1)
		{
			TV_VT(pvarParamDefValue) = VTYPE_PWSTR;
			pvarParamDefValue->wstrLen = 0;
			return true;
		}
    case eMethNext:
		break;
	case eMethReplace:
		if (lParamNum == 1)
		{
			TV_VT(pvarParamDefValue) = VTYPE_PWSTR;
			pvarParamDefValue->wstrLen = 0;
			return true;
		}
	default:
	{
		TV_VT(pvarParamDefValue) = VTYPE_EMPTY;
		return false;
	}
	}

	TV_VT(pvarParamDefValue) = VTYPE_EMPTY;
    return false;
} 
//---------------------------------------------------------------------------//
bool CAddInNative::HasRetVal(const long lMethodNum)
{ 
    switch(lMethodNum)
    { 
    case eMethNext:
		return true;
	case eMethReplace:
		return true;
    case eMethIsMatch:
        return true;
	case eMethCount:
		return true;
	case eMethVersion:
		return true;
    default:
        return false;
    }

    return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::CallAsProc(const long lMethodNum,
	tVariant* paParams, const long lSizeArray)
{
	switch (lMethodNum)
	{
	case eMethMatches:
		return search(paParams);
	case eMethIsMatch:
		break;
	case eMethReplace:
		break;
	case eMethCount:
		break;
	case eMethVersion:
		break;
	case eMethNext:
	{
		if (iCurrentPosition >= m_PropCountOfItemsInSearchResult) {
			return true;
		}
		else {
			iCurrentPosition++;
			return true;
		}
	}
	break;
	default:
		return false;
	}
	return true;
}
//---------------------------------------------------------------------------//
bool CAddInNative::CallAsFunc(const long lMethodNum,
                tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{ 
	switch (lMethodNum)
	{
		// Method acceps one argument of type BinaryData ant returns its copy
	case eMethMatches:
		break;
	case eMethIsMatch:
	{
		match(pvarRetValue, paParams);
		return true;
	}
	break;
	case eMethReplace:
		return replace(pvarRetValue, paParams);
	case eMethNext:
	{
		iCurrentPosition++;
		if (iCurrentPosition >= m_PropCountOfItemsInSearchResult) {
			TV_VT(pvarRetValue) = VTYPE_BOOL;
			pvarRetValue->bVal = false;
			return true;
		}
		else {
			TV_VT(pvarRetValue) = VTYPE_BOOL;
			pvarRetValue->bVal = true;
			return true;
		}
	}
	break;
	case eMethCount:{
		TV_VT(pvarRetValue) = VTYPE_I4;
		pvarRetValue->lVal = m_PropCountOfItemsInSearchResult;
		return true;
	}
	case eMethVersion: {
		version(pvarRetValue);
		return true;
	}
	default:
		return false;
	}
	return true;
}

//---------------------------------------------------------------------------//
// Платформа передает в компоненту текущее название локали.
// Название локали зависит от системы и возможно от настроек платформы,
// может принимать самые различные значения: "ru_RU", "rus", "ru" и т.д.
// Название локали можно использовать для инициализации локали в стандартной библиотеке шаблонов (STL)
// языка C++. От установленной локали в STL зависит: 
// - правила конвертации из строки в число (напр. символ разделения дробной части)
// - мультибайтовая кодировка (UTF-8, CP1521 и т.д.) в которую будет преобразовываться строка из широких символов функцией wcstombs.
// - мультибайтовая кодировка, которая будет принята за основу при преобразовании строки из мультибайтовых символов в широкие функцией mbstowcs.
// - множество прочих национальных особенностей 
// На текущий момент (v1С = 8.3.6.1760), в функцию могут передаваться названия локалей которые не подходят для передачи
// в функцию setlocale и она возвращает NULL, с этим надо бы разобраться. Если функция setlocale возвращает NULL,
// то в неё передается пустая строка, это приводит к установке текущих языковых настроек операционной системы.
// 
void CAddInNative::SetLocale(const WCHAR_T* loc)
{
#if !defined( __linux__ ) && !defined(__APPLE__)
	// если платформа передала неправильное название локали, то устанавливаем системную локаль по умолчанию
	// если вовсе не установить локаль, то STL будет поддерживать только символы латиницы!!!
	if (_wsetlocale(LC_ALL, loc) == NULL) _wsetlocale(LC_ALL, L"");
#else
	int size = 0;
	char *mbstr = 0;
	wchar_t *tmpLoc = 0;
	convFromShortWchar(&tmpLoc, loc);
	size = wcslen(tmpLoc) + 1;
	mbstr = new char[size * MBCMAXSIZE];

	if (!mbstr)
	{
		delete[] tmpLoc;
		return;
	}

	memset(mbstr, 0, size * MBCMAXSIZE);
	size = wcstombs(mbstr, tmpLoc, size * MBCMAXSIZE);

	char* res = setlocale(LC_ALL, mbstr);
	if (res == NULL) setlocale(LC_ALL, "");
	delete[] tmpLoc;
	delete[] mbstr;
#endif
}

/////////////////////////////////////////////////////////////////////////////
// LocaleBase
//---------------------------------------------------------------------------//
bool CAddInNative::setMemManager(void* mem)
{
    m_iMemory = (IMemoryManager*)mem;
    return m_iMemory != 0;
}

bool CAddInNative::search(tVariant * paParams)
{
	SetLastError("");
	vResults.clear();

	boost::wsmatch wsmMatch;
	boost::wregex* pattern = NULL;
	bool bClearPttern = false;

#if defined( __linux__ ) || defined(__APPLE__)
	// Сконвертируем в строку с wchar_t символами
	wchar_t* str_wchar_t1 = 0;
	convFromShortWchar(&str_wchar_t1, paParams[0].pwstrVal, paParams[0].wstrLen + 1);

	std::wstring str(str_wchar_t1);

	wchar_t* str_wchar_t2 = 0;
	convFromShortWchar(&str_wchar_t2, paParams[1].pwstrVal, paParams[1].wstrLen + 1);
	try
	{
		if (paParams[1].wstrLen == 0)
		{
			if (wcsPattern.length() > 0)
				pattern = &rePattern;
			else
				return true;
		}
		else
		{
			bClearPttern = true;
			pattern = new boost::wregex(str_wchar_t2, (bIgnoreCase) ? boost::regex::icase : boost::regex_constants::normal);
		}

		if (bGlobal)
		{
			std::wstring::const_iterator start = str.begin();
			std::wstring::const_iterator end = str.end();
			//boost::regex_constants::match_flag_type flags = boost::match_default;

			while (start < end &&
				boost::regex_search(start, end, wsmMatch, *pattern))
			{
				for (auto r : wsmMatch) {
					if (r.length() == 0)
						continue;
					vResults.push_back(r);
				}
				start = wsmMatch[0].second;
}
		}
		else
		{
			boost::regex_search(str, wsmMatch, *pattern);
			for (auto r : wsmMatch) {
				if (r.length() == 0)
					continue;
				vResults.push_back(r);
			}
		}

	}
	catch (const std::exception& e)
	{
		SetLastError(e.what());
		vResults.clear();
		iCurrentPosition = -1;
		m_PropCountOfItemsInSearchResult = 0;
		if (bClearPttern && pattern != NULL)
			delete pattern;

		if (bThrowExceptions)
			return false;
		else
			return true;
	}

	delete[] str_wchar_t1;
	delete[] str_wchar_t2;

#else
	std::wstring str(paParams[0].pwstrVal);
	try
	{
		if (paParams[1].wstrLen == 0)
		{
			if (wcsPattern.length() > 0)
				pattern = &rePattern;
			else
				return true;
		}
		else
		{
			bClearPttern = true;
			pattern = new boost::wregex(paParams[1].pwstrVal, (bIgnoreCase) ? boost::regex::icase : boost::regex_constants::normal);
		}

		if (bGlobal)
		{
			std::wstring::const_iterator start = str.begin();
			std::wstring::const_iterator end = str.end();
			//boost::regex_constants::match_flag_type flags = boost::match_default;

			while (start < end &&
				boost::regex_search(start, end, wsmMatch, *pattern))
			{
				for (auto r : wsmMatch) {
					if (r.length() == 0)
						continue;
					vResults.push_back(r);
				}
				start = wsmMatch[0].second;
			}
		}
		else
		{
			boost::regex_search(str, wsmMatch, *pattern);
			for (auto r : wsmMatch) {
				if (r.length() == 0)
					continue;
				vResults.push_back(r);
			}
		}
	}
	catch (const std::exception& e)
	{
		SetLastError(e.what());
		vResults.clear();
		iCurrentPosition = -1;
		m_PropCountOfItemsInSearchResult = 0;
		if (bClearPttern && pattern != NULL)
			delete pattern;
		if (bThrowExceptions)
			return false;
		else
		return true;
	}
#endif
	iCurrentPosition = -1;
	m_PropCountOfItemsInSearchResult = vResults.size();
	if (bClearPttern && pattern != NULL)
		delete pattern;
	return true;
}
bool CAddInNative::replace(tVariant * pvarRetValue, tVariant * paParams)
{
	SetLastError("");
	TV_VT(pvarRetValue) = VTYPE_PWSTR;
	iCurrentPosition = 0;
	m_PropCountOfItemsInSearchResult = 0;
	vResults.clear();

	boost::wregex* pattern = NULL;
	bool bClearPttern = false;

#if defined( __linux__ ) || defined(__APPLE__)
	// Сконвертируем в строку с wchar_t символами
	wchar_t* str_wchar_t1 = 0;
	convFromShortWchar(&str_wchar_t1, paParams[0].pwstrVal, paParams[0].wstrLen + 1);
	std::wstring str(str_wchar_t1);

	wchar_t* str_wchar_t3 = 0;
	convFromShortWchar(&str_wchar_t3, paParams[2].pwstrVal, paParams[2].wstrLen + 1);
	std::wstring replacement(str_wchar_t3);

	wchar_t* str_wchar_t2 = 0;
	convFromShortWchar(&str_wchar_t2, paParams[1].pwstrVal, paParams[1].wstrLen + 1);
	std::wstring res;
	try
	{
		if (paParams[1].wstrLen == 0)
		{
			if (wcsPattern.length() > 0)
				pattern = &rePattern;
			else
				return true;
		}
		else
		{
			bClearPttern = true;
			pattern = new boost::wregex(str_wchar_t2, (bIgnoreCase) ? boost::regex::icase : boost::regex_constants::normal);
		}	
		if (bGlobal)
			res = boost::regex_replace(str, *pattern, replacement);
		else
			res = boost::regex_replace(str, *pattern, replacement, boost::regex_constants::format_first_only);

	}
	catch (const std::exception& e)
	{
		vResults.clear();
		iCurrentPosition = -1;
		m_PropCountOfItemsInSearchResult = 0;
		if (bClearPttern && pattern != NULL)
			delete pattern;
		SetLastError(e.what());
		if (bThrowExceptions)
			return false;
		else
			return true;
	}

	delete[] str_wchar_t1;
	delete[] str_wchar_t2;
	delete[] str_wchar_t3;

	if (m_iMemory->AllocMemory((void**)&pvarRetValue->pwstrVal, (res.length() + 1) * sizeof(WCHAR_T)))
	{

		WCHAR_T* str_WCHAR_T = 0;
		convToShortWchar(&str_WCHAR_T, res.c_str(), res.length() + 1);
		memcpy(pvarRetValue->pwstrVal, str_WCHAR_T, (res.length() + 1) * sizeof(WCHAR_T));
		pvarRetValue->wstrLen = res.length();
		return true;
}
#else

	std::wstring str(paParams[0].pwstrVal);
	std::wstring res;
	try
	{
		boost::wregex* pattern;
		if (paParams[1].wstrLen == 0)
		{
			if (wcsPattern.length() > 0)
				pattern = &rePattern;
			else
				return true;
		}
		else
		{
			bClearPttern = true;
			pattern = new boost::wregex(paParams[1].pwstrVal, (bIgnoreCase) ? boost::regex::icase : boost::regex_constants::normal);
		}
		if (bGlobal)
			res = boost::regex_replace(str, *pattern, paParams[2].pwstrVal);
		else
			res = boost::regex_replace(str, *pattern, paParams[2].pwstrVal, boost::regex_constants::format_first_only);
	}
	catch (const std::exception& e)
	{
		vResults.clear();
		iCurrentPosition = -1;
		m_PropCountOfItemsInSearchResult = 0;
		if (bClearPttern && pattern != NULL)
			delete pattern;

		SetLastError(e.what());
		if (bThrowExceptions)
			return false;
		else
			return true;
	}

	if (m_iMemory->AllocMemory((void**)&pvarRetValue->pwstrVal, (res.length() + 1) * sizeof(wchar_t)))
	{
		memcpy(pvarRetValue->pwstrVal, res.c_str(), (res.length() + 1) * sizeof(wchar_t));
		pvarRetValue->wstrLen = res.length();
		return true;
	}
#endif

	if (bClearPttern && pattern != NULL)
		delete pattern;

	return false;
}

bool CAddInNative::match(tVariant * pvarRetValue, tVariant * paParams)
{
	SetLastError("");
	TV_VT(pvarRetValue) = VTYPE_BOOL;

	boost::wregex* pattern = NULL;
	bool bClearPttern = false;

#if defined( __linux__ ) || defined(__APPLE__)
	wchar_t* str_wchar_t1 = 0;
	convFromShortWchar(&str_wchar_t1, paParams[0].pwstrVal, paParams[0].wstrLen + 1);
	wchar_t* str_wchar_t2 = 0;
	convFromShortWchar(&str_wchar_t2, paParams[1].pwstrVal, paParams[1].wstrLen + 1);
	try
	{
		if (paParams[1].wstrLen == 0)
		{
			if (wcsPattern.length() > 0)
				pattern = &rePattern;
			else
				return true;
		}
		else
		{
			bClearPttern = true;
			pattern = new boost::wregex(str_wchar_t2, (bIgnoreCase) ? boost::regex::icase : boost::regex_constants::normal);
		}
		pvarRetValue->bVal = boost::regex_match(str_wchar_t1, *pattern);

	}
	catch (const std::exception& e)
	{
		vResults.clear();
		iCurrentPosition = -1;
		m_PropCountOfItemsInSearchResult = 0;
		if (bClearPttern && pattern != NULL)
			delete pattern;
		SetLastError(e.what());
		if (bThrowExceptions)
			return false;
		else
			return true;
	}
	delete[] str_wchar_t1;
	delete[] str_wchar_t2;
#else
	try
	{
		boost::wregex* pattern;
		if (paParams[1].wstrLen == 0)
		{
			if (wcsPattern.length() > 0)
				pattern = &rePattern;
			else
				return true;
		}
		else
			pattern = new boost::wregex(paParams[1].pwstrVal, (bIgnoreCase) ? boost::regex::icase : boost::regex_constants::normal);

		pvarRetValue->bVal = boost::regex_match(paParams[0].pwstrVal, *pattern);
	}
	catch (const std::exception& e)
	{
		vResults.clear();
		iCurrentPosition = -1;
		m_PropCountOfItemsInSearchResult = 0;
		if (bClearPttern && pattern != NULL)
			delete pattern;
		SetLastError(e.what());
		if (bThrowExceptions)
			return false;
		else
			return true;
	}
#endif
	iCurrentPosition = 0;
	m_PropCountOfItemsInSearchResult = 0;
	vResults.clear();
	if (bClearPttern && pattern != NULL)
		delete pattern;
	return true;
}

void CAddInNative::version(tVariant * pvarRetValue)
{
	TV_VT(pvarRetValue) = VTYPE_PWSTR;
	std::basic_string<char16_t> res = u"6";

	if (m_iMemory->AllocMemory((void**)&pvarRetValue->pwstrVal, (res.length() + 1) * sizeof(char16_t)))
	{
		memcpy(pvarRetValue->pwstrVal, res.c_str(), (res.length() + 1) * sizeof(char16_t));
		pvarRetValue->wstrLen = res.length();
	}
}
//---------------------------------------------------------------------------//
long CAddInNative::findName(const wchar_t* names[], const wchar_t* name, 
                        const uint32_t size) const
{
    long ret = -1;
    for (uint32_t i = 0; i < size; i++)
    {
        if (!wcscmp(names[i], name))
        {
            ret = i;
            break;
        }
    }
    return ret;
}

//---------------------------------------------------------------------------//
uint32_t convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, uint32_t len)
{
    if (!len)
        len = ::wcslen(Source) + 1;

    if (!*Dest)
        *Dest = new WCHAR_T[len];

    WCHAR_T* tmpShort = *Dest;
    wchar_t* tmpWChar = (wchar_t*) Source;
    uint32_t res = 0;

    ::memset(*Dest, 0, len * sizeof(WCHAR_T));
#if defined( __linux__ ) || defined(__APPLE__)
    size_t succeed = (size_t)-1;
    size_t f = len * sizeof(wchar_t), t = len * sizeof(WCHAR_T);
    const char* fromCode = sizeof(wchar_t) == 2 ? "UTF-16" : "UTF-32";
    iconv_t cd = iconv_open("UTF-16LE", fromCode);
    if (cd != (iconv_t)-1)
    {
        succeed = iconv(cd, (char**)&tmpWChar, &f, (char**)&tmpShort, &t);
        iconv_close(cd);
        if(succeed != (size_t)-1)
            return (uint32_t)succeed;
    }
#endif //__linux__
    for (; len; --len, ++res, ++tmpWChar, ++tmpShort)
    {
        *tmpShort = (WCHAR_T)*tmpWChar;
    }

    return res;
}
//---------------------------------------------------------------------------//
uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len)
{
    if (!len)
        len = getLenShortWcharStr(Source) + 1;

    if (!*Dest)
        *Dest = new wchar_t[len];

    wchar_t* tmpWChar = *Dest;
    WCHAR_T* tmpShort = (WCHAR_T*)Source;
    uint32_t res = 0;

    ::memset(*Dest, 0, len * sizeof(wchar_t));
#if defined( __linux__ ) || defined(__APPLE__)
    size_t succeed = (size_t)-1;
    const char* fromCode = sizeof(wchar_t) == 2 ? "UTF-16" : "UTF-32";
    size_t f = len * sizeof(WCHAR_T), t = len * sizeof(wchar_t);
    iconv_t cd = iconv_open("UTF-32LE", fromCode);
    if (cd != (iconv_t)-1)
    {
        succeed = iconv(cd, (char**)&tmpShort, &f, (char**)&tmpWChar, &t);
        iconv_close(cd);
        if(succeed != (size_t)-1)
            return (uint32_t)succeed;
    }
#endif //__linux__
    for (; len; --len, ++res, ++tmpWChar, ++tmpShort)
    {
        *tmpWChar = (wchar_t)*tmpShort;
    }

    return res;
}
//---------------------------------------------------------------------------//

uint32_t getLenShortWcharStr(const WCHAR_T* Source)
{
    uint32_t res = 0;
    WCHAR_T *tmpShort = (WCHAR_T*)Source;

    while (*tmpShort++)
        ++res;

    return res;
}
//---------------------------------------------------------------------------//

#ifdef LINUX_OR_MACOS
WcharWrapper::WcharWrapper(const WCHAR_T* str) : m_str_WCHAR(NULL),
                           m_str_wchar(NULL)
{
    if (str)
    {
        int len = getLenShortWcharStr(str);
        m_str_WCHAR = new WCHAR_T[len + 1];
        memset(m_str_WCHAR,   0, sizeof(WCHAR_T) * (len + 1));
        memcpy(m_str_WCHAR, str, sizeof(WCHAR_T) * len);
        ::convFromShortWchar(&m_str_wchar, m_str_WCHAR);
    }
}
#endif
//---------------------------------------------------------------------------//
WcharWrapper::WcharWrapper(const wchar_t* str) :
#ifdef LINUX_OR_MACOS
    m_str_WCHAR(NULL),
#endif 
    m_str_wchar(NULL)
{
    if (str)
    {
        int len = wcslen(str);
        m_str_wchar = new wchar_t[len + 1];
        memset(m_str_wchar, 0, sizeof(wchar_t) * (len + 1));
        memcpy(m_str_wchar, str, sizeof(wchar_t) * len);
#ifdef LINUX_OR_MACOS
        ::convToShortWchar(&m_str_WCHAR, m_str_wchar);
#endif
    }

}
//---------------------------------------------------------------------------//
WcharWrapper::~WcharWrapper()
{
#ifdef LINUX_OR_MACOS
    if (m_str_WCHAR)
    {
        delete [] m_str_WCHAR;
        m_str_WCHAR = NULL;
    }
#endif
    if (m_str_wchar)
    {
        delete [] m_str_wchar;
        m_str_wchar = NULL;
    }
}
//---------------------------------------------------------------------------//

// Устанавливает значение переменной sErrorDescription класса CAddInNative.
// Если переменная sErrorDescription не пустая, то значение добавляется в начало строки,
// а уже существующий текст, присоединяется через двоеточие справа
// такой подход принят для реализации механизма вложенности описания ошибки
// на самом первом этапе, в текст ошибки добавляется её первопричина, а на более 
// высоких уровнях её можно дополнить описанием о выполняемых действиях
//
void CAddInNative::SetLastError(const char* error) {

	if (error == "") sErrorDescription.clear();
	if (sErrorDescription.length() != 0) sErrorDescription = std::string(error) + ": " + sErrorDescription;
	else sErrorDescription = error;

}