#pragma once
#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace gcc {

// Base class for instructions.
class Op
{
public:
	virtual std::shared_ptr<Op> resolve(const std::vector<int>& codeid_offset) const = 0;
	virtual std::ostream& to_stream(std::ostream& os) const = 0;
	friend std::ostream& operator<<(std::ostream& os, const Op& me) {
		return me.to_stream(os);
	}
};

// Operation sequence
typedef std::vector<std::shared_ptr<Op>> OperationSequence;

inline void Append(OperationSequence* ops, const OperationSequence& op)
{
	ops->insert(ops->end(), op.begin(), op.end());
}

inline void Append(OperationSequence* ops, std::shared_ptr<Op> op)
{
	ops->push_back(op);
}

// Simple, zero parameter instruction.
class OpZ : public Op
{
public:
	explicit OpZ(const std::string& name) : name(name) {}
	virtual std::ostream& to_stream(std::ostream& os) const {
		return os << name;
	}

private:
	std::string name;
};

#define PP_CAT(a,b) a##b
#define DefineOpZ(name)                   \
	class PP_CAT(Op,name) : public OpZ {  \
	public:                               \
		PP_CAT(Op,name)() : OpZ(#name) {} \
		virtual std::shared_ptr<Op> resolve(const std::vector<int>& codeid_offset) const { \
			return std::make_shared<PP_CAT(Op,name)>(); \
		} \
	}

DefineOpZ(ADD);
DefineOpZ(SUB);
DefineOpZ(MUL);
DefineOpZ(DIV);
DefineOpZ(CEQ);
DefineOpZ(CGT);
DefineOpZ(CGTE);
DefineOpZ(ATOM);
DefineOpZ(CONS);
DefineOpZ(CAR);
DefineOpZ(CDR);

DefineOpZ(RTN);
DefineOpZ(JOIN);

// LDC
class OpLDC : public Op {
public:
	explicit OpLDC(int value) : value(value) {}
	virtual std::ostream& to_stream(std::ostream& os) const {
		return os << "LDC " << value;
	}
	virtual std::shared_ptr<Op> resolve(const std::vector<int>& codeid_offset) const {
		return std::make_shared<OpLDC>(value);
	}

private:
	int value;
};

// LD
class OpLD : public Op {
public:
	explicit OpLD(int depth, int index) : depth(depth), index(index) {}
	virtual std::ostream& to_stream(std::ostream& os) const {
		return os << "LD " << depth << " " << index;
	}
	virtual std::shared_ptr<Op> resolve(const std::vector<int>& codeid_offset) const {
		return std::make_shared<OpLD>(depth, index);
	}

private:
	int depth;
	int index;
};

// LDF
class OpLDF : public Op {
public:
	explicit OpLDF(int id) : id(id) {}
	virtual std::ostream& to_stream(std::ostream& os) const {
		return os << "LDF " << id;
	}
	virtual std::shared_ptr<Op> resolve(const std::vector<int>& codeid_offset) const {
		return std::make_shared<OpLDF>(codeid_offset[id]);
	}

private:
	int id;
};

// AP
class OpAP : public Op {
public:
	explicit OpAP(int n) : n(n) {}
	virtual std::ostream& to_stream(std::ostream& os) const {
		return os << "AP " << n;
	}
	virtual std::shared_ptr<Op> resolve(const std::vector<int>& codeid_offset) const {
		return std::make_shared<OpAP>(n);
	}

private:
	int n;
};

// RAP
class OpRAP : public Op {
public:
	explicit OpRAP(int n) : n(n) {}
	virtual std::ostream& to_stream(std::ostream& os) const {
		return os << "RAP " << n;
	}
	virtual std::shared_ptr<Op> resolve(const std::vector<int>& codeid_offset) const {
		return std::make_shared<OpRAP>(n);
	}

private:
	int n;
};

// DUM
class OpDUM : public Op {
public:
	explicit OpDUM(int n) : n(n) {}
	virtual std::ostream& to_stream(std::ostream& os) const {
		return os << "DUM " << n;
	}
	virtual std::shared_ptr<Op> resolve(const std::vector<int>& codeid_offset) const {
		return std::make_shared<OpDUM>(n);
	}

private:
	int n;
};
// SEL
class OpSEL : public Op {
public:
	explicit OpSEL(int tid, int eid) : tid(tid), eid(eid) {}
	virtual std::ostream& to_stream(std::ostream& os) const {
		return os << "SEL " << tid << " " << eid;
	}
	virtual std::shared_ptr<Op> resolve(const std::vector<int>& codeid_offset) const {
		return std::make_shared<OpSEL>(codeid_offset[tid], codeid_offset[eid]);
	}

private:
	int tid, eid;
};

}  // namespace gcc
