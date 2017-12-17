#include "string_tools.h"
#include <ctype.h>
#include <assert.h>

namespace ctm
{
	std::string TrimLeft(std::string& strInOut)
	{
		int pos = strInOut.find_first_not_of(BLANK_SET);
		if(pos != strInOut.npos)
		{
			strInOut = strInOut.substr(pos);
		}
		return strInOut;
	}

	std::string TrimRight(std::string& strInOut)
	{
		int pos = strInOut.find_last_not_of(BLANK_SET);
		if(pos != strInOut.npos)
		{
			strInOut = strInOut.substr(0, pos + 1);
		}
		return strInOut;
	}

	std::string Trimmed(std::string& strInOut)
	{
		TrimLeft(strInOut);
		return TrimRight(strInOut);
	}

	std::string ToUpper(std::string& strInOut)
	{
		for(int i = 0; i < strInOut.size(); ++i)
		{
			strInOut[i] = toupper(strInOut[i]);
		}
		return strInOut;
	}

	std::string ToLower(std::string& strInOut)
	{
		for(int i = 0; i < strInOut.size(); ++i)
		{
			strInOut[i] = tolower(strInOut[i]);
		}
		return strInOut;
	}

	std::string RemoveFlag(std::string& strInOut, const std::string& strFlag)
	{
		int pos =  strInOut.find(strFlag);
		while(pos != strInOut.npos)
		{
			strInOut.erase(pos, strFlag.size());
			pos = strInOut.find(strFlag, pos);
		}
		return strInOut;
	}

	std::string RemoveFlagOnce(std::string& strInOut, const std::string& strFlag)
	{
		int pos  = strInOut.find(strFlag);
		if(pos != strInOut.npos)
		{
			strInOut.erase(pos, strFlag.size());
		}
		return strInOut;
	}

	std::string Replace(std::string& strInOut, const std::string& subSrc, const std::string& subDst)
	{
		int pos = strInOut.find(subSrc);
		while(pos != strInOut.npos)
		{
			strInOut.replace(pos, subSrc.size(), subDst);
			pos = strInOut.find(subSrc, pos + subDst.size());
		}
		return strInOut;
	}

	std::string ReplaceOnce(std::string& strInOut, const std::string& subSrc, const std::string& subDst)
	{
		int pos = strInOut.find(subSrc);
		if(pos != strInOut.npos)
		{
			strInOut.replace(pos, subSrc.size(), subDst);
		}
		return strInOut;
	}

	bool StartsWith(const std::string& strIn, const std::string& substr, bool bCase)
	{
		bool bRet = true;
		int i = 0;
		if(bCase)
		{
			for(i = 0; i < strIn.size() && i < substr.size(); i++)
			{
				if(toupper(strIn[i]) != toupper(substr[i])){
					break;
				}
			}
		}
		else
		{
			for(i = 0; i < strIn.size() && i < substr.size(); i++)
			{
				if(strIn[i] != substr[i]){
					break;
				}
			}
		}

		if(i < substr.size())
		{
			bRet = false;
		}

		return bRet;
	}

	bool EndsWith(const std::string& strIn, const std::string& substr, bool bCase)
	{
		bool bRet = true;
		int i = strIn.size() - 1;
		int j = substr.size() - 1;
		if(bCase)
		{
			for(; i > 0 && j > 0; i--, j--)
			{
				if(toupper(strIn[i]) != toupper(substr[j])){
					break;
				}
			}
		}
		else
		{
			for(; i > 0 && j > 0; i--, j--)
			{
				if(strIn[i] != substr[j]){
					break;
				}
			}
		}

		if(j > 0)
		{
			bRet = false;
		}
		
		return bRet;
	}

	void CutString(std::string strInput, std::vector<std::string>& vecOutput, const std::string flag, bool bjumpSpace)
	{
		vecOutput.clear();
		int begin = 0;
		int end = 0;

		if(!EndsWith(strInput, flag)) strInput += flag;

		while(begin < strInput.size() && (end = strInput.find(flag, begin)) != std::string::npos)
		{
			if(bjumpSpace && end <= begin)
			{
				begin = end + flag.size();
				continue;
			}

			vecOutput.push_back(strInput.substr(begin, end - begin));
			begin = end + flag.size();
		}
	}

	void CutStringFirstOf(std::string strInput, std::vector<std::string>& vecOutput, const std::string flagSet, bool bjumpSpace)
	{
		vecOutput.clear();
		int begin = 0;
		int end = 0;

		if(flagSet.find(strInput[strInput.size() -1]) == std::string::npos) strInput += flagSet[0];

		while(begin < strInput.size() && (end = strInput.find_first_of(flagSet, begin)) != std::string::npos)
		{
			if(bjumpSpace && end <= begin)
			{
				begin = end + 1;
				continue;
			}

			vecOutput.push_back(strInput.substr(begin, end - begin));
			begin = end + 1;
		}
	}

