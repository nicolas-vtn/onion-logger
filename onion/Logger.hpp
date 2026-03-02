#pragma once

#include <filesystem>
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

		// ------ Constructors / Destructors ------
	  public:
		Logger(const Logger&) = delete;
		Logger& operator=(const Logger&) = delete;

		Logger(const std::filesystem::path& logFilePath);
		Logger(const std::filesystem::path& logInfosFilePath, const std::filesystem::path& logErrorsFilePath);
		~Logger();

		// ------ Getters / Setters ------
	  public:
		void SetLogInfos(bool logInfos);
		bool GetLogInfos() const;
		void SetLogInfosFilePath(const std::filesystem::path& filePath);

		void SetLogErrors(bool logErrors);
		bool GetLogErrors() const;
		void SetLogErrorsFilePath(const std::filesystem::path& filePath);

		// ------ Setup / Initializations ------
	  private:
		void SetupBuffers();

		// ------ Private Members ------
	  private:
		bool m_LogErrors = true;
		bool m_LogInfos = true;

		std::filesystem::path m_LogInfosFilePath;
		std::filesystem::path m_LogErrorsFilePath;

		std::ofstream m_LogInfosFile;
		std::ofstream m_LogErrorsFile;

		std::unique_ptr<TeeBuf> m_CoutTee;
		std::unique_ptr<TeeBuf> m_CerrTee;

		std::streambuf* m_OldCoutBuf = nullptr;
		std::streambuf* m_OldCerrBuf = nullptr;
	};

} // namespace onion
