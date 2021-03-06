#include <algorithm>
#include <cctype>
#include <functional>
#include <iomanip>
#include <numeric>

#include <array>
#include <bitset>
#include <iostream>
#include <map>
#include <set>
#include <unordered_set>
#include <string>
#include <vector>
#include <queue>
#include <deque>
#include <utility>
#include <optional>
#include <thread>
#include <tuple>
#include <mutex>
#include <regex>

#include "intcode.hpp"
#include "input_utilities.hpp"
#include "search_algorithms.hpp"

std::pair<int64_t, int64_t> day_1(const std::string& input_filepath)
{
	int64_t module_fuel_requirements{};
	int64_t fuel_fuel_requirements{};

	for (auto module_weight : next_file_line<int64_t>(input_filepath))
	{
		int64_t module_fuel_requirement = module_weight / 3 - 2;

		module_fuel_requirements += module_fuel_requirement;

		int64_t module_fuel_weight = module_weight;
		while ((module_fuel_weight = module_fuel_weight / 3 - 2) && module_fuel_weight > 0)
		{
			fuel_fuel_requirements += module_fuel_weight;
		}
	}

	return { module_fuel_requirements, fuel_fuel_requirements };
}

std::pair<int64_t, int64_t> day_2(const std::string& input_filepath)
{
	std::vector<int64_t> opcodes;

	for (auto line : next_file_line(input_filepath))
		for (auto opcode : next_line_token<int64_t>(line))
			opcodes.push_back(opcode);

	int64_t dummy;

	IntcodeVM part1(opcodes);
	part1.memory[1] = 12;
	part1.memory[2] = 2;
	part1.run(dummy);

	for (int64_t i = 0; i < 100; i++)
	{
		for (int64_t j = 0; j < 100; j++)
		{
			IntcodeVM part2(opcodes);
			part2.memory[1] = i;
			part2.memory[2] = j;
			part2.run(dummy);

			if (part2.memory[0] == 19690720)
				return { part1.memory[0], 100 * i + j };
		}
	}

	throw std::invalid_argument("the program has no solution");
}


std::pair<int64_t, int64_t> day_3(const std::string& input_filepath)
{
	using point_t   = std::pair<int64_t, int64_t>;
	using segment_t = std::pair<point_t, point_t>;
	using wire_t    = std::vector<segment_t>;

	std::vector<wire_t> wires;
	std::vector<point_t> intersections;

	auto segments_intersect = [](const segment_t& s1, const segment_t& s2) -> std::pair<bool, point_t>
	{
		point_t origin = { 0, 0 };

		if (s1.first.second != s1.second.second)
			return { false, origin };

		if (s2.first.first != s2.second.first)
			return { false, origin };

		if (s1.first == origin && s2.first == origin)
			return { false, origin };

		if (s1.first.first <= s2.first.first &&
			s1.second.first >= s2.second.first &&
			s1.first.second >= s2.first.second &&
			s1.second.second <= s2.second.second
			)
		{
			return { true, { s2.first.first, s1.first.second } };
		}

		return { false, origin };
	};

	for (auto wire : next_file_line(input_filepath))
	{
		point_t start = { 0, 0 };
		point_t end = { 0, 0 };

		wires.push_back({});

		for (auto segment : next_line_token(wire))
		{
			auto direction = segment[0];
			auto distance  = lexical_cast<size_t>(segment.substr(1));

			switch (direction)
			{
			case 'U':
				end.second += distance;
				break;

			case 'D':
				end.second -= distance;
				break;

			case 'R':
				end.first += distance;
				break;

			case 'L':
				end.first -= distance;
				break;

			default:
				throw std::invalid_argument("invalid input file");
			}

			// make sure segments go left to right or down to up
			wires.back().push_back({ std::min(start, end), std::max(start, end) });
			start = end;
		}

		for (size_t i = 0; i < wires.size() - 1; i++)
		{
			for (const auto& segment1 : wires[i])
			{
				for (const auto& segment2 : wires.back())
				{
					if (auto[intersect, intersection] = segments_intersect(segment1, segment2); intersect)
					{
						intersections.push_back(intersection);
					}

					else if (auto[intersect, intersection] = segments_intersect(segment2, segment1); intersect)
					{
						intersections.push_back(intersection);
					}
				}
			}
		}
	}

	auto is_along_the_segment = [](const point_t& point, const segment_t& segment)
	{
		// left to right
		if (segment.first.second == segment.second.second)
		{
			if (point.second == segment.first.second)
			{
				return point.first >= segment.first.first && point.first <= segment.second.first;
			}
		}

		// down to up
		else if (segment.first.first == segment.second.first)
		{
			if (point.first == segment.first.first)
			{
				return point.second >= segment.first.second && point.second <= segment.second.second;
			}
		}

		else
			throw std::runtime_error("invalid wire configuration");

		return false;
	};
	auto segment_length = [](const segment_t& segment)
	{
		// left to right
		if (segment.first.second == segment.second.second)
		{
			return std::abs(segment.second.first - segment.first.first);
		}

		// down to up
		else if (segment.first.first == segment.second.first)
		{
			return std::abs(segment.second.second - segment.first.second);
		}

		else
			throw std::runtime_error("invalid wire configuration");
	};

	std::vector<std::vector<int64_t>> total_steps;

	for (const auto& intersection : intersections)
	{
		std::vector<int64_t> steps_to_intersection;

		for (const auto& wire : wires)
		{
			uint64_t steps_to_current_intersection = 0;

			segment_t last_segment = { { 0, 0 }, { 0, 0} };
			for (const auto& segment : wire)
			{
				if (is_along_the_segment(intersection, segment))
				{
					point_t starting_point = segment.first;

					if (!(starting_point == last_segment.first || starting_point == last_segment.second))
					{
						starting_point = segment.second;
					}

					steps_to_current_intersection += segment_length({ starting_point, intersection });

					break;
				}

				else
				{
					steps_to_current_intersection += segment_length(segment);
				}

				last_segment = segment;
			}

			steps_to_intersection.push_back(steps_to_current_intersection);
		}

		total_steps.emplace_back(steps_to_intersection);
	}

	std::vector<int64_t> signal_delays;
	std::transform(total_steps.begin(), total_steps.end(), std::back_inserter(signal_delays),
		[](const auto& circuit) 
	{
		return std::accumulate(circuit.begin(), circuit.end(), int64_t{});
	});

	auto manhattan_distance_from_origin = [](const point_t& p)
	{
		return std::abs(p.first) + std::abs(p.second);
	};

	return { 
		manhattan_distance_from_origin(
			*std::min_element(intersections.begin(), intersections.end(),  
				[=](const auto& p1, const auto& p2) {
					return std::less<int64_t>()(
						manhattan_distance_from_origin(p1),
						manhattan_distance_from_origin(p2)
					);
				}
			)
		),
		*std::min_element(signal_delays.begin(), signal_delays.end())
	};
}

std::pair<int64_t, int64_t> day_4(const std::string& input_filepath)
{
	std::vector<int64_t> ranges;

	for (auto line : next_file_line(input_filepath))
		for (auto value : next_line_token<int64_t>(line, '-'))
			ranges.push_back(value);

	int64_t lower = ranges[0];
	int64_t upper = ranges[1];

	auto is_valid_password = [](int64_t value) -> std::pair<bool, bool>
	{
		static bool has_digit[10] = { false };

		int64_t next_lowest_allowed = 1;
		int64_t last_digit = -1;
		std::vector<int64_t> digit_adjacent(10, 1);
		bool has_adjacent = false;		

		for (int64_t divider = 100000, i = 0; divider; value %= divider, divider /= 10, i++)
		{
			int64_t current_digit = value / divider;

			if (current_digit < next_lowest_allowed)
				return { false, false };
			
			else
				next_lowest_allowed = current_digit;

			if (current_digit == last_digit)
			{
				has_adjacent = true;

				digit_adjacent[current_digit]++;
			}

			has_digit[current_digit] = true;

			last_digit = current_digit;
		}

		return { has_adjacent, std::any_of(digit_adjacent.begin(), digit_adjacent.end(), 
			[](auto v) { return v == 2; }) 
		};
	};

	int64_t valid_passwords_part_1 = 0;
	int64_t valid_passwords_part_2 = 0;

	for (auto i = lower; i <= upper; i++)
	{
		auto[valid1, valid2] = is_valid_password(i);

		if (valid1)
			valid_passwords_part_1++;

		if (valid2)
			valid_passwords_part_2++;
	}
	
	return { valid_passwords_part_1, valid_passwords_part_2 };
}

std::pair<int64_t, int64_t> day_5(const std::string& input_filepath)
{
	std::vector<int64_t> memory;

	for (auto line : next_file_line(input_filepath))
		for (auto word : next_line_token<int64_t>(line))
			memory.push_back(word);

	std::vector<int64_t> results;
	
	std::vector<int64_t> inputs = { 1, 5 };
	for (auto input : inputs)
	{
		IntcodeVM vm(memory, 0);

		int64_t result;
		execution_state_t state = execution_state_t::halted;

		while ((state = vm.run(result, input)) != execution_state_t::halted)
			results.push_back(result);
	}

	return { *(results.end() - 2), *(results.end() - 1) };
}

int64_t steps_to_root(const std::string& child, std::map<std::string, std::string>& child_parent, std::string root_name = "COM")
{
	if (child == root_name)
		return 0;

	return 1 + steps_to_root(child_parent[child], child_parent, root_name);
}

bool is_in_orbit(const std::string& child, const std::string& parent, std::map<std::string, std::string>& child_parent)
{
	if (child == "COM")
		return false;

	if (child_parent[child] == parent)
		return true;
	
	return is_in_orbit(child_parent[child], parent, child_parent);
}

std::pair<int64_t, int64_t> day_6(const std::string& input_filepath)
{
	std::map<std::string, std::string> child_parent;

	for (auto line : next_file_line(input_filepath))
	{
		std::vector<std::string> relationship;
		for (auto word : next_line_token(line, ')'))
			relationship.push_back(word);

		auto parent = relationship[0];
		auto child  = relationship[1];

		child_parent[child] = parent;
	}

	int64_t orbits{};

	for (auto[child, parent] : child_parent)
		orbits += steps_to_root(child, child_parent);

	std::string common = child_parent["SAN"];
	while (!is_in_orbit("YOU", common, child_parent))
		common = child_parent[common];

	int64_t orbit_shifts = 
		steps_to_root("YOU", child_parent, common) +
		steps_to_root("SAN", child_parent, common) - 
		2;

	return { orbits, orbit_shifts };
}

