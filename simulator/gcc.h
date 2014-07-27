#ifndef SIMULATOR_GCC_H_
#define SIMULATOR_GCC_H_

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

#include <glog/logging.h>

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
  TSEL,
  TAP,
  TRAP,
  ST,
  DBUG,
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
    heap_.resize(10 * 1024 * 1024);
    for (size_t i = 0; i < heap_.size(); ++i) {
      heap_[i].visited = false;
    }
    current_heap_pos_ = 0;
    max_heap_pos_ = 0;
  }

  Value Run(int32_t ip, int32_t env, Value arg1, Value arg2, Value arg3) {
    // Prologue.
    reg_c_ = ip;
    data_stack_.clear();
    // Keep ref to avoid GC.
    data_stack_.push_back(arg1);
    data_stack_.push_back(arg2);
    data_stack_.push_back(arg3);

    control_stack_.clear();
    control_stack_.push_back(ControlValue {ControlTag::STOP, 0});

    int32_t frame = AllocFrame(3, -1);
    heap_[frame].frame.parent = env;
    heap_[frame].frame.values[0] = arg1;
    heap_[frame].frame.values[1] = arg2;
    heap_[frame].frame.values[2] = arg3;
    reg_e_ = frame;

    int max_step = 3072 * 1000;
    for (int step = 0; step < max_step; ++step) {
      if (RunStep())
        break;
    }

    return data_stack_.back();
  }

  bool RunStep() {
    // std::cerr << "reg_c: " << reg_c_ << std::endl;

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
      case GccMnemonic::JOIN:
        return Join();
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
      case GccMnemonic::TSEL:
        return Tsel(code_[reg_c_].arg1, code_[reg_c_].arg2);
      case GccMnemonic::TAP:
        return Tap(code_[reg_c_].arg1);
      case GccMnemonic::TRAP:
        return Trap(code_[reg_c_].arg1);
      case GccMnemonic::ST:
        return St(code_[reg_c_].arg1, code_[reg_c_].arg2);
      case GccMnemonic::DBUG:
        return Dbug();
    }
    assert(false);
    return true;
  }

  Value ConsCar(const Value& v) {
    return heap_[v.value].cons.car;
  }

  Value ConsCdr(const Value& v) {
    return heap_[v.value].cons.cdr;
  }

  int32_t GetIp(const Value& v) {
    return heap_[v.value].closure.f;
  }

  int32_t GetEnv(const Value& v) {
    return heap_[v.value].closure.env;
  }

  void RunFullGC(const Value& ai_state, const Value& step) {
    data_stack_.clear();
    control_stack_.clear();
    RunGC();
    Visit(ai_state);
    Visit(step);
    CleanUp();
  }

  Value Create2Tuple(const Value& v1, const Value& v2) {
    return { ValueTag::CONS, AllocCons(v1, v2) };
  }

  Value Create3Tuple(const Value& v1, const Value& v2, const Value& v3) {
    return Create2Tuple(v1, Create2Tuple(v2, v3));
  }

  Value Create4Tuple(const Value& v1, const Value& v2,
                     const Value& v3, const Value& v4) {
    return Create2Tuple(v1, Create3Tuple(v2, v3, v4));
  }

  Value Create5Tuple(const Value& v1, const Value& v2, const Value& v3,
                     const Value& v4, const Value& v5) {
    return Create2Tuple(v1, Create4Tuple(v2, v3, v4, v5));
  }

  void LoadProgram(const std::string& program) {
    code_.clear();
    std::stringstream ss(program);
    std::string line;
    while (getline(ss, line)) {
      std::stringstream stream(line);
      std::string mnemonic;
      stream >> mnemonic;
      if (mnemonic == "LDC") {
        int32_t arg;
        stream >> arg;
        code_.push_back(Code { GccMnemonic::LDC, arg });
        continue;
      }
      if (mnemonic == "LD") {
        int32_t arg1, arg2;
        stream >> arg1 >> arg2;
        code_.push_back(Code { GccMnemonic::LD, arg1, arg2 });
        continue;
      }
      if (mnemonic == "ADD") {
        code_.push_back(Code { GccMnemonic::ADD });
        continue;
      }
      if (mnemonic == "SUB") {
        code_.push_back(Code { GccMnemonic::SUB });
        continue;
      }
      if (mnemonic == "MUL") {
        code_.push_back(Code { GccMnemonic::MUL });
        continue;
      }
      if (mnemonic == "DIV") {
        code_.push_back(Code { GccMnemonic::DIV });
        continue;
      }
      if (mnemonic == "CEQ") {
        code_.push_back(Code { GccMnemonic::CEQ });
        continue;
      }
      if (mnemonic == "CGT") {
        code_.push_back(Code { GccMnemonic::CGT });
        continue;
      }
      if (mnemonic == "CGTE") {
        code_.push_back(Code { GccMnemonic::CGTE });
        continue;
      }
      if (mnemonic == "ATOM") {
        code_.push_back(Code { GccMnemonic::ATOM });
        continue;
      }
      if (mnemonic == "CONS") {
        code_.push_back(Code { GccMnemonic::CONS });
        continue;
      }
      if (mnemonic == "CAR") {
        code_.push_back(Code { GccMnemonic::CAR });
        continue;
      }
      if (mnemonic == "CDR") {
        code_.push_back(Code { GccMnemonic::CDR });
        continue;
      }
      if (mnemonic == "SEL") {
        int32_t arg1, arg2;
        stream >> arg1 >> arg2;
        code_.push_back(Code { GccMnemonic::SEL, arg1, arg2 });
        continue;
      }
      if (mnemonic == "JOIN") {
        code_.push_back(Code { GccMnemonic::JOIN });
        continue;
      }
      if (mnemonic == "LDF") {
        int32_t arg;
        stream >> arg;
        code_.push_back(Code { GccMnemonic::LDF, arg });
        continue;
      }
      if (mnemonic == "AP") {
        int32_t arg;
        stream >> arg;
        code_.push_back(Code { GccMnemonic::AP, arg });
        continue;
      }
      if (mnemonic == "RTN") {
        code_.push_back(Code { GccMnemonic::RTN });
        continue;
      }
      if (mnemonic == "DUM") {
        int32_t arg;
        stream >> arg;
        code_.push_back(Code { GccMnemonic::DUM, arg });
        continue;
      }
      if (mnemonic == "RAP") {
        int32_t arg;
        stream >> arg;
        code_.push_back(Code { GccMnemonic::RAP, arg });
        continue;
      }
      if (mnemonic == "TSEL") {
        int32_t arg1, arg2;
        stream >> arg1 >> arg2;
        code_.push_back(Code { GccMnemonic::TSEL, arg1, arg2 });
        continue;
      }
      if (mnemonic == "TAP") {
        int32_t arg;
        stream >> arg;
        code_.push_back(Code { GccMnemonic::TAP, arg });
        continue;
      }
      if (mnemonic == "TRAP") {
        int32_t arg;
        stream >> arg;
        code_.push_back(Code { GccMnemonic::TRAP, arg });
        continue;
      }
      if (mnemonic == "ST" ) {
        int32_t arg1, arg2;
        stream >> arg1 >> arg2;
        code_.push_back(Code { GccMnemonic::ST, arg1, arg2 });
        continue;
      }
      if (mnemonic == "DBUG") {
        code_.push_back(Code { GccMnemonic::DBUG });
        continue;
      }

      std::cerr << line << std::endl;
      abort();
      // Skip otherwise.
    }
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
    heap_[frame].frame.parent = e;
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
    if (heap_[reg_e_].frame.values.size() != n)
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

  bool Tsel(int32_t t, int32_t f) {
    Value x = data_stack_.back();
    data_stack_.pop_back();
    if (x.tag != ValueTag::INT)
      return OnError(ErrorType::TAG_MISMATCH);
    if (x.value == 0) {
      reg_c_ = f;
    } else {
      reg_c_ = t;
    }
    return false;
  }

  bool Tap(int32_t n) {
    Value x = data_stack_.back();
    data_stack_.pop_back();
    if (x.tag != ValueTag::CLOSURE)
      return OnError(ErrorType::TAG_MISMATCH);
    int32_t f = heap_[x.value].closure.f;
    int32_t e = heap_[x.value].closure.env;
    int32_t frame = AllocFrame(n, e);
    heap_[frame].frame.parent = e;
    for (int i = n - 1; i >= 0; --i) {
      heap_[frame].frame.values[i] = data_stack_.back();
      data_stack_.pop_back();
    }

    reg_e_ = frame;
    reg_c_ = f;
    return false;
  }

  bool Trap(int32_t n) {
    Value x = data_stack_.back();
    data_stack_.pop_back();
    if (x.tag != ValueTag::CLOSURE)
      return OnError(ErrorType::TAG_MISMATCH);
    int32_t f = heap_[x.value].closure.f;
    int32_t fp = heap_[x.value].closure.env;
    if (!heap_[reg_e_].frame.is_dum)
      return OnError(ErrorType::FRAME_MISMATCH);
    if (heap_[reg_e_].frame.values.size() != n)
      return OnError(ErrorType::FRAME_MISMATCH);
    if (reg_e_ != fp)
      return OnError(ErrorType::FRAME_MISMATCH);
    for (int i = n - 1; i >= 0; --i) {
      heap_[fp].frame.values[i] = data_stack_.back();
      data_stack_.pop_back();
    }
    heap_[fp].frame.is_dum = false;
    reg_e_ = fp;
    reg_c_ = f;
    return false;
  }

  bool St(int32_t n, int32_t i) {
    int32_t fp = reg_e_;
    while (n > 0) {
      fp = heap_[fp].frame.parent;
      --n;
    }
    if (heap_[fp].frame.is_dum)
      return OnError(ErrorType::FRAME_MISMATCH);
    Value v = data_stack_.back();
    data_stack_.pop_back();
    heap_[fp].frame.values[i] = v;
    ++reg_c_;
    return false;
  }

  bool Dbug() {
    Value x = data_stack_.back();
    data_stack_.pop_back();
    ++reg_c_;
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
    if (max_heap_pos_ < current_heap_pos_) {
      max_heap_pos_ = current_heap_pos_;
    }
    for (size_t i = 0; i < max_heap_pos_; ++i) {
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

  void PrintValue(const Value& x) {
    std::stringstream s;
    PrintValueInternal(x, &s);
    LOG(ERROR) << s.str();
  }

  void PrintValueInternal(const Value& x, std::ostream* os) {
    switch (x.tag) {
      case ValueTag::INT:
        *os << x.value;
        break;
      case ValueTag::CONS:
        *os << "(";
        PrintValueInternal(heap_[x.value].cons.car, os);
        *os << " ";
        PrintValueInternal(heap_[x.value].cons.cdr, os);
        *os << ")";
        break;
      case ValueTag::CLOSURE:
        *os << "[" << heap_[x.value].closure.f << "]";
        break;
    }
  }

  bool OnError(ErrorType e) {
    LOG(ERROR) << "Error: " << static_cast<int>(e) << ", " << reg_c_;
    // TODO error debug logi?
    return true;
  }

  int32_t reg_c_;
  int32_t reg_e_;

  std::vector<Code> code_;
  std::vector<Value> data_stack_;
  std::vector<ControlValue> control_stack_;

  std::vector<HeapValue> heap_;
  size_t current_heap_pos_;
  size_t max_heap_pos_;

  DISALLOW_COPY_AND_ASSIGN(GCC);
};

#endif
