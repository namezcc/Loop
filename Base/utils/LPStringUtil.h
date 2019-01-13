#ifndef LP_STRING_TUIL_H
#define LP_STRING_TUIL_H
#include <string>
#include <vector>
#include <sstream>

using namespace std;

namespace Loop{

	template<typename>
	struct cvto;
	template<>
	struct cvto<string>
	{
		template<typename T>
		static typename std::enable_if<!std::is_same<T, string>::value,string>::type To(T t)
		{
			return std::to_string(t);
		}
		template<typename T>
		static typename std::enable_if<std::is_same<T, string>::value, string>::type To(T t)
		{
			return t;
		}
	};

	template<>
	struct cvto<int>
	{
		static int To(string s)
		{
			return std::stoi(s);
		}
	};

	template<>
	struct cvto<long>
	{
		static long To(string s)
		{
			return std::stol(s);
		}
	};

	template<>
	struct cvto<float>
	{
		static float To(string s)
		{
			return std::stof(s);
		}
	};

	template<>
	struct cvto<double>
	{
		static double To(string s)
		{
			return std::stod(s);
		}
	};

	template<typename R,typename T>
	static R Cvto(T t)
	{
		return cvto<R>::To(t);
	}

	template<typename T>
	static vector<T> SplitVec(string& str, const std::string& delimiter = ",")
	{
		vector<T> res;
		string::size_type pos1, pos2;
		pos2 = str.find(delimiter);
		pos1 = 0;
		while (string::npos != pos2)
		{
			if (pos2>pos1)
				res.push_back(Cvto<T>(str.substr(pos1, pos2 - pos1)));

			pos1 = pos2 + delimiter.size();
			pos2 = str.find(delimiter, pos1);
		}
		if (pos1 != str.length())
			res.push_back(Cvto<T>(str.substr(pos1)));

		return res;
	}

	template<typename T>
	static void SplitVec(string& str, vector<T> res, const std::string& delimiter = ",")
	{
		string::size_type pos1, pos2;
		pos2 = str.find(delimiter);
		pos1 = 0;
		while (string::npos != pos2)
		{
			if (pos2>pos1)
				res.push_back(Cvto<T>(str.substr(pos1, pos2 - pos1)));

			pos1 = pos2 + delimiter.size();
			pos2 = str.find(delimiter, pos1);
		}
		if (pos1 != str.length())
			res.push_back(Cvto<T>(str.substr(pos1)));
	}

	static void Spliteach(const string& str, const char* delimiter, std::vector<std::string>& res)
	{
		auto p = str.c_str();
		size_t last = 0;
		for (size_t i = 0; i < str.size(); i++)
		{
			auto c = delimiter;
			while (*c != '\0')
			{
				if (*p == *c++)
				{
					if (i >last)
						res.push_back(move(string(str.c_str() + last, str.c_str() + i)));
					last = i + 1;
					break;
				}
			}
			++p;
		}
		if (str.size()>last)
			res.push_back(move(string(str.c_str() + last, str.c_str() + str.size())));
	}

	static void Split(const std::string& str, const std::string& delimiter, std::vector<std::string>& res)
	{
		string::size_type pos1, pos2;
		pos2 = str.find(delimiter);
		pos1 = 0;
		while (string::npos != pos2)
		{
			if (pos2>pos1)
				res.push_back(str.substr(pos1, pos2 - pos1));

			pos1 = pos2 + delimiter.size();
			pos2 = str.find(delimiter, pos1);
		}
		if (pos1 != str.length())
			res.push_back(str.substr(pos1));
	}
}

template<typename R, typename T>
static R loop_cast(T&& t)
{
	std::stringstream s;
	R r;
	s << t;
	s >> r;
	return r;
}

struct Slice
{
	Slice(char* nb,char* ne) :_pb(nb), _pe(ne)
	{};
	Slice(const char* nb,const char* ne)
	{
		_pb = (char*)nb;
		_pe = (char*)ne;
	};
	string CutWord()
	{
		Slice s = CutWordSlice();
		return string(s._pb, s._pe);
	}

	Slice CutWordSlice()
	{
		char *b = _pb;
		while (_pb < _pe && isspace(*b)) ++b;
		char *e = b;
		while (e < _pe && !isspace(*e)) ++e;
		_pb = e;
		return Slice(b, e);
	}

	string CutToChar(char c)
	{
		char* e = _pb;
		while (e < _pe && *e != c) e++;
		char* b = _pb;
		_pb = e;
		return string(b, e);
	}

	void CutHead(size_t n)
	{
		while (n > 0 && _pb < _pe)
		{
			--n;
			++_pb;
		}
	}

	bool Over()
	{
		return _pb == _pe;
	}

protected:
	char* _pb;
	char* _pe;
};

#endif