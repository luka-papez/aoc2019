#pragma once

#include <algorithm>
#include <memory>

#include <map>
#include <vector>

#include "input_utilities.hpp"

using memory_t = std::map<int64_t, int64_t>;
enum class mode_t
{
	positional = 0,
	immediate,
	relative
};

enum class execution_state_t
{
	normal = 0,
	halted,
	consumed_value,
	provided_value,
	requested_value
};

struct InstructionParameter
{
	mode_t m_mode;

	InstructionParameter(mode_t mode) : m_mode(mode) {}

	int64_t get_value(int64_t word, memory_t& memory, int64_t relative_base) const
	{
		if (m_mode == mode_t::positional)
			return memory[word];

		else if (m_mode == mode_t::relative)
			return memory[word + relative_base];

		return word;
	}

	int64_t get_dest(int64_t word, int64_t relative_base) const
	{
		if (m_mode == mode_t::positional)
			return word;

		else if (m_mode == mode_t::relative)
			return word + relative_base;

		throw std::runtime_error("invalid mode");
	}
};

using parameters_t = std::vector<InstructionParameter>;

struct IntcodeVM;
struct IntcodeInstruction
{
public:
	int64_t size;
	int64_t ip_increment;

	IntcodeInstruction(int64_t size) : size(size), ip_increment(size) { }
	IntcodeInstruction(int64_t size, int64_t ip_increment) : size(size), ip_increment(ip_increment) { }

	execution_state_t execute(IntcodeVM& vm, int64_t& output, int64_t input, parameters_t& parameters);
	virtual std::string to_string() = 0;

protected:
	virtual execution_state_t execute_specific(IntcodeVM& vm, int64_t& output, int64_t input, parameters_t& parameters, std::vector<int64_t>& arguments) = 0;
};

struct IntcodeVM
{
	int64_t instruction_pointer;
	int64_t relative_base;

	memory_t memory;

	IntcodeVM(memory_t& memory, int64_t ip = 0) :
		memory(memory), instruction_pointer(ip), relative_base(0) {}

	IntcodeVM(std::vector<int64_t>& memory_, int64_t ip = 0) :
		instruction_pointer(ip), relative_base(0)
	{
		for (size_t i = 0; i < memory_.size(); i++)
			memory[static_cast<int64_t>(i)] = memory_[i];
	}

	IntcodeVM(const std::string& input_filepath, int64_t ip = 0)
		: instruction_pointer(ip), relative_base(0)
	{
		int64_t i = 0;

		for (auto line : next_file_line(input_filepath))
		{
			for (auto word : next_line_token<int64_t>(line))
			{
				memory[i++] = word;
			}
		}
	}

	static std::pair<std::shared_ptr<IntcodeInstruction>, parameters_t> IntcodeVM::parse_word(int64_t word);

	execution_state_t run(int64_t& output, int64_t input, bool request_input = false)
	{
		execution_state_t state = execution_state_t::normal;
		do
		{
			auto[instruction, parameters] = parse_word(memory[instruction_pointer]);

			if (request_input && instruction->to_string() == "Input")
			{
				state = execution_state_t::requested_value;
				break;
			}

			state = instruction->execute(*this, output, input, parameters);
		} while (state == execution_state_t::normal);

		return state;
	}

	execution_state_t run(int64_t& output, bool request_input = false)
	{
		int64_t dummy_input = 0;

		return run(output, dummy_input, request_input);
	}

	execution_state_t step(int64_t& output, int64_t input)
	{
		auto[instruction, parameters] = parse_word(memory[instruction_pointer]);

		return instruction->execute(*this, output, input, parameters);
	}

	template <typename InputContainer, typename OutputContainer>
	int64_t run_on(OutputContainer& output, InputContainer& input)
	{
		int64_t vm_output = 0;
		size_t consumed = 0;
		execution_state_t state{};

		while ((state = run(vm_output, input[consumed], input.size() <= consumed)) != execution_state_t::halted)
		{
			switch (state)
			{
			case execution_state_t::consumed_value:
				consumed++;
				break;
			case execution_state_t::provided_value:
				output.push_back(static_cast<OutputContainer::value_type>(vm_output));
				break;
			case execution_state_t::requested_value:
				return vm_output;
			default:
				break;
			}
		}

		return vm_output;
	}

	template <typename InputContainer>
	int64_t run_on(InputContainer& input)
	{
		std::vector<int64_t> output;

		return run_on(output, input);
	}
};

