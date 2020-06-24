#pragma once

#include <algorithm>
#include <experimental/generator>
#include <functional>
#include <map>
#include <unordered_map>
#include <queue>
#include <tuple>
#include <type_traits>


namespace search_algorithms {

	static constexpr auto infinite_cost_s = std::numeric_limits<int64_t>::max();

	template <typename T, typename = std::void_t<>>
	struct is_std_hashable : std::false_type{};

	template <typename T>
	struct is_std_hashable<
		T, 
		std::void_t<
			decltype(
				std::declval<std::hash<T>>()
			)
		>
	> : std::true_type{};

	template <typename T>
	constexpr bool is_std_hashable_v = is_std_hashable<T>::value;

	template <typename T1, typename T2>
	using mapping_t = std::conditional_t<
		is_std_hashable_v<T1>,
		std::unordered_map<T1, T2>,
		std::map<T1, T2>
	>;

	template <
		typename WorldType,
		typename NodeType
	>
	struct AStar
	{
		WorldType &world;

		std::function<bool(WorldType&, const NodeType&, const NodeType&)> explore;
		std::function<bool(const WorldType&, const NodeType&)> solution_found;
		std::function<std::experimental::generator<NodeType>(const WorldType&, const NodeType&)> neighbourhood;
		std::function<int64_t(const WorldType&, const NodeType&, const NodeType&)> transition_cost;
		std::function<int64_t(const WorldType&, const NodeType&, const NodeType&)> guiding_heuristic;

		AStar(
			WorldType &world,
			std::function<bool(WorldType&, const NodeType&, const NodeType&)> explore,
			std::function<bool(const WorldType&, const NodeType&)> solution_found,
			std::function<std::experimental::generator<NodeType>(const WorldType&, const NodeType&)> neighbourhood,
			std::function<int64_t(const WorldType&, const NodeType&, const NodeType&)> transition_cost,
			std::function<int64_t(const WorldType&, const NodeType&, const NodeType&)> guiding_heuristic
		) : world(world),
			explore(explore),
			solution_found(solution_found),
			neighbourhood(neighbourhood),
			transition_cost(transition_cost),
			guiding_heuristic(guiding_heuristic)
		{

		}

		AStar(
			WorldType &world,
			std::function<bool(const WorldType&, const NodeType&)> solution_found,
			std::function<std::experimental::generator<NodeType>(const WorldType&, const NodeType&)> neighbourhood,
			std::function<int64_t(const WorldType&, const NodeType&, const NodeType&)> transition_cost,
			std::function<int64_t(const WorldType&, const NodeType&, const NodeType&)> guiding_heuristic
		) : AStar(world, trivial_explore, solution_found, neighbourhood, transition_cost, guiding_heuristic)
			{ }

		/**
			In case of success returns the shortest found path.
			In case of failure returns the path to last explored node.
		*/
		bool search(const NodeType &start, std::vector<NodeType> &path)
		{
			using node_with_cost_t = std::pair<NodeType, int64_t>;
			auto cost_comparer = [](const node_with_cost_t &p1, const node_with_cost_t &p2)
			{
				return p1.second > p2.second;
			};

			using storage_t = std::vector<node_with_cost_t>;
			
			std::priority_queue<storage_t::value_type, storage_t, decltype(cost_comparer)> frontier(cost_comparer);
			mapping_t<NodeType, NodeType> came_from;
			mapping_t<NodeType, int64_t> cost_to;

			auto reconstruct_path = [&](const NodeType &start, NodeType current)
			{
				path.push_back(current);

				while (current != start)
				{
					current = came_from[current];
					path.push_back(current);
				}

				std::reverse(path.begin(), path.end());
			};

			frontier.push({ start, 0 });
			came_from[start] = start;
			cost_to[start] = 0;

			auto current = start;
			NodeType last_explored_node{};

			while (!frontier.empty())
			{
				auto[wanted_node, cost] = frontier.top(); (void) cost;
				frontier.pop();

				if (!explore(world, current, wanted_node))
				{
					// this path is no longer available
					continue;
				}

				current = wanted_node;
				last_explored_node = current;					

				if (solution_found(world, current))
				{
					reconstruct_path(start, current);

					return true;
				}

				for (auto &next : neighbourhood(world, current))
				{
					auto new_cost = cost_to[current] + transition_cost(world, current, next);

					if (!cost_to.count(next) || new_cost < cost_to[next])
					{
						cost_to[next] = new_cost;
						frontier.push({ next, new_cost + guiding_heuristic(world, current, next) });
						came_from[next] = current;
					}
				}
			}

			reconstruct_path(start, last_explored_node);

			return false;
		}

		static int64_t unit_transition_cost(const WorldType&, const NodeType&, const NodeType&)
		{
			return 1;
		}

		static int64_t null_heuristic(const WorldType&, const NodeType&, const NodeType&)
		{
			return 0;
		}

		static bool no_solution(const WorldType&, const NodeType&)
		{
			return false;
		}

		static bool trivial_explore(WorldType&, const NodeType&, const NodeType&)
		{
			return true;
		}
	};
}