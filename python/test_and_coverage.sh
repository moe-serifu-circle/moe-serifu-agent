#!/bin/bash

# run coverage and unittests
coverage run --omit **/virtualenvs/**,**/tests/**  -m unittest discover -p "*_test.py"

exit_code=$?

# build html report
coverage html

exit $exit_code
