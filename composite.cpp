#include <cmath>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

class Expression {
public:
    virtual ~Expression() = default;

    virtual double evaluate(double x) const = 0;
    virtual std::shared_ptr<Expression> derivative() const = 0;
    virtual std::string toString() const = 0;

    virtual bool isConstant(double& out) const { (void)out; return false; }
};

class Constant : public Expression {
    double value;

public:
    explicit Constant(double v) : value(v) {}

    double evaluate(double) const override { return value; }

    std::shared_ptr<Expression> derivative() const override {
        return std::make_shared<Constant>(0.0);
    }

    std::string toString() const override {
        std::ostringstream os;
        os << value;
        return os.str();
    }

    bool isConstant(double& out) const override {
        out = value;
        return true;
    }
};

class Variable : public Expression {
public:
    double evaluate(double x) const override { return x; }

    std::shared_ptr<Expression> derivative() const override {
        return std::make_shared<Constant>(1.0);
    }

    std::string toString() const override { return "x"; }
};

class BinaryOperation : public Expression {
protected:
    std::shared_ptr<Expression> left;
    std::shared_ptr<Expression> right;

public:
    BinaryOperation(std::shared_ptr<Expression> l, std::shared_ptr<Expression> r)
        : left(std::move(l)), right(std::move(r)) {}
};

class Add : public BinaryOperation {
public:
    using BinaryOperation::BinaryOperation;

    double evaluate(double x) const override {
        return left->evaluate(x) + right->evaluate(x);
    }

    std::shared_ptr<Expression> derivative() const override {
        return std::make_shared<Add>(left->derivative(), right->derivative());
    }

    std::string toString() const override {
        return "(" + left->toString() + " + " + right->toString() + ")";
    }
};

class Subtract : public BinaryOperation {
public:
    using BinaryOperation::BinaryOperation;

    double evaluate(double x) const override {
        return left->evaluate(x) - right->evaluate(x);
    }

    std::shared_ptr<Expression> derivative() const override {
        return std::make_shared<Subtract>(left->derivative(), right->derivative());
    }

    std::string toString() const override {
        return "(" + left->toString() + " - " + right->toString() + ")";
    }
};

class Multiply : public BinaryOperation {
public:
    using BinaryOperation::BinaryOperation;

    double evaluate(double x) const override {
        return left->evaluate(x) * right->evaluate(x);
    }

    std::shared_ptr<Expression> derivative() const override {
        return std::make_shared<Add>(
            std::make_shared<Multiply>(left->derivative(), right),
            std::make_shared<Multiply>(left, right->derivative())
        );
    }

    std::string toString() const override {
        return "(" + left->toString() + " * " + right->toString() + ")";
    }
};

class Power : public BinaryOperation {
public:
    using BinaryOperation::BinaryOperation;

    double evaluate(double x) const override {
        return std::pow(left->evaluate(x), right->evaluate(x));
    }

    std::shared_ptr<Expression> derivative() const override {
        double n;
        if (!right->isConstant(n)) {
            throw std::runtime_error("Power::derivative solo soporta exponente constante (sin dynamic_cast).");
        }

        return std::make_shared<Multiply>(
            std::make_shared<Multiply>(
                std::make_shared<Constant>(n),
                std::make_shared<Power>(left, std::make_shared<Constant>(n - 1.0))
            ),
            left->derivative()
        );
    }

    std::string toString() const override {
        return "(" + left->toString() + " ^ " + right->toString() + ")";
    }
};

class Divide : public BinaryOperation {
public:
    using BinaryOperation::BinaryOperation;

    double evaluate(double x) const override {
        return left->evaluate(x) / right->evaluate(x);
    }

    std::shared_ptr<Expression> derivative() const override {
        auto u_prime_v = std::make_shared<Multiply>(left->derivative(), right);
        auto u_v_prime = std::make_shared<Multiply>(left, right->derivative());
        auto numerator = std::make_shared<Subtract>(u_prime_v, u_v_prime);

        auto denom = std::make_shared<Power>(right, std::make_shared<Constant>(2.0));
        return std::make_shared<Divide>(numerator, denom);
    }

    std::string toString() const override {
        return "(" + left->toString() + " / " + right->toString() + ")";
    }
};

class UnaryFunction : public Expression {
protected:
    std::shared_ptr<Expression> arg;

public:
    explicit UnaryFunction(std::shared_ptr<Expression> a) : arg(std::move(a)) {}
};


class Cos : public UnaryFunction {
public:
    using UnaryFunction::UnaryFunction;

    double evaluate(double x) const override {
        return std::cos(arg->evaluate(x));
    }

    std::shared_ptr<Expression> derivative() const override;

    std::string toString() const override {
        return "cos(" + arg->toString() + ")";
    }
};

class Sin : public UnaryFunction {
public:
    using UnaryFunction::UnaryFunction;

    double evaluate(double x) const override {
        return std::sin(arg->evaluate(x));
    }

    std::shared_ptr<Expression> derivative() const override {
        return std::make_shared<Multiply>(
            std::make_shared<Cos>(arg),
            arg->derivative()
        );
    }

    std::string toString() const override {
        return "sin(" + arg->toString() + ")";
    }
};

std::shared_ptr<Expression> Cos::derivative() const {
    return std::make_shared<Multiply>(
        std::make_shared<Multiply>(
            std::make_shared<Constant>(-1.0),
            std::make_shared<Sin>(arg)
        ),
        arg->derivative()
    );
}

int main() {
    auto x = std::make_shared<Variable>();
    auto expr =
        std::make_shared<Multiply>(
            std::make_shared<Add>(
                std::make_shared<Power>(x,std::make_shared<Constant>(2)
                    ),
                    std::make_shared<Multiply>(
                        std::make_shared<Constant>(3),
                            x
                        )
                        ),
                        std::make_shared<Sin>(x)
                        );

    auto deriv = expr->derivative();
    std::cout << "f(x) = " << expr->toString() << std::endl;
    std::cout << "f’(x) = " << deriv->toString() << std::endl;
    std::cout << "f(2) = " << expr->evaluate(2) << std::endl;

    return 0;
}