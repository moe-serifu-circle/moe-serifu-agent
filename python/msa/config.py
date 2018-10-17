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
        
