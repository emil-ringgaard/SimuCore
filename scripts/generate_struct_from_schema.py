import json
import os
from pathlib import Path
import shutil

from simucore_pytest.core.schemas import generate_simucore_schemas


class CppFunction:
    def __init__(self, name: str, generate_with_nlohmann = True,  to_json_arguments: str | None = None, from_json_arguments: str | None = None):
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
    "integer": "unsigned int",
    "number": "double",
    "boolean": "bool"
}

include_dir = Path(__file__).parent.parent.joinpath("include").joinpath("SimuCore").joinpath("generated")
schema_dir = Path(__file__).parent.joinpath('generated')

# Global tracking of generated types
generated_structs = []
generated_type_names = set()  # Track all generated type names
type_definitions = {}  # Store type definitions to avoid duplicates

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
    with open(simucore_base_config) as base_config:
        simucore_base_config = json.load(base_config)
    with open(schema_path) as base_schema:
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


def get_canonical_type_name(schema_def, property_name=None):
    """
    Generate a canonical name for a type based on its schema definition.
    This helps identify when two schemas represent the same type.
    """
    # Create a signature based on the schema structure
    if "title" in schema_def:
        return schema_def["title"]

    # For enums, use the enum values as part of the signature
    if "enum" in schema_def:
        enum_values = sorted(schema_def["enum"])
        return f"Enum_{'_'.join(enum_values)}"

    # For objects, create a signature based on properties
    if schema_def.get("type") == "object" and "properties" in schema_def:
        prop_signature = []
        for prop, details in sorted(schema_def["properties"].items()):
            prop_type = details.get("type", "unknown")
            prop_signature.append(f"{prop}_{prop_type}")
        return f"Object_{'_'.join(prop_signature)}"

    # Fallback to property-based naming
    if property_name:
        return f"Type_{property_name.capitalize()}"

    return "UnknownType"


def generate_enum(name, schema_def, root_schema):
    """Generate enum definition and serialization functions"""
    if name in generated_type_names:
        return name

    # Check if we already have this enum type
    canonical_name = get_canonical_type_name(schema_def)
    if canonical_name in type_definitions:
        return type_definitions[canonical_name]

    enum_values = schema_def["enum"]

    # Build enum definition
    enum_lines = [f"enum class {name} {{"]
    enum_lines.append(", ".join(enum_values))
    enum_lines.append("};")
    generated_structs.append("\n".join(enum_lines))

    # Build enum serialization
    enum_func = CppFunction(
        generate_with_nlohmann=False,
        name=name,
    )

    to_json_body = ["switch(u) {"] + [
        f"case {name}::{val}: j = \"{val}\"; break;"
        for val in enum_values
    ] + ["}"]

    enum_func._to_json = to_json_body
    from_json_body = ["std::string s = j.get<std::string>();"]
    for value in enum_values:
        from_json_body.append(f"if (s == \"{value}\") u = {name}::{value};")
    enum_func._from_json = from_json_body
    generated_structs.append(enum_func.generate())

    generated_type_names.add(name)
    type_definitions[canonical_name] = name
    return name


