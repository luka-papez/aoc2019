#pragma once

#include <experimental/generator>
#include <sstream>
#include <fstream>

template <typename DataType>
DataType lexical_cast(const std::string& data_str)
{
	if constexpr (std::is_same_v<DataType, std::string>)
		return data_str;

	std::istringstream iss(data_str);
	DataType data;

	if (!(iss >> data))
		throw std::runtime_error("could not parse input as a required type");

	return data;
}

template <typename StreamType, typename DataType>
std::experimental::generator<DataType> next_stream_token(
	const std::string& stream_target,
	char delimiter
)
{
	StreamType stream(stream_target);
	std::string token;

	while (std::getline(stream, token, delimiter))
	{
		co_yield lexical_cast<DataType>(token);
	}
}

template <typename DataType = std::string>
std::experimental::generator<DataType> next_file_line(const std::string& filename)
{
	for (auto line : next_stream_token<std::ifstream, DataType>(filename, '\n'))
		co_yield line;
}

template <typename DataType = std::string>
std::experimental::generator<DataType> next_line_token(const std::string& line, char delimiter = ',')
{
	for (auto token : next_stream_token<std::istringstream, DataType>(line, delimiter))
		co_yield token;
}