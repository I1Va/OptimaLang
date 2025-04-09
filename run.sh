#!/bin/bash

action=""
graphics=0
filename=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        -S|-C|-E)
            if [[ -n "$action" ]]; then
                echo "Error: Multiple actions specified" >&2
                exit 1
            fi
            action="${1#-}"
            shift
            if [[ $# -eq 0 ]]; then
                echo "Error: Missing filename for -$action" >&2
                exit 1
            fi
            filename="$1"
            shift
            if [[ $# -gt 0 ]]; then
                echo "Error: Extra arguments after filename" >&2
                exit 1
            fi
            ;;
        --AST)
            if [[ -n "$action" ]]; then
                echo "Error: Multiple actions specified" >&2
                exit 1
            fi
            action="AST"
            shift
            ;;
        --graphics)
            if [[ "$action" != "AST" ]]; then
                echo "Error: --graphics requires --AST" >&2
                exit 1
            fi
            graphics=1
            shift
            ;;
        *)
            if [[ -z "$filename" ]]; then
                filename="$1"
                shift
            else
                echo "Error: Unexpected argument $1" >&2
                exit 1
            fi
            ;;
    esac
done

if [[ -z "$action" ]]; then
    echo "Error: No action specified. Use -S, -C, -E, or --AST." >&2
    exit 1
fi

if [[ -z "$filename" ]]; then
    echo "Error: Filename not provided" >&2
    exit 1
fi

if [[ ! -f "$filename" ]]; then
    echo "Error: File '$filename' not found" >&2
    exit 1
fi

case "$action" in
    S)
        short_filename="${filename%.*}"
        cd ./FrontEnd && make -s launch -f Makefile LAUNCH_FLAGS="-i=./../$filename -o=./../BackEnd/ast.txt" && cd ..
	    cd ./BackEnd && make -s launch -f Makefile LAUNCH_FLAGS="-i=./ast.txt -o=./code.out" && cd ..
        mv ./BackEnd/asm_code.txt ./$short_filename.asm
        ;;
    C)
        short_filename="${filename%.*}"
        make -s build
        cd ./FrontEnd && make -s launch -f Makefile LAUNCH_FLAGS="-i=./../$filename -o=./../BackEnd/ast.txt" && cd ..
	    cd ./BackEnd && make -s launch -f Makefile LAUNCH_FLAGS="-i=./ast.txt -o=./../$short_filename.out" && cd ..
        ;;
    E)
        cd ./Processor && make launch -f Makefile LAUNCH_FLAGS="-i=./../$filename"
        ;;
    AST)
        short_filename="${filename%.*}"
        cd ./FrontEnd && make -s launch -f Makefile LAUNCH_FLAGS="-i=./../$filename -o=./ast.txt" && cd ..
        mv ./FrontEnd/ast.txt ./$short_filename.AST
        if [[ $graphics -eq 1 ]]; then
            mv ./FrontEnd/logs/gr_img.png ./${short_filename}_AST.png
        fi
        ;;
    *)
        echo "Error: Invalid action" >&2
        exit 1
        ;;
esac

if [[ $? -ne 0 ]]; then
    echo "Error: Command execution failed" >&2
    exit 1
fi
