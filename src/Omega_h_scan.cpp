#include "Omega_h_scan.hpp"

#include "Omega_h_functors.hpp"
#include "Omega_h_loop.hpp"

namespace Omega_h {

template <typename T>
struct ExclScan : public SumFunctor<I64> {
  Read<T> in_;
  Write<LO> out_;
  ExclScan(Read<T> in, Write<LO> out) : in_(in), out_(out) {}
  OMEGA_H_DEVICE void operator()(
      Int i, value_type& update, bool final_pass) const {
    update += in_[i];
    if (final_pass) out_[i + 1] = static_cast<LO>(update);
  }
};

template <typename T>
LOs offset_scan(Read<T> a) {
  Write<LO> out(a.size() + 1);
  out.set(0, 0);
  parallel_scan(a.size(), ExclScan<T>(a, out), "offset_scan");
  return out;
}

template LOs offset_scan(Read<I8> a);
template LOs offset_scan(Read<I32> a);

struct FillRight : public MaxFunctor<I64> {
  using value_type = I64;
  Write<LO> a_;
  FillRight(Write<LO> a) : a_(a) {}
  OMEGA_H_DEVICE void operator()(
      LO i, value_type& update, bool final_pass) const {
    if (a_[i] > update) update = a_[i];
    if (final_pass && (a_[i] == -1)) a_[i] = static_cast<LO>(update);
  }
};

void fill_right(Write<LO> a) {
  parallel_scan(a.size(), FillRight(a), "fill_right");
}

}  // end namespace Omega_h
