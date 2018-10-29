from msa.var import Expander
import sys

try:
    ex = Expander()
    ex.register_var("test")
    ex.set_value("test", "something")
    success = ""
    failed = False

    if ex.expand("$test") == "something":
        success = "passed"
    else:
        success = "failed"
        failed = True
    print("test", success, "with basic expanding")

    if ex.expand("\\\\\$\$\\\\") == "\\$$\\":
        success = "passed"
    else:
        success = "failed"
        failed = True
    print("test", success, "with escape character abuse")

    if ex.expand("$test, \$test") == "something, $test":
        success = "passed"
    else:
        success = "failed"
        failed = True
    print("test", success, "with complex expanding")
    
    if failed:
        sys.exit(1)
except Exception as e:
    print(e)
    sys.exit(1)


