#ifndef BLOCK_H_
#define BLOCK_H_

#include <cstddef>

namespace almalloc {

class Block {
 public:
  size_t GetSize() const;
  Block* GetPrev() const;
  Block* GetNext() const;
  void SetPrev(Block* fbp);
  void SetNext(Block* fbp);
 private:
  Block* prev;
  Block* next;
  size_t size;
  bool is_free;
};
}
#endif // BLOCK_H_
