#include "Logger.hpp"

#include <onion/DateTime.hpp>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <thread>

namespace onion
{
	// ---------------- TeeBuf ----------------

	Logger::TeeBuf::TeeBuf(std::streambuf* consoleBuf,
						   std::streambuf* fileBuf,
						   const std::string& level,
						   bool makeConsoleRed)
		: m_ConsoleBuf(consoleBuf), m_FileBuf(fileBuf), m_Level(level), m_MakeConsoleRed(makeConsoleRed)
	{
	}

	std::string Logger::TeeBuf::GetTimestamp()
	{
		return DateTime::UtcNow().toString("%d-%m-%Y %H:%M:%S");
	}

	std::string Logger::TeeBuf::GetThreadId()
	{
		std::ostringstream oss;
		oss << std::this_thread::get_id();
		return oss.str();
	}

	int Logger::TeeBuf::overflow(int c)
	{
		using traits = std::streambuf::traits_type;
		if (traits::eq_int_type(c, traits::eof()))
			return traits::not_eof(c);

		if (m_AtLineStart)
		{
			if (m_MakeConsoleRed)
				WriteString(m_ConsoleBuf, "\033[31m");

			WritePrefix();
			m_AtLineStart = false;
		}

		const int r1 = m_ConsoleBuf->sputc(c);
		const int r2 = m_FileBuf->sputc(c);

		if (c == '\n')
		{
			if (m_MakeConsoleRed)
				WriteString(m_ConsoleBuf, "\033[0m");

			m_AtLineStart = true;

			m_ConsoleBuf->pubsync();
			m_FileBuf->pubsync();
		}

		return (r1 == EOF || r2 == EOF) ? EOF : c;
	}

	int Logger::TeeBuf::sync()
	{
		int const r1 = m_ConsoleBuf->pubsync();
		int const r2 = m_FileBuf->pubsync();
		return (r1 == 0 && r2 == 0) ? 0 : -1;
	}

	void Logger::TeeBuf::WritePrefix()
	{
		std::string prefix = GetTimestamp() + " [T:" + GetThreadId() + "] : " + m_Level + " : ";

		WriteString(m_ConsoleBuf, prefix);
		WriteString(m_FileBuf, prefix);
	}

	void Logger::TeeBuf::WriteString(std::streambuf* buf, const std::string& str)
	{
		for (char ch : str)
			buf->sputc(ch);
	}

	// ---------------- Logger ----------------

	Logger::Logger(const std::string& logFilePath)
		: m_LogFilePath(logFilePath), m_LogFile(logFilePath, std::ios::app),
		  m_CoutTee(std::cout.rdbuf(), m_LogFile.rdbuf(), "LOG", false),
		  m_CerrTee(std::cerr.rdbuf(), m_LogFile.rdbuf(), "ERR", true)
	{
		if (!m_LogFile.is_open())
			throw std::runtime_error("Failed to open log file: " + logFilePath);

		m_OldCoutBuf = std::cout.rdbuf(&m_CoutTee);
		m_OldCerrBuf = std::cerr.rdbuf(&m_CerrTee);
	}

	Logger::~Logger()
	{
		std::cout.rdbuf(m_OldCoutBuf);
		std::cerr.rdbuf(m_OldCerrBuf);
	}

} // namespace onion
