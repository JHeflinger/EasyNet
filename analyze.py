import os
import re

statement = "\n"

def clean_sloc_line(input_string):
    pattern = r'[ \n\r\t{};()]'
    cleaned_string = re.sub(pattern, '', input_string)
    return cleaned_string

def trim_whitespace(input_string):
    pattern = r'[ \n\r\t]'
    cleaned_string = re.sub(pattern, '', input_string)
    return cleaned_string

def pad_num(num):
    num_spaces = max(0, 15 - len(str(num)))
    padded_string = ' ' * num_spaces + str(num)
    return padded_string

def analyze_code(directory):
    total_sloc = 0
    total_files = 0
    total_hdrs = 0
    total_srcs = 0
    total_classes = 0
    total_structs = 0
    total_typedefs = 0
    total_includes = 0
    for root, dirs, files in os.walk(directory):
        for file in files:
            filepath = os.path.join(root, file)
            current_sloc = 0
            if ".cpp" in filepath:
                total_srcs += 1
            if ".h" in filepath:
                total_hdrs += 1
            total_files += 1
            with open(filepath, 'r') as file:
                for line in file:
                    if len(clean_sloc_line(line)) > 0:
                        current_sloc += 1
                    if len(line) > 6 and "struct" == line[0:6]:
                        total_structs += 1
                    if len(line) > 5 and "class" == line[0:5]:
                        total_classes += 1
                    if len(line) > 7 and "typedef" == line[0:7]:
                        total_typedefs += 1
                    if len(line) > 8 and "#include" == line[0:8]:
                        total_includes += 1
            total_sloc += current_sloc
    return ("CODE ANALYSIS: \n" + 
            "┌──────────────────┬───────────────┐\n"
            "│1. TOTAL SLOC     │" + pad_num(total_sloc) + "│\n" +
            "├──────────────────┼───────────────┤\n"
            "│2. HEADER FILES   │" + pad_num(total_hdrs) + "│\n" +
            "├──────────────────┼───────────────┤\n"
            "│3. SOURCE FILES   │" + pad_num(total_srcs) + "│\n" +
            "├──────────────────┼───────────────┤\n"
            "│4. TOTAL FILES    │" + pad_num(total_files) + "│\n" +
            "├──────────────────┼───────────────┤\n"
            "│5. STRUCTS        │" + pad_num(total_structs) + "│\n" +
            "├──────────────────┼───────────────┤\n"
            "│6. CLASSES        │" + pad_num(total_classes) + "│\n" +
            "├──────────────────┼───────────────┤\n"
            "│7. TYPEDEFS       │" + pad_num(total_typedefs) + "│\n" +
            "├──────────────────┼───────────────┤\n"
            "│8. INCLUDES       │" + pad_num(total_includes) + "│\n" +
            "└──────────────────┴───────────────┘\n")

def analyze_progress():
    stage = ""
    minor_tasks = 0
    medium_tasks = 0
    major_tasks = 0
    completed_tasks = 0
    sub_tasks = 0
    total_tasks = 0
    with open("TODO.txt", 'r') as file:
       for line in file:
            if "MINOR" in line:
                stage = "MINOR"
                continue
            elif "MEDIUM" in line:
                stage = "MEDIUM"
                continue
            elif "MAJOR" in line:
                stage = "MAJOR"
                continue
            elif "COMPLETED" in line:
                stage = "COMPLETED"
                continue
            elif len(line) > 3:
                total_tasks += 1
                if "\t\t" == line[0:2] or "        " == line[0:8]:
                    sub_tasks += 1
                elif "\t" == line[0:1] or "    " == line[0:4]:
                    if stage == "MINOR":
                        minor_tasks += 1
                    if stage == "MEDIUM":
                        medium_tasks += 1
                    if stage == "MAJOR":
                        major_tasks += 1
                    if stage == "COMPLETED":
                        completed_tasks += 1
    return ("PROGRESS ANALYSIS: \n" + 
            "┌──────────────────┬───────────────┐\n"
            "│1. TOTAL TASKS    │" + pad_num(total_tasks) + "│\n" +
            "├──────────────────┼───────────────┤\n"
            "│2. MAJOR TASKS    │" + pad_num(major_tasks) + "│\n" +
            "├──────────────────┼───────────────┤\n"
            "│3. MEDIUM TASKS   │" + pad_num(medium_tasks) + "│\n" +
            "├──────────────────┼───────────────┤\n"
            "│4. MINOR TASKS    │" + pad_num(minor_tasks) + "│\n" +
            "├──────────────────┼───────────────┤\n"
            "│5. SUBTASKS       │" + pad_num(sub_tasks) + "│\n" +
            "├──────────────────┼───────────────┤\n"
            "│6. COMPLETED      │" + pad_num(completed_tasks) + "│\n" +
            "└──────────────────┴───────────────┘\n")


print("Performing project analysis...")
print("Analyzing SLOC...")
statement += analyze_code("include") + "\n"


print(statement)
