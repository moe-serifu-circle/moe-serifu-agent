class Expander:
    """Holds variables for substitution in strings"""

    def __init__(self):
        self.substitutions = {}

    def register_var(self, var: str) -> None:
        if var in self.substitutions:
            raise NameError("Variable " + var + " already exists")
        elif var == "":
            raise NameError("Variable name cannot be blank")
        self.substitutions[var] = ("", False)

    def register_protected(self, var: str, val: str) -> None:
        if var in self.substitutions:
            raise NameError("Variable " + var + " already exists")
        elif var == "":
            raise NameError("Variable name cannot be blank")
        self.substitutions[var] = (val, True)

    def unregister_var(self, var: str) -> None:
        if var in self.substitutions:
            if self.substitutions[var][1]:
                raise Exception("Cannot unregister " + var + ", variable is protected")
            self.substitutions.pop(var)

    def unregister_protected(self, var: str) -> None:
        if var in self.substitutions:
            if not self.substitutions[var][1]:
                raise Exception("Cannot unregister " + var + ", variable is not protected")
            self.substitutions.pop(var)

    def set_value(self, var: str, val: str) -> None:
        if var not in self.substitutions:
            raise NameError("Variable " + var + " does not exist")
        if self.substitutions[var][1]:
            raise Exception("Cannot change " + var + ", variable is protected")
        self.substitutions[var] = val

    def get_value(self, var: str) -> str:
        if var in self.substitutions:
            return self.substitutions[var][0]
        else:
            return ""

    def expand(self, text: str) -> str:
        """Replaces any variables within a string to their values if any, variables are preceded by $"""

        skip = False  # skip escaped characters
        replace = []
        for index, char in enumerate(text):
            if skip:
                skip = False
            elif char == "\\":
                skip = True
                replace.append((index+1, index+1))  # this is a somewhat dirty way of removing escape characters, but works since an empty string cannot be a variable,
            elif char == "$":
                if text[index + 1].isidentifier():
                    end = index + 2
                    while text[index + 1:end + 1].isidentifier() and end < len(text):
                        end += 1
                    replace.append((index+1, end))

        text = self._replace(0, text, replace)

        return text

    def _replace(self, last_end, text, group) -> str:
        if len(group) == 0:
            return text

        start, end = group[0]
        start -= last_end
        end -= last_end
        var = text[start:end]
        return text[:start - 1] + self.get_value(var) + self._replace(end, text[end:], group[1:])
