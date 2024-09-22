#ifndef FREE_LIST_H_
#define FREE_LIST_H_

#include <cstddef>
#include "block.h"

namespace almalloc {

class FreeBlock : public Block {
 public:
  size_t GetSize() const;
  FreeBlock* GetPrevFree() const;
  FreeBlock* GetNextFree() const;
  void SetPrevFree(FreeBlock* fbp);
  void SetNextFree(FreeBlock* fbp);
 private:
  FreeBlock* prev_free;
  FreeBlock* next_free;
};

}

#endif // FREE_LIST_H_
