#include "block.h"

namespace almalloc {


size_t Block::GetSize() const {
  return this->size;
}
Block* Block::GetPrev() const {
  return this->prev;
}
Block* Block::GetNext() const {
  return this->next;
}
bool Block::IsFree() const {
  return this->is_free;
}
void Block::SetPrev(Block* bp) {
  this->prev = bp;
}
void Block::SetNext(Block* bp) {
  this->next = bp;
}
void Block::Resize(size_t s) {
  this->size = s;
}
void Block::SetFree(bool b) {
  this->is_free = b;
}


} // namespace almalloc
