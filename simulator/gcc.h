#ifndef SIMULATOR_GCC_H_
#define SIMULATOR_GCC_H_

#include <vector>

#include "common.h"

enum class GccMnemonic : uint32_t {
  LDC,
  LD,
  ADD,
  SUB,
  MUL,
  DIV,
  CEQ,
  CGT,
  CGTE,
  ATOM,
  CONS,
  CAR,
  CDR,
  SEL,
  JOIN,
  LDF,
  AP,
  RTN,
  DUM,
  RAP,
};

struct Code {
  GccMnemonic mnemonic;
  int32_t arg1;
  int32_t arg2;
};

enum class ValueTag : uint32_t {
  INT,
  CONS,
  CLOSURE,
};

struct Value {
  ValueTag tag;
  int32_t value;
};

struct ConsValue {
  Value car;
  Value cdr;
};

struct FrameValue {
  int32_t parent;
  bool is_dum;
  std::vector<Value> values;
};

struct ClosureValue {
  int32_t f;
  int32_t env;
};

struct HeapValue {
  bool visited;
  union {
    ConsValue cons;
    ClosureValue closure;
  };
  FrameValue frame;
};

enum class ControlTag : uint32_t {
  STOP,
  ENV,
  RET,
  JOIN,
};

struct ControlValue {
  ControlTag tag;
  int32_t value;
};

enum class ErrorType {
  FRAME_MISMATCH,
  TAG_MISMATCH,
  CONTROL_MISMATCH,
};

class GCC {
 public:
  GCC() {
  }

  bool Run() {
    int max_step = 3072 * 1000;
    for (int step = 0; step < max_step; ++step) {
      if (RunStep())
        break;
    }
  }

  bool RunStep() {
    switch(code_[reg_c_].mnemonic) {
      case GccMnemonic::LDC:
        return Ldc(code_[reg_c_].arg1);
      case GccMnemonic::LD:
        return Ld(code_[reg_c_].arg1, code_[reg_c_].arg2);
      case GccMnemonic::ADD:
        return Add();
      case GccMnemonic::SUB:
        return Sub();
      case GccMnemonic::MUL:
        return Mul();
      case GccMnemonic::DIV:
        return Div();
      case GccMnemonic::CEQ:
        return Ceq();
      case GccMnemonic::CGT:
        return Cgt();
      case GccMnemonic::CGTE:
        return Cgte();
      case GccMnemonic::ATOM:
        return Atom();
      case GccMnemonic::CONS:
        return Cons();
      case GccMnemonic::CAR:
        return Car();
      case GccMnemonic::CDR:
        return Cdr();
      case GccMnemonic::SEL:
        return Sel(code_[reg_c_].arg1, code_[reg_c_].arg2);
      case GccMnemonic::LDF:
        return Ldf(code_[reg_c_].arg1);
      case GccMnemonic::AP:
        return Ap(code_[reg_c_].arg1);
      case GccMnemonic::RTN:
        return Rtn();
      case GccMnemonic::DUM:
        return Dum(code_[reg_c_].arg1);
      case GccMnemonic::RAP:
        return Rap(code_[reg_c_].arg1);
    }
    assert(false);
    return true;
  }

 private:
  bool Ldc(int32_t n) {
    data_stack_.push_back(Value { ValueTag::INT, n });
    ++reg_c_;
    return false;
  }

  bool Ld(int32_t n, int32_t i) {
    int32_t fp = reg_e_;
    while (n > 0) {
      fp = heap_[fp].frame.parent;
      --n;
    }
    if (heap_[fp].frame.is_dum)
      return OnError(ErrorType::FRAME_MISMATCH);
    data_stack_.push_back(heap_[fp].frame.values[i]);
    ++reg_c_;
    return false;
  }

