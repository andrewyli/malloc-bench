#include "block.h"

namespace almalloc {


size_t Block::GetSize() const {
  return 0;
}
Block* Block::GetPrev() const {
  return 0;
}
Block* Block::GetNext() const {
  return 0;
}
void Block::SetPrev(Block* bp) {
  this->prev = bp;
}
void Block::SetNext(Block* bp) {
  this->next = bp;
}

} // namespace almalloc
