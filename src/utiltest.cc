#include "absl/container/flat_hash_map.h"
#include "absl/flags/flag.h"
#include "absl/status/statusor.h"

#include "src/allocator_interface.h"
#include "src/heap_factory.h"
#include "src/tracefile_reader.h"

ABSL_FLAG(bool, effective_util, false,
          "If set, uses a \"more fair\" measure of memory utilization, "
          "rounding up each allocation size to its alignment requirement.");

namespace bench {

size_t RoundUp(size_t size) {
  if (!absl::GetFlag(FLAGS_effective_util)) {
    return size;
  }

  if (size <= 8) {
    return 8;
  }
  return (size + 0xf) & ~0xf;
}

absl::StatusOr<double> MeasureUtilization(TracefileReader& reader,
                                          HeapFactory& heap_factory) {
  absl::flat_hash_map<void*, std::pair<void*, size_t>> id_to_ptrs;

  heap_factory.Reset();
  initialize_heap(heap_factory);

  size_t total_allocated_bytes = 0;
  size_t max_allocated_bytes = 0;
  for (TraceLine line : reader) {
    switch (line.op) {
      case TraceLine::Op::kMalloc: {
        void* ptr = malloc(line.input_size);
        if (ptr != nullptr) {
          id_to_ptrs[line.result] = { ptr, line.input_size };
          total_allocated_bytes += RoundUp(line.input_size);
        }
        break;
      }
      case TraceLine::Op::kCalloc: {
        void* ptr = calloc(line.nmemb, line.input_size);
        size_t allocated_bytes = line.nmemb * line.input_size;
        if (ptr != nullptr) {
          id_to_ptrs[line.result] = { ptr, allocated_bytes };
          total_allocated_bytes += RoundUp(allocated_bytes);
        }
        break;
      }
      case TraceLine::Op::kRealloc: {
        void* new_ptr =
            realloc(id_to_ptrs[line.input_ptr].first, line.input_size);
        if (line.input_ptr != nullptr) {
          total_allocated_bytes -= RoundUp(id_to_ptrs[line.input_ptr].second);
          id_to_ptrs.erase(line.input_ptr);
        }
        total_allocated_bytes += RoundUp(line.input_size);
        id_to_ptrs[line.result] = { new_ptr, line.input_size };
        break;
      }
      case TraceLine::Op::kFree: {
        if (line.input_ptr == nullptr) {
          free(nullptr);
          break;
        }

        free(id_to_ptrs[line.input_ptr].first);
        total_allocated_bytes -= RoundUp(id_to_ptrs[line.input_ptr].second);
        id_to_ptrs.erase(line.input_ptr);
        break;
      }
    }

    max_allocated_bytes = std::max(total_allocated_bytes, max_allocated_bytes);
  }

  if (total_allocated_bytes != 0) {
    for (const auto& [id, ptr] : id_to_ptrs) {
      printf("%p: %p %zu\n", id, ptr.first, ptr.second);
    }
    return absl::InternalError(
        "Tracefile does not free all the memory it allocates.");
  }

  size_t total_size = 0;
  for (const auto& heap : heap_factory.Instances()) {
    total_size += heap->Size();
  }
  return static_cast<double>(max_allocated_bytes) / total_size;
}

}  // namespace bench
