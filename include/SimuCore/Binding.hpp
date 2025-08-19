#pragma once
#include <SimuCore/Signal.hpp>

class BindingBase {
public:
    virtual void updateInputs() = 0;
    virtual ~BindingBase() = default;
};


template<typename T>
class Binding : public BindingBase {
public:
    Signal<T>* outvar;
    Signal<T>* invar;

    Binding(Signal<T>& o, Signal<T>& i) : outvar(&o), invar(&i) {}

    void updateInputs() override {
        invar->value = outvar->value;  // Copy value
    }
};