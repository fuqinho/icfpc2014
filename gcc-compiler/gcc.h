#pragma once
#include "common.h"

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
	virtual bool assoc_left() const { return true; }
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
class OpZ : public Op {
protected:
	explicit OpZ(const std::string& name) : name(name) {}
	virtual std::ostream& to_stream(std::ostream& os) const {
		return os << name;
	}
	std::string name;
};

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
DefineOpZ(CAR);
DefineOpZ(CDR);
DefineOpZ(RTN);
DefineOpZ(JOIN);
DefineOpZ(DBUG);

class OpCONS : public OpZ {
public:
	OpCONS() : OpZ("CONS") {}
	virtual std::shared_ptr<Op> resolve(const std::vector<int>& codeid_offset) const {
		return std::make_shared<OpCONS>();
	}
	virtual bool assoc_left() const { return false; }
};

// Op with single int modifier.
class OpI : public Op {
protected:
	OpI(const std::string& name, int n) : name(name), n(n) {}
	virtual std::ostream& to_stream(std::ostream& os) const {
		return os << name << " " << n;
	}
	std::string name;
	int n;
};

#define DefineOpI(name)                           \
	class PP_CAT(Op,name) : public OpI {          \
	public:                                       \
		explicit PP_CAT(Op,name)(int n) : OpI(#name, n) {} \
		virtual std::shared_ptr<Op> resolve(const std::vector<int>& codeid_offset) const { \
			return std::make_shared<PP_CAT(Op,name)>(n); \
		} \
	}

DefineOpI(LDC);
DefineOpI(DUM);
DefineOpI(AP);
DefineOpI(RAP);
DefineOpI(TAP);
DefineOpI(TRAP);

class OpLDF : public OpI {
public:
	explicit OpLDF(int n) : OpI("LDF", n) {}
	virtual std::shared_ptr<Op> resolve(const std::vector<int>& codeid_offset) const {
		return std::make_shared<OpLDF>(codeid_offset[n]);
	}
};

// LD
class OpLD : public Op {
public:
	OpLD(int depth, int index) : depth(depth), index(index) {}
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

// ST
class OpST : public Op {
public:
	OpST(int depth, int index) : depth(depth), index(index) {}
	virtual std::ostream& to_stream(std::ostream& os) const {
		return os << "ST " << depth << " " << index;
	}
	virtual std::shared_ptr<Op> resolve(const std::vector<int>& codeid_offset) const {
		return std::make_shared<OpST>(depth, index);
	}

private:
	int depth;
	int index;
};

// SEL
class OpSEL : public Op {
public:
	OpSEL(int tid, int eid) : tid(tid), eid(eid) {}
	virtual std::ostream& to_stream(std::ostream& os) const {
		return os << "SEL " << tid << " " << eid;
	}
	virtual std::shared_ptr<Op> resolve(const std::vector<int>& codeid_offset) const {
		return std::make_shared<OpSEL>(codeid_offset[tid], codeid_offset[eid]);
	}

private:
	int tid, eid;
};


// TSEL
class OpTSEL : public Op {
public:
	OpTSEL(int tid, int eid) : tid(tid), eid(eid) {}
	virtual std::ostream& to_stream(std::ostream& os) const {
		return os << "TSEL " << tid << " " << eid;
	}
	virtual std::shared_ptr<Op> resolve(const std::vector<int>& codeid_offset) const {
		return std::make_shared<OpTSEL>(codeid_offset[tid], codeid_offset[eid]);
	}

private:
	int tid, eid;
};

}  // namespace gcc