std::pair<int64_t, int64_t> day_7(const std::string& input_filepath)
{
	std::vector<int64_t> amplifier_program;

	for (auto line : next_file_line(input_filepath))
		for (auto word : next_line_token<int64_t>(line))
			amplifier_program.push_back(word);

	std::vector<int64_t> thruster_outputs;

	std::array<int64_t, 5> phases = { 0, 1, 2, 3, 4 };
	do
	{
		int64_t amplifier_output = 0;

		for (size_t i = 0; i < phases.size(); i++)
		{
			IntcodeVM amplifier(amplifier_program);

			amplifier.run(amplifier_output, phases[i]);
			amplifier.run(amplifier_output, amplifier_output);
			amplifier.run(amplifier_output);
		}
		
		thruster_outputs.push_back(amplifier_output);

	} while (std::next_permutation(phases.begin(), phases.end()));

	std::vector<int64_t> feedback_outputs;

	do
	{
		std::array<IntcodeVM, 5> amplifiers = {
			IntcodeVM(amplifier_program),
			IntcodeVM(amplifier_program),
			IntcodeVM(amplifier_program),
			IntcodeVM(amplifier_program),
			IntcodeVM(amplifier_program)
		};

		int64_t amplifier_output = 0;
		int64_t last_amplifier_output = -1;

		for (size_t i = 0; i < phases.size(); i++)
		{
			auto &amplifier = amplifiers[i];
			amplifier.run(amplifier_output, phases[i] + 5);
		}

		bool halted = false;
		do
		{
			for (size_t i = 0; i < phases.size(); i++)
			{
				auto &amplifier = amplifiers[i];

				if (amplifier.run(amplifier_output, amplifier_output) != execution_state_t::consumed_value)
				{
					halted = true;
					break;
				}

				if (amplifier.run(amplifier_output) != execution_state_t::provided_value)
				{
					halted = true;
					break;
				}

				if (i == phases.size() - 1)
					last_amplifier_output = amplifier_output;
			}
		} while (!halted);

		feedback_outputs.push_back(last_amplifier_output);

	} while (std::next_permutation(phases.begin(), phases.end()));

	return { 
		*std::max_element(thruster_outputs.begin(), thruster_outputs.end()),
		*std::max_element(feedback_outputs.begin(), feedback_outputs.end())
	};
}
std::pair<int64_t, int64_t> day_8(const std::string& input_filepath)
{
	std::string image;

	for (auto line : next_file_line(input_filepath))
		image = line;

	size_t width = 25;
	size_t height = 6;
	size_t layer_size = width * height;

	if (image.size() % layer_size)
		throw std::invalid_argument("malformed input");

	std::vector<std::string> layers;

	for (size_t i = 0; i < image.size(); i += layer_size)
		layers.push_back(image.substr(i, layer_size));

	auto layer_with_least_zeros = *std::min_element(layers.begin(), layers.end(), [](const auto& l1, const auto& l2)
	{
		return std::count(l1.begin(), l1.end(), '0') < std::count(l2.begin(), l2.end(), '0');
	});

	int64_t part1 = std::count(layer_with_least_zeros.begin(), layer_with_least_zeros.end(), '1') *
		std::count(layer_with_least_zeros.begin(), layer_with_least_zeros.end(), '2');

	for (size_t i = 0; i < height; i++)
	{
		for (size_t j = 0; j < width; j++)
		{
			for (auto layer : layers)
			{
				auto color = layer[i * width + j];
				
				if (color != '2')
				{
					std::cout << (color == '0' ? ' ' : '#');
					break;
				}
			}
		}

		std::cout << std::endl;
	}	

	return { part1, 0 };
}

std::pair<int64_t, int64_t> day_9(const std::string& input_filepath)
{
	std::vector<int64_t> boost;

	for (auto line : next_file_line(input_filepath))
		for (auto word : next_line_token<int64_t>(line))
			boost.push_back(word);

	std::vector<int64_t> results;

	for (int64_t input : { 1, 2 })
	{
		execution_state_t state{};
		IntcodeVM vm(boost);

		int64_t result = 0;
		do
		{
			state = vm.run(result, input);
		} while (state != execution_state_t::halted);

		results.push_back(result);
	}

	return { results[0], results[1] };
}

template <int64_t StartingAngle = 90>
struct clockwise_line_comparator
{
	bool operator()(double v1, double v2) const
	{
		v1 *= -1;
		v2 *= -1;

		const auto twopi  = std::atan(1) * 8;
		const auto offset = twopi - static_cast<float>(StartingAngle) / 180 * std::atan(1) * 4;

		v1 += offset;
		v2 += offset;

		if (v1 > twopi)
			v1 -= twopi;

		if (v2 > twopi)
			v2 -= twopi;

		return v1 > v2;
	}
};

std::pair<int64_t, int64_t> day_10(const std::string& input_filepath)
{
	std::vector<std::string> asteroid_field;

	for (auto line : next_file_line(input_filepath))
		asteroid_field.emplace_back(line);

	size_t length_y = asteroid_field.size();
	size_t length_x = asteroid_field[0].size();

	std::map<std::pair<size_t, size_t>, size_t> asteroids_visible;
	auto asteroid = '#';

	for (size_t current_y = 0; current_y < length_y; current_y++)
	{
		for (size_t current_x = 0; current_x < length_x; current_x++)
		{
			if (asteroid_field[current_y][current_x] != asteroid)
				continue;

			std::set<double> slopes;

			for (size_t i = 0; i < length_y; i++)
			{
				for (size_t j = 0; j < length_x; j++)
				{
					if (current_y == i && current_x == j)
						continue;

					if (asteroid_field[i][j] != asteroid)
						continue;

					int64_t position_y = static_cast<int64_t>(i) - current_y;
					int64_t position_x = static_cast<int64_t>(j) - current_x;

					slopes.insert(std::atan2(position_y, position_x));
				}
			}

			asteroids_visible[{ current_y, current_x }] = slopes.size();
		}
	}

	auto station_location = std::max_element(asteroids_visible.begin(), asteroids_visible.end(), 
		[](const auto& p1, const auto& p2)
	{
		return p1.second < p2.second;
	});

	int64_t station_location_y = station_location->first.first;
	int64_t station_location_x = station_location->first.second;

	using priority_item = std::pair<
		int64_t,
		std::pair<
		size_t, size_t
		>
	>;

	std::map<
		double, 
		std::priority_queue<
			priority_item,
			std::vector<priority_item>,
			std::greater<priority_item>
		>,
		clockwise_line_comparator<90>
	> slope_asteroids;

	for (size_t i = 0; i < length_y; i++)
	{
		for (size_t j = 0; j < length_x; j++)
		{
			if (asteroid_field[i][j] != asteroid)
				continue;

			int64_t position_y = static_cast<int64_t>(i) - station_location_y;
			int64_t position_x = static_cast<int64_t>(j) - station_location_x;

			slope_asteroids[std::atan2(position_y, position_x)].push(
				{ position_x * position_x + position_y * position_y, { i, j } }
			);
		}
	}

	size_t asteroids_destroyed = 0;
	std::pair<size_t, size_t> asteroid_target;
	size_t bet_index = 200;

	do
	{
		for (auto[slope, asteroids] : slope_asteroids)
		{
			if (asteroids.empty())
				continue;

			asteroid_target = asteroids.top().second;
			asteroids.pop();
			asteroids_destroyed++;

			if (asteroids_destroyed == bet_index)
				break;

		}
	} while (asteroids_destroyed < bet_index);

	return { station_location->second, asteroid_target.second * 100 + asteroid_target.first };
}

struct HullPainter 
{
	enum direction_t
	{
		up = 0,
		left = 1,
		down = 2,
		right = 3
	};

	enum turn_t
	{
		ccw = 1,
		cw = -1
	};

	IntcodeVM& vm;
	execution_state_t state{};
	int64_t position_x{};
	int64_t position_y{};
	int64_t next_turn{};

	int64_t initial_color;
	int64_t current_direction;

	using panel_t = std::pair<int64_t, int64_t>;
	std::map<panel_t, char> panel_colors;

	HullPainter(IntcodeVM& vm, int64_t initial_color = 0, int64_t initial_direction = up) :
		vm(vm), initial_color(initial_color), current_direction(initial_direction)
	{

	}

	void paint()
	{
		panel_colors[{ position_x, position_y }] = initial_color ? '*' : ' ';

		while (true)
		{
			int64_t output = -1;

			int64_t current_color = panel_colors[{ position_x, position_y }] == ' ' ? 0 : 1;

			state = vm.run(output, current_color);
			if (state == execution_state_t::halted) break;

			state = vm.run(output, current_color);
			if (state == execution_state_t::halted) break;
			panel_colors[{ position_x, position_y }] = output ? '*' : ' ';

			state = vm.run(output, current_color);
			if (state == execution_state_t::halted) break;
			next_turn = output == 0 ? ccw : cw;

			current_direction = (4 + current_direction + next_turn) % 4;

			switch (current_direction)
			{
			case up:	position_y--; break;
			case left:	position_x--; break;
			case down:	position_y++; break;
			case right: position_x++; break;
			}
		}
	}
};

std::pair<int64_t, int64_t> day_11(const std::string& input_filepath)
{
	std::vector<int64_t> program_words;

	for (auto line : next_file_line(input_filepath))
		for (auto word : next_line_token<int64_t>(line))
			program_words.push_back(word);

	IntcodeVM part_1_vm(program_words);
	HullPainter part_1_painter(part_1_vm);

	IntcodeVM part_2_vm(program_words);
	HullPainter part_2_painter(part_2_vm, 1);

	part_1_painter.paint();
	part_2_painter.paint();
	display(part_2_painter.panel_colors);

	return { part_1_painter.panel_colors.size(), -1 };
}

