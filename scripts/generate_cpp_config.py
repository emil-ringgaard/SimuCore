from pathlib import Path
import json
from generate_config_struct import type_map, schema_path, output_path


def cpp_value(value):
    """Convert Python JSON value → valid C++ initializer expression"""
    if isinstance(value, str):
        return f"\"{value}\""
    elif isinstance(value, bool):
        return "true" if value else "false"
    elif isinstance(value, (int, float)):
        return str(value)
    elif isinstance(value, list):
        if not value:
            return "{}"
        return "{" + ", ".join(cpp_value(v) for v in value) + "}"
    elif isinstance(value, dict):
        if not value:
            return "{}"
        return "{" + ", ".join(cpp_value(v) for v in value.values()) + "}"
    else:
        return "{}"  # fallback


def generate_initializer(root_name, schema, data, indent=0):
    """Recursively build C++ aggregate initializer for given data"""
    props = schema.get("properties", {})
    items = []

    for prop, details in props.items():
        if prop not in data:
            items.append("{}")
            continue

        value = data[prop]
        t = details.get("type")

        if t in type_map:
            items.append(cpp_value(value))

        elif t == "object":
            items.append(generate_initializer(f"{root_name}_{prop.capitalize()}", details, value, indent + 1))

        elif t == "array":
            items.append(cpp_value(value))

    return "{" + ", ".join(items) + "}"



def generate_cpp_config(env):
    env.Execute("$PYTHONEXE -m pip install jsonschema")
    from jsonschema import validate
    if not Path(env['PROJECT_DIR']).joinpath("SimuCoreBaseConfig.json").is_file():
        raise RuntimeError("No configuration found in project!!! Please create a config named SimuCoreBaseConfig.json based on TODO:Schema")
    simucore_base_config = Path(env['PROJECT_DIR']).joinpath('SimuCoreBaseConfig.json')
    with open(simucore_base_config, 'r') as base_config:
        simucore_base_config = json.load(base_config)
    with open(schema_path, 'r') as base_schema:
        schema = json.load(base_schema)
    validate(instance=simucore_base_config, schema=schema)
    root_name = schema.get("title", "Root")
    initializer = generate_initializer(root_name, schema, simucore_base_config)
    with open(output_path, 'a') as new_config:
        new_config.write(f"\ninline const {root_name} config_instance = {initializer};\n")