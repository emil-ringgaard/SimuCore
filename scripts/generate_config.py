from generate_struct_from_schema import include_dir, schema_dir
from pathlib import Path
from simucore_schema import Config, generate_simcore_schema
import json

type_map = {
    'integer': 'int',
    'boolean': 'bool',
    'string': 'std::string',
    'number': 'double'
}

def generate_config_class(env = None):
    schema_info = generate_simcore_schema(Config)
    with open (schema_info['schema_path'], 'r') as schema:
        config_schema = json.load(schema)
    from jsonschema import validate
    if env:
        if not Path(env['PROJECT_DIR']).joinpath("SimuCoreBaseConfig.json").is_file():
            raise RuntimeError("No configuration found in project!!! Please create a config named SimuCoreBaseConfig.json based on TODO:Schema")
        simucore_base_config = Path(env['PROJECT_DIR']).joinpath('SimuCoreBaseConfig.json')
    else:
        simucore_base_config = Path(__file__).parent.joinpath('SimuCoreBaseConfig.json')
    with open(simucore_base_config, 'r') as base_config:
        simucore_base_config = json.load(base_config)
    validate(instance=simucore_base_config, schema=config_schema)
    properties = config_schema['properties']
    parameters = []
    initializer_list = [f'Component(parent, name)']
    for parameter_name, detail in properties.items():
        parameter_value = str(simucore_base_config[parameter_name]).lower()
        parameter_json_type = detail["type"]
        if parameter_json_type == 'string':
            parameter_value = f"\"{parameter_value}\""
        parameters.append(f'Parameter<{type_map[parameter_json_type]}> {parameter_name};')
        initializer_list.append(f'{parameter_name}(this, \"{parameter_name}\", {parameter_value})')
    cpp_template = """
#pragma once
#include <SimuCore/Signal.hpp>
#include <SimuCore/Component.hpp>
namespace SimuCore {{
class {class_name} : public Component {{
public:
    {member_variables}

    {class_name}({constructor_args})
        : {initializer_list} {{
    }}

    {member_functions}
}};
inline Config config(nullptr, "Config");
}}
"""
    custom_class_values = {
        "class_name": Config.__name__,
        "member_variables": "\n".join(parameters),
        "constructor_args": "Component *parent, std::string name",
        "initializer_list": ",\n".join(initializer_list),
        "member_functions": "\n".join(["void execute() override {}", "void init() override {}"])
    }
    with open (include_dir.joinpath(Config.__name__ + ".hpp"), 'w') as config:
        config.write(cpp_template.format(**custom_class_values))

if '__main__' == __name__:
    generate_config_class()