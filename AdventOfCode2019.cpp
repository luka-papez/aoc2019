#include <algorithm>
#include <cctype>
#include <functional>
#include <iomanip>
#include <numeric>

#include <array>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <queue>
#include <deque>

#include "intcode.hpp"
#include "input_utilities.hpp"

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
	part1.run(dummy, 0);

	for (int64_t i = 0; i < 100; i++)
	{
		for (int64_t j = 0; j < 100; j++)
		{
			IntcodeVM part2(opcodes);
			part2.memory[1] = i;
			part2.memory[2] = j;
			part2.run(dummy, 0);

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
			amplifier.run(amplifier_output, 0);
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

				if (amplifier.run(amplifier_output, 0) != execution_state_t::provided_value)
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

	for (auto input : { 1, 2 })
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

			auto current_color = panel_colors[{ position_x, position_y }] == ' ' ? 0 : 1;

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

	enum arrow_key_t : int64_t
	{
		up = 72,
		down = 80,
		left = 75,
		right = 77
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

using position_t = std::pair<int64_t, int64_t>;

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

	case direction_t::east:
		lhs.first--;
		break;

	case direction_t::west:
		lhs.first++;
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

	auto manhattan_distance = [](position_t p1, position_t p2) -> int64_t
	{
		return std::abs(p2.second - p1.second) + std::abs(p2.first - p1.first);
	};
	std::vector<position_t> path_to_oxygen;
	a_star(droid.world, {}, droid.oxygen_position, manhattan_distance, path_to_oxygen);

	int64_t steps_taken = flood_fill(droid.world, droid.oxygen_position);

	return { path_to_oxygen.size() - 1, steps_taken };
}

int main(int argc, char* argv[])
{
	size_t day;

	std::cout << "which day do you want to solve?" << std::endl;
	std::cin >> day;

	auto input_filepath = "inputs/day" + std::to_string(day) + "input.txt";

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
		{ 15, day_15 }
	};

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

