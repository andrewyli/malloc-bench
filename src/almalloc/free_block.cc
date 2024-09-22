#include "free_block.h"

namespace almalloc {

size_t Block::GetSize() const {
  return this->size;
}

FreeBlock* FreeBlock::GetPrevFree() const {
  return this->prev_free;
}

FreeBlock* FreeBlock::GetNextFree() const {
  return this->next_free;
}

void FreeBlock::SetPrevFree(FreeBlock* fbp) {
  this->prev_free = fbp;
}

void FreeBlock::SetNextFree(FreeBlock* fbp) {
  this->next_free = fbp;
}

}
