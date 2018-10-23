class Expander:
    """Holds variables for substitution in strings"""

    def __init__(self):
        self.substitutions = {}

    def register_var(self, var: str) -> None:
        if var in self.substitutions:
            raise NameError("Variable " + var + " already exists")
        self.substitutions[var] = ""

    def set_value(self, var: str, val: str) -> None:
        self.substitutions[var] = val

    