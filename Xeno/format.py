import sys, os, re, time

def main():
    if len(sys.argv) < 2:
        print("no args found!"); time.sleep(5)
        return
    file_name = sys.argv[1]
    if not os.path.isfile(file_name):
        print(f"{file_name} was not found"); time.sleep(5)
        return
    with open(file_name, 'r') as file:
        contents = file.read()

        def preserve_spaces(match):
            return match.group(0).replace('\n', ' ')
        
        contents = re.sub(r'"[^"]*"', preserve_spaces, contents)
        contents = re.sub(r'\n+', ' ', contents)
        contents = re.sub(r'(\S)\s{2,}(\S)', r'\1 \2', contents)
        contents = contents.replace('"', '\\"')
        
        with open("formatted.txt", 'w') as formatted:
            formatted.write(f'"{contents}"')

if __name__ == "__main__":
    main()