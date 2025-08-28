import json
import os
import shutil
from pathlib import Path
from simucore_schema import generate_simucore_schemas


class CppFunction:
    def __init__(self, name: str, generate_with_nlohmann = True,  to_json_arguments: str = None, from_json_arguments: str = None):
        self.name = name
        self._to_json = []
        self._from_json = []
        self.to_json_arguments = to_json_arguments
        self.from_json_arguments = from_json_arguments
        self.generate_with_nlohmann = generate_with_nlohmann

    def adl_serializer(self, key: str, value: str):
        self._to_json.append(f"{{\"{key}\", {value}}},")
        self._from_json.append(f"j.at(\"{key}\").get_to({value});")
    
    def generate(self):
        code = f"inline void to_json(nlohmann::json &j, const {self.name} &u) {{\n" if not self.to_json_arguments else self.to_json_arguments
        if self.generate_with_nlohmann:
            code += "j = nlohmann::json {"
        for line in self._to_json:
            code += line + "\n"
        if self.generate_with_nlohmann:
            code += "};"
        code += "}\n"
        code += f"inline void from_json(const nlohmann::json &j, {self.name} &u) {{\n" if not self.from_json_arguments else self.from_json_arguments

        for line in self._from_json:
            code += line + "\n"
        code += "}"
        return code

type_map = {
    "string": "std::string",
    "integer": "uint",
    "number": "double",
    "boolean": "bool"
}

include_dir = Path(__file__).parent.parent.joinpath("include").joinpath("SimuCore").joinpath("generated")
schema_dir = Path(__file__).parent.joinpath('generated')

generated_structs = []

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



def generate_cpp_config(env, schema_path):
    from jsonschema import validate
    if not Path(env['PROJECT_DIR']).joinpath("SimuCoreBaseConfig.json").is_file():
        raise RuntimeError("No configuration found in project!!! Please create a config named SimuCoreBaseConfig.json based on TODO:Schema")
    simucore_base_config = Path(env['PROJECT_DIR']).joinpath('SimuCoreBaseConfig.json')
    with open(simucore_base_config, 'r') as base_config:
        simucore_base_config = json.load(base_config)
    with open(schema_path, 'r') as base_schema:
        schema = json.load(base_schema)
    validate(instance=simucore_base_config, schema=schema)
    initializer = generate_initializer('Config', schema, simucore_base_config)
    with open(include_dir.joinpath('SimuCoreBaseConfig.hpp'), 'a') as new_config:
        new_config.write(f"\nnamespace SimuCore {{\n inline const Config config = {initializer};\n }}\n")

def resolve_ref(schema, ref: str):
    # e.g. "#/$defs/InputUpdate"
    parts = ref.lstrip("#/").split("/")
    resolved = schema
    for p in parts:
        resolved = resolved[p]
    return resolved


def generate_struct(name, schema, root_schema):
    props = schema.get("properties", {})
    lines = [f"struct {name} {{"]

    # Each struct gets its own CppFunction
    cppFunction = CppFunction(
        name=name,
    )
    variantCppJson: CppFunction = None
    for prop, details in props.items():
        has_default_value = "default" in details
        if "$ref" in details:
            details = resolve_ref(root_schema, details["$ref"])

        t = details.get("type")


        if "enum" in details:
            # Create enum name
            enum_name = f"{name}_{prop.capitalize()}Enum"
            enum_values = details["enum"]

            # Build enum definition
            enum_lines = [f"enum class {enum_name} {{"]
            enum_lines.append(", ".join(enum_values))
            enum_lines.append("};")
            generated_structs.append("\n".join(enum_lines))

            # Build enum -> json using CppFunction
            enum_func = CppFunction(
                generate_with_nlohmann=False,
                name=enum_name,
            )
            enum_func.adl_serializer("// handled via switch", "// dummy")  # placeholder

            to_json_body = ["switch(u) {"] + [
                f"case {enum_name}::{val}: j = \"{val}\"; break;"
                for val in enum_values
            ] + ["}"]

            enum_func._to_json = to_json_body  # replace placeholder with switch
            from_json_body = ["std::string s = j.get<std::string>();"]
            for value in enum_values:
                from_json_body.append(f"if (s == \"{value}\") u = {enum_name}::{value};")
            enum_func._from_json = from_json_body
            generated_structs.append(enum_func.generate())

            # Use enum in struct
            if has_default_value:
                lines.append(f"    {enum_name} {prop} = {enum_name}::{details['default']};")
            else:
                lines.append(f"    {enum_name} {prop};")
            # Add struct field mapping
            cppFunction.adl_serializer(prop, f"u.{prop}")

        elif t in type_map:  # primitive
            if has_default_value:
                default_value = details['default']
                lines.append(f"    {type_map[t]} {prop} = {type_map[t]}({str(default_value).lower()});")
            else:
                lines.append(f"    {type_map[t]} {prop};")
            cppFunction.adl_serializer(prop, f"u.{prop}")

        elif t == "object":  # nested struct
            child_name = f"{name}_{prop.capitalize()}"
            generate_struct(child_name, details, root_schema)
            lines.append(f"    {child_name} {prop};")
            cppFunction.adl_serializer(prop, f"u.{prop}")

        elif t == "array":
            items = details.get("items", {})
            if "$ref" in items:
                items = resolve_ref(root_schema, items["$ref"])
            item_type = items.get("type")

            if item_type in type_map:
                lines.append(f"    std::vector<{type_map[item_type]}> {prop};")
            elif item_type == "object":
                child_name = f"{name}_{prop.capitalize()}Item"
                generate_struct(child_name, items, root_schema)
                lines.append(f"    std::vector<{child_name}> {prop};")
            else:
                lines.append(f"    // TODO: unsupported array type for {prop}")

            cppFunction.adl_serializer(prop, f"u.{prop}")
        
        else:
            lines.append(f"    // TODO: unsupported type for {prop}")

    lines.append("};")
    generated_structs.append("\n".join(lines))
    generated_structs.append(cppFunction.generate())
    if variantCppJson:
        generated_structs.append(variantCppJson.generate())



def generate_header(schema_filename, header_file_filename):
    header_file = include_dir.joinpath(header_file_filename)
    schema_path = schema_dir.joinpath(schema_filename)

    with open(schema_path, "r") as f:
        schema = json.load(f)
    root_name = schema.get("title", "Root")
    generate_struct(root_name, schema, schema)  # pass schema twice

    os.makedirs(os.path.dirname(header_file), exist_ok=True)
    with open(header_file, "w") as f:
        f.write("#pragma once\n\n#include <string>\n#include <vector>\n#include <variant>\n#include <SimuCore/json.hpp>\n\nnamespace SimuCore { \n\n")
        f.write("\n\n".join(generated_structs))
        f.write("}")
    generated_structs.clear()
    print(f"Generated struct(s) written to {header_file}")


def generate_cpp_and_header_files(env = None):
    shutil.rmtree(include_dir, ignore_errors=True)
    schemas = generate_simucore_schemas()
    for schema_info in schemas:
        generate_header(f'{schema_info["name"]}.schema.json', f'{schema_info["name"]}.hpp')

if __name__ == '__main__':
    generate_cpp_and_header_files()