struct ArcadeCabinet
{
	IntcodeVM game;

	ArcadeCabinet(std::vector<int64_t>& program_words, int64_t coins = 1)
		: game(program_words) 
	{
		game.memory[0] = coins;
	}

	int64_t score;
	using point_t = std::pair<int64_t, int64_t>;
	std::map<point_t, char> screen;

	enum tile_t : int64_t
	{
		empty,
		wall,
		block,
		paddle,
		ball
	};

	void render()
	{
		int64_t last_y = 0;
		for (auto[position, tile] : screen)
		{
			if (position.first != last_y)
			{
				last_y = position.first;
				std::cout << std::endl;
			}

			std::cout << tile;
		}
		std::cout << std::endl;
	}

	void play()
	{
		execution_state_t state{};
		point_t ball_position{};
		point_t paddle_position{};
		int64_t moving_direction{};

		auto update_moving_direction = [&]()
		{
			if (paddle_position.second == ball_position.second)
				moving_direction = 0;
				
			else
				moving_direction = ball_position.second < paddle_position.second ? -1 : 1;
		};

		do
		{
			update_moving_direction();

			int64_t x;
			state = game.run(x, moving_direction);
			if (state == execution_state_t::consumed_value) 
				state = game.run(x, moving_direction);

			if (state == execution_state_t::halted) break;

			int64_t y;
			state = game.run(y, moving_direction);
			if (state == execution_state_t::consumed_value) 
				state = game.run(y, moving_direction);

			if (state == execution_state_t::halted) break;

			int64_t tile_id;
			state = game.run(tile_id, moving_direction);
			if (state == execution_state_t::consumed_value) 
				state = game.run(tile_id, moving_direction);

			if (state == execution_state_t::halted) break;

			if (x == -1)
			{
				score = tile_id;
				continue;
			}

			point_t position = { y, x };

			switch (tile_id)
			{
			case empty:
				screen[position] = ' ';
				break;

			case wall:
				screen[position] = '#';
				break;

			case block:
				screen[position] = '|';
				break;

			case paddle:
				screen[position] = '_';
				paddle_position = position;	
				render();
				break;

			case ball:
				screen[position] = 'o';
				ball_position = position;
				render();
				break;

			default:
				break;
			}
		} while (state != execution_state_t::halted);
	}
};

std::pair<int64_t, int64_t> day_12(const std::string& input_filepath)
{
	using vec3i = std::array<int64_t, 3>;

	std::vector<vec3i> positions;

	for (auto line : next_file_line(input_filepath))
	{
		size_t i = 0;
		vec3i body{};

		std::replace_if(line.begin(), line.end(), 
			[](const auto c) 
		{ 
			return !(std::isdigit(c) || c == ',' || c == '-'); 
		}, ' ');
			
		for (auto val : next_line_token<int64_t>(line, ','))
		{
			body[i] = val;
			i++;
		}

		positions.push_back(body);
	}

	std::vector<vec3i> velocities(positions.size(), vec3i{});

	int64_t part1{};

	std::vector<vec3i> original_positions(positions);
	std::vector<vec3i> original_velocities(velocities);

	std::vector<int64_t> periods(3, 0);
	int64_t periods_found{};

	for (size_t step = 1; periods_found != 3 || step <= 1000; step++)
	{
		for (size_t i = 0; i < positions.size(); i++)
		{
			for (size_t j = 0; j < positions.size(); j++)
			{
				if (i == j)
					continue;

				for (size_t dim = 0; dim < 3; dim++)
				{
					if (positions[i][dim] == positions[j][dim])
						continue;

					velocities[i][dim] += positions[i][dim] > positions[j][dim] ? -1 : 1;
				}
			}
		}

		std::array<bool, 3> dimensions_match = { true, true, true };

		int64_t energy{};
		for (size_t i = 0; i < positions.size(); i++)
		{
			int64_t potential{};
			int64_t kinetic{};

			for (size_t dim = 0; dim < 3; dim++)
			{
				positions[i][dim] += velocities[i][dim];

				potential += std::abs(positions[i][dim]);
				kinetic += std::abs(velocities[i][dim]);

				dimensions_match[dim] = dimensions_match[dim] && positions[i][dim] == original_positions[i][dim];
			}

			energy += potential * kinetic;
		}

		for (size_t dim = 0; dim < 3; dim++)
		{
			if (!periods[dim] && dimensions_match[dim])
			{
				periods[dim] = step + 1;
				periods_found++;
			}
		}

		if (step == 1000)
			part1 = energy;
	}

	int64_t part2 = periods.front();
	for (size_t i = 1; i < periods.size(); i++)
		part2 = std::lcm(part2, periods[i]);

	return { part1, part2 };
}

std::pair<int64_t, int64_t> day_13(const std::string& input_filepath)
{
	std::vector<int64_t> program_words;

	for (auto line : next_file_line(input_filepath))
		for (auto word : next_line_token<int64_t>(line))
			program_words.push_back(word);

	ArcadeCabinet part1(program_words);
	part1.play();

	ArcadeCabinet part2(program_words, 2);
	part2.play();

	return { 
		std::count_if(part1.screen.begin(), part1.screen.end(), [](const auto& p) { return p.second == '|'; }), 
		part2.score
	};
}


inline std::string trim(const std::string &s)
{
	auto wsfront = std::find_if_not(s.begin(), s.end(), [](int c) {return std::isspace(c); });
	auto wsback = std::find_if_not(s.rbegin(), s.rend(), [](int c) {return std::isspace(c); }).base();
	return (wsback <= wsfront ? std::string() : std::string(wsfront, wsback));
}

std::pair<std::string, int64_t> parse_ingredient(std::string str)
{
	std::pair<std::string, int64_t> value;
	
	str = trim(str);

	auto space = str.find(' ');
	value.second = std::stoll(str.substr(0, space));
		
	value.first = str.substr(space);
	value.first = trim(value.first);

	return value;
}


using ingredient_t = std::pair<std::string, int64_t>;

using recipes_t = std::map<
	ingredient_t,
	std::vector<ingredient_t>
>;

bool produce(
	ingredient_t& target, 
	const recipes_t& recipes, 
	std::map<std::string, int64_t>& available,
	std::deque<ingredient_t>& requirements
)
{
	if (target.first == "ORE")
		return false;

	// find out how to produce some quantity
	for (auto[outcome, ingredients] : recipes)
	{
		if (outcome.first != target.first)
			continue;

		auto target_required = target.second;

		// write down the ingredients needed
		std::map<std::string, int64_t> required;

		while (available[outcome.first] < target_required)
		{
			int64_t factor = (target_required - available[outcome.first]) / outcome.second;
			factor = std::max(factor, 1ll);

			available[outcome.first] += factor * outcome.second;

			for (auto[type, amount] : ingredients)
			{
				required[type] += factor * amount;
			}
		}

		for (auto[type, amount] : required)
		{
			if (type == "ORE")
				requirements.push_back({ type, amount });
			else
				requirements.push_front({ type, amount });
		}
		
		// produce it
		available[outcome.first] -= target_required;
	}

	return true;
}

int64_t ore_for_fuel(int64_t fuel_amount, const recipes_t& recipes)
{
	std::map<std::string, int64_t> available;
	std::deque<ingredient_t> requirements;

	requirements.push_front(parse_ingredient(std::to_string(fuel_amount) + " FUEL"));

	ingredient_t current_target;
	int64_t ore_spent = 0;

	while (true)
	{
		current_target = requirements.front();
		requirements.pop_front();

		if (current_target.first == "ORE")
		{
			ore_spent = std::accumulate(requirements.begin(), requirements.end(), current_target.second,
				[](const auto cur, const auto& p)
			{
				return p.second + cur;
			});

			break;
		}

		produce(current_target, recipes, available, requirements);
	}

	return ore_spent;
}

std::pair<int64_t, int64_t> day_14(const std::string& input_filepath)
{
	recipes_t recipes;

	for (auto line : next_file_line(input_filepath))
	{
		line.erase(std::remove(line.begin(), line.end(), '>'), line.end());

		std::vector<std::string> tokens;
		for (auto token : next_line_token(line, '='))
			tokens.push_back(token);
		
		ingredient_t result = parse_ingredient(tokens.back());

		for (auto ingredient : next_line_token(tokens[0], ','))
		{
			recipes[result].emplace_back(parse_ingredient(ingredient));
		}
	}

	int64_t ore_for_one_fuel = ore_for_fuel(1, recipes);
	
	int64_t cargo_ore = 1000000000000;
	int64_t lower_bound = cargo_ore / ore_for_one_fuel;
	int64_t upper_bound = cargo_ore - recipes.size();
	int64_t middle{};

	while (upper_bound - lower_bound > 1)
	{
		middle = lower_bound + (upper_bound - lower_bound) / 2;

		int64_t result = ore_for_fuel(middle, recipes);

		if (result > cargo_ore)
			upper_bound = middle - 1;
		else
			lower_bound = middle;
	}

	return { ore_for_one_fuel, lower_bound };
}

enum direction_t : int64_t
{
	north = 1,
	south,
	west,
	east,
	direction_last
};

enum symbols_t : char
{
	droid = 'D',
	wall = '#',
	space = '.',
	oxygen = 'x'
};

position_t operator+(position_t lhs, int64_t direction)
{
	switch (direction)
	{
	case direction_t::north:
		lhs.second--;
		break;

	case direction_t::south:
		lhs.second++;
		break;

	case direction_t::west:
		lhs.first--;
		break;

	case direction_t::east:
		lhs.first++;
		break;
	}

	return lhs;
}

position_t operator-(position_t lhs, int64_t direction)
{
	switch (direction)
	{
	case direction_t::north:
		lhs.second++;
		break;

	case direction_t::south:
		lhs.second--;
		break;

	case direction_t::west:
		lhs.first++;
		break;

	case direction_t::east:
		lhs.first--;
		break;
	}

	return lhs;
}

struct RepairDroid
{
	std::map<position_t, char> world;
	position_t oxygen_position;

	enum status_t : int64_t
	{
		blocked,
		success,
		finished
	};

	RepairDroid(position_t starting = { 0, 0 }) : world{}, oxygen_position{}
	{
		world[starting] = droid;
	}

