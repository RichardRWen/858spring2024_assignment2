#include <iostream>
#include <string>

#include "parlay/primitives.h"
#include "parlay/sequence.h"
#include "parlay/internal/get_time.h"

#include "hotspots.h"
#include "examples/helper/graph_utils.h"

using vertex = int;
using nested_seq = parlay::sequence<parlay::sequence<vertex>>;
using graph = nested_seq;
using utils = graph_utils<vertex>;

int main(int argc, char* argv[]) {
	auto usage = "Usage: ./hotspots <n> || ./hotspots <filename>";
	if (argc != 2) std::cout << usage << std::endl;
	else {
		long n = 0;
		graph G;
		try { n = std::stol(argv[1]); }
		catch (...) {}
		if (n == 0) {
			G = utils::read_symmetric_graph_from_file(argv[1]);
			n = G.size();
		} else {
			G = utils::rmat_symmetric_graph(n, 20 * n);
		}

		// select n / 100 vertices as the hotspots
		parlay::random_generator gen(0);
		std::uniform_int_distribution<vertex> dis(0, G.size() - 1);
		auto U = parlay::tabulate(n / 100 + 1, [&] (long i) {
				auto r = gen[i];
				return dis(r);});

		utils::print_graph_stats(G);
		std::cout << "num hotspots = " << U.size() << std::endl;

		parlay::sequence<vertex> result;
		parlay::internal::timer t("time");
		for (int i = 0; i < 3; i++) {
			result = hotspots(U, G);
			t.next("hotspot");
		}

		// find which vertex appears most commonly in the result
		auto r = parlay::histogram_by_index(result, G.size() + 1);
		std::cout << "hottest spot: " << (parlay::max_element(r) - r.begin()) << std::endl;
	}
}
