from typing import List, Dict

COMMENT_CHAR = '#'
SECTION_HEADER_START_CHAR = '['
SECTION_HEADER_END_CHAR = ']'


class Section:
    """Holds a group of keys, each key can have multiple or no values assigned"""

    def __init__(self, name: str):
        self._name = name
        self._arr = {}

    def set(self, key: str, index: int, val: str) -> None:
        if not self.has(key):
            self.create_key(key)
        if len(self.get_all(key)) < index+1:
            self._arr[key] += [""] * ((index + 1) - len(self._arr[key]))  # if list is smaller than required, initialize values up to required index to ""
        self._arr[key][index] = val

    def push(self, key: str, val: str) -> None:
        """Adds a value to the end of a key, even if there are empty values"""
        if not self.has(key):
            self.create_key(key)
        self._arr[key].append(val)

    def create_key(self, key: str) -> None:
        self._arr[key] = []

    def has(self, key: str) -> bool:
        keys = list(self._arr.keys())
        if key in keys:
            return True
        return False

    def get_entries(self) -> List[str]:
        """Returns a list of all existing keys, even if the keys are empty"""
        return list(self._arr.keys())

    def get_all(self, key: str) -> List[str]:
        """Returns a list of all values within a given key"""
        try:
            return list(self._arr[key])
        except KeyError:
            raise IndexError("Key "+str(key)+" does not exist")

    def __getitem__(self, key: str) -> str:
        """For use with the [] operator, returns first value within a key"""
        try:
            return str(self._arr[key][0])
        except (KeyError, IndexError):
            raise IndexError("Key "+str(key)+" either contains no values, or does not exist yet")


class Config:
    """A wrapper class for storing and accessing multiple sections"""

    def __init__(self, config_file: str, sections: Dict[str, Section]):
        self.filepath = config_file
        self.sections = sections  # a dict where the keys are the names of the Sections


def load(filepath: str) -> Config:
    """Loads a configuration file into a Config object for use within the code"""

    file = open(filepath, "r")
    lines = file.readlines()
    file.close()

    cur_sect = None
    sections = {}
    for line in lines:
        line = line.strip()
        if len(line) == 0 or line[0] == COMMENT_CHAR:  # checking for whitespace only
            continue
        elif line[0] == SECTION_HEADER_START_CHAR and line[-1] == SECTION_HEADER_END_CHAR:
            cur_sect = line[1:-1]
            if cur_sect not in sections:
                sections[cur_sect] = Section(cur_sect)
        elif "=" in line:
            split = line.index("=")
            key = line[:split].strip()
            val = line[split+1:].strip()
            index = 0
            if "[" in key and key[-1] == "]":
                starti = key.index("[")
                try:
                    index = int(key[starti+1:-1])
                    if index < 0:
                        raise Exception() #Only used to run code in the exception block, alongside any exception raised by the previous line
                except Exception:
                    raise ValueError("Key index must be a non-negative integer")
                else:
                    key = key[:starti]

            if len(key) == 0:
                raise ValueError("Key must not be blank")

            sect = sections[cur_sect]
            sect.set(key, index, val)

    return Config(filepath, sections)


def save(config: Config, filepath: str) -> None:
    """Saves a Config object as a file to the provided file path"""

    if type(config) != Config:
        raise TypeError("Can only save instances of Config")

    file = open(filepath, "w")
    for section_name in sorted(config.sections.keys()):
        section = config.sections[section_name]
        file.write(SECTION_HEADER_START_CHAR + section_name + SECTION_HEADER_END_CHAR + "\n")
        for key in sorted(section.get_entries()):
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


class ConfigError(Exception):
    """A type of exception to be used when encountering invalid configuration settings"""

    def __init__(self, sec: str, key: str, val: str, msg: str, index: int = 0):
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

    def key(self) -> str:
        return self._key

    def value(self) -> str:
        return self._val

    def section(self) -> str:
        return self._sec

    def message(self) -> str:
        return self._msg

    def index(self) -> int:
        return self._index
