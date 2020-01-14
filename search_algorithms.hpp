#pragma once

#include <algorithm>
#include <experimental/generator>
#include <functional>
#include <queue>
#include <tuple>

namespace search_algorithms {
	static constexpr auto infinite_cost_s = std::numeric_limits<int64_t>::max();

	template <
		typename WorldType,
		typename NodeType
	>
	struct AStar
	{
		const WorldType &world;

		std::function<std::experimental::generator<NodeType>(const WorldType&, const NodeType&)> neighbourhood;
		std::function<bool(const WorldType&, const NodeType&)> solution_found;
		std::function<int64_t(const WorldType&, const NodeType&, const NodeType&)> transition_cost;
		std::function<int64_t(const WorldType&, const NodeType&, const NodeType&)> guiding_heuristic;

		AStar(
			const WorldType &world,
			std::function<bool(const WorldType&, const NodeType&)> solution_found,
			std::function<std::experimental::generator<NodeType>(const WorldType&, const NodeType&)> neighbourhood,
			std::function<int64_t(const WorldType&, const NodeType&, const NodeType&)> transition_cost,
			std::function<int64_t(const WorldType&, const NodeType&, const NodeType&)> guiding_heuristic
		) : world(world),
			solution_found(solution_found),
			neighbourhood(neighbourhood),
			transition_cost(transition_cost),
			guiding_heuristic(guiding_heuristic)
		{

		}

		bool search(const NodeType &start, std::vector<NodeType> &path)
		{
			using node_with_cost_t = std::pair<NodeType, int64_t>;
			auto cost_comparer = [](const node_with_cost_t &p1, const node_with_cost_t &p2)
			{
				return p1.second > p2.second;
			};

			using storage_t = std::vector<node_with_cost_t>;
			std::priority_queue<storage_t::value_type, storage_t, decltype(cost_comparer)> frontier(cost_comparer);
			std::unordered_map<NodeType, NodeType> came_from;
			std::unordered_map<NodeType, int64_t> cost_to;

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

			while (!frontier.empty())
			{
				auto[current, cost] = frontier.top(); 
				(void)cost; // supress unused warnings because VS doesn't support some attributes
				frontier.pop();

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

			return false;
		}
	};

	template <
		typename WorldType,
		typename NodeType
	>
	int64_t unit_transition_cost(const WorldType&, const NodeType&, const NodeType&)
	{
		return 1;
	}

	template <
		typename WorldType,
		typename NodeType
	>
	int64_t null_heuristic(const WorldType&, const NodeType&, const NodeType&)
	{
		return 0;
	}
}