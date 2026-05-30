#pragma once
#include <SimuCore/Component.hpp>
#include <SimuCore/json.hpp>

class ApplicationTree
{
public:
    ApplicationTree(Component *root) : _root(root) {}
    const nlohmann::json getApplicationTreeAsJson();
    ~ApplicationTree() {}

private:
    Component *_root;
};