  bool Add() {
    Value y = data_stack_.back();
    data_stack_.pop_back();
    Value x = data_stack_.back();
    data_stack_.pop_back();
    if (x.tag != ValueTag::INT || y.tag != ValueTag::INT)
      return OnError(ErrorType::TAG_MISMATCH);
    data_stack_.push_back(Value { ValueTag::INT, x.value + y.value});
    ++reg_c_;
    return false;
  }

  bool Sub() {
    Value y = data_stack_.back();
    data_stack_.pop_back();
    Value x = data_stack_.back();
    data_stack_.pop_back();
    if (x.tag != ValueTag::INT || y.tag != ValueTag::INT)
      return OnError(ErrorType::TAG_MISMATCH);
    data_stack_.push_back(Value { ValueTag::INT, x.value - y.value });
    ++reg_c_;
    return false;
  }

  bool Mul() {
    Value y = data_stack_.back();
    data_stack_.pop_back();
    Value x = data_stack_.back();
    data_stack_.pop_back();
    if (x.tag != ValueTag::INT || y.tag != ValueTag::INT)
      return OnError(ErrorType::TAG_MISMATCH);
    data_stack_.push_back(Value { ValueTag::INT, x.value * y.value });
    ++reg_c_;
    return false;
  }

  bool Div() {
    Value y = data_stack_.back();
    data_stack_.pop_back();
    Value x = data_stack_.back();
    data_stack_.pop_back();
    if (x.tag != ValueTag::INT || y.tag != ValueTag::INT)
      return OnError(ErrorType::TAG_MISMATCH);
    data_stack_.push_back(Value { ValueTag::INT, x.value / y.value });
    ++reg_c_;
    return false;
  }

  bool Ceq() {
    Value y = data_stack_.back();
    data_stack_.pop_back();
    Value x = data_stack_.back();
    data_stack_.pop_back();
    if (x.tag != ValueTag::INT || y.tag != ValueTag::INT)
      return OnError(ErrorType::TAG_MISMATCH);
    data_stack_.push_back(
        Value {ValueTag::INT, static_cast<int32_t>(x.value == y.value)});
    ++reg_c_;
    return false;
  }

  bool Cgt() {
    Value y = data_stack_.back();
    data_stack_.pop_back();
    Value x = data_stack_.back();
    data_stack_.pop_back();
    if (x.tag != ValueTag::INT || y.tag != ValueTag::INT)
      return OnError(ErrorType::TAG_MISMATCH);
    data_stack_.push_back(
        Value { ValueTag::INT, static_cast<int32_t>(x.value > y.value) });
    ++reg_c_;
    return false;
  }

  bool Cgte() {
    Value y = data_stack_.back();
    data_stack_.pop_back();
    Value x = data_stack_.back();
    data_stack_.pop_back();
    if (x.tag != ValueTag::INT || y.tag != ValueTag::INT)
      return OnError(ErrorType::TAG_MISMATCH);
    data_stack_.push_back(
        Value {ValueTag::INT, static_cast<int32_t>(x.value >= y.value)});
    ++reg_c_;
    return false;
  }

  bool Atom() {
    Value x = data_stack_.back();
    data_stack_.pop_back();
    data_stack_.push_back(
        Value { ValueTag::INT, static_cast<int32_t>(x.tag == ValueTag::INT) });
    ++reg_c_;
    return false;
  }

  bool Cons() {
    Value y = data_stack_.back();
    data_stack_.pop_back();
    Value x = data_stack_.back();
    data_stack_.pop_back();

    int32_t z = AllocCons(x, y);
    data_stack_.push_back(Value {ValueTag::CONS, z});
    ++reg_c_;
    return false;
  }

  bool Car() {
    Value x = data_stack_.back();
    data_stack_.pop_back();
    if (x.tag != ValueTag::CONS)
      return OnError(ErrorType::TAG_MISMATCH);
    data_stack_.push_back(heap_[x.value].cons.car);
    ++reg_c_;
    return false;
  }

  bool Cdr() {
    Value x = data_stack_.back();
    data_stack_.pop_back();
    if (x.tag !=ValueTag::CONS)
      return OnError(ErrorType::TAG_MISMATCH);
    data_stack_.push_back(heap_[x.value].cons.cdr);
    ++reg_c_;
    return false;
  }