	void explore(IntcodeVM vm, position_t current, int64_t direction)
	{
		auto moved_to = current;
		moved_to = moved_to + direction;

		if (world[moved_to])
			return;

		int64_t output;
		vm.run(output, direction);
		vm.run(output, direction);

		switch (output)
		{
		case blocked:
			world[moved_to] = wall;
			return;

		case success:
			world[moved_to] = space;
			break;

		case finished:
			world[moved_to] = oxygen;
			oxygen_position = moved_to;
			break;
		}

		step(vm, moved_to);
	}

	void step(IntcodeVM vm, position_t current = {})
	{
		for (int64_t direction = north; direction < direction_last; direction++)
		{
			explore(vm, current, direction);
		}
	}
};

using position_with_priority_t = std::pair<position_t, int64_t>;
auto priority_comparer = [](const position_with_priority_t& p1, const position_with_priority_t& p2)
{
	return p1.second < p2.second;
};	

bool a_star(
	std::map<position_t, char>& world,
	position_t start, 
	position_t goal, 
	std::function <int64_t(position_t, position_t)> heuristic,
	std::vector<position_t>& path
)
{
	using storage_t = std::vector<position_with_priority_t>;
	std::priority_queue<storage_t::value_type, storage_t, decltype(priority_comparer)> frontier(priority_comparer);
	std::map<position_t, position_t> came_from;
	std::map<position_t, int64_t> cost_to;

	frontier.push({ start, 0 });
	came_from[start] = start;
	cost_to[start]   = 0;

	while (!frontier.empty())
	{
		auto top = frontier.top();
		frontier.pop();

		position_t current = top.first;
		if (current == goal)
		{
			current = goal;
			path.push_back(current);

			while (current != start)
			{
				current = came_from[current];
				path.push_back(current);
			}

			return true;
		}

		for (int64_t direction = north; direction < direction_last; direction++)
		{
			auto next = current + direction;

			if (world[next] == wall)
				continue;

			auto new_cost = cost_to[current] + 1;

			if (cost_to.find(next) == cost_to.end() || new_cost < cost_to[next])
			{
				cost_to[next] = new_cost;
				frontier.push({ next, new_cost + heuristic(next, goal) });
				came_from[next] = current;
			}
		}
	}

	return false;
}

int64_t flood_fill(std::map<position_t, char>& world, position_t start)
{
	std::queue<position_t> currently_have_oxygen;
	std::queue<position_t> will_have_oxygen({ start });

	int64_t steps_needed = 0;
	
	do
	{
		std::swap(currently_have_oxygen, will_have_oxygen);

		do
		{
			auto current = currently_have_oxygen.front();
			currently_have_oxygen.pop();

			for (int64_t direction = north; direction < direction_last; direction++)
			{
				auto next = current + direction;
				if (world[next] == wall || world[next] == oxygen)
					continue;

				world[next] = oxygen;
				will_have_oxygen.push(next);
			}
		} while (!currently_have_oxygen.empty());
		
		steps_needed++;

	} while (!will_have_oxygen.empty());

	return steps_needed - 1; // because i'm counting the first one as a step
}

auto manhattan_distance(position_t p1, position_t p2) -> int64_t
{
	return std::abs(p2.second - p1.second) + std::abs(p2.first - p1.first);
}

std::pair<int64_t, int64_t> day_15(const std::string& input_filepath)
{
	std::vector<int64_t> opcodes;

	for (auto line : next_file_line(input_filepath))
		for (auto opcode : next_line_token<int64_t>(line))
			opcodes.push_back(opcode);

	IntcodeVM program(opcodes);
	RepairDroid droid;

	droid.step(program);
	display(droid.world);

	std::vector<position_t> path_to_oxygen;
	a_star(droid.world, {}, droid.oxygen_position, manhattan_distance, path_to_oxygen);

	int64_t steps_taken = flood_fill(droid.world, droid.oxygen_position);

	return { path_to_oxygen.size() - 1, steps_taken };
}

void fft(std::string& input, std::string& output)
{
	std::array<int64_t, 4> pattern = { 0, 1, 0, -1 };

	for (int64_t line = 0; line < input.size(); line++)
	{
		int64_t current = 0;
		for (int64_t i = 0; i < input.size(); i++)
		{
			current += input[i] * pattern[(1 + i) / (1 + line) % pattern.size()];
		}

		if (current < 0)
			current *= -1;

		output[line] = current % 10;
	}
}

int64_t get_digits(std::string& input, int64_t n = 8, int64_t offset = 0)
{
	int64_t result = 0;

	for (int64_t i = n - 1, radix = 1; i >= 0; i--, radix *= 10)
		result += radix * input[i + offset];

	return result;
}

// not a fan of these tricks
int64_t extract_message(std::string& input, int64_t num_phases, int64_t repetitions)
{
	std::string partials;
	partials.reserve(input.size() * repetitions);
	for (size_t i = 0; i < repetitions; i++)
		partials += input;
	
	int64_t offset = get_digits(input, 7, 0);

	partials = partials.substr(offset);

	for (size_t phase = 0; phase < num_phases; phase++)
	{
		for (int64_t i = partials.size() - 2; i >= 0; i--)
			partials[i] = (partials[i] + partials[i + 1]) % 10;
	}

	return get_digits(partials, 8);
}

std::pair<int64_t, int64_t> day_16(const std::string& input_filepath)
{
	std::string input;

	for (auto line : next_file_line(input_filepath))
		input = line;

	for (auto& c : input)
		c -= '0';

	std::string buffer = input;
	std::string original_input = input;

	int64_t num_phases = 100;

	for (size_t phase = 0; phase < num_phases; phase++)
	{
		fft(input, buffer);
		std::swap(buffer, input);
	}

	int64_t part1 = get_digits(input, 8, 0);

	return { part1, extract_message(original_input, num_phases, 10000) };
}

enum hull_t : char
{
	scaffolding = '#',
	robot_up = '^',
	robot_down = 'v',
	robot_left = '<',
	robot_right = '>',
	robot_space = 'X'
};

void find_intersections(std::map<position_t, char>& world, std::vector<position_t>& intersections)
{
	auto robot = std::find_if(world.begin(), world.end(), [](const auto& p) { return !(p.second == space || p.second == scaffolding); });
	auto scaffolds = std::count_if(world.begin(), world.end(), [](const auto& p) { return p.second == scaffolding; });

	std::set<position_t> visited;
	std::queue<position_t> places_to_go({ robot->first });

	do
	{
		auto current = places_to_go.front();
		places_to_go.pop();

		if (visited.count(current))
			continue;

		int64_t available_directions = 0;

		for (int64_t direction = north; direction < direction_last; direction++)
		{
			auto next = current + direction;
			if (!world.count(next) || visited.count(next) || world[next] == space)
				continue;

			available_directions++;
			places_to_go.push(next);
		}

		if (available_directions > 1)
			intersections.push_back(current);

		visited.insert(current);

	} while (visited.size() != scaffolds);
}

std::pair<int64_t, int64_t> day_17(const std::string& input_filepath)
{
	IntcodeVM part1_vm(input_filepath);

	std::map<position_t, char> world;
	position_t current{};
	int64_t output = 0;

	while (part1_vm.run(output) != execution_state_t::halted)
	{
		if (output == '\n')
		{
			current.first = 0;
			current.second++;
			continue;
		}
		
		world[current] = static_cast<char>(output);
		current.first++;
	}

	display(world);

	std::vector<position_t> intersections;
	find_intersections(world, intersections);
	auto part1_solution = std::accumulate(intersections.begin(), intersections.end(), 0ll, [](const auto acc, const auto& p)
	{
		return acc + p.first * p.second;
	});

	// manual, probably can be solved by using a_star and instruction string length as heuristic but nah
	std::string main = "A,B,A,B,C,B,A,C,B,C\n";
	std::string A = "L,12,L,8,R,10,R,10\n";
	std::string B = "L,6,L,4,L,12\n";
	std::string C = "R,10,L,8,L,4,R,10\n";
	std::string p = "n\n";
	std::string input = main + A + B + C + p;

	IntcodeVM part2_vm(input_filepath);
	part2_vm.memory[0] = 2;

	return { part1_solution, part2_vm.run_on(input) };
}

using world_t = std::map<position_t, char>;

std::experimental::generator<position_t> get_neighbouring_tiles(const world_t &world, const position_t &current)
{
	for (int64_t direction = north; direction < direction_last; direction++)
		co_yield current + direction;
}

using start_point_t = char;
using keys_t = std::bitset<26>;
using position_and_keys_t = std::pair<position_t, keys_t>;
using part2_position_and_keys_t = std::pair<std::array<char, 4>, std::bitset<26>>;
using keys_and_doors_en_route_t = std::pair<keys_t, keys_t>;
using end_point_t = std::pair<char, keys_and_doors_en_route_t>;
using position_and_items_t = std::pair<position_t, keys_and_doors_en_route_t>;

using transformed_world_t = std::unordered_map<start_point_t, std::vector<end_point_t>>;

