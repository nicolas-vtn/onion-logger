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
						   const std::string& appName,
						   bool makeConsoleRed)
		: m_ConsoleBuf(consoleBuf), m_FileBuf(fileBuf), m_Level(level), m_AppName(appName),
		  m_MakeConsoleRed(makeConsoleRed)
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
		std::string prefix;
		if (m_AppName.empty())
			prefix = GetTimestamp() + " [T:" + GetThreadId() + "] : " + m_Level + " : ";
		else
		{
			prefix = GetTimestamp() + " [" + m_AppName + "][T:" + GetThreadId() + "] : " + m_Level + " : ";
		}

		WriteString(m_ConsoleBuf, prefix);
		WriteString(m_FileBuf, prefix);
	}

	void Logger::TeeBuf::WriteString(std::streambuf* buf, const std::string& str)
	{
		for (char ch : str)
			buf->sputc(ch);
	}

	// ---------------- Logger ----------------

	Logger::Logger(const std::filesystem::path& logFilePath, const std::string& appName)
		: Logger(logFilePath, logFilePath, appName)
	{
	}

	Logger::Logger(const std::filesystem::path& logInfosFilePath,
				   const std::filesystem::path& logErrorsFilePath,
				   const std::string& appName)
		: m_LogInfosFilePath(logInfosFilePath), m_LogInfosFile(m_LogInfosFilePath, std::ios::app),
		  m_LogErrorsFilePath(logErrorsFilePath), m_LogErrorsFile(m_LogErrorsFilePath, std::ios::app),
		  m_AppName(appName)
	{
		SetupBuffers();
	}

	Logger::~Logger()
	{
		if (m_OldCoutBuf)
			std::cout.rdbuf(m_OldCoutBuf);

		if (m_OldCerrBuf)
			std::cerr.rdbuf(m_OldCerrBuf);
	}

	void Logger::SetAppName(const std::string& appName)
	{
		m_AppName = appName;
		if (m_CoutTee)
			m_CoutTee->SetAppName(appName);
		if (m_CerrTee)
			m_CerrTee->SetAppName(appName);
	}

	std::string Logger::GetAppName() const
	{
		return m_AppName;
	}

	void Logger::SetLogInfos(bool logInfos)
	{
		m_LogInfos = logInfos;
		SetupBuffers();
	}

	bool Logger::GetLogInfos() const
	{
		return m_LogInfos;
	}

	void Logger::SetLogInfosFilePath(const std::filesystem::path& filePath)
	{
		// Backup the State
		bool wasLoggingInfos = m_LogInfos;

		// Stop Logging
		if (wasLoggingInfos)
		{
			SetLogInfos(false);
		}

		// Change the file Buffer
		m_LogInfosFilePath = filePath;
		m_LogInfosFile = std::ofstream(m_LogInfosFilePath, std::ios::app);

		// Resume Logging
		if (wasLoggingInfos)
		{
			SetLogInfos(true);
		}
	}

	void Logger::SetLogErrors(bool logErrors)
	{
		m_LogErrors = logErrors;
		SetupBuffers();
	}

	bool Logger::GetLogErrors() const
	{
		return m_LogErrors;
	}

	void Logger::SetLogErrorsFilePath(const std::filesystem::path& filePath)
	{
		// Backup the State
		bool wasLoggingErrors = m_LogErrors;

		// Stop Logging
		if (wasLoggingErrors)
		{
			SetLogErrors(false);
		}

		// Change the file Buffer
		m_LogErrorsFilePath = filePath;
		m_LogErrorsFile = std::ofstream(m_LogErrorsFilePath, std::ios::app);

		// Resume Logging
		if (wasLoggingErrors)
		{
			SetLogErrors(true);
		}
	}

	void Logger::SetupBuffers()
	{
		// Reset previously initialized buffers if not needed anymore
		if (!m_LogInfos && m_CoutTee != nullptr)
		{
			m_CoutTee.reset();
			std::cout.rdbuf(m_OldCoutBuf);
			m_OldCoutBuf = nullptr;
		}

		if (!m_LogErrors && m_CerrTee != nullptr)
		{
			m_CerrTee.reset();
			std::cerr.rdbuf(m_OldCerrBuf);
			m_OldCerrBuf = nullptr;
		}

		// Initialize Buffers
		if (m_LogInfos && m_CoutTee == nullptr)
		{
			m_CoutTee = std::make_unique<TeeBuf>(std::cout.rdbuf(), m_LogInfosFile.rdbuf(), "LOG", m_AppName, false);
			m_OldCoutBuf = std::cout.rdbuf(m_CoutTee.get());

			if (!m_LogInfosFile.is_open())
				throw std::runtime_error("Failed to open log file: " + m_LogInfosFilePath.string());
		}

		if (m_LogErrors && m_CerrTee == nullptr)
		{
			m_CerrTee = std::make_unique<TeeBuf>(std::cerr.rdbuf(), m_LogErrorsFile.rdbuf(), "ERR", m_AppName, true);
			m_OldCerrBuf = std::cerr.rdbuf(m_CerrTee.get());

			if (!m_LogErrorsFile.is_open())
				throw std::runtime_error("Failed to open log file: " + m_LogErrorsFilePath.string());
		}
	}

} // namespace onion
