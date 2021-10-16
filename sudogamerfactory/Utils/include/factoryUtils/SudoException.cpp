
#include "SudoException.h"
#include <sstream>


SudoException::SudoException( int line,const char* file ) noexcept
	:
	line( line ),
	file( file )
{}

const char* SudoException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* SudoException::GetType() const noexcept
{
	return "SudoGamer Exception";
}

int SudoException::GetLine() const noexcept
{
	return line;
}

const std::string& SudoException::GetFile() const noexcept
{
	return file;
}

std::string SudoException::GetOriginString() const noexcept
{
	std::ostringstream oss;
	oss << "[File] " << file << std::endl
		<< "[Line] " << line;
	return oss.str();
}