template<typename T>
inline void hash_combine(std::size_t& seed, const T& val)
{
	std::hash<T> hasher;
	seed ^= hasher(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace std {
	template <typename T1, typename T2>
	struct hash<pair<T1, T2>>
	{
		size_t operator()(const pair<T1, T2> &value) const
		{
			size_t seed = 0;
			hash_combine(seed, value.first);
			hash_combine(seed, value.second);

			return seed;
		}
	};

	template <>
	struct hash<position_t>
	{
		size_t operator()(const position_t &value) const
		{
			size_t seed = 0;
			hash_combine(seed, value.first);
			hash_combine(seed, value.second);
			return seed;
		}
	};

	template <> 
	struct hash<position_and_keys_t>
	{
		size_t operator()(const position_and_keys_t &value) const
		{
			size_t seed = 0;
			hash_combine(seed, value.first);
			hash_combine(seed, value.second);
			return seed;
		}
	};

	template <>
	struct hash<position_and_items_t>
	{
		size_t operator()(const position_and_items_t &value) const
		{
			size_t seed = 0;
			hash_combine(seed, value.first);
			hash_combine(seed, value.second.first);
			hash_combine(seed, value.second.second);
			return seed;
		}
	};

	template <>
	struct hash<part2_position_and_keys_t>
	{
		size_t operator()(const part2_position_and_keys_t &value) const
		{
			size_t seed = 0;
			hash_combine(seed, value.first[0]);
			hash_combine(seed, value.first[1]);
			hash_combine(seed, value.first[2]);
			hash_combine(seed, value.first[3]);
			hash_combine(seed, value.second);
			return seed;
		}
	};
}

std::experimental::generator<position_and_items_t> get_next_steps_mark_keys_and_doors(const world_t &world, const position_and_items_t &node)
{
	auto[pos, keys_and_doors] = node;

	for (auto next_pos : get_neighbouring_tiles(world, pos))
	{
		if (world.find(next_pos) == world.end())
			continue;

		auto tile = world.at(next_pos);

		if (tile == wall)
			continue;

		auto next_keys  = keys_and_doors.first;
		auto next_doors = keys_and_doors.second;

		if (std::isupper(tile))
			next_doors[tile - 'A'] = true;

		else if (std::islower(tile))
			next_keys[tile - 'a'] = true;
		
		co_yield{ next_pos, { next_keys, next_doors } };
	}
}

std::experimental::generator<position_and_keys_t> get_next_steps(const world_t &world, const position_and_keys_t &node)
{
	auto[pos, keys] = node;

	for (auto next_pos : get_neighbouring_tiles(world, pos))
	{
		if (world.find(next_pos) == world.end() || world.at(next_pos) == wall)
			continue;

		auto tile = world.at(next_pos);

		if (std::isupper(tile))
		{
			if (!keys[std::tolower(tile) - 'a'])
				continue;
		}

		auto next_keys = keys;
		if (std::islower(tile))
			next_keys[tile - 'a'] = true;

		co_yield{ next_pos, next_keys };
	}	
}

std::experimental::generator<part2_position_and_keys_t> node_expander_18_2(const transformed_world_t &world, const part2_position_and_keys_t &node)
{
	auto &[starting_pos, starting_keys] = node;
	std::remove_const_t<decltype(starting_pos)> possible_steps = starting_pos;

	for (auto &current_pos : possible_steps)
	{
		for (auto &[destination, keys_and_doors_en_route] : world.at(current_pos))
		{
			if (std::isdigit(destination))
				continue;

			auto&[keys_en_route, doors_en_route] = keys_and_doors_en_route;
			auto next_keys = starting_keys | keys_en_route;

			if ((next_keys & doors_en_route).count() < doors_en_route.count())
				continue;

			next_keys[destination - 'a'] = true;

			current_pos = destination;

			co_yield{ possible_steps, next_keys };

			possible_steps = starting_pos;
		}
	}
}

std::pair<int64_t, int64_t> day_18(const std::string& input_filepath)
{
	world_t world;
	std::map<char, position_t> relevant_locations;

	uint8_t y = 0;
	for (auto line : next_file_line(input_filepath))
	{
		uint8_t x = 0;
		for (uint8_t x = 0; x < line.size(); x++)
		{
			world[{ x, y }] = line[x];

			if (std::islower(line[x]) || line[x] == '@')
				relevant_locations[line[x]] = { x, y };
		}

		y++;
	}

	display(world, false);
	auto origin = relevant_locations['@'];
	auto total_keys = (relevant_locations.size() - 1); // @ is counted as relevant
	auto route_found_1 = [&](const world_t&, const position_and_keys_t &node) { return node.second.count() == total_keys; };

	search_algorithms::AStar<world_t, position_and_keys_t> astar_1(
		world,
		route_found_1,
		get_next_steps,
		search_algorithms::AStar<world_t, position_and_keys_t>::unit_transition_cost,
		search_algorithms::AStar<world_t, position_and_keys_t>::null_heuristic
	);

	std::vector<position_and_keys_t> path_1;
	astar_1.search({ origin, {} }, path_1);
	
	auto part_1 = path_1.size() - 1; // path includes the starting point but it doesn't need a step to get there

	// part 2
	decltype(part2_position_and_keys_t::first) origins_2;
	auto route_found_2 = [&](const transformed_world_t&, const part2_position_and_keys_t &node) { return node.second.count() == total_keys; };

	size_t idx = 0;
	origin.first--;
	origin.second--;
	world[origin] = '0';
	origins_2[idx++] = world[origin];
	origin.first++;
	world[origin] = '#';
	origin.first++;
	world[origin] = '1';
	origins_2[idx++] = world[origin];
	origin.second++;
	world[origin] = '#';
	origin.first--;
	world[origin] = '#';
	origin.first--;
	world[origin] = '#';
	origin.second++;
	world[origin] = '2';
	origins_2[idx++] = world[origin];
	origin.first++;
	world[origin] = '#';
	origin.first++;
	world[origin] = '3';
	origins_2[idx++] = world[origin];

	display(world, false);

	transformed_world_t adjacency_graph;
	std::map<char, std::map<char, size_t>> distances;

	for (auto&[p1, start] : world)
	{
		for (auto&[p2, goal] : world)
		{
			if (start == goal)
				continue;

			if (start == wall || start == space || goal == wall || goal == space)
				continue;

			if (std::isupper(start) || std::isupper(goal) || std::isdigit(goal))
				continue;

			search_algorithms::AStar<world_t, position_and_items_t> meta_astar(
				world,
				[&](const auto &world, const auto &current) { return world.at(current.first) == goal; },
				get_next_steps_mark_keys_and_doors,
				search_algorithms::AStar<world_t, position_and_items_t>::unit_transition_cost,
				search_algorithms::AStar<world_t, position_and_items_t>::null_heuristic
			);

			std::vector<position_and_items_t> path;
			if (meta_astar.search({ p1, {} }, path))
			{
				adjacency_graph[start].push_back({ goal, { path.back().second.first, path.back().second.second} });
				distances[start][goal] = path.size() - 1;
			}
		}
	}

	search_algorithms::AStar<transformed_world_t, part2_position_and_keys_t> astar_2(
		adjacency_graph,
		route_found_2,
		node_expander_18_2,
		[&](const auto &world, const auto &current, const auto &next) -> int64_t
	{
		for (size_t i = 0; i < current.first.size(); i++)
		{
			for (size_t j = 0; j < next.first.size(); j++)
			{
				auto current_pos = current.first[j];
				auto next_pos = next.first[j];

				if (current_pos != next_pos)
					return distances[current_pos][next_pos];
			}
		}

		return search_algorithms::infinite_cost_s;
	},
		search_algorithms::AStar<transformed_world_t, part2_position_and_keys_t>::null_heuristic
	);

	std::vector<part2_position_and_keys_t> path_2;
	astar_2.search({ origins_2, {} }, path_2);

	int64_t part_2 = 0;

	for (size_t i = 0; i < path_2.size() - 1; i++)
	{
		for (size_t j = 0; j < std::tuple_size_v<part2_position_and_keys_t::first_type>; j++)
		{
			auto previous_step = path_2[i].first[j];
			auto next_step = path_2[i + 1].first[j];	

			if (previous_step != next_step)
			{
				std::cout << "Robot " << j << " collects key " << next_step << std::endl;
				part_2 += distances[previous_step][next_step];

				break;
			}
		}
	}

	return { part_1, part_2 };
}

std::pair<int64_t, int64_t> day_19(const std::string& input_filepath)
{
	std::vector<int64_t> opcodes;
	
	for (auto line : next_file_line(input_filepath))
		for (auto opcode : next_line_token<int64_t>(line))
			opcodes.push_back(opcode);

	int64_t output;

	std::map<position_t, char> world;

	uint8_t part1 = 0;

	for (int64_t y = 0; y < 50; y++)
	{
		for (int64_t x = 0; x < 50; x++)
		{
			IntcodeVM drones(opcodes);

			drones.run(output, x);
			drones.run(output, y);
			drones.run(output);

			world[{ static_cast<int8_t>(x), static_cast<int8_t>(y) }] = output ? '#' : '.';

			if (output)
				part1++;
		}
	}

	display(world, false);

	int64_t ship_size = 100;
	ship_size--; // makes it easier to work later

	int64_t start_x = 0;
	int64_t y = ship_size;

	while (true)
	{
		int64_t x = start_x;

		while (true)
		{
			// I bet they made the program non-reentrant so we couldn't brute force this
			IntcodeVM drones(opcodes);
			drones.run(output, x);
			drones.run(output, y);
			drones.run(output);

			if (output)
			{
				start_x = x;
				break;
			}

			x++;
		}

		IntcodeVM drones(opcodes);
		drones.run(output, x + ship_size);
		drones.run(output, y - ship_size);
		drones.run(output);

		if (output)
		{
			break;
		}

		y++;
	}

	return { part1, 10000 * start_x + (y - ship_size) };
}

void parse_portals(world_t world /* intentional copy */, std::unordered_map<position_t, std::string> &position_portal)
{
	auto[width, height] = std::max_element(world.begin(), world.end())->first;

	for (uint8_t y = 0; y <= height; y++)
	{
		for (uint8_t x = 0; x <= width; x++)
		{
			position_t pos = { x, y };

			if (std::isupper(world[pos]))
			{
				// outer left or inner right portal
				if (world[pos + east] == space)
				{
					auto other = pos - east;
					position_portal[pos] = { world[other], world[pos] };
				}

				// outer right portal
				else if (world[pos + west] == space)
				{
					auto other = pos - west;
					position_portal[pos] = { world[pos], world[other] };
				}

				// outer top portal
				else if (world[pos + south] == space)
				{
					auto other = pos - south;
					position_portal[pos] = { world[other], world[pos] };
				}

				// outer bottom portal
				else if (world[pos + north] == space)
				{
					auto other = pos - north;
					position_portal[pos] = { world[pos], world[other] };
				}
			}
		}
	}
}

std::pair<int64_t, int64_t> day_20(const std::string &input_filepath)
{
	world_t world;

	uint8_t y = 0;
	for (auto line : next_file_line(input_filepath))
	{
		for (uint8_t x = 0; x < line.size(); x++)
		{
			world[{ x, y }] = line[x];
		}

		y++;
	}

	display(world, false);

	std::unordered_map<position_t, std::string> position_portal;
	parse_portals(world, position_portal);

	std::unordered_map<position_t, position_t> portal_destination;
	for (auto &[position, portal] : position_portal)
	{
		if (auto dest = std::find_if(position_portal.begin(), position_portal.end(), [&](const auto &p)
		{
			return p.first != position && p.second == portal;
		}); dest != position_portal.end())
		{
			portal_destination[position] = dest->first;
		}
	}

	auto get_neighbours_with_portals = [&](const world_t &world, const position_t &current) -> std::experimental::generator<position_t>
	{
		auto yielding_position = current;

		if (portal_destination.find(current) != portal_destination.end())
		{
			yielding_position = portal_destination[current];
		}

		for (auto &next_pos : get_neighbouring_tiles(world, yielding_position))
		{
			if (world.find(next_pos) == world.end())
				continue;

			auto tile = world.at(next_pos);

			if (!(tile == space || std::isupper(tile)))
				continue;

			co_yield next_pos;
		}
	};

	auto portals_distance_function = [&](const world_t &world, const position_t &current, const position_t &next) -> int64_t
	{
		return position_portal.find(current) == position_portal.end() ? 1 : 0;
	};

	auto starting_point = std::find_if(position_portal.begin(), position_portal.end(), [](const auto &p)
	{
		return p.second == "AA";
	})->first;

	search_algorithms::AStar<world_t, position_t> astar(
		world,
		[&](const auto &world, const auto &current)
	{
		if (position_portal.find(current) == position_portal.end())
			return false;

		return position_portal[current] == "ZZ";
	},
		get_neighbours_with_portals,
		portals_distance_function,
		search_algorithms::AStar<world_t, position_t>::null_heuristic
	);

	std::vector<position_t> path;
	astar.search(starting_point, path);

	auto path_length_ignoring_portals = [&](std::vector<position_t> &path)
	{
		return std::count_if(
			path.begin(),
			path.end(),
			[&](const auto &p)
		{
			return position_portal.find(p) == position_portal.end();
		});
	};

	using position_and_level_t = std::pair<position_t, int16_t>;

	auto[world_width, world_height] = std::max_element(world.begin(), world.end())->first;
	auto is_outer_portal = [&](const position_t &portal)
	{
		return portal.first == 1 || portal.second == 1 || portal.first == world_width - 1 || portal.second == world_height - 1;
	};

	auto get_neighbours_with_portals_and_levels = [&](const world_t &world, const position_and_level_t &current) -> std::experimental::generator<position_and_level_t>
	{
		position_t yielding_position;
		int16_t current_level;

		yielding_position = current.first;
		current_level = current.second;

		auto next_level = current_level;

		if (portal_destination.find(yielding_position) != portal_destination.end())
		{
			next_level += is_outer_portal(yielding_position) ? -1 : 1;

			if (next_level < 0)
				return;

			yielding_position = portal_destination[yielding_position];
		}

		for (auto &next_pos : get_neighbouring_tiles(world, yielding_position))
		{
			if (world.find(next_pos) == world.end())
				continue;

			auto tile = world.at(next_pos);

			if (!(tile == space || std::isupper(tile)))
				continue;

			co_yield{ next_pos, next_level };
		}
	};

	search_algorithms::AStar<world_t, position_and_level_t> astar_2(
		world,
		[&](const auto &world, const auto &current)
	{
		if (position_portal.find(current.first) == position_portal.end())
			return false;

		return position_portal[current.first] == "ZZ" && current.second == 0;
	},
		get_neighbours_with_portals_and_levels,
		[&](const auto &world, const auto &p1, const auto &p2)
	{
		return portals_distance_function(world, p1.first, p2.first);
	},
		search_algorithms::AStar<world_t, position_and_level_t>::null_heuristic
	);

	std::vector<position_and_level_t> path_2;
	astar_2.search({ starting_point, 0 }, path_2);

	std::vector<position_t> flattened_path_2;
	std::transform(path_2.begin(), path_2.end(), std::back_inserter(flattened_path_2), [](const auto &p) { return p.first; });

	return { path_length_ignoring_portals(path) - 1, path_length_ignoring_portals(flattened_path_2) - 1 };
}

std::pair<int64_t, int64_t> day_21(const std::string& input_filepath)
{
	std::vector<int64_t> outputs;

	// if can land 4 tiles away and hole in between -> jump
	IntcodeVM walker(input_filepath);
	std::string walker_script= R"(OR A J
AND B J
AND C J
NOT J J
AND D J
WALK
)";
	int64_t part1 = walker.run_on(outputs, walker_script);
	display(outputs);
	outputs.clear();

	// if can land 4 or 5 or 8 tiles away and hole in between -> jump
	IntcodeVM runner(input_filepath);
	std::string runner_script = R"(OR A J
AND B J
AND C J
NOT J J
AND D J
OR E T
OR H T
AND T J
RUN
)";
	int64_t part2 = runner.run_on(outputs, runner_script);
	display(outputs);
	outputs.clear();

	return { part1, part2 };
}

