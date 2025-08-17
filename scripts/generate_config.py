import json
import os

schema_path = os.path.join(os.path.dirname(__file__), "simucore_config.schema.json")
output_path = os.path.join(os.path.dirname(__file__), "include", "schema_struct.h")

type_map = {
    "string": "std::string",
    "integer": "int",
    "number": "double",
    "boolean": "bool"
}

generated_structs = []

def generate_struct(name, schema):
    props = schema.get("properties", {})
    lines = [f"struct {name} {{"]

    for prop, details in props.items():
        t = details.get("type")

        if t in type_map:  # primitive
            lines.append(f"    {type_map[t]} {prop};")

        elif t == "object":  # nested struct
            child_name = f"{name}_{prop.capitalize()}"
            generate_struct(child_name, details)
            lines.append(f"    {child_name} {prop};")

        elif t == "array":
            items = details.get("items", {})
            item_type = items.get("type")

            if item_type in type_map:  # array of primitives
                lines.append(f"    std::vector<{type_map[item_type]}> {prop};")

            elif item_type == "object":  # array of objects
                child_name = f"{name}_{prop.capitalize()}Item"
                generate_struct(child_name, items)
                lines.append(f"    std::vector<{child_name}> {prop};")

            else:
                lines.append(f"    // TODO: unsupported array type for {prop}")

        else:
            lines.append(f"    // TODO: unsupported type for {prop}")

    lines.append("};")
    generated_structs.append("\n".join(lines))


with open(schema_path, "r") as f:
    schema = json.load(f)

root_name = schema.get("title", "Root")
generate_struct(root_name, schema)

os.makedirs(os.path.dirname(output_path), exist_ok=True)
with open(output_path, "w") as f:
    f.write("#pragma once\n\n#include <string>\n#include <vector>\n\n")
    f.write("\n\n".join(generated_structs))

print(f"Generated struct(s) written to {output_path}")
