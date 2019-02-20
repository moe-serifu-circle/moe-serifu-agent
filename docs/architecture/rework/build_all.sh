

rm *.png


function build_diagram(){
    echo "Building $1.png"

    if [[ $2 != "" && $3 != "" ]]; then
        mmdc -i $1.mermaid -o $1.png -w $2 -H $3
    else
        mmdc -i $1.mermaid -o $1.png 
    fi
}

build_diagram startup_and_echo_sequence_diagram 1568 3184
build_diagram user_script_submission
build_diagram user_script_submission_interactive
build_diagram user_hook_script_submission