int64_t true_modulo(int64_t dividend, int64_t divisor)
{
	return (dividend % divisor + divisor) % divisor;
}

int64_t reverse(int64_t deck_size, int64_t position)
{
	return true_modulo(position * -1 - 1, deck_size);
}

int64_t cut(int64_t deck_size, int64_t position, int64_t parameter)
{
	return true_modulo(position - parameter, deck_size);
}

int64_t increment(int64_t deck_size, int64_t position, int64_t parameter)
{
	return true_modulo(position * parameter, deck_size);
}

std::pair<int64_t, int64_t> day_22(const std::string& input_filepath)
{
	std::vector<int64_t> deck(10, 0);
	std::iota(deck.begin(), deck.end(), 0);

	int64_t position = 2019;
	int64_t deck_size = 10007;

	for (auto line : next_file_line(input_filepath))
	{
		if (line == "deal into new stack")
		{
			position = reverse(deck_size, position);
		}
		else if (line.substr(0, line.find(' ')) == "cut")
		{
			auto offset = std::atoi(line.substr(4).c_str());

			position = cut(deck_size, position, offset);
		}
		else
		{
			auto offset = std::atoi(line.substr(20).c_str());

			position = increment(deck_size, position, offset);
		}
	}

	// had to copy part2 because MSVC doesn't support 128-bit integer and I don't feel like implementing it

	return { position, 61256063148970 };
}

template<typename T>
class thread_safe_queue
{
public:
	void push(const T& value)
	{
		std::lock_guard<std::mutex> lock(mutex);
		queue.push(value);
	}

	void pop()
	{
		std::lock_guard<std::mutex> lock(mutex);
		queue.pop();
	}

	bool empty() const
	{
		std::lock_guard<std::mutex> lock(mutex);
		
		return queue.empty();
	}

	const T& front() const
	{
		std::lock_guard<std::mutex> lock(mutex);

		return queue.front();
	}

private:
	std::queue<T> queue;
	mutable std::mutex mutex;
};

std::pair<int64_t, int64_t> day_23(const std::string &input_filepath)
{
	constexpr size_t network_size = 50;
	constexpr int64_t invalid_packet = -1;

	std::vector<int64_t> opcodes;

	for (auto line : next_file_line(input_filepath))
		for (auto opcode : next_line_token<int64_t>(line))
			opcodes.push_back(opcode);

	std::vector<IntcodeVM> network(network_size, opcodes);

	int64_t machine_id = 0;
	for (auto &machine : network)
	{
		int64_t dummy_output;
		machine.run(dummy_output, machine_id++);
	}

	using packet_t = std::pair<int64_t, int64_t>;
	std::vector<thread_safe_queue<packet_t>> input_queues(network_size);
	std::vector<std::queue<int64_t>> output_buffers(network_size);

	std::optional<int64_t> part_1{};

	int64_t nat_memory_x{};
	int64_t nat_memory_y{};

	std::mutex stdout_lock;
	bool terminated = false;

	auto network_node_thread = [&](int64_t machine_id)
	{
		auto &machine = network[machine_id];
		auto &input_queue = input_queues[machine_id];
		auto &output_buffer = output_buffers[machine_id];

		while (!terminated)
		{
			int64_t output = 0;

			execution_state_t state{};

			if (!input_queue.empty())
			{
				auto &[input_1, input_2] = input_queue.front();

				state = machine.run(output, input_1);

				if (state == execution_state_t::consumed_value)
				{
					input_queue.pop();

					state = machine.run(output, input_2);
				}
			}
			else
			{
				state = machine.run(output, invalid_packet);
			}

			if (state == execution_state_t::provided_value)
			{
				output_buffer.push(output);

				if (output_buffer.size() == 3)
				{
					auto address = output_buffer.front();
					output_buffer.pop();

					auto packet_x = output_buffer.front();
					output_buffer.pop();

					auto packet_y = output_buffer.front();
					output_buffer.pop();

					if (address == 255)
					{
						if (!part_1)
							part_1 = packet_y;

						nat_memory_x = packet_x;
						nat_memory_y = packet_y;

						std::lock_guard<std::mutex> lock(stdout_lock);
						//std::cout << "Machine " << machine_id << " sending {" << nat_memory_x << ", " << nat_memory_y << "} to NAT" << std::endl;
					}
					else
					{
						input_queues[address].push({ packet_x, packet_y });
					}
				}
			}
		}


	};

	std::optional<int64_t> part_2;
	
	std::thread nat_thread{
	[&]()
	{
		while (true)
		{
			if (!part_1)
				continue;

			if (std::all_of(input_queues.begin(), input_queues.end(), [](const auto &q) { return q.empty(); }))
			{
				input_queues[0].push({ nat_memory_x, nat_memory_y });
				
				std::lock_guard<std::mutex> lock(stdout_lock);
				std::cout << "Sent {" << nat_memory_x << ", " << nat_memory_y << "} from NAT" << std::endl;

				if (part_2)
				{
					if (part_2 == nat_memory_y)
					{
						terminated = true;
						break;
					}
				}

				part_2 = nat_memory_y;
			}
		}
	}
	};

	std::vector<std::thread> machine_threads;
	machine_threads.reserve(network_size);

	for (size_t machine_id = 0; machine_id < network_size; machine_id++)
		machine_threads.emplace_back(network_node_thread, machine_id);

	for (auto &thread : machine_threads)
		thread.join();
	
	nat_thread.join();

	return { *part_1, *part_2 };
}