#define SRC(x) int64_t src##x = parameters[x - 1].get_value(arguments[x - 1], vm.memory, vm.relative_base)
#define LOC(x) int64_t dest = parameters[x - 1].get_dest(arguments[x - 1], vm.relative_base)

struct Add : public IntcodeInstruction
{
	Add(int64_t size) : IntcodeInstruction(size) {}

	virtual execution_state_t execute_specific(IntcodeVM& vm, int64_t& output, int64_t input, parameters_t& parameters, std::vector<int64_t>& arguments)
	{
		auto& memory = vm.memory;

		SRC(1);
		SRC(2);
		LOC(3);

		vm.memory[dest] = src1 + src2;

		return execution_state_t::normal;
	}

	virtual std::string to_string()
	{
		return "Add";
	}
};

struct Multiply : public IntcodeInstruction
{
	Multiply(int64_t size) : IntcodeInstruction(size) {}

	virtual execution_state_t execute_specific(IntcodeVM& vm, int64_t& output, int64_t input, parameters_t& parameters, std::vector<int64_t>& arguments)
	{
		auto& memory = vm.memory;

		SRC(1);
		SRC(2);
		LOC(3);

		vm.memory[dest] = src1 * src2;

		return execution_state_t::normal;
	}

	virtual std::string to_string()
	{
		return "Multiply";
	}
};

struct JumpIfTrue : public IntcodeInstruction
{
	JumpIfTrue(int64_t size) : IntcodeInstruction(size, 0) {}

	virtual execution_state_t execute_specific(IntcodeVM& vm, int64_t& output, int64_t input, parameters_t& parameters, std::vector<int64_t>& arguments)
	{
		auto& memory = vm.memory;

		SRC(1);
		SRC(2);

		vm.instruction_pointer = (src1 > 0) ? src2 : (vm.instruction_pointer + size);

		return execution_state_t::normal;
	}

	virtual std::string to_string()
	{
		return "JumpIfTrue";
	}
};

struct JumpIfFalse : public IntcodeInstruction
{
	JumpIfFalse(int64_t size) : IntcodeInstruction(size, 0) {}

	virtual execution_state_t execute_specific(IntcodeVM& vm, int64_t& output, int64_t input, parameters_t& parameters, std::vector<int64_t>& arguments)
	{
		auto& memory = vm.memory;

		SRC(1);
		SRC(2);

		vm.instruction_pointer = (src1 == 0) ? src2 : (vm.instruction_pointer + size);

		return execution_state_t::normal;
	}

	virtual std::string to_string()
	{
		return "JumpIfFalse";
	}
};

struct LessThan : public IntcodeInstruction
{
	LessThan(int64_t size) : IntcodeInstruction(size) {}

	virtual execution_state_t execute_specific(IntcodeVM& vm, int64_t& output, int64_t input, parameters_t& parameters, std::vector<int64_t>& arguments)
	{
		auto& memory = vm.memory;

		SRC(1);
		SRC(2);
		LOC(3);

		vm.memory[dest] = (src1 < src2) ? 1 : 0;

		return execution_state_t::normal;
	}

	virtual std::string to_string()
	{
		return "LessThan";
	}
};

struct Equals : public IntcodeInstruction
{
	Equals(int64_t size) : IntcodeInstruction(size) {}

	virtual execution_state_t execute_specific(IntcodeVM& vm, int64_t& output, int64_t input, parameters_t& parameters, std::vector<int64_t>& arguments)
	{
		auto& memory = vm.memory;

		SRC(1);
		SRC(2);
		LOC(3);

		vm.memory[dest] = (src1 == src2) ? 1 : 0;

		return execution_state_t::normal;
	}

	virtual std::string to_string()
	{
		return "Equals";
	}
};

struct AdjustBase : public IntcodeInstruction
{
	AdjustBase(int64_t size) : IntcodeInstruction(size) {}

	virtual execution_state_t execute_specific(IntcodeVM& vm, int64_t& output, int64_t input, parameters_t& parameters, std::vector<int64_t>& arguments)
	{
		SRC(1);

		vm.relative_base += src1;

		return execution_state_t::normal;
	}

	virtual std::string to_string()
	{
		return "AdjustBase";
	}
};

struct Input : public IntcodeInstruction
{
	Input(int64_t size) : IntcodeInstruction(size) {}