	static char hex[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	
	char Hex2Bin(char hex)
	{
		char c = 0x00;
		if (hex >= '0' && hex <= '9') c = hex - '0';
		else if (hex >= 'A' && hex <= 'F') c = 10 + hex - 'A';
		else if (hex >= 'a' && hex <= 'f') c = 10 + hex - 'a';
		return c;
	}

	inline std::string Bin2Hex(char c, bool bUp = false, bool bPrefix = false)
	{
		std::string strHex;
		if(bPrefix) strHex += "0x";
		strHex += bUp ? toupper(hex[int((c & 0xf0) >> 4)]) : hex[int((c & 0xf0) >> 4)];
		strHex += bUp ? toupper(hex[int((c & 0x0f))]) : hex[int((c & 0x0f))];
		return strHex;
	}

	std::string EncodeHex(const std::string& strIn,  bool bUp, bool bPrefix)
	{
		std::string strHex;
		for(int i = 0; i < strIn.size(); ++i)
		{
			strHex += Bin2Hex(strIn[i], bUp, bPrefix);
		}
		return strHex;
	}

	std::string DecodeHex(const std::string& strIn)
	{
		std::string strTmp(strIn);
		RemoveFlag(strTmp, "0x");
		RemoveFlag(strTmp, "0X");
		std::string deHex(strTmp.size() / 2, ' ');
		char t = 0x00;
		char h = 0x00;
		for(int i = 0; i < strTmp.size(); i += 2)
		{
			h = Hex2Bin(strTmp[i]);
			t = Hex2Bin(strTmp[i + 1]);
			deHex[i/2] = ((h << 4) | t);	
		}

		return deHex;
	}

	static char base64[] = {'A','B','C','D','E','F','G','H',
							'I','J','K','L','M','N','O','P',
							'Q','R','S','T','U','V','W','X',
							'Y','Z','a','b','c','d','e','f',
							'g','h','i','j','k','l','m','n',
							'o','p','q','r','s','t','u','v',
							'w','x','y','z','0','1','2','3',
							'4','5','6','7','8','9','+','/','='};
	
	int B64Code2Index(char c)
	{
		int index = 64;
		if (c >= 'A' && c <= 'Z') index = c - 'A';
		else if (c >= 'a' && c <= 'z') index = 26 + c - 'a';
		else if (c >= '0' && c <= '9') index = 52 + c - '0';
		else if (c == '+') index = 62;
		else if (c == '/') index = 63;
		else if (c == '=') index = 64;
		else index = -1;
		assert(index != -1);
		return index;
	}
	
	std::string EncodeBase64(const std::string& strIn)
	{
		int mod = strIn.size() % 3;
		int step = strIn.size() / 3;
		int dstlen = step * 4 + (mod ? 4 : 0);
		std::string strDst(dstlen, '=');
		int n = 0, m =0;
		for(int i = 0; i < step; ++i)
		{
			n = i * 3;
			m = i * 4;
			strDst[m] = base64[((strIn[n] & 0xfc) >> 2)];
			strDst[m + 1] = base64[((strIn[n] & 0x03) << 4) + ((strIn[n + 1] & 0xf0) >> 4)];
			strDst[m + 2] = base64[((strIn[n + 1] & 0x0f) << 2) + ((strIn[n + 2] & 0xc0) >> 6)];
			strDst[m + 3] = base64[(strIn[n + 2] & 0x3f)];
		}

		n = step * 3;
		m = step * 4;
		if (mod == 1)
		{
			strDst[m] = base64[((strIn[n] & 0xfc) >> 2)];
			strDst[m + 1] = base64[((strIn[n] & 0x03) << 4)];
			strDst[m + 2] = base64[64];
			strDst[m + 3] = base64[64];
		} 
		else if (mod == 2) 
		{
			strDst[m] = base64[((strIn[n] & 0xfc) >> 2)];
			strDst[m + 1] = base64[((strIn[n] & 0x03) << 4) + ((strIn[n + 1] & 0xf0) >> 4)];
			strDst[m + 2] = base64[((strIn[n + 1] & 0x0f) << 2)] ;
			strDst[m + 3] = base64[64];;
		}
		
		return strDst;
	}
	
	std::string DecodeBase64(const std::string& strIn)
	{
		int step = strIn.size() / 4;
		std::string strDst;
		char tmp[4] = {0};
		for(int i = 0; i < step; ++i)
		{
			tmp[0] = B64Code2Index(strIn[i * 4]);
			tmp[1] = B64Code2Index(strIn[i * 4 + 1]);
			tmp[2] = B64Code2Index(strIn[i * 4 + 2]);
			tmp[3] = B64Code2Index(strIn[i * 4 + 3]); 
			strDst += ((tmp[0] << 2) & 0xfc) + ((tmp[1] >> 4) & 0x03);
			strDst += ((tmp[1] << 4) & 0xf0) + ((tmp[2] >> 2) & 0x0f);
			strDst += ((tmp[2] << 6) & 0xc0) +  tmp[3] ;
		}
		
		if(tmp[2] == 64)
		{
			strDst.erase(strDst.size() - 1, 1);
		}
		
		if(tmp[3] == 64)
		{
			strDst.erase(strDst.size() - 1, 1);
		}

		return strDst;
	}

	bool IsNumbers(const std::string& strIn)
	{
		for(int i = 0; i < strIn.size(); ++i)
		{
			if(strIn[i] < '0' && strIn[i] > '9') return false;
		}
		
		return true;
	}

	std::string Join(std::vector<std::string>& vecIn, const std::string& strFlag)
	{
		std::string strOut;
		if(vecIn.size() > 0)
		{
			int i;
			for(i = 0; i < vecIn.size() - 1; ++i) {
				strOut += vecIn[i] + strFlag;
			}
			strOut += vecIn[i];
		}

		return strOut;
	}

}