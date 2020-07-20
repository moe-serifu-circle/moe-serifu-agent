class SeverRequest:
    def __init__(self, source, verb, route, data, url_variables):
        self.source = source
        self.data = data
        self.verb = verb
        self.route = route
        self.url_variables = url_variables
