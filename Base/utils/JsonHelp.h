#ifndef JSON_HELP_H
#define JSON_HELP_H

#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include <stdexcept>

using namespace rapidjson;

class JsonHelp
{
public:
	JsonHelp()
	{}

	bool ParseFile(const std::string& file)
	{
		FILE* fp = fopen(file.c_str(), "r");
		if (fp == NULL)
			throw std::logic_error("JsonHelp open file error");

		char readBuffer[65535];
		FileReadStream is(fp, readBuffer, 65535);
		m_doc.ParseStream(is);
		fclose(fp);
		if (m_doc.HasParseError())
		{
			fprintf(stdout, "Error (%u):%s\n", static_cast<unsigned>(m_doc.GetErrorOffset()), GetParseError_En(m_doc.GetParseError()));
			return false;
		}
		return true;
	}

	bool ParseString(const std::string& buff)
	{
		m_doc.Parse(buff.c_str());
		if (m_doc.HasParseError())
		{
			fprintf(stdout, "Error (%u):%s\n", static_cast<unsigned>(m_doc.GetErrorOffset()), GetParseError_En(m_doc.GetParseError()));
			return false;
		}
		return true;
	}

	~JsonHelp()
	{
	};

	bool WriteTo(const std::string& file)
	{
		FILE* fp = fopen(file.c_str(), "w");
		if (fp == NULL)
			return false;

		char buffer[65535];
		FileWriteStream os(fp, buffer, 65535);
		Writer<FileWriteStream> writer(os);
		m_doc.Accept(writer);
		fclose(fp);
		return true;
	}

	Value* GetMember(const std::string& member)
	{
		Value::MemberIterator it = m_doc.FindMember(member.c_str());
		if (it == m_doc.MemberEnd())
			return NULL;
		return &it->value;
	}

	std::string GetJsonStr()
	{
		StringBuffer buffer;
		Writer<StringBuffer> writer(buffer);
		m_doc.Accept(writer);
		return buffer.GetString();
	}

	Document& GetDocument()
	{
		return m_doc;
	}

private:
	Document m_doc;
};

#endif
