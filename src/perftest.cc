
#include <cstddef>
#include <cstdlib>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/statusor.h"
#include "absl/time/time.h"

#include "src/allocator_interface.h"
#include "src/heap_factory.h"
#include "src/tracefile_reader.h"

namespace bench {

struct TimeOp {
  TraceLine line;
  size_t arg_idx;
  size_t res_idx;
};

namespace {

std::pair<std::vector<TimeOp>, size_t> ComputeOps(
    const TracefileReader& reader) {
  absl::flat_hash_map<void*, size_t> idx_map;
  std::vector<size_t> free_idxs;
  size_t next_idx = 0;

  auto get_new_idx = [&free_idxs, &next_idx]() {
    if (free_idxs.empty()) {
      return next_idx++;
    }

    size_t res = free_idxs.back();
    free_idxs.pop_back();
    return res;
  };

  auto free_idx = [&free_idxs](size_t idx) { free_idxs.push_back(idx); };

  std::vector<TimeOp> ops;
  ops.reserve(reader.size());
  for (const TraceLine& line : reader) {
    switch (line.op) {
      case TraceLine::Op::kMalloc: {
        size_t idx = get_new_idx();
        ops.push_back(TimeOp{
            .line = line,
            .res_idx = idx,
        });
        idx_map[line.result] = idx;
        break;
      }
      case TraceLine::Op::kCalloc: {
        size_t idx = get_new_idx();
        ops.push_back(TimeOp{
            .line = line,
            .res_idx = idx,
        });
        idx_map[line.result] = idx;
        break;
      }
      case TraceLine::Op::kRealloc: {
        if (line.input_ptr == nullptr) {
          size_t idx = get_new_idx();
          ops.push_back(TimeOp{
              .line = {
                .op = TraceLine::Op::kMalloc,
                .input_size = line.input_size,
                .result = line.result,
              },
              .res_idx = idx,
          });
          idx_map[line.result] = idx;
          break;
        }

        if (line.input_ptr != line.result) {
          size_t old_idx = idx_map[line.input_ptr];
          size_t idx = get_new_idx();
          ops.push_back(TimeOp{
              .line = line,
              .arg_idx = old_idx,
              .res_idx = idx,
          });
          idx_map[line.result] = idx;
          idx_map.erase(line.input_ptr);
          free_idx(old_idx);
        } else {
          ops.push_back(TimeOp{
              .line = line,
              .arg_idx = idx_map[line.input_ptr],
              .res_idx = idx_map[line.input_ptr],
          });
        }
        break;
      }
      case TraceLine::Op::kFree: {
        if (line.input_ptr == nullptr) {
          break;
        }
        size_t idx = idx_map[line.input_ptr];
        ops.push_back(TimeOp{
            .line = line,
            .arg_idx = idx,
        });
        idx_map.erase(line.input_ptr);
        free_idx(idx);
        break;
      }
    }
  }

  return { ops, next_idx };
}

}  // namespace

// Runs at least 1000000 ops, and returns the average MOps/s.
absl::StatusOr<double> TimeTrace(TracefileReader& reader,
                                 HeapFactory& heap_factory,
                                 size_t min_desired_ops) {
  size_t num_repetitions = (min_desired_ops - 1) / reader.size() + 1;

  auto [ops, max_allocs] = ComputeOps(reader);
  std::vector<void*> ptrs(max_allocs);

  heap_factory.Reset();

  absl::Time start = absl::Now();
  initialize_heap(heap_factory);
  for (size_t t = 0; t < num_repetitions; t++) {
    for (const TimeOp& op : ops) {
      switch (op.line.op) {
        case TraceLine::Op::kMalloc: {
          void* ptr = malloc(op.line.input_size);
          ptrs[op.res_idx] = ptr;
          break;
        }
        case TraceLine::Op::kCalloc: {
          void* ptr = calloc(op.line.nmemb, op.line.input_size);
          ptrs[op.res_idx] = ptr;
          break;
        }
        case TraceLine::Op::kRealloc: {
          void* ptr = realloc(ptrs[op.arg_idx], op.line.input_size);
          ptrs[op.res_idx] = ptr;
          break;
        }
        case TraceLine::Op::kFree: {
          free(ptrs[op.arg_idx]);
          break;
        }
      }
    }
  }
  absl::Time end = absl::Now();

  size_t total_ops = num_repetitions * ops.size();
  double seconds = absl::FDivDuration((end - start), absl::Seconds(1));
  return total_ops / seconds / 1000000;
}

}  // namespace bench