  bool Sel(int32_t t, int32_t f) {
    Value x = data_stack_.back();
    data_stack_.pop_back();
    if (x.tag != ValueTag::INT)
      return OnError(ErrorType::TAG_MISMATCH);
    control_stack_.push_back(ControlValue {ControlTag::JOIN, reg_c_ + 1});
    reg_c_ = (x.value == 0) ? f : t;
    return false;
  }

  bool Join() {
    ControlValue x = control_stack_.back();
    control_stack_.pop_back();
    if (x.tag != ControlTag::JOIN)
      return OnError(ErrorType::CONTROL_MISMATCH);
    reg_c_ = x.value;
    return false;
  }

  bool Ldf(int32_t f) {
    int32_t x = AllocClosure(f, reg_e_);
    data_stack_.push_back(Value {ValueTag::CLOSURE, x});
    ++reg_c_;
    return false;
  }

  bool Ap(int32_t n) {
    Value x = data_stack_.back();
    data_stack_.pop_back();
    if (x.tag != ValueTag::CLOSURE)
      return OnError(ErrorType::TAG_MISMATCH);
    int32_t f = heap_[x.value].closure.f;
    int32_t e = heap_[x.value].closure.env;
    int32_t frame = AllocFrame(n, e);
    for (int i = n - 1; i >= 0; --i) {
      heap_[frame].frame.values[i] = data_stack_.back();
      data_stack_.pop_back();
    }

    control_stack_.push_back(ControlValue {ControlTag::ENV, reg_e_});
    control_stack_.push_back(ControlValue {ControlTag::RET, reg_c_ + 1});
    reg_e_ = frame;
    reg_c_ = f;
    return false;
  }

  bool Rtn() {
    ControlValue x = control_stack_.back();
    control_stack_.pop_back();
    if (x.tag == ControlTag::STOP)
      return true;
    if (x.tag != ControlTag::RET)
      return OnError(ErrorType::CONTROL_MISMATCH);
    ControlValue y = control_stack_.back();
    control_stack_.pop_back();
    assert(y.tag == ControlTag::ENV);
    reg_e_ = y.value;
    reg_c_ = x.value;
    return false;
  }

  bool Dum(int32_t n) {
    int32_t fp = AllocFrame(n, -1);
    for (size_t i = 0; i < n; ++i) {
      heap_[fp].frame.values[i] = Value { ValueTag::INT, 0 };
    }
    heap_[fp].frame.parent = reg_e_;
    heap_[fp].frame.is_dum = true;
    reg_e_ = fp;
    ++reg_c_;
    return false;
  }

  bool Rap(int32_t n) {
    Value x = data_stack_.back();
    data_stack_.pop_back();
    if (x.tag != ValueTag::CLOSURE)
      return OnError(ErrorType::TAG_MISMATCH);
    int32_t f = heap_[x.value].closure.f;
    int32_t fp = heap_[x.value].closure.env;
    if (!heap_[reg_e_].frame.is_dum)
      return OnError(ErrorType::FRAME_MISMATCH);
    if (!heap_[reg_e_].frame.values.size() != n)
      return OnError(ErrorType::FRAME_MISMATCH);
    if (reg_e_ != fp)
      return OnError(ErrorType::FRAME_MISMATCH);
    for (int i = n - 1; i >= 0; --i) {
      heap_[fp].frame.values[i] = data_stack_.back();
      data_stack_.pop_back();
    }
    int32_t ep = heap_[reg_e_].frame.parent;
    control_stack_.push_back(ControlValue { ControlTag::ENV, ep });
    control_stack_.push_back(ControlValue {ControlTag::RET, reg_c_ + 1});
    heap_[fp].frame.is_dum = false;
    reg_e_ = fp;
    reg_c_ = f;
    return false;
  }

