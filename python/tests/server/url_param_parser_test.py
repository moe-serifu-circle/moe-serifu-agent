import unittest

from msa.server.url_param_parser import UrlParamParser


class UrlParamParserTest(unittest.TestCase):
    def test_no_params_match(self):
        parser = UrlParamParser("/fake_route")
        self.assertTrue(parser.match("/fake_route"))

    def test_no_params_match_trailing_slash(self):
        parser = UrlParamParser("/fake_route")
        self.assertTrue(parser.match("/fake_route/"))

    def test_no_params_no_match(self):
        parser = UrlParamParser("/fake_route")
        self.assertTrue(not parser.match("/not_the_same_route"))

    def test_no_params_no_match_trailing_slash(self):
        parser = UrlParamParser("/fake_route")
        self.assertTrue(not parser.match("/not_the_same_route/"))

    def test_no_parms_parses(self):
        parser = UrlParamParser("/fake_route")
        self.assertEqual({}, parser.resolve_params("/fake_route"))

    def test_no_params_trailing_slash_parses(self):
        parser = UrlParamParser("/fake_route")
        self.assertEqual({}, parser.resolve_params("/fake_route/"))

    def test_no_params_no_match_does_not_parse(self):
        parser = UrlParamParser("/fake_route")
        self.assertEqual({}, parser.resolve_params("/not_the_same_route"))

    def test_no_params_no_match_trailing_slash_does_not_parse(self):
        parser = UrlParamParser("/fake_route")
        self.assertEqual({}, parser.resolve_params("/not_the_same_route/"))

    def test_one_param_matches(self):
        parser = UrlParamParser("/fake_route/{resource}")
        self.assertTrue(parser.match("/fake_route/moe"))

    def test_one_param_matches_trailing_slash(self):
        parser = UrlParamParser("/fake_route/{resource}")
        self.assertTrue(parser.match("/fake_route/moe/"))

    def test_one_param_no_match(self):
        parser = UrlParamParser("/fake_route/{resource}")
        self.assertTrue(not parser.match("/not_the_same_route/moe"))

    def test_one_param_no_match_trailing_slash(self):
        parser = UrlParamParser("/fake_route/{resource}")
        self.assertTrue(not parser.match("/not_the_same_route/moe/"))

    def test_one_param_parses(self):
        parser = UrlParamParser("/fake_route/{resource}")
        self.assertEqual(parser.resolve_params("/fake_route/moe"), {"resource": "moe"})

    def test_one_param_trailing_slash_parses(self):
        parser = UrlParamParser("/fake_route/{resource}")
        self.assertEqual(parser.resolve_params("/fake_route/moe/"), {"resource": "moe"})

    def test_one_param_no_match_does_not_parse(self):
        parser = UrlParamParser("/fake_route/{resource}")
        self.assertEqual(parser.resolve_params("/not_the_same_route/moe"), {})

    def test_one_param_no_match_trailing_slash_does_not_parse(self):
        parser = UrlParamParser("/fake_route/{resource}")
        self.assertEqual(parser.resolve_params("/not_the_same_route/moe/"), {})

    def test_multiple_params_matches(self):
        parser = UrlParamParser("/fake_route/{resource}/{action}")
        self.assertTrue(parser.match("/fake_route/moe/smile"))

    def test_multiple_params_trailing_slash_matches(self):
        parser = UrlParamParser("/fake_route/{resource}/{action}")
        self.assertTrue(parser.match("/fake_route/moe/smile/"))

    def test_multiple_params_no_match(self):
        parser = UrlParamParser("/fake_route/{resource}/{action}")
        self.assertTrue(not parser.match("/not_the_same_route/moe/smile"))

    def test_multiple_params_no_match_trailing_slash(self):
        parser = UrlParamParser("/fake_route/{resource}/{action}")
        self.assertTrue(not parser.match("/not_the_same_route/moe/smile/"))

    def test_multiple_params_parses(self):
        parser = UrlParamParser("/fake_route/{resource}/{action}")
        self.assertEqual(
            {"resource": "moe", "action": "smile"},
            parser.resolve_params("/fake_route/moe/smile"),
        )

    def test_multiple_params_trailing_slash_parses(self):
        parser = UrlParamParser("/fake_route/{resource}/{action}")
        self.assertEqual(
            {"resource": "moe", "action": "smile"},
            parser.resolve_params("/fake_route/moe/smile/"),
        )

    def test_multiple_params_no_match_does_not_parse(self):
        parser = UrlParamParser("/fake_route/{resource}/{action}")
        self.assertEqual({}, parser.resolve_params("/not_the_same_route/moe/smile"))

    def test_multiple_params_no_match_trailing_slash_does_not_parse(self):
        parser = UrlParamParser("/fake_route/{resource}/{action}")
        self.assertEqual({}, parser.resolve_params("/not_the_same_route/moe/smile/"))

    def test_missing_params_too_short_does_not_match(self):
        parser = UrlParamParser("/fake_route/{resource}/{action}")
        self.assertTrue(not parser.match("/fake_route/moe"))

    def test_missing_params_too_long_does_not_match(self):
        parser = UrlParamParser("/fake_route/{resource}/{action}")
        self.assertTrue(not parser.match("/not_the_same_route/moe/smile"))

    def test_missing_params_too_short_does_not_parse(self):
        parser = UrlParamParser("/fake_route/{resource}/{action}")
        self.assertEqual({}, parser.resolve_params("/fake_route/moe"))

    def test_missing_params_too_long_does_not_parse(self):
        parser = UrlParamParser("/fake_route/{resource}/{action}")
        self.assertEqual({}, parser.resolve_params("/fake_route/moe/test/extra"))