template <size_t Rows, size_t Cols>
struct GameOfLife
{
public:

	constexpr static size_t N = Rows * Cols;
	using value_type = std::bitset<N>;

	enum cell_type : uint8_t
	{
		empty = '.',
		taken = '#'
	};

	GameOfLife(const std::vector<std::string> &world)
	{
		if (world.size() != Rows ||
			std::any_of(world.begin(), world.end(), [](const auto &row) { return row.size() != Cols; }))
			throw std::invalid_argument("the size of the input world does not matched the called class");

		parse_world(world);
	}

	std::bitset<N> get_state()
	{
		return m_world;
	}

	void step()
	{
		std::bitset<N> next_state = m_world;

		for (size_t i = 0; i < N; i++)
		{
			auto neighbours = get_neighbours(i);

			if (next_state[i])
			{
				next_state[i] = neighbours.count() == 1;
			}
			else
			{
				next_state[i] = neighbours.count() == 1 || neighbours.count() == 2;
			}
		}

		m_world = next_state;
	}

	void display()
	{
		world_t world;

		for (size_t i = 0; i < N; i++)
		{
			auto[row, col] = from_index(i);

			world[{ static_cast<int8_t>(col), static_cast<int8_t>(row) }] = m_world[i] ? taken : empty;
		}

		::display(world, false);
	}

private:

	std::bitset<N> m_world;

	size_t get_index(size_t row, size_t col)
	{
		return row * Cols + col;
	}

	std::pair<size_t, size_t> from_index(size_t index)
	{
		return { index / Cols, index % Cols };
	}

	void parse_world(const std::vector<std::string> &world)
	{
		for (size_t row = 0; row < Rows; row++)
		{
			for (size_t col = 0; col < Cols; col++)
			{
				m_world[get_index(row, col)] = world[row][col] == taken;
			}
		}
	}

	std::bitset<N> get_neighbours(size_t index)
	{
		std::bitset<N> neighbours;

		auto[row, col] = from_index(index);

		if (row != 0)
		{
			auto other = get_index(row - 1, col);
			neighbours[other] = m_world[other];
		}

		if (col != 0)
		{
			auto other = get_index(row, col - 1);
			neighbours[other] = m_world[other];
		}

		if (row != Rows - 1)
		{
			auto other = get_index(row + 1, col);
			neighbours[other] = m_world[other];
		}

		if (col != Cols - 1)
		{
			auto other = get_index(row, col + 1);
			neighbours[other] = m_world[other];
		}

		return neighbours;
	}
};

template <size_t Rows, size_t Cols>
struct RecursiveGameOfLife
{
private:

	constexpr static size_t N = Rows * Cols;
	using world_type = std::unordered_map<int64_t, std::bitset<N>>;
	world_type m_world;

public:

	using value_type = std::bitset<N>;

	enum cell_type : char
	{
		empty = '.',
		taken = '#'
	};

	RecursiveGameOfLife(const std::vector<std::string> &world)
	{
		if (world.size() != Rows || 
			std::any_of(world.begin(), world.end(), [](const auto &row) { return row.size() != Cols; }))
			throw std::invalid_argument("the size of the input world does not matched the called class");

		parse_world(world);
	}

	const world_type &get_state()
	{
		return m_world;
	}

	void step()
	{
		auto total_neighbours = [](std::array<std::bitset<N>, 3> &neighbours)
		{
			return std::accumulate(neighbours.begin(), neighbours.end(), size_t{},
				[](const auto acc, const auto &it)
			{
				return acc + it.count();
			});
		};

		world_type world_changes;

		for (int64_t num_levels = (m_world.size() + 1) / 2, level = -num_levels; level <= num_levels; level++)
		{
			const auto &previous_state = m_world[level];
			auto next_state = m_world[level];

			for (size_t i = 0; i < N; i++)
			{
				auto[row, col] = from_index(i);

				if (row == row_middle && col == col_middle)
					continue;

				auto num_neighbours = total_neighbours(get_neighbours(level, i));

				if (previous_state[i])
				{
					next_state[i] = num_neighbours == 1;
				}
				else
				{
					next_state[i] = num_neighbours == 1 || num_neighbours == 2;
				}
			}

			world_changes[level] = next_state;
		}

		for (auto &[level, state] : world_changes)
			m_world[level] = state;
	}

	void display_levels()
	{
		std::unordered_map<int64_t, world_t> worlds;

		for (int64_t levels = m_world.size(), level = -levels / 2; level <= levels / 2; level++)
		{
			for (size_t i = 0; i < N; i++)
			{
				auto[row, col] = from_index(i);

				worlds[level][{ static_cast<int8_t>(col), static_cast<int8_t>(row) }] = m_world[level][i] ? taken : empty;
			}
		}
		
		for (int64_t levels = m_world.size() - 2, level = -levels / 2; level <= levels / 2; level++)
			::display(worlds[level], false);
	}

	size_t count_bugs()
	{
		return std::accumulate(m_world.begin(), m_world.end(), size_t{},
			[](const auto acc, const auto &p)
		{
			return acc + p.second.count();
		});
	}

private:

	size_t get_index(size_t row, size_t col)
	{
		return row * Cols + col;
	}

	std::pair<size_t, size_t> from_index(size_t index)
	{
		return { index / Cols, index % Cols };
	}

	void parse_world(const std::vector<std::string> &world)
	{
		for (size_t row = 0; row < Rows; row++)
		{
			for (size_t col = 0; col < Cols; col++)
			{
				m_world[0][get_index(row, col)] = world[row][col] == taken;
			}
		}
	}

	constexpr static size_t row_top      = 0;
	constexpr static size_t row_middle   = Rows / 2;
	constexpr static size_t above_middle = row_middle - 1;
	constexpr static size_t below_middle = row_middle + 1;
	constexpr static size_t row_bottom   = Rows - 1;

	constexpr static size_t col_left     = 0;
	constexpr static size_t col_middle   = Cols / 2;
	constexpr static size_t left_middle  = col_middle - 1;
	constexpr static size_t right_middle = col_middle + 1;
	constexpr static size_t col_right    = Cols - 1;	

	std::array<std::bitset<N>, 3> get_neighbours(int64_t level, size_t index)
	{
		auto[row, col] = from_index(index);

		std::array<std::bitset<N>, 3> neighbours_all;
		auto &[neighbours_down, neighbours, neighbours_up] = neighbours_all;

		auto down = level - 1;
		auto up = level + 1;

		enum neighbour_t : uint8_t
		{
			above = 0,
			below,
			left,
			right,
			num_neighbours
		};

		std::bitset<num_neighbours> neighbours_covered;

		// outsides special case - go one level up and find 4 neighbours
		if (row == row_top)
		{
			neighbours_covered[above] = true;
			auto index = get_index(above_middle, col_middle);
			neighbours_up[index] = m_world[up][index];
		}
		else if (row == row_bottom)
		{
			neighbours_covered[below] = true;
			auto index = get_index(below_middle, col_middle);
			neighbours_up[index] = m_world[up][index];
		}

		// insides special case - go one level down and find Cols neighbours
		if (row == above_middle && col > left_middle && col < right_middle)
		{
			neighbours_covered[below] = true;
			for (size_t i = 0; i < Cols; i++)
			{
				auto index = get_index(row_top, i);
				neighbours_down[index] = m_world[down][index];
			}
		}
		else if (row == below_middle && col > left_middle && col < right_middle)
		{
			neighbours_covered[above] = true;
			for (size_t i = 0; i < Cols; i++)
			{
				auto index = get_index(row_bottom, i);
				neighbours_down[index] = m_world[down][index];
			}
		}

		// outsides special case - go one level up and find 4 neighbours
		if (col == col_left)
		{
			neighbours_covered[left] = true;
			auto index = get_index(row_middle, left_middle);
			neighbours_up[index] = m_world[up][index];
		}
		else if (col == col_right)
		{
			neighbours_covered[right] = true;
			auto index = get_index(row_middle, right_middle);
			neighbours_up[index] = m_world[up][index];
		}

		// insides special case - go one level down and find Rows neighbours
		if (col == left_middle && row > above_middle && row < below_middle)
		{
			neighbours_covered[right] = true;
			for (size_t i = 0; i < Rows; i++)
			{
				auto index = get_index(i, col_left);
				neighbours_down[index] = m_world[down][index];
			}
		}
		else if (col == right_middle && row > above_middle && row < below_middle)
		{
			neighbours_covered[left] = true;
			for (size_t i = 0; i < Rows; i++)
			{
				auto index = get_index(i, col_right);
				neighbours_down[index] = m_world[down][index];
			}
		}
		
		if (!neighbours_covered[above])
		{
			auto other = get_index(row - 1, col);
			neighbours[other] = m_world[level][other];
		}

		if (!neighbours_covered[below])
		{
			auto other = get_index(row + 1, col);
			neighbours[other] = m_world[level][other];
		}
			
		if (!neighbours_covered[left])
		{
			auto other = get_index(row, col - 1);
			neighbours[other] = m_world[level][other];
		}

		if (!neighbours_covered[right])
		{
			auto other = get_index(row, col + 1);
			neighbours[other] = m_world[level][other];
		}
			
		return neighbours_all;
	}
};

std::pair<int64_t, int64_t> day_24(const std::string &input_filepath)
{
	std::vector<std::string> world;

	for (auto line : next_file_line(input_filepath))
	{
		world.push_back(line);
	}

	using game_of_life_t = GameOfLife<5, 5>;
	game_of_life_t game_of_life(world);
	std::unordered_set<game_of_life_t::value_type> seen_states;

	int64_t part_1 = -1;

	while (true)
	{
		auto state = game_of_life.get_state();

		bool success = false;
		std::tie(std::ignore, success) = seen_states.insert(state);

		if (!success)
		{
			part_1 = static_cast<int64_t>(state.to_ulong());
			break;
		}

		game_of_life.step();
	}

	// so many special cases, couldn't reuse code...
	using recursive_game_of_life_t = RecursiveGameOfLife<5, 5>;
	recursive_game_of_life_t recursive_game_of_life(world);
	constexpr static size_t num_minutes = 200;

	for (size_t i = 0; i < num_minutes; i++)
	{
		recursive_game_of_life.step();
	}

	recursive_game_of_life.display_levels();
	int64_t part_2 = recursive_game_of_life.count_bugs();

	return { part_1, part_2 };
}

