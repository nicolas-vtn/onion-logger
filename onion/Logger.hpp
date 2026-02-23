#pragma once

#include <fstream>
#include <streambuf>
#include <string>

namespace onion
{
	class Logger
	{
	  private:
		class TeeBuf : public std::streambuf
		{
		  public:
			TeeBuf(std::streambuf* consoleBuf, std::streambuf* fileBuf, const std::string& level, bool makeConsoleRed);

		  protected:
			int overflow(int c) override;
			int sync() override;

		  private:
			static std::string GetTimestamp();
			static std::string GetThreadId();

			void WritePrefix();
			static void WriteString(std::streambuf* buf, const std::string& str);

		  private:
			std::streambuf* m_ConsoleBuf;
			std::streambuf* m_FileBuf;

			std::string m_Level;
			bool m_MakeConsoleRed;

			bool m_AtLineStart = true;
		};

	  public:
		Logger(const Logger&) = delete;
		Logger& operator=(const Logger&) = delete;

		Logger(const std::string& logFilePath);
		~Logger();

	  private:
		std::string m_LogFilePath;
		std::ofstream m_LogFile;

		TeeBuf m_CoutTee;
		TeeBuf m_CerrTee;

		std::streambuf* m_OldCoutBuf = nullptr;
		std::streambuf* m_OldCerrBuf = nullptr;
	};

} // namespace onion
