from msa.var import Expander

try:
    ex = Expander()
    ex.register_var("test")
    ex.set_value("test", "something")
    success = ""

    if ex.expand("$test") == "something":
        success = "passed"
    else:
        success = "failed"
    print("test", success, "with basic expanding")

    if ex.expand("\\\\\$\$\\\\") == "\\$$\\":
        success = "passed"
    else:
        success = "failed"
    print("test", success, "with escape character abuse")

    if ex.expand("$test, \$test") == "something, $test":
        success = "passed"
    else:
        success = "failed"
    print("test", success, "with complex expanding")
except Exception as e:
    print(e)

input("Press enter to exit")
