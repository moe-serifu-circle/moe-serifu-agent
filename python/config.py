import io

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
        if line[0] == COMMENT_CHAR or len(line.strip()) == 0: #checking for whitespace only
            pass
        elif line[0] == SECTION_HEADER_START_CHAR and line.strip()[-1] == SECTION_HEADER_END_CHAR:
            cur_sect = line[1:-1]
            if cur_sect not in list(sections.keys()):
                sections[cur_sect] = Section(cur_sect)
        elif "=" in line:
            index = line.index("=")
            key = line[:index].strip()
            val = line[index+1:].strip()
            if "[" in val and val[-1] == "]":
                starti = (len(val)-1)-val[::-1].index("[") #finds final instance of '[' in line, in case another instance is within val
                try:
                    #end of point I can write here
            

def save(config, filepath):
    if type(config) != Config:
        raise TypeError("Can only save instances of Config")

class Config():
    def __init__(self, config_file, sections):
        self.filepath = config_file
        self.sections = sections #a dict where the keys are the names of the Sections

class Section():
    def __init__(self, name):
        self._name = name
        self._arr = {}

    def set(self, key, val):
        self._arr[key] = [val]

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
        
