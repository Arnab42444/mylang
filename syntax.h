/* SPDX-License-Identifier: BSD-2-Clause */

#pragma once
#include "lexer.h"
#include <variant>
#include <unordered_map>

class Literal;
class LiteralInt;
class ExprList;
class CallExpr;
class Expr01;
class Expr03;
class Expr04;
class Expr06;
class Stmt;
class Block;
class LValue;
class Identifier;

class EvalValue {

public:
    variant<nullptr_t, long, LValue *> value;

    EvalValue() : value(nullptr) { }
    EvalValue(long val) : value(val) { }
    explicit EvalValue(LValue *val) : value(val) { }

    virtual ~EvalValue() = default;

    template <class T>
    T get() const {
        return std::get<T>(value);
    }

    template <class T>
    bool is() const {
        return holds_alternative<T>(value);
    }

    bool isLValueP() const {
        return holds_alternative<LValue *>(value);
    }
};

class LValue {

public:
    variant<long> value;

    template <class T>
    LValue(T &&arg) : value(forward<T>(arg)) { }

    template <class T>
    T get() const {
        return std::get<T>(value);
    }

    void put(EvalValue v) {

        if (v.is<long>())
            value = v.get<long>();
        else
            throw TypeErrorEx();
    }

    EvalValue eval() const {
        return EvalValue(std::get<long>(value));
    }
};

ostream &operator<<(ostream &s, const EvalValue &c);

class EvalContext {

public:

    unordered_map<string, LValue> vars;
};

class Construct {

public:
    const char *const name;

    Construct(const char *name) : name(name) { }
    virtual ~Construct() = default;
    virtual EvalValue eval(EvalContext *) const = 0;
    virtual void serialize(ostream &s, int level = 0) const = 0;
};

class SingleChildConstruct : public Construct {

public:
    unique_ptr<Construct> elem;

    SingleChildConstruct(const char *name) : Construct(name) { }

    virtual EvalValue eval(EvalContext *ctx) const {
        return elem->eval(ctx);
    }

    virtual void serialize(ostream &s, int level = 0) const;
};


class MultiOpConstruct : public Construct {

public:
    vector<pair<Op, unique_ptr<Construct>>> elems;

    MultiOpConstruct(const char *name) : Construct(name) { }
    virtual void serialize(ostream &s, int level = 0) const;
};

class MultiElemConstruct : public Construct {

public:
    vector<unique_ptr<Construct>> elems;

    MultiElemConstruct(const char *name) : Construct(name) { }
    virtual void serialize(ostream &s, int level = 0) const;
};

inline ostream &operator<<(ostream &s, const Construct &c)
{
    c.serialize(s);
    return s;
}

class Literal : public Construct {

public:
    long value;

    Literal(long v) : Construct("Literal"), value(v) { }
};

class LiteralInt : public Literal {

public:

    LiteralInt(long v) : Literal(v) { }

    virtual EvalValue eval(EvalContext *ctx) const {
        return value;
    }

    virtual void serialize(ostream &s, int level = 0) const;
};

class Identifier : public Construct {

public:
    string value;

    template <class T>
    Identifier(T &&arg) : Construct("Id"), value(forward<T>(arg)) { }

    virtual void serialize(ostream &s, int level = 0) const;
    virtual EvalValue eval(EvalContext *ctx) const;
};

class ExprList : public MultiElemConstruct {

public:

    ExprList() : MultiElemConstruct("ExprList") { }

    virtual EvalValue eval(EvalContext *ctx) const {
        return EvalValue();
    }
};

class CallExpr : public Construct {

public:
    unique_ptr<Identifier> id;
    unique_ptr<ExprList> args;

    CallExpr() : Construct("CallExpr") { }
    virtual void serialize(ostream &s, int level = 0) const;
    virtual EvalValue eval(EvalContext *ctx) const;
};

class Expr01 : public SingleChildConstruct {

public:
    Expr01() : SingleChildConstruct("Expr01") { }
};

class Expr02 : public MultiOpConstruct {

public:

    Expr02() : MultiOpConstruct("Expr02") { }
    virtual EvalValue eval(EvalContext *ctx) const;
};


class Expr03 : public MultiOpConstruct {

public:

    Expr03() : MultiOpConstruct("Expr03") { }
    virtual EvalValue eval(EvalContext *ctx) const;
};

class Expr04 : public MultiOpConstruct {

public:

    Expr04() : MultiOpConstruct("Expr04") { }
    virtual EvalValue eval(EvalContext *ctx) const;
};

class Expr06 : public MultiOpConstruct {

public:

    Expr06() : MultiOpConstruct("Expr06") { }
    virtual EvalValue eval(EvalContext *ctx) const;
};

class Expr14 : public Construct {

public:
    unique_ptr<Construct> lvalue;
    unique_ptr<Construct> rvalue;
    Op op;

    Expr14() : Construct("Expr14"), op(Op::invalid) { }
    virtual void serialize(ostream &s, int level = 0) const;
    virtual EvalValue eval(EvalContext *ctx) const;
};

class Stmt : public SingleChildConstruct {

public:
    Stmt() : SingleChildConstruct("Stmt") { }
};

class Block : public MultiElemConstruct {

public:
    Block() : MultiElemConstruct("Block") { }
    virtual EvalValue eval(EvalContext *ctx) const;
};