def generate_struct(name, schema, root_schema):
    """Generate struct definition and serialization functions"""
    if name in generated_type_names:
        return name

    # Check if we already have this struct type
    canonical_name = get_canonical_type_name(schema)
    if canonical_name in type_definitions:
        return type_definitions[canonical_name]

    props = schema.get("properties", {})
    lines = [f"struct {name} {{"]

    # Each struct gets its own CppFunction
    cppFunction = CppFunction(name=name)

    for prop, details in props.items():
        has_default_value = "default" in details
        if "$ref" in details:
            details = resolve_ref(root_schema, details["$ref"])

        t = details.get("type")

        if "enum" in details:
            # Use canonical naming for enums
            enum_canonical = get_canonical_type_name(details)
            if enum_canonical in type_definitions:
                enum_name = type_definitions[enum_canonical]
            else:
                enum_name = f"{prop.capitalize()}Enum"
                enum_name = generate_enum(enum_name, details, root_schema)

            # Use enum in struct
            if has_default_value:
                lines.append(f"    {enum_name} {prop} = {enum_name}::{details['default']};")
            else:
                lines.append(f"    {enum_name} {prop};")

            cppFunction.adl_serializer(prop, f"u.{prop}")

        elif t in type_map:  # primitive
            if has_default_value:
                default_value = details['default']
                lines.append(f"    {type_map[t]} {prop} = {type_map[t]}({str(default_value).lower()});")
            else:
                lines.append(f"    {type_map[t]} {prop};")
            cppFunction.adl_serializer(prop, f"u.{prop}")

        elif t == "object":  # nested struct
            # Check if this object type already exists
            obj_canonical = get_canonical_type_name(details)
            if obj_canonical in type_definitions:
                child_name = type_definitions[obj_canonical]
            else:
                child_name = f"{prop.capitalize()}"
                child_name = generate_struct(child_name, details, root_schema)

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
                # Check if this array item type already exists
                item_canonical = get_canonical_type_name(items)
                if item_canonical in type_definitions:
                    child_name = type_definitions[item_canonical]
                else:
                    child_name = f"{prop.capitalize()}Item"
                    child_name = generate_struct(child_name, items, root_schema)
                lines.append(f"    std::vector<{child_name}> {prop};")
            else:
                lines.append(f"    // TODO: unsupported array type for {prop}")

            cppFunction.adl_serializer(prop, f"u.{prop}")

        else:
            lines.append(f"    // TODO: unsupported type for {prop}")

    lines.append("};")
    generated_structs.append("\n".join(lines))
    generated_structs.append(cppFunction.generate())

    generated_type_names.add(name)
    type_definitions[canonical_name] = name
    return name


def generate_header(schema_filename, header_file_filename):
    header_file = include_dir.joinpath(header_file_filename)
    schema_path = schema_dir.joinpath(schema_filename)

    with open(schema_path) as f:
        schema = json.load(f)

    # First, generate all $defs types (shared types)
    if "$defs" in schema:
        for def_name, def_schema in schema["$defs"].items():
            generate_struct(def_name, def_schema, schema)

    # Then generate the root type
    root_name = schema.get("title", "Root")
    generate_struct(root_name, schema, schema)

    os.makedirs(os.path.dirname(header_file), exist_ok=True)
    with open(header_file, "w") as f:
        f.write("#pragma once\n\n#include <string>\n#include <vector>\n#include <variant>\n#include <SimuCore/json.hpp>\n\nnamespace SimuCore { \n\n")
        f.write("\n\n".join(generated_structs))
        f.write("\n\n}")

    print(f"Generated struct(s) written to {header_file}")


def generate_cpp_and_header_files(env = None):
    # Clear global state
    global generated_structs, generated_type_names, type_definitions
    generated_structs.clear()
    generated_type_names.clear()
    type_definitions.clear()

    shutil.rmtree(include_dir, ignore_errors=True)
    schemas = generate_simucore_schemas(env)

    # Generate all schemas into a single header to avoid duplicates
    all_schemas = {}
    for schema_info in schemas:
        schema_path = schema_dir.joinpath(f'{schema_info["name"]}.schema.json')
        with open(schema_path) as f:
            schema_data = json.load(f)
            all_schemas[schema_info["name"]] = schema_data

    # First pass: collect all $defs from all schemas
    for schema_name, schema_data in all_schemas.items():
        if "$defs" in schema_data:
            for def_name, def_schema in schema_data["$defs"].items():
                generate_struct(def_name, def_schema, schema_data)

    # Second pass: generate root types
    for schema_name, schema_data in all_schemas.items():
        root_name = schema_data.get("title", schema_name)
        generate_struct(root_name, schema_data, schema_data)

    # Write everything to a single header file
    header_file = include_dir.joinpath("Communication.hpp")
    os.makedirs(os.path.dirname(header_file), exist_ok=True)
    with open(header_file, "w") as f:
        f.write("#pragma once\n\n#include <string>\n#include <vector>\n#include <variant>\n#include <SimuCore/json.hpp>\n\nnamespace SimuCore { \n\n")
        f.write("\n\n".join(generated_structs))
        f.write("\n\n}")

    print(f"All types generated in single header: {header_file}")


if __name__ == '__main__':
    generate_cpp_and_header_files()