	virtual execution_state_t execute_specific(IntcodeVM& vm, int64_t& output, int64_t input, parameters_t& parameters, std::vector<int64_t>& arguments)
	{
		LOC(1);

		vm.memory[dest] = input;

		return execution_state_t::consumed_value;
	}

	virtual std::string to_string()
	{
		return "Input";
	}
};

struct Output : public IntcodeInstruction
{
	Output(int64_t size) : IntcodeInstruction(size) {}

	virtual execution_state_t execute_specific(IntcodeVM& vm, int64_t& output, int64_t input, parameters_t& parameters, std::vector<int64_t>& arguments)
	{
		SRC(1);

		output = src1;

		return execution_state_t::provided_value;
	}

	virtual std::string to_string()
	{
		return "Output";
	}
};

struct Halt : public IntcodeInstruction
{
	Halt(int64_t size) : IntcodeInstruction(size) {}

	virtual execution_state_t execute_specific(IntcodeVM& vm, int64_t& output, int64_t input, parameters_t& parameters, std::vector<int64_t>& arguments)
	{
		return execution_state_t::halted;
	}

	virtual std::string to_string()
	{
		return "Halt";
	}
};

void get_digits(int64_t value, std::vector<char>& digits, int64_t n = 7)
{
	for (auto divider = static_cast<int64_t>(std::pow(10, n - 1)); divider; value %= divider, divider /= 10)
	{
		digits.push_back(static_cast<char>(value / divider));
	}
}

std::pair<std::shared_ptr<IntcodeInstruction>, parameters_t> IntcodeVM::parse_word(int64_t word)
{
	static std::map<int64_t, std::shared_ptr<IntcodeInstruction>> instructions = {
		{ 1,  std::make_shared<Add>(4) },
		{ 2,  std::make_shared<Multiply>(4) },
		{ 3,  std::make_shared<Input>(2) },
		{ 4,  std::make_shared<Output>(2) },
		{ 5,  std::make_shared<JumpIfTrue>(3) },
		{ 6,  std::make_shared<JumpIfFalse>(3) },
		{ 7,  std::make_shared<LessThan>(4) },
		{ 8,  std::make_shared<Equals>(4) },
		{ 9,  std::make_shared<AdjustBase>(2) },
		{ 99, std::make_shared<Halt>(1) },
	};

	int64_t opcode = word % 100;
	int64_t argument_modes = word / 100;

	if (!instructions.count(opcode))
		throw std::runtime_error("illegal instruction encountered");

	auto instruction = instructions[opcode];

	std::vector<char> modes;
	get_digits(argument_modes, modes, instruction->size - 1);
	std::reverse(modes.begin(), modes.end());

	parameters_t parameters;
	std::transform(modes.begin(), modes.end(), std::back_inserter(parameters), [](char m)
	{
		return InstructionParameter(static_cast<mode_t>(m));
	});

	return { instruction, parameters };
}

execution_state_t IntcodeInstruction::execute(IntcodeVM& vm, int64_t& output, int64_t input, parameters_t& parameters)
{
	std::vector<int64_t> arguments;

	for (int64_t i = 0; i < size - 1; i++)
	{
		arguments.push_back(vm.memory[vm.instruction_pointer + 1 + i]);
	}

	execution_state_t state = execute_specific(vm, output, input, parameters, arguments);

	vm.instruction_pointer += ip_increment;

	return state;
}

using position_t = std::pair<int8_t, int8_t>;

template <typename PositionType>
void display(std::map<PositionType, char>& screen, bool extra_space = true)
{
	auto compare_y = [](const auto& p1, const auto& p2) { return p1.first.second < p2.first.second; };
	auto min_x = std::min_element(screen.begin(), screen.end())->first.first;
	auto min_y = std::min_element(screen.begin(), screen.end(), compare_y)->first.second;
	auto max_x = std::max_element(screen.begin(), screen.end())->first.first;
	auto max_y = std::max_element(screen.begin(), screen.end(), compare_y)->first.second;

	for (auto y = min_y; y <= max_y; y++)
	{
		for (auto x = min_x; x <= max_x; x++)
		{
			auto val = screen[{ x, y }];

			std::cout << (val ? val : ' ');
			
			if (extra_space)
				std::cout << ' ';
		}

		std::cout << std::endl;
	}

	std::cout << std::endl;
}

template <typename Container>
void display(Container& world)
{
	for (auto& c : world)
		std::cout << static_cast<char>(c);
}