  int32_t AllocCons(const Value& x, const Value& y) {
    if (current_heap_pos_ >= heap_.size()) {
      RunGC();
      Visit(x);
      Visit(y);
      CleanUp();
    }

    while (current_heap_pos_ < heap_.size()) {
      if (heap_[current_heap_pos_].visited == false)
        break;
      ++current_heap_pos_;
    }

    if (current_heap_pos_ >= heap_.size())
      // TODO
      exit(1);

    heap_[current_heap_pos_].cons.car = x;
    heap_[current_heap_pos_].cons.cdr = y;
    return static_cast<int32_t>(current_heap_pos_++);
  }

  int32_t AllocClosure(int32_t f, int32_t env) {
    if (current_heap_pos_ >= heap_.size()) {
      RunGC();
      CleanUp();
    }

    while (current_heap_pos_ < heap_.size()) {
      if (heap_[current_heap_pos_].visited == false)
        break;
      ++current_heap_pos_;
    }

    if (current_heap_pos_ >= heap_.size())
      // TODO
      exit(1);

    heap_[current_heap_pos_].closure.f = f;
    heap_[current_heap_pos_].closure.env = env;
    return static_cast<int32_t>(current_heap_pos_++);
  }

  int32_t AllocFrame(int32_t n, int32_t env) {
    if (current_heap_pos_ >= heap_.size()) {
      RunGC();
      Visit(env);
      CleanUp();
    }

    while (current_heap_pos_ < heap_.size()) {
      if (heap_[current_heap_pos_].visited == false)
        break;
      ++current_heap_pos_;
    }

    if (current_heap_pos_ >= heap_.size())
      // TODO
      exit(1);

    heap_[current_heap_pos_].frame.parent = -1;
    heap_[current_heap_pos_].frame.is_dum = false;
    heap_[current_heap_pos_].frame.values.resize(n);
    return current_heap_pos_++;
  }

  void RunGC() {
    for (size_t i = 0; i < heap_.size(); ++i) {
      heap_[i].visited = false;
    }

    for (size_t i = 0; i < data_stack_.size(); ++i) {
      Visit(data_stack_[i]);
    }

    for (size_t i = 0; i < control_stack_.size(); ++i) {
      Visit(control_stack_[i]);
    }

    Visit(reg_e_);
  }

  void CleanUp() {
    for (size_t i = 0; i < heap_.size(); ++i) {
      if (!heap_[i].visited) {
        heap_[i].frame.values.clear();
      }
    }
    current_heap_pos_ = 0;
  }

  void Visit(const Value& v) {
    if (v.tag == ValueTag::INT)
      return;

    if (v.tag == ValueTag::CONS) {
      if (heap_[v.value].visited) {
        // Do nothing.
        return;
      }

      heap_[v.value].visited = true;
      Visit(heap_[v.value].cons.car);
      Visit(heap_[v.value].cons.cdr);
      return;
    }

    if (v.tag == ValueTag::CLOSURE) {
      if (heap_[v.value].visited) {
        // Do nothing.
        return;
      }

      heap_[v.value].visited = true;
      Visit(heap_[v.value].closure.env);
      return;
    }
  }

  void Visit(const ControlValue& v) {
    if (v.tag == ControlTag::ENV) {
      Visit(v.value);
      return;
    }
  }

  void Visit(int32_t fp) {
    if (fp == -1)
      return;
    if (heap_[fp].visited)
      return;
    heap_[fp].visited = true;

    Visit(heap_[fp].frame.parent);
    for (const auto& v : heap_[fp].frame.values) {
      Visit(v);
    }
  }

  bool OnError(ErrorType e) {
    // TODO error debug log?
    return true;
  }

  int32_t reg_c_;
  int32_t reg_e_;

  std::vector<Code> code_;
  std::vector<Value> data_stack_;
  std::vector<ControlValue> control_stack_;

  std::vector<HeapValue> heap_;
  size_t current_heap_pos_;

  DISALLOW_COPY_AND_ASSIGN(GCC);
};

#endif
