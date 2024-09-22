#ifndef BLOCK_H_
#define BLOCK_H_

#include <cstddef>

namespace almalloc {

class Block {
 public:
  size_t GetSize() const;
  Block* GetPrev() const;
  Block* GetNext() const;
  bool IsFree() const;
  void SetPrev(Block* fbp);
  void SetNext(Block* fbp);
  void Resize(size_t s);
  void SetFree(bool is_free);
 private:
  Block* prev;
  Block* next;
  size_t size;
  bool is_free;
};
}
#endif // BLOCK_H_
