#!/usr/bin/env python3
"""
generate_cc.py - pblink-cc C++ Binding Generator Script

Reads proto/topics.yaml and automatically generates:
- GeneratedMessages.hpp: .pb.h includes & type aliases
- GeneratedTopicTraits.inc: DEFINE_TOPIC_TRAITS macro calls
- GeneratedClientDispatch.inc: Client buffer init & frame dispatch code
"""
import os
import argparse
import yaml

def snake_to_pascal(snake_case_string):
    return "".join(word.capitalize() for word in snake_case_string.split('_'))

def write_if_changed(filepath, content):
    os.makedirs(os.path.dirname(filepath), exist_ok=True)
    if os.path.exists(filepath):
        with open(filepath, 'r') as f:
            if f.read() == content:
                return
    with open(filepath, 'w') as f:
        f.write(content)

def main():
    parser = argparse.ArgumentParser(description="pblink-cc C++ Generator")
    parser.add_argument('-y', '--yaml-file', required=True, help="Path to topics.yaml")
    parser.add_argument('-o', '--output-dir', required=True, help="Output directory for generated headers")
    args = parser.parse_args()

    with open(args.yaml_file, 'r') as f:
        config = yaml.safe_load(f)

    topics = config.get('topics', [])

    # 1. Generate GeneratedMessages.hpp
    msg_lines = [
        "// AUTO-GENERATED from topics.yaml by generate_cc.py. DO NOT EDIT.",
        "#pragma once",
        "",
        "#include <pb_encode.h>",
        "#include <pb_decode.h>",
        ""
    ]

    for topic in topics:
        t_name = topic['name']
        msg_lines.append(f'#include "{t_name}.pb.h"')

    msg_lines.append("")
    msg_lines.append("namespace pblink {")
    msg_lines.append("")

    for topic in topics:
        t_name = topic['name']
        pascal_name = snake_to_pascal(t_name)
        msg_lines.append(f"using {pascal_name:<28} = px4_pblink_msgs_{pascal_name};")

    msg_lines.append("")
    msg_lines.append("} // namespace pblink\n")

    write_if_changed(os.path.join(args.output_dir, "GeneratedMessages.hpp"), "\n".join(msg_lines))

    # 2. Generate GeneratedTopicTraits.inc
    trait_lines = [
        "// AUTO-GENERATED from topics.yaml by generate_cc.py. DO NOT EDIT.",
        ""
    ]

    for topic in topics:
        t_name = topic['name']
        pascal_name = snake_to_pascal(t_name)
        msg_type_id = topic.get('msg_type_id', 0)
        hex_id = f"0x{msg_type_id:02X}"
        trait_lines.append(f"DEFINE_TOPIC_TRAITS({pascal_name:<28}, {hex_id}, px4_pblink_msgs_{pascal_name})")

    trait_lines.append("")
    write_if_changed(os.path.join(args.output_dir, "GeneratedTopicTraits.inc"), "\n".join(trait_lines))

    # 3. Generate GeneratedClientDispatch.inc
    dispatch_lines = [
        "// AUTO-GENERATED from topics.yaml by generate_cc.py. DO NOT EDIT.",
        "",
        "#ifdef PBLINK_INIT_BUFFERS_SECTION"
    ]

    for topic in topics:
        pascal_name = snake_to_pascal(topic['name'])
        dispatch_lines.append(f"    INIT_BUFFER({pascal_name})")

    dispatch_lines.extend([
        "#endif // PBLINK_INIT_BUFFERS_SECTION",
        "",
        "#ifdef PBLINK_DISPATCH_SWITCH_SECTION"
    ])

    for topic in topics:
        pascal_name = snake_to_pascal(topic['name'])
        dispatch_lines.append(f"    HANDLE_TOPIC({pascal_name})")

    dispatch_lines.extend([
        "#endif // PBLINK_DISPATCH_SWITCH_SECTION",
        ""
    ])

    write_if_changed(os.path.join(args.output_dir, "GeneratedClientDispatch.inc"), "\n".join(dispatch_lines))
    print(f"Generated C++ bindings for {len(topics)} topics from topics.yaml in {args.output_dir}")

if __name__ == '__main__':
    main()
