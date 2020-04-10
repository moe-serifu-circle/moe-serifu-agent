import re

url_param_re = re.compile("/{([0-9a-zA-Z_]+)}/?")


class UrlParamParser:
    def __init__(self, route):
        self.route = route
        self.fields = []

        self._build()

    def _build(self):
        url_matcher = self.route

        match = url_param_re.search(self.route)
        if match:
            for grp in match.groups():
                self.fields.append(grp)

                url_matcher = url_matcher.replace("{"+grp+"}", "(.+)")

        self.matcher = re.compile("^"+url_matcher+"/?$")

    def match(self, route):
        return self.matcher.match(route) is not None

    def resolve_params(self, route):
        params = {}

        match = self.matcher.search(route)
        if not match:
            return {}

        groups = list(match.groups())

        if len(groups) != len(self.fields):
            raise Exception(f"Field and group counts do not match while resolving params. "
                            "Groups: {len(groups)} Fields: {len(self.fields)}")

        for grp, field in zip(groups, self.fields):
            params[field] = grp

        return params
