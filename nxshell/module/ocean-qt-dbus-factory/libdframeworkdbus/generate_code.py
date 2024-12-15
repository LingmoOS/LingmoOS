#!/usr/bin/env python3

import os
import glob
import subprocess

dirname = os.path.dirname
abspath = os.path.abspath
project_root = "../"

def get_xml_files():
    xml_dir = os.path.join(project_root, "xml")
    return (os.path.join(xml_dir, fname) for fname in os.listdir(xml_dir))

def xml_to_generated_name(xml_file):
    xml_name = os.path.basename(xml_file)
    return xml_name.replace(".xml", "").replace(".", "_").lower()

def xml_to_class_name(xml_file):
    xml_name = os.path.basename(xml_file)
    return xml_name.replace(".xml", "").split(".")[-1]

def generate_one(xml_file, dest_dir):
    binary_path = os.path.join(project_root, "bin", "qdbusxml2cpp-fix")
    generated_name = xml_to_generated_name(xml_file)
    class_name = xml_to_class_name(xml_file)
    file_name = os.path.join(dest_dir, generated_name)

    print(binary_path, "-c", class_name, "-p", file_name, xml_file)

    subprocess.call([binary_path, "-c", class_name, "-p", file_name, xml_file])

def main():
    xml_files = get_xml_files()

    # generate generated code dir
    generated_dir = "generated"
    if not os.path.exists(generated_dir):
        os.mkdir(generated_dir)
    else:
        for file in glob.glob1(generated_dir, "*"):
            os.remove(os.path.join(generated_dir, file))

    # generate generated.pri
    generated_pri = os.path.join(os.path.abspath(generated_dir),
                                "generated.pri")
    with open(generated_pri, 'w') as pri:
        print("generating the source code...")
        for xml in xml_files:
            generate_one(xml, generated_dir)

        print("generating the pri file...")
        pwdfy = lambda x: "$$PWD/%s" % x
        sources = list(map(pwdfy, sorted(glob.glob1(generated_dir, "*.cpp"))))
        headers = list(map(pwdfy, sorted(glob.glob1(generated_dir, "*.h"))))
        pri.write("HEADERS += %s\n" % " ".join(headers))
        pri.write("SOURCES += %s" % " ".join(sources))
        print("done.")

if __name__ == '__main__':
    main()
