#pragma once
#include <SimuCore/Binding.hpp>
#include <vector>

class Component;

class Component {
    protected:
        static std::vector<BindingBase*> allBindings;
        std::vector<Component*> subcomponents;
        virtual void update() = 0;

    public:
        Component(Component* parent = nullptr) {
            if (parent) {
                parent->subcomponents.push_back(this);
            }
        }
        virtual ~Component() {
            for (auto* c : subcomponents) delete c;
        }
        void updateAll() {
            update();                              // Update self
            for (auto* c : subcomponents)          // Update children
                c->updateAll();
        }
        static void updateAllBindings() {
            for (auto* b : allBindings)
                b->updateInputs();
        }
        template<typename T>
        void bind(Signal<T>& outvar, Signal<T>& invar) {
            allBindings.push_back(new Binding<T>(outvar, invar));
        }
};