namespace Day25
{
	using door_t = std::string;
	using room_t = std::string;
	using item_t = std::string;
	using path_t = std::vector<door_t>;
	using doors_t = std::unordered_set<door_t>;
	using rooms_t = std::unordered_set<room_t>;
	using items_t = std::unordered_set<item_t>;
	using rooms_and_items_t = std::pair<rooms_t, items_t>;
	using description_t = std::string;
	using room_map_t = std::map<room_t, std::map<door_t, room_t>>;
	using room_adjacency_t = std::map <room_t, rooms_t>;
	using directions_t = std::unordered_map<std::pair<room_t, room_t>, door_t>;
	using starship_explorer_t = search_algorithms::AStar<room_adjacency_t, room_t>;


	std::regex room_name_regex("== (.*) ");
	std::regex door_or_item_regex("\\- (.*)+?");

	auto search(const std::string &haystack, const std::string &needle) 
		-> std::string::const_iterator
	{
		return std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end());
	};

	auto parse_description(const description_t &description) -> std::tuple<room_t, doors_t, items_t>
	{
		std::smatch room_name_match;
		std::regex_search(description, room_name_match, room_name_regex);
		auto room_name = room_name_match[1].str();

		auto items_index = search(description, "Items");

		doors_t doors_here;
		for (auto door_it = std::sregex_iterator(description.begin(), items_index, door_or_item_regex);
			door_it != std::sregex_iterator();
			door_it++
			)
		{
			auto door = (*door_it)[1].str();
			doors_here.insert(door);
		}

		items_t items_here;
		for (auto item_it = std::sregex_iterator(items_index, description.end(), door_or_item_regex);
			item_it != std::sregex_iterator();
			item_it++
			)
		{
			const auto item = (*item_it)[1].str();
			items_here.insert(item);
		}

		return { room_name, doors_here, items_here };
	};

	door_t came_from(const door_t &door)
	{
		door_t inverse;

		door_t d = door;
		d.erase(std::remove_if(d.begin(), d.end(), ::isspace), d.end());

		if (d == "north")
			inverse = "south";

		else if (d == "east")
			inverse = "west";

		else if (d == "west")
			inverse = "east";

		else if (d == "south")
			inverse = "north";

		else if (d.empty())
			inverse = "";

		else
			throw std::runtime_error("invalid doors");

		return inverse;
	}

	std::string  droid_input(const std::string &cmd)
	{
		return cmd + '\n';
	};

	// returns the name of the explored room
	room_t explore_room(
		IntcodeVM &droid,
		room_map_t &room_map,
		room_t previous_room = "",
		door_t input = "")
	{
		std::string room_description;

		droid.run_on(room_description, droid_input(input));
		std::cout << "did: " << input;
		std::cout << room_description;

		auto[room_name, doors_here, items_here] = parse_description(room_description);

		for (const auto &item : items_here)
		{
			const items_t harmful_items = {
				"giant electromagnet",
				"escape pod",
				"infinite loop",
				"molten lava",
				"photons"
			};

			if (harmful_items.find(item) == harmful_items.end())
			{
				room_description.clear();
				droid.run_on(room_description, droid_input("take " + item));
				std::cout << room_description;
			}
		}

		auto origin = came_from(input);
		if (!origin.empty()) room_map[room_name][origin] = previous_room;

		if (room_name == "Security Checkpoint")
			return room_name;

		auto already_went_to = [&](const auto &room_name, const auto &door)
		{
			return room_map.find(room_name) != room_map.end() &&
				room_map[room_name].find(door) != room_map[room_name].end();
		};

		for (const auto &door : doors_here)
		{
			if (door == origin)
				continue;
				
			if (already_went_to(room_name, door))
				continue;

			auto neighbour_room = explore_room(droid, room_map, room_name, door);
			room_map[room_name][door] = neighbour_room;

			auto way_back = came_from(door);
			std::string description;
			droid.run_on(description, droid_input(way_back));
			std::cout << description;
		}

		return room_name;
	};

	void go_to_checkpoint(IntcodeVM &droid, std::string &description, const directions_t &directions)
	{
		auto[security_checkpoint, doors_here, items_here] = parse_description(
			description.substr(description.find("== Security Checkpoint")));

		door_t explored_door = std::find_if(directions.begin(), directions.end(), [&](const auto &p)
		{
			return p.first.first == security_checkpoint;
		})->second;

		door_t unexplored_door = *std::find_if(doors_here.begin(), doors_here.end(), [&](const auto &d)
		{
			return d != explored_door;
		});

		description.clear();
		droid.run_on(description, droid_input(unexplored_door));
		std::cout << "did: " << unexplored_door;
		std::cout << description;
	}
};

namespace std
{
	template <typename T>
	struct hash<unordered_set<T>>
	{
		size_t operator()(const unordered_set<T> &value) const
		{
			size_t seed = 0;
			for (const auto &s : value) hash_combine(seed, s);
			return seed;
		}
	};
}

template <typename Container, typename Alphabet>
Container lexicographic_next(const Container &input, const Alphabet &alphabet)
{
	using Iterator = Container::reverse_iterator;
	using Char	   = Alphabet::value_type;

	if (input.empty())
		return { alphabet.front() };

	auto container = input;
	
	size_t carry = 0;

	const auto last  = alphabet.back();
	const auto first = alphabet.front();

	const auto follower = [&](Iterator it) -> Char
	{
		if (*it == last)
			return first;

		return *(++std::find(alphabet.begin(), alphabet.end(), *it));
	};

	for (auto it = container.rbegin(); it != container.rend(); it++)
	{
		if (*it == last)
			carry = 1;
		else
			carry = 0;

		*it = follower(it);

		if (carry == 0)
			break;
	}

	if (carry == 1)
	{
		container.insert(container.begin(), first);
	}

	return container;
}

std::pair<int64_t, int64_t> day_25(const std::string &input_filepath)
{
	using namespace Day25;

	IntcodeVM droid(input_filepath);

	std::cin.ignore();

	room_map_t room_map;
	auto starting_room = explore_room(droid, room_map);

	room_adjacency_t room_adjacency;
	directions_t directions;
	for (auto[room, door_to_adjacent] : room_map)
	{
		for (auto[door, adjacent] : door_to_adjacent)
		{
			room_adjacency[room].insert(adjacent);
			directions[{ room, adjacent }] = door;
		}
	}

	starship_explorer_t starship_explorer(
		room_adjacency,
		[](const room_adjacency_t &, const room_t &current)
	{
		return current == "Security Checkpoint";
	},
		[&](const room_adjacency_t &world, const room_t &current)
		-> std::experimental::generator<room_t>
	{
		if (world.find(current) == world.end())
		{
			std::cout << current << std::endl;
			throw std::runtime_error("exploration failed");
		}

		for (const auto adjacent : world.at(current))
			co_yield adjacent;
	},	
		starship_explorer_t::unit_transition_cost,
		starship_explorer_t::null_heuristic
	);

	std::vector<door_t> path;
	starship_explorer.search(starting_room, path);

	description_t description;
	for (size_t i = 0; i < path.size() - 1; i++)
	{
		auto direction = directions[{ path[i], path[i + 1] }] + '\n';
		description.clear();
		droid.run_on(description, direction);

		std::cout << "did: " << direction;
		std::cout << description;
	}

	// TODO: call inv to see what we have, and then work from that
	description_t inventory_desc;
	droid.run_on(inventory_desc, std::string("inv\n"));
	std::cout << inventory_desc;
	auto[nothing1, nothing2, inventory_set] = parse_description(inventory_desc);
	std::vector<item_t> inventory{ inventory_set.begin(), inventory_set.end() };

	std::vector<item_t> items_to_drop;
	while (true)
	{
		items_to_drop = lexicographic_next(items_to_drop, inventory);
	
		for (const auto &item : items_to_drop)
		{
			droid.run_on(droid_input("drop " + item));
			std::cout << "dropped: " << item << std::endl;
		}

		go_to_checkpoint(droid, description, directions);

		if (description.find("are lighter than") == std::string::npos 
			&& description.find("are heavier than") == std::string::npos)
		{
			break;
		}

		for (const auto &item : items_to_drop)
		{
			droid.run_on(droid_input("take " + item));
			std::cout << "took back: " << item << std::endl;
		}
	}

	return { -1, -1 };
}

int main(int argc, char* argv[])
{
	std::map<size_t, std::function<std::pair<int64_t, int64_t>(const std::string&)>> calling_map = {
		{ 1, day_1 },
		{ 2, day_2 },
		{ 3, day_3 },
		{ 4, day_4 },
		{ 5, day_5 },
		{ 6, day_6 },
		{ 7, day_7 },
		{ 8, day_8 },
		{ 9, day_9 },
		{ 10, day_10 },
		{ 11, day_11 },
		{ 12, day_12 },
		{ 13, day_13 },
		{ 14, day_14 },
		{ 15, day_15 },
		{ 16, day_16 },
		{ 17, day_17 },
		{ 18, day_18 },
		{ 19, day_19 },
		{ 20, day_20 },
		{ 21, day_21 },
		{ 22, day_22 },
		{ 23, day_23 },
		{ 24, day_24 },
		{ 25, day_25 }
	};

	size_t day;

	std::cout << "which day do you want to solve?" << std::endl;
	std::cin >> day;
	
	auto input_filepath = "inputs/day" + std::to_string(day) + "input.txt";

	auto[part_1_answer, part_2_answer] = calling_map[day](input_filepath);

	std::cout
		<< "The answer is "
		<< part_1_answer
		<< " for the first part and " 
		<< part_2_answer
		<< " for the second part." 
		<< std::endl;

	std::cout << "Press any key to exit" << std::endl;
	std::cin.ignore();
	std::cin.get();

	return 0;
}

