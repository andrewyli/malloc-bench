#ifndef BLOCK_H_
#define BLOCK_H_

#include <cstddef>

namespace almalloc {

class Block {
 public:
  size_t GetSize() const;
  Block* GetPrev();
  Block* GetNext();
  void SetPrev(Block* fbp);
  void SetNext(Block* fbp);
 private:
  Block* prev;
  Block* next;
  size_t size;
  bool is_free;
};

#endif // FREE_LIST_H_
