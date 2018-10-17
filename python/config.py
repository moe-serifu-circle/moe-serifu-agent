COMMENT_CHAR = '#'
SECTION_HEADER_START_CHAR = '['
SECTION_HEADER_END_CHAR = ']'

def load(filepath):
    file = open(filepath, "r")
    lines = file.readlines()
    file.close()

    cur_sect = None
    sections = {}
    for line in lines:
        line = line.strip()
        if len(line) == 0 or line[0] == COMMENT_CHAR: #checking for whitespace only
            pass
        elif line[0] == SECTION_HEADER_START_CHAR and line[-1] == SECTION_HEADER_END_CHAR:
            cur_sect = line[1:-1]
            if cur_sect not in list(sections.keys()):
                sections[cur_sect] = Section(cur_sect)
        elif "=" in line:
            split = line.index("=")
            key = line[:split].strip()
            val = line[split+1:].strip()
            index = 0
            if "[" in key and key[-1] == "]":
                starti = key.index("[")
                try:
                    index = int(key[starti:-1])
                    if index < 0:
                        raise Exception()
                    key = key[:starti]
                except:
                    raise ValueError("Key index must be a non-negative integer")

            if len(key)==0:
                raise ValueError("Key must not be blank")

            sect = sections[cur_sect]
            sect.set(key, index, val)

    return Config(filepath, sections)

def save(config, filepath):
    if type(config) != Config:
        raise TypeError("Can only save instances of Config")

    file = open(filepath, "w")
    for section_name in config.sections:
        section = config.sections[section_name]
        file.write(SECTION_HEADER_START_CHAR + section_name + SECTION_HEADER_END_CHAR + "\n")
        for key in section.get_entries():
            entries = section.get_all(key)
            use_index = False
            if len(entries) > 1:
                use_index = True
            for index, entry in enumerate(entries):
                line = key
                if use_index:
                    line += "["+str(index)+"]"
                line += " = " + entry
            file.write(line + "\n")
        file.write("\n")
    file.close()

class Config():
    def __init__(self, config_file, sections):
        self.filepath = config_file
        self.sections = sections #a dict where the keys are the names of the Sections

class Section():
    def __init__(self, name):
        self._name = name
        self._arr = {}

    def set(self, key, index, val):
        if not self.has(key):
            self.create_key(key)
        if len(self.get_all(key)) < index+1:
            self._arr[key] += [""]*((index+1)-len(self._arr[key]))
        self._arr[key][index] = val

    def push(self, key, val):
        if not self.has(key):
            self.create_key(key)
        self._arr[key].append(val)

    def create_key(self, key):
        self._arr[key] = []

    def has(self, key):
        keys = list(self._arr.keys())
        if key in keys:
            return True
        return False

    def get_entries(self):
        return list(self._arr.keys())

    def get_all(self, key):
        try:
            return list(self._arr[key])
        except KeyError:
            raise IndexError("Key "+str(key)+" does not exist")

    def __getitem__(self, key):
        try:
            return str(self._arr[key][0])
        except (KeyError, IndexError):
            raise IndexError("Key "+str(key)+" either contains no values, or does not exist yet")

class ConfigError(Exception):
    def __init__(self, sec, key, val, msg, index=0):
        cache = str(sec)+"."+str(key)
        if index != 0:
            cache += "[" + str(index) + "]"
        cache += " \""+str(val)+"\" " + msg
        super().__init__(cache)
        self._sec = sec
        self._key = key
        self._val = val
        self._msg = msg
        self._index = index

    def key():
        return self._key

    def value():
        return self._val

    def section():
        return self._sec

    def message():
        return self._msg

    def index():
        return self._index
        
