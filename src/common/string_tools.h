#ifndef CTM_COMMON_STRING_TOOLS_H__
#define CTM_COMMON_STRING_TOOLS_H__

#include <vector>
#include <string>

#define BLANK_SET "\t\f\v\n\r "

namespace ctm
{
	std::string TrimLeft(std::string& strInOut);

	std::string TrimRight(std::string& strInOut);

	std::string Trimmed(std::string& strInOut);

	std::string ToUpper(std::string& strInOut);

	std::string ToLower(std::string& strInOut);

	std::string RemoveFlag(std::string& strInOut, const std::string& strFlag = " ");

	std::string RemoveFlagOnce(std::string& strInOut, const std::string& strFlag = " ");

	std::string Replace(std::string& strInOut, const std::string& subSrc, const std::string& subDst);

	std::string ReplaceOnce(std::string& strInOut, const std::string& subSrc, const std::string& subDst);

	bool StartsWith(const std::string& strIn, const std::string& substr, bool bCase = true);

	bool EndsWith(const std::string& strIn, const std::string& substr, bool bCase = true);
		
	void CutString(const std::string& strIn, 
		std::vector<std::string>& vecOutput, 
		const std::string flag = " ", 
		bool bjumpSpace = true);

	void CutStringFirstOf(const std::string& strIn, 
		std::vector<std::string>& vecOutput, 
		const std::string flagSet = BLANK_SET, 
		bool bjumpSpace = true);

	std::string EncodeHex(const std::string& strIn, bool bUp = false, bool bPrefix = false);

	std::string DecodeHex(const std::string& strIn);

	std::string EncodeBase64(const std::string& strIn);
	
	std::string DecodeBase64(const std::string& strIn);

	bool IsNumbers(const std::string& strIn);

	inline bool IsDigits(const std::string& strIn) { return IsNumbers(strIn);}

	bool IsSpace(const std::string& strIn);

	bool IsAlnum(const std::string& strIn);

	std::string Join(const std::vector<std::string>& vecIn, const std::string& strFlag = " ");

	std::vector<std::string>& Split(const std::string& strIn, std::vector<std::string>& outVec, const std::string sep = " ");

	std::string BaseFileName(const std::string& strPathFileName);

	std::string PathName(const std::string& strPathFileName);
}

#endif