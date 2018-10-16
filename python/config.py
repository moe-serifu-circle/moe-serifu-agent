class config_error(Exception):
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
        
