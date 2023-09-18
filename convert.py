import re

inputFileName=r'test.c'
supplementaryFileName=r'Supplementary.txt'
outputFileName=r'test_out.c'

with open(inputFileName,'r+') as file:
    Original_File = file.readlines()

with open(supplementaryFileName,'r+') as file:
    Splmtry_File = file.read()

insert_line=0
for x in range(len(Original_File)):
    if Original_File[x][0]!="#" and not Original_File[x].isspace():
        insert_line=x
        break
    print(Original_File[x])

with open(outputFileName,'w+') as file:
    for x in range(insert_line):
        file.write(Original_File[x])
        print(Original_File[x])

with open(outputFileName,'a+') as file:
    file.write('\n')
    file.write(Splmtry_File)
    file.write('\n\n')

def ptr_tranform(string):

    pattern=r'(\w+).use_count\(\)'
    match = re.search(pattern, string)
    if match:
        ptr = match.group(1)
        replacement = f"ptr_use_count({ptr})"
        return re.sub(pattern, replacement, string)

    pattern = r'std::shared_ptr<(\w+)>\s+(\w+)\s*=\s*std::make_shared<(\w+)>\(\)'
    match = re.search(pattern, string)
    if match:
        typ=match.group(1)
        nam=match.group(2)
        replacement = f'MAKE_SHARED({nam}, {typ})'
        return re.sub(pattern, replacement, string)

    pattern = r'swap\(\s*(\w+)\s*,\s*(\w+)\s*\)'
    match = re.search(pattern, string)
    if match:
        ptr1=match.group(1)
        ptr2=match.group(2)
        replacement = f'ptr_swap({ptr1},{ptr2})'
        return re.sub(pattern, replacement, string)

    pattern = r'(\w+\W*)\s+(\w+)\s*=\s*(\w+)\.get\(\)'
    match = re.search(pattern, string)
    if match:
        typ=match.group(1)
        ptr=match.group(2)
        shptr=match.group(3)
        replacement = f'{typ} {ptr} = ({typ})shared_ptr_get({shptr})'
        return re.sub(pattern, replacement, string)

    pattern = r'(\w+)\.~shared_ptr\(\)'
    match = re.search(pattern, string)
    if match:
        ptr=match.group(1)
        replacement = f'shared_ptr_destructor(&{ptr})'
        return re.sub(pattern, replacement, string)

    pattern = r'(\w+)\.reset\(new\s+(\w+)\)'
    match = re.search(pattern, string)
    if match:
        ptr=match.group(1)
        typ=match.group(2)
        replacement = f'MAKE_SHARED_RESET({ptr},{typ})'
        return re.sub(pattern, replacement, string)

    pattern = r'std::weak_ptr<\w+>\s+(\w+)\((\w+)\)'
    match = re.search(pattern, string)
    if match:
        wkp=match.group(1)
        shp=match.group(2)
        replacement = f'MAKE_WEAKPTR({wkp}, {shp})'
        return re.sub(pattern, replacement, string)

    pattern = r'std::shared_ptr<\w+>\s+(\w+)\s*=\s*(\w+)\.lock\(\)'
    match = re.search(pattern, string)
    if match:
        shp=match.group(1)
        wkp=match.group(2)
        replacement = f'WEAK_PTR_LOCK({shp}, {wkp})'
        return re.sub(pattern, replacement, string)

    pattern = r'(\w+)\.expired\(\)'
    match = re.search(pattern, string)
    if match:
        wkp=match.group(1)
        replacement = f'weak_ptr_expired({wkp})'
        return re.sub(pattern, replacement, string)

    pattern = r'std::shared_ptr<(\w+)>\s+(\w+)\s*=\s*(\w+)'
    match = re.search(pattern, string)
    if match:
        toptr=match.group(2)
        fromptr=match.group(3)
        replacement = f'MAKE_SHARED_COPY({toptr}, {fromptr})'
        return re.sub(pattern, replacement, string)

    return string

with open(outputFileName,'a+') as file:
    for index in range(len(Original_File)-insert_line):
        str=Original_File[index+insert_line]
        str=ptr_tranform(str)
        file.write(str)
        print(str)














