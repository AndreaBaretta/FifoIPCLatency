/*
 * core_to_core.hpp
 *
 *  Created on: Feb 17, 2021
 *      Author: andrea
 */

#pragma once

#include "fifo.hpp"
#include "benchmark.hpp"
#include <sstream>
#include <vector>
#include <optional>

namespace benchmark {

	template <
		std::size_t cache_line_size,
		bool avg,
		bool test_mode,
		bool use_memcpy,
		bool use_avx256
	>
	class core_to_core_t {

		fifo_t<cache_line_size, use_memcpy, use_avx256> fifo_1;
		fifo_t<cache_line_size, use_memcpy, use_avx256> fifo_2;

	public:
		const std::size_t core_1;
		const std::size_t core_2;
		const std::size_t num_tries;

		const std::size_t fifo_size;
		const std::size_t message_size;

		std::vector<int> thread_1_round_time_nano;
		std::vector<int> thread_2_round_time_nano;

		using cache_line_type = fifo_t<cache_line_size, use_memcpy, use_avx256>::cache_line_type;
		using aligned_cache_line_type = fifo_t<cache_line_size, use_memcpy, use_avx256>::aligned_cache_line_type;

		class thread_1_t {
			core_to_core_t& core_to_core;
		public:
			thread_1_t(core_to_core_t& core_to_core) : core_to_core(core_to_core) {
			};
			thread_1_t(const thread_1_t&) = delete;
			thread_1_t(thread_1_t&&) = delete;

			int operator()() {
				benchmark::pin_this_thread_to_core(core_to_core.core_1);
				std::vector<aligned_cache_line_type> msg_sent{core_to_core.message_size};
				std::vector<aligned_cache_line_type> msg_read{core_to_core.message_size};


//				std::cerr << "warmup" << std::endl;
//				for (std::size_t i = 0; i < core_to_core.num_tries; ++i) {
//					while (!core_to_core.fifo_1.try_write_message(msg_sent)) {}
//					while (!core_to_core.fifo_2.try_read_message(msg_read)) {}
//				}
//
//				std::cout << "Past warmup" << std::endl;

				long t1;
				long start = benchmark::get_thread_time_nano();

				for (std::size_t i = 0; i < core_to_core.num_tries; ++i, (i==core_to_core.num_tries) ? ((std::cout << " T1-DONE" << std::endl), ((void)0)) : (void)0) {

					if constexpr (!avg) {
						t1 = benchmark::get_thread_time_nano();
					}
//					if (i%1000 == 0) {
//						std::cout << i/1000%10;
//						std::cout.flush();
//					}
//					if ((core_to_core.num_tries - i) < 100) {
//						std::cout << '.' << (i%10);
//						std::cout.flush();
//					}

					//Creates the message
//					for (std::size_t x = 0; x < core_to_core.message_size; ++x) {
//						aligned_cache_line_type& cache_line = msg_sent[x];
//						cache_line[0] = -(i+1);
//					}

//					std::cout << "Generated message" << std::endl;
//					for (const auto& aligned_cache_line : msg_sent) {
//						for (const long v : aligned_cache_line.cache_line) std::cout << v << ", ";
//					}
//					std::cout << std::endl;
//					std::cout << "At iteration i = " << i << std::endl;
//					if constexpr (!avg) {
//						t1 = benchmark::get_thread_time_nano();
//					}
					while (!core_to_core.fifo_1.try_write_message(msg_sent)) {}
					while (!core_to_core.fifo_2.try_read_message(msg_read)) {}

//					msg_read.value()[0][0] = 22;

//					std::cout << "Received message back " << std::endl;

					if constexpr (test_mode) {
//						std::cout << "In test mode" << std::endl;
						if (msg_read.value() != msg_sent) {
							std::cout << "Failed at i = " << i << std::endl;
							std::cout << "Expecting: ";
							for (const auto& aligned_cache_line : msg_sent) {
								for (const long v : aligned_cache_line.cache_line) std::cout << v << ", ";
							}
							std::cout << std::endl;
							std::cout << " Received: ";
							for (const auto& aligned_cache_line : msg_read.value()) {
								for (const long v : aligned_cache_line.cache_line) std::cout << v << ", ";
							}
							std::cout << std::endl;
							throw std::string("Bah humbug");
						}
					}

					static_assert(avg);
					if constexpr (!avg) {
						core_to_core.thread_1_round_time_nano[i] = benchmark::get_thread_time_nano() - t1;
					}
				}


				long end = benchmark::get_thread_time_nano();
				const auto latency = (end - start)/static_cast<long>(core_to_core.num_tries);
				const auto cache_to_cache = latency / 6;
				std::cout << "Thread 1 core: " << core_to_core.core_2 << " end: " << end << "  start: " << start << "  avg: " << latency << " cache_to_cache: " << cache_to_cache << " ns" << std::endl;
				std::cout.flush();
				return 0;
			}
		};

		class thread_2_t {
			core_to_core_t& core_to_core;
		public:
			thread_2_t(core_to_core_t& core_to_core) : core_to_core(core_to_core) {};
			thread_2_t(const thread_1_t&) = delete;
			thread_2_t(thread_1_t&&) = delete;

			int operator()() {
				benchmark::pin_this_thread_to_core(core_to_core.core_2);
//				std::vector<aligned_cache_line_type> msg{core_to_core.message_size};
				std::vector<aligned_cache_line_type> msg_read{core_to_core.message_size};

//				for (std::size_t i = 0; i < core_to_core.num_tries; ++i) {
//					while (!core_to_core.fifo_1.try_read_message(msg_read)) {}
//					while (!core_to_core.fifo_2.try_write_message(msg_read)) {}
//				}

				std::cout << "Thread 2: starting for-loop" << std::endl;
				long t1;

				for (std::size_t i = 0; i < core_to_core.num_tries; ++i) {
//					t1 = benchmark::get_thread_time_nano();
//						while (!core_to_core.fifo_1.try_read_message().has_value()) {}

					while (!core_to_core.fifo_1.try_read_message(msg_read)) {}

//					std::cout << "Got message" << std::endl;
//						std::cout << "Received in thread 2: ";
//						for (const auto& aligned_cache_line : msg_read.value()) {
//							for (const long v : aligned_cache_line.cache_line) std::cout << v << ", ";
//						}
//						std::cout << std::endl;
					while (!core_to_core.fifo_2.try_write_message(msg_read)) {}
//					core_to_core.thread_2_round_time_nano[i] = benchmark::get_thread_time_nano() - t1;
				}

				std::cout << "Thread 2, over" << std::endl;
				return 0;
			}
		};

		thread_1_t thread_1;
		thread_2_t thread_2;

		core_to_core_t(const int core_1, const int core_2, const int num_tries, const std::size_t fifo_size, const std::size_t message_size) :
			fifo_1(fifo_size, message_size), fifo_2(fifo_size, message_size), core_1(core_1), core_2(core_2), num_tries(num_tries), fifo_size(fifo_size), message_size(message_size),
			thread_1_round_time_nano(num_tries), thread_2_round_time_nano(num_tries), thread_1(*this), thread_2(*this) {}
	};